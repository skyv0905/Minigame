#include "GameServer.h"
#include <enet/enet.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>
#include "Network/PacketSerializer.h"

namespace Minigame::Server
{
    namespace
    {
        template<typename T>
        bool SendPacket(ENetPeer* peer, const T& packet, enet_uint32 flags, Minigame::Network::PacketChannelType channel)
        {
            Minigame::Network::ByteBuffer data = Minigame::Network::Serialize(packet);

            ENetPacket* enetPacket = enet_packet_create(data.data(), data.size(), flags);
            if (enetPacket == nullptr)
            {
                return false;
            }

            if (enet_peer_send(peer, static_cast<enet_uint8>(channel), enetPacket) != 0)
            {
                enet_packet_destroy(enetPacket);
                return false;
            }

            return true;
        }
    }

    struct ClientSession
    {
        ENetPeer* peer = nullptr;
        std::uint32_t playerId = 0;
        bool ready = false;
    };

    class GameServer::Impl
    {
    public:
        bool initialized = false;
        bool gameStarted = false;
        std::atomic_bool running = false;

        ENetHost* server = nullptr;
        std::unordered_map<ENetPeer*, ClientSession> sessions;

        static constexpr std::uint32_t TickRate = 30;
        static constexpr double TickInterval = 1.0 / TickRate;
        std::uint32_t serverTick = 0;
        std::uint32_t matchStartTick = 0;
        double tickAccumulator = 0.0;
        std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

        std::uint32_t FindAvailablePlayerId() const;
        void OnPlayerConnected(ENetPeer* peer);
        void OnPlayerDisconnected(ENetPeer* peer);

        void OnAllPlayersReady();
    };

	GameServer::GameServer() : impl(std::make_unique<Impl>())
	{
        if (enet_initialize() == 0)
        {
            impl->initialized = true;

            ENetAddress address{};
            address.host = ENET_HOST_ANY;
            address.port = 5000;

            impl->server = enet_host_create(&address, 2, 2, 0, 0);
        }
	}

    GameServer::~GameServer()
    {
        if (impl->initialized)
        {
            if (impl->server)
            {
                enet_host_destroy(impl->server);
                impl->server = nullptr;
            }
            enet_deinitialize();
            impl->initialized = false;
            std::cout << "Server Closed\n";
        }
    }

	void GameServer::Run()
	{
        if (!impl->initialized)
        {
            std::cerr << "Failed to Initialize ENet\n";
            return;
        }
        if (impl->server == nullptr)
        {
            std::cerr << "Failed to Create Server Host\n";
            return;
        }

        impl->running.store(true);
        std::cout << "Server Started at 5000\n";

        std::thread consoleThread([this]()
            {
                std::string command;

                while (impl->running.load() && std::getline(std::cin, command))
                {
                    if (command == "stop")
                    {
                        Stop();
                        break;
                    }

                    std::cout << "Unknown Command: " << command << '\n';
                }
            });

        while (impl->running.load())
        {
            Loop();
        }

        if (consoleThread.joinable())
        {
            consoleThread.join();
        }
	}

    void GameServer::Stop()
    {
        impl->running.store(false);
    }

	void GameServer::Loop()
	{
        GetPackets();

        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed = now - impl->previousTime;
        impl->previousTime = now;
        impl->tickAccumulator += elapsed.count();

        constexpr int maxUpdatesPerLoop = 5;
        int updateCount = 0;

        while (impl->tickAccumulator >= Impl::TickInterval && updateCount < maxUpdatesPerLoop)
        {
            Update();
            impl->serverTick++;
            impl->tickAccumulator -= Impl::TickInterval;
            updateCount++;
        }
	}

