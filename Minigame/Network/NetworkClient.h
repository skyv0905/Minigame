#pragma once
#include <memory>
#include <string>
#include <optional>
#include "Network/PacketSerializer.h"

namespace Minigame::Network
{
	class NetworkClient
	{
	public:
		NetworkClient();
		~NetworkClient();

		NetworkClient(const NetworkClient&) = delete;
		NetworkClient& operator=(const NetworkClient&) = delete;

		bool Connect(const std::string& address, unsigned short port);
		void Disconnect();
		bool IsConnected() const;
		std::uint32_t GetPlayerId() const;

		void Update(float deltaTime);

		template<typename T>
		bool SendPacket(const T& packet, PacketSendType packetSendType, PacketChannelType channel)
		{
			return SendSerializedPacket(Serialize(packet), packetSendType, channel);
		}

		std::optional<GameStartPacket> ConsumeGameStartPacket();
		std::optional<GameClosedPacket> ConsumeGameClosedPacket();
		std::optional<BulletSpawnPacket> ConsumeBulletSpawnPacket();
		std::optional<BulletDestroyPacket> ConsumeBulletDestroyPacket();
		std::optional<WorldStatePacket> GetLatestWorldState() const;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;

		bool SendSerializedPacket(const ByteBuffer& data, PacketSendType packetSendType, PacketChannelType channel);
	};
}
