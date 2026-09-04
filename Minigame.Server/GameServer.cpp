#include "GameServer.h"
#include <enet/enet.h>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include "Network/PacketSerializer.h"
#include "ClientSession.h"
#include "ServerDataLoader.h"
#include "ServerMapBuilder.h"
#include "ServerWorld.h"

extern "C" const char* GetApplicationDirectory(void);

namespace Minigame::Server
{
    namespace
    {
        std::int8_t EncodeDirection(float value)
        {
            const long rounded = std::lround(std::clamp(value, -1.0f, 1.0f) * 127.0f);
            return static_cast<std::int8_t>(rounded);
        }

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

    class GameServer::Impl
    {
    public:
        bool initialized = false;
        bool gameStarted = false;
        bool gameFinished = false;
        std::atomic_bool running = false;

        ENetHost* server = nullptr;
        std::unordered_map<ENetPeer*, ClientSession> sessions;
        ServerWorld world;
        ServerMapBuilder mapBuilder;
        std::mt19937 randomEngine{ std::random_device{}() };
        int currentStage = 1;
        float stageTimerRemaining = 0.0f;
        bool allStagesSpawned = false;

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
        bool TryFinishGame();
        bool TryGoNextStage();
        void BroadcastStageChanged(const Minigame::Network::StageChangedPacket& packet);
        void BroadcastBulletEvents();
        void BroadcastStatEvents();
        void BroadcastPowerUpEvents();
        void BroadcastWorldState();
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

                    if (!impl->gameStarted || packet->sequence <= session->second.lastInputSequence)
                        break;

                    session->second.lastInputSequence = packet->sequence;
                    impl->world.SetPlayerInput(session->second.playerId, *packet);
                    break;
                }

                case Minigame::Network::PacketType::PlayerFire:
                {
                    auto packet = Minigame::Network::Deserialize<Minigame::Network::PlayerFirePacket>(data);
                    auto session = impl->sessions.find(event.peer);
                    if (!packet || session == impl->sessions.end() || !impl->gameStarted)
                    {
                        std::cerr << "Invalid PlayerFire Packet\n";
                        break;
                    }

                    impl->world.QueuePlayerFire(session->second.playerId, *packet);
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
        if (!impl->gameStarted && !impl->gameFinished && impl->sessions.size() == 2 &&
            std::all_of(impl->sessions.begin(), impl->sessions.end(), [](const auto& entry)
                {
                    return entry.second.ready;
                }))
        {
            impl->OnAllPlayersReady();
        }

        if (impl->gameStarted)
        {
            impl->world.Update(static_cast<float>(Impl::TickInterval), impl->serverTick);
            impl->BroadcastStatEvents();
            impl->BroadcastPowerUpEvents();
            impl->BroadcastBulletEvents();
            if (impl->TryFinishGame())
                return;
            if (impl->TryGoNextStage())
                return;
            impl->BroadcastWorldState();
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
        world.RemovePlayer(playerId);

        if (sessions.empty())
        {
            gameFinished = false;
            world.Reset();
        }

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

        Minigame::Network::GameStartPacket packet{};
        std::uniform_int_distribution<std::uint32_t> seedDistribution(1, (std::numeric_limits<std::uint32_t>::max)());
        packet.randomSeed = seedDistribution(randomEngine);

        const std::string dataDirectory = ::GetApplicationDirectory();
        ServerDataLoader dataLoader;
        const auto sceneData = dataLoader.LoadJson(dataDirectory + "Data/Scene/MultiModeLevel.json");
        const auto prefabData = dataLoader.LoadJson(dataDirectory + "Data/Prefab/Prefab.json");
        if (!sceneData || !prefabData)
        {
            std::cerr << "Failed to load server map data\n";
            return;
        }

        if (!mapBuilder.Build(*sceneData, *prefabData, world, packet.randomSeed))
        {
            std::cerr << "Failed to build server map\n";
            return;
        }

        for (auto& [peer, session] : sessions)
            session.lastInputSequence = 0;

        matchStartTick = serverTick;
        packet.startTick = matchStartTick;
        currentStage = 1;
        stageTimerRemaining = mapBuilder.GetNextSpawnCooldown(currentStage);
        allStagesSpawned = false;

        std::cout << "Random Seed: " << packet.randomSeed << '\n';

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

    bool GameServer::Impl::TryFinishGame()
    {
        bool anyPlayerDead = false;
        std::uint8_t winnerPlayerId = 0;
        for (const auto& [playerId, player] : world.GetPlayers())
        {
            if (world.IsPlayerDead(playerId))
            {
                anyPlayerDead = true;
            }
            else
            {
                winnerPlayerId = static_cast<std::uint8_t>(playerId);
            }
        }
        if (!anyPlayerDead)
            return false;

        Minigame::Network::GameResultPacket packet{ winnerPlayerId };
        for (auto& [peer, session] : sessions)
        {
            if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Control))
            {
                std::cerr << "Failed to send GameResult to player " << session.playerId << '\n';
            }
        }

        gameStarted = false;
        gameFinished = true;
        return true;
    }