    void GameServer::GetPackets()
    {
        ENetEvent event{};
        while (enet_host_service(impl->server, &event, 10) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << "Client Connected\n";
                impl->OnPlayerConnected(event.peer);
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE:
            {
                //std::cout << "Packed Received: " << event.packet->dataLength << " bytes\n";
                std::span<const std::uint8_t> data(event.packet->data, event.packet->dataLength);

				const auto packetType = Minigame::Network::ReadPacketType(data);
				if (!packetType)
				{
					std::cerr << "Invalid Packet Type\n";
					enet_packet_destroy(event.packet);
					break;
				}

                switch (packetType.value())
                {
                case Minigame::Network::PacketType::PlayerReady:
                {
                    auto packet = Minigame::Network::Deserialize<Minigame::Network::PlayerReadyPacket>(data);
                    auto session = impl->sessions.find(event.peer);
                    if (!packet || session == impl->sessions.end() || impl->gameStarted)
                    {
                        std::cerr << "Invalid PlayerReady Packet\n";
                        break;
                    }

                    session->second.ready = true;
                    std::cout << "Player " << session->second.playerId << " Ready\n";
                    break;
                }
                case Minigame::Network::PacketType::PlayerInput:
                {
                    auto packet = Minigame::Network::Deserialize<Minigame::Network::PlayerInputPacket>(data);
                    if (!packet)
                    {
                        std::cerr << "Invalid PlayerInput Packet\n";
                        break;
                    }

                    auto session = impl->sessions.find(event.peer);
                    if (session == impl->sessions.end())
                    {
                        break;
                    }

                    //std::cout << "Player " << session->second.playerId
                    //    << " input: " << packet->moveX << ", " << packet->moveY << '\n';
                    //HandlePlayerInput(session->second.playerId, *packet);
                    break;
                }

                default:
                {
                    break;
                }
                }

                enet_packet_destroy(event.packet);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "Client Disconnected\n";
                impl->OnPlayerDisconnected(event.peer);
                break;
            }

            default:
                break;
            }
        }
    }

	void GameServer::Update()
	{
        if (!impl->gameStarted && impl->sessions.size() == 2 &&
            std::all_of(impl->sessions.begin(), impl->sessions.end(), [](const auto& entry)
                {
                    return entry.second.ready;
                }))
        {
            impl->OnAllPlayersReady();
        }
	}

    std::uint32_t GameServer::Impl::FindAvailablePlayerId() const
    {
        constexpr std::uint32_t maxPlayers = 2;

        for (std::uint32_t playerId = 1; playerId <= maxPlayers; playerId++)
        {
            bool used = false;
            for (const auto& [peer, session] : sessions)
            {
                if (session.playerId == playerId)
                {
                    used = true;
                    break;
                }
            }

            if (!used)
                return playerId;
        }

        return 0;
    }

    void GameServer::Impl::OnPlayerConnected(ENetPeer* peer)
    {
        auto playerId = FindAvailablePlayerId();
        if (playerId == 0)
        {
            //SendJoinDeny(peer);
            enet_peer_disconnect_later(peer, 0);
            return;
        }

        ClientSession session{};
        session.peer = peer;
        session.playerId = playerId;
        session.ready = false;

        sessions.emplace(peer, session);

        Minigame::Network::AssignPlayerPacket packet{};
        packet.playerId = playerId;

        SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Control);
    }

    void GameServer::Impl::OnPlayerDisconnected(ENetPeer* peer)
    {
        auto session = sessions.find(peer);
        if (session == sessions.end())
            return;

        std::uint32_t playerId = session->second.playerId;
        sessions.erase(session);

        if (gameStarted)
        {
            Minigame::Network::GameClosedPacket packet{};

            for (auto& [peer, session] : sessions)
            {
                if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Control))
                {
                    std::cerr << "Failed to send GameClosed to player " << session.playerId << '\n';
                    return;
                }
            }

            gameStarted = false;
        }
    }

    void GameServer::Impl::OnAllPlayersReady()
    {
        if (gameStarted)
            return;

        matchStartTick = serverTick;
        Minigame::Network::GameStartPacket packet{};
        packet.randomSeed = 12345;
        packet.startTick = matchStartTick;

        for (auto& [peer, session] : sessions)
        {
            if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Control))
            {
                std::cerr << "Failed to send GameStart to player " << session.playerId << '\n';
                return;
            }
        }

        gameStarted = true;
    }
}
