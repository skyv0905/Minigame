#include "NetworkClient.h"
#include <enet/enet.h>
#include <iostream>
#include "Network/PacketSerializer.h"

namespace Minigame::Network
{
	class NetworkClient::Impl
	{
	public:
		bool initialized = false;
		bool connected = false;
		bool disconnecting = false;

		std::uint32_t playerId = 0;

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
		enet_peer_disconnect(impl->server, 0);
	}

	bool NetworkClient::IsConnected() const
	{
		return impl->connected;
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
				std::cout << "Packet Received: " << event.packet->dataLength << " bytes\n";
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
				std::cout << "Server Disconnected\n";
				break;
			}

			default:
				break;
			}
		}
	}

	void NetworkClient::Test()
	{

	}

	bool NetworkClient::SendReliable(const std::string& message)
	{
		if (!impl->connected || impl->server == nullptr || message.empty())
			return false;

		ENetPacket* packet = enet_packet_create(message.data(), message.size(), ENET_PACKET_FLAG_RELIABLE);

		if (packet == nullptr)
			return false;

		if (enet_peer_send(impl->server, 0, packet) != 0)
		{
			enet_packet_destroy(packet);
			return false;
		}

		return true;
	}
}
