#include "NetworkClient.h"
#include <enet/enet.h>
#include <iostream>

namespace Minigame::Network
{
	class NetworkClient::Impl
	{
	public:
		bool initialized = false;
		bool connected = false;
		bool disconnecting = false;

		std::uint32_t playerId = 0;

		std::optional<GameStartPacket> pendingGameStart;
		std::optional<GameClosedPacket> pendingGameClosed;
		std::optional<WorldStatePacket> latestWorldState;

		ENetHost* client = nullptr;
		ENetPeer* server = nullptr;
	};

	NetworkClient::NetworkClient() : impl(std::make_unique<Impl>())
	{
		if (enet_initialize() == 0)
		{
			impl->initialized = true;
			impl->client = enet_host_create(nullptr, 1, 2, 0, 0);
		}
	}

	NetworkClient::~NetworkClient()
	{
		if (impl->initialized)
		{
			if (impl->client)
			{
				if (impl->server)
				{
					enet_peer_disconnect(impl->server, 0);
					enet_host_flush(impl->client);
				}
				enet_host_destroy(impl->client);
				impl->server = nullptr;
				impl->client = nullptr;
			}
			enet_deinitialize();
			impl->initialized = false;
		}
	}

	bool NetworkClient::Connect(const std::string& address, unsigned short port)
	{
		if (impl->client == nullptr || impl->server != nullptr)
			return false;

		ENetAddress serverAddress{};
		serverAddress.port = port;

		if (enet_address_set_host(&serverAddress, address.c_str()) != 0)
			return false;

		impl->server = enet_host_connect(impl->client, &serverAddress, 2, 0);

		return impl->server != nullptr;
	}

	void NetworkClient::Disconnect()
	{
		if (impl->server == nullptr || impl->disconnecting)
			return;

		impl->disconnecting = true;
		impl->playerId = 0;
		impl->latestWorldState.reset();
		enet_peer_disconnect(impl->server, 0);
	}

	bool NetworkClient::IsConnected() const
	{
		return impl->connected;
	}

	std::uint32_t NetworkClient::GetPlayerId() const
	{
		return impl->playerId;
	}

	void NetworkClient::Update(float deltaTime)
	{
		if (impl->client == nullptr)
			return;

		ENetEvent event{};
		while (enet_host_service(impl->client, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
			{
				impl->connected = true;
				impl->disconnecting = false;
				std::cout << "Server Connected\n";
				break;
			}

			case ENET_EVENT_TYPE_RECEIVE:
			{
				//std::cout << "Packet Received: " << event.packet->dataLength << " bytes\n";
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
				case Minigame::Network::PacketType::AssignPlayer:
				{
					auto packet = Minigame::Network::Deserialize<Minigame::Network::AssignPlayerPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid AssignPlayer Packet\n";
						break;
					}

					impl->playerId = packet->playerId;
					std::cout << "Assigned Player ID: " << impl->playerId << '\n';
					break;
				}
				case PacketType::GameStart:
				{
					auto packet = Deserialize<GameStartPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid GameStart Packet\n";
						break;
					}

					impl->pendingGameStart = *packet;
					break;
				}
				case PacketType::GameClosed:
				{
					auto packet = Deserialize<GameClosedPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid GameClosed Packet\n";
						break;
					}

					impl->pendingGameClosed = *packet;
					break;
				}
				case PacketType::WorldState:
				{
					auto packet = Deserialize<WorldStatePacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid WorldState Packet\n";
						break;
					}

					impl->latestWorldState = *packet;
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
				impl->connected = false;
				impl->disconnecting = false;
				impl->server = nullptr;
				impl->playerId = 0;
				impl->latestWorldState.reset();
				std::cout << "Server Disconnected\n";
				break;
			}

			default:
				break;
			}
		}
	}

	std::optional<GameStartPacket> NetworkClient::ConsumeGameStartPacket()
	{
		if (!impl->pendingGameStart)
			return std::nullopt;

		auto packet = impl->pendingGameStart;
		impl->pendingGameStart.reset();
		return packet;
	}

	std::optional<GameClosedPacket> NetworkClient::ConsumeGameClosedPacket()
	{
		if (!impl->pendingGameClosed)
			return std::nullopt;

		auto packet = impl->pendingGameClosed;
		impl->pendingGameClosed.reset();
		return packet;
	}

	std::optional<WorldStatePacket> NetworkClient::GetLatestWorldState() const
	{
		return impl->latestWorldState;
	}

	bool NetworkClient::SendSerializedPacket(const ByteBuffer& data, PacketSendType packetSendType, PacketChannelType channel)
	{
		if (!impl->connected || impl->server == nullptr || data.empty())
			return false;

		const enet_uint8 channelID = static_cast<enet_uint8>(channel);
		if (channelID >= 2)
			return false;
		
		enet_uint32 flags = 0;
		if (packetSendType == PacketSendType::Reliable)
			flags = ENET_PACKET_FLAG_RELIABLE;

		ENetPacket* packet = enet_packet_create(data.data(), data.size(), flags);
		if (packet == nullptr)
		{
			return false;
		}

		if (enet_peer_send(impl->server, channelID, packet) != 0)
		{
			enet_packet_destroy(packet);
			return false;
		}

		return true;
	}
}