    bool GameServer::Impl::TryGoNextStage()
    {
        if (allStagesSpawned)
            return false;

        stageTimerRemaining = (std::max)(0.0f, stageTimerRemaining - static_cast<float>(TickInterval));
        const bool allMobsDefeated = world.GetMobs().empty();
        if (!allMobsDefeated && stageTimerRemaining > 0.0f)
            return false;

        const int nextStage = currentStage + 1;
        if (mapBuilder.HasStage(nextStage))
        {
            StageSpawnResult result{};
            if (!mapBuilder.SpawnMobAndPowerUps(world, nextStage, result))
            {
                std::cerr << "Failed to spawn stage " << nextStage << '\n';
                return false;
            }

            currentStage = nextStage;
            stageTimerRemaining = mapBuilder.GetNextSpawnCooldown(currentStage);
            BroadcastStageChanged(Minigame::Network::StageChangedPacket{ static_cast<std::uint16_t>(currentStage), result.firstObjectId, result.objectCount });
            return false;
        }

        allStagesSpawned = true;
        return false;
    }

    void GameServer::Impl::BroadcastStageChanged(const Minigame::Network::StageChangedPacket& packet)
    {
        for (auto& [peer, session] : sessions)
        {
            if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Gameplay))
            {
                std::cerr << "Failed to send StageChanged to player " << session.playerId << '\n';
            }
        }
    }

    void GameServer::Impl::BroadcastWorldState()
    {
        Minigame::Network::WorldStatePacket packet{};
        packet.serverTick = serverTick;

        for (const auto& [playerId, player] : world.GetPlayers())
        {
            auto state = std::find_if(packet.players.begin(), packet.players.end(), [playerId](const auto& playerState) { return playerState.playerId == 0; });
            if (state == packet.players.end())
                break;

            state->playerId = static_cast<std::uint8_t>(playerId);
            state->positionX = Minigame::Network::EncodePosition(player.position.x);
            state->positionY = Minigame::Network::EncodePosition(player.position.y);
        }

        for (const auto& [objectId, mob] : world.GetMobs())
        {
            if (packet.mobCount >= packet.mobs.size())
                break;

            auto& state = packet.mobs[packet.mobCount++];
            state.objectId = objectId;
            state.targetPlayerId = static_cast<std::uint8_t>(mob.targetPlayerId);
            state.positionX = Minigame::Network::EncodePosition(mob.position.x);
            state.positionY = Minigame::Network::EncodePosition(mob.position.y);
        }

        for (auto& [peer, session] : sessions)
        {
            if (!SendPacket(peer, packet, 0, Minigame::Network::PacketChannelType::Gameplay))
            {
                std::cerr << "Failed to send WorldState to player " << session.playerId << '\n';
            }
        }
    }

    void GameServer::Impl::BroadcastBulletEvents()
    {
        for (const ServerBullet& bullet : world.ConsumeSpawnedBullets())
        {
            Minigame::Network::BulletSpawnPacket packet{};
            packet.bulletId = bullet.objectId;
            packet.createdFrom = bullet.createdFrom;
            packet.fireSequence = bullet.fireSequence;
            packet.positionX = Minigame::Network::EncodePosition(bullet.spawnPosition.x);
            packet.positionY = Minigame::Network::EncodePosition(bullet.spawnPosition.y);
            packet.serverPositionX = Minigame::Network::EncodePosition(bullet.position.x);
            packet.serverPositionY = Minigame::Network::EncodePosition(bullet.position.y);
            packet.directionX = EncodeDirection(bullet.direction.x);
            packet.directionY = EncodeDirection(bullet.direction.y);
            packet.moveSpeed = Minigame::Network::EncodePosition(bullet.moveSpeed);
            packet.maxDistance = Minigame::Network::EncodePosition(bullet.maxDistance);
            for (auto& [peer, session] : sessions)
            {
                if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Gameplay))
                {
					std::cerr << "Failed to send BulletSpawn to player " << session.playerId << '\n';
                }
            }
        }

        for (const Minigame::Network::BulletDestroyPacket& packet : world.ConsumeDestroyedBulletPackets())
        {
            for (auto& [peer, session] : sessions)
            {
                if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Gameplay))
                {
					std::cerr << "Failed to send BulletDestroy to player " << session.playerId << '\n';
                }
            }
        }
    }

    void GameServer::Impl::BroadcastStatEvents()
    {
        for (const Minigame::Network::HpChangedPacket& packet : world.ConsumeHpChangedPackets())
        {
            for (auto& [peer, session] : sessions)
            {
                if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Gameplay))
                {
                    std::cerr << "Failed to send HpChanged to player " << session.playerId << '\n';
                }
            }
        }

        for (const Minigame::Network::ExpChangedPacket& packet : world.ConsumeExpChangedPackets())
        {
            for (auto& [peer, session] : sessions)
            {
                if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Gameplay))
                {
                    std::cerr << "Failed to send ExpChanged to player " << session.playerId << '\n';
                }
            }
        }

        for (const Minigame::Network::PlayerStatsChangedPacket& packet : world.ConsumePlayerStatsChangedPackets())
        {
            for (auto& [peer, session] : sessions)
            {
                if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Gameplay))
                {
                    std::cerr << "Failed to send PlayerStatsChanged to player " << session.playerId << '\n';
                }
            }
        }
    }

    void GameServer::Impl::BroadcastPowerUpEvents()
    {
        for (const Minigame::Network::PowerUpCollectedPacket& packet : world.ConsumePowerUpCollectedPackets())
        {
            for (auto& [peer, session] : sessions)
            {
                if (!SendPacket(peer, packet, ENET_PACKET_FLAG_RELIABLE, Minigame::Network::PacketChannelType::Gameplay))
                {
                    std::cerr << "Failed to send PowerUpCollected to player " << session.playerId << '\n';
                }
            }
        }
    }
}
