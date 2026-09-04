#include "NetworkClient.h"
#include <enet/enet.h>
#include <deque>
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
		std::optional<GameResultPacket> pendingGameResult;
		std::optional<WorldStatePacket> latestWorldState;
		std::deque<BulletSpawnPacket> pendingBulletSpawns;
		std::deque<BulletDestroyPacket> pendingBulletDestroys;
		std::deque<ExpChangedPacket> pendingExpChanges;
		std::deque<HpChangedPacket> pendingHpChanges;
		std::deque<PlayerStatsChangedPacket> pendingPlayerStatsChanges;
		std::deque<PowerUpCollectedPacket> pendingPowerUpCollections;
		std::deque<StageChangedPacket> pendingStageChanges;

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
		impl->pendingBulletSpawns.clear();
		impl->pendingBulletDestroys.clear();
		impl->pendingExpChanges.clear();
		impl->pendingHpChanges.clear();
		impl->pendingPlayerStatsChanges.clear();
		impl->pendingPowerUpCollections.clear();
		impl->pendingStageChanges.clear();
		impl->pendingGameResult.reset();
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
				case PacketType::BulletSpawn:
				{
					auto packet = Deserialize<BulletSpawnPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid BulletSpawn Packet\n";
						break;
					}

					impl->pendingBulletSpawns.push_back(*packet);
					break;
				}
				case PacketType::BulletDestroy:
				{
					auto packet = Deserialize<BulletDestroyPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid BulletDestroy Packet\n";
						break;
					}

					impl->pendingBulletDestroys.push_back(*packet);
					break;
				}
				case PacketType::ExpChanged:
				{
					auto packet = Deserialize<ExpChangedPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid ExpChanged Packet\n";
						break;
					}

					impl->pendingExpChanges.push_back(*packet);
					break;
				}
				case PacketType::HpChanged:
				{
					auto packet = Deserialize<HpChangedPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid HpChanged Packet\n";
						break;
					}

					impl->pendingHpChanges.push_back(*packet);
					break;
				}
				case PacketType::PlayerStatsChanged:
				{
					auto packet = Deserialize<PlayerStatsChangedPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid PlayerStatsChanged Packet\n";
						break;
					}

					impl->pendingPlayerStatsChanges.push_back(*packet);
					break;
				}
				case PacketType::PowerUpCollected:
				{
					auto packet = Deserialize<PowerUpCollectedPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid PowerUpCollected Packet\n";
						break;
					}

					impl->pendingPowerUpCollections.push_back(*packet);
					break;
				}
				case PacketType::StageChanged:
				{
					auto packet = Deserialize<StageChangedPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid StageChanged Packet\n";
						break;
					}

					impl->pendingStageChanges.push_back(*packet);
					break;
				}
				case PacketType::GameResult:
				{
					auto packet = Deserialize<GameResultPacket>(data);
					if (!packet)
					{
						std::cerr << "Invalid GameResult Packet\n";
						break;
					}

					impl->pendingGameResult = *packet;
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
				impl->pendingBulletSpawns.clear();
				impl->pendingBulletDestroys.clear();
				impl->pendingExpChanges.clear();
				impl->pendingHpChanges.clear();
				impl->pendingPlayerStatsChanges.clear();
				impl->pendingPowerUpCollections.clear();
				impl->pendingStageChanges.clear();
				impl->pendingGameResult.reset();
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

	std::optional<BulletSpawnPacket> NetworkClient::ConsumeBulletSpawnPacket()
	{
		if (impl->pendingBulletSpawns.empty())
			return std::nullopt;

		const BulletSpawnPacket packet = impl->pendingBulletSpawns.front();
		impl->pendingBulletSpawns.pop_front();
		return packet;
	}

	std::optional<BulletDestroyPacket> NetworkClient::ConsumeBulletDestroyPacket()
	{
		if (impl->pendingBulletDestroys.empty())
			return std::nullopt;

		const BulletDestroyPacket packet = impl->pendingBulletDestroys.front();
		impl->pendingBulletDestroys.pop_front();
		return packet;
	}

	std::optional<ExpChangedPacket> NetworkClient::ConsumeExpChangedPacket()
	{
		if (impl->pendingExpChanges.empty())
			return std::nullopt;

		const ExpChangedPacket packet = impl->pendingExpChanges.front();
		impl->pendingExpChanges.pop_front();
		return packet;
	}

	std::optional<HpChangedPacket> NetworkClient::ConsumeHpChangedPacket()
	{
		if (impl->pendingHpChanges.empty())
			return std::nullopt;

		const HpChangedPacket packet = impl->pendingHpChanges.front();
		impl->pendingHpChanges.pop_front();
		return packet;
	}

	std::optional<PlayerStatsChangedPacket> NetworkClient::ConsumePlayerStatsChangedPacket()
	{
		if (impl->pendingPlayerStatsChanges.empty())
			return std::nullopt;

		const PlayerStatsChangedPacket packet = impl->pendingPlayerStatsChanges.front();
		impl->pendingPlayerStatsChanges.pop_front();
		return packet;
	}

	std::optional<PowerUpCollectedPacket> NetworkClient::ConsumePowerUpCollectedPacket()
	{
		if (impl->pendingPowerUpCollections.empty())
			return std::nullopt;

		const PowerUpCollectedPacket packet = impl->pendingPowerUpCollections.front();
		impl->pendingPowerUpCollections.pop_front();
		return packet;
	}

	std::optional<GameResultPacket> NetworkClient::ConsumeGameResultPacket()
	{
		if (!impl->pendingGameResult)
			return std::nullopt;

		auto packet = impl->pendingGameResult;
		impl->pendingGameResult.reset();
		return packet;
	}

	std::optional<StageChangedPacket> NetworkClient::ConsumeStageChangedPacket()
	{
		if (impl->pendingStageChanges.empty())
			return std::nullopt;

		const StageChangedPacket packet = impl->pendingStageChanges.front();
		impl->pendingStageChanges.pop_front();
		return packet;
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
