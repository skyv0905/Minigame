#include "PacketSerializer.h"

#include <bit>
#include <cstddef>

namespace Minigame::Network
{
#pragma region common utils
    namespace
    {
        constexpr std::size_t HeaderSize = 2 + 2 + 4;

        void WriteUInt8(ByteBuffer& buffer, std::uint8_t value)
        {
            buffer.push_back(value);
        }

        void WriteUInt16(ByteBuffer& buffer, std::uint16_t value)
        {
            buffer.push_back(static_cast<std::uint8_t>(value >> 8));
            buffer.push_back(static_cast<std::uint8_t>(value));
        }

        void WriteUInt32(ByteBuffer& buffer, std::uint32_t value)
        {
            buffer.push_back(static_cast<std::uint8_t>(value >> 24));
            buffer.push_back(static_cast<std::uint8_t>(value >> 16));
            buffer.push_back(static_cast<std::uint8_t>(value >> 8));
            buffer.push_back(static_cast<std::uint8_t>(value));
        }

        void WriteFloat(ByteBuffer& buffer, float value)
        {
            WriteUInt32(buffer, std::bit_cast<std::uint32_t>(value));
        }

        template<typename T>
        void WriteHeader(ByteBuffer& buffer)
        {
            WriteUInt16(buffer, static_cast<std::uint16_t>(PacketTraits<T>::Type));
            WriteUInt16(buffer, ProtocolVersion);
            WriteUInt32(buffer, PacketTraits<T>::PayloadSize);
        }

        template<typename T>
        void WriteHeader(ByteBuffer& buffer, std::uint32_t payloadSize)
        {
            WriteUInt16(buffer, static_cast<std::uint16_t>(PacketTraits<T>::Type));
            WriteUInt16(buffer, ProtocolVersion);
            WriteUInt32(buffer, payloadSize);
        }

        bool ReadUInt8(std::span<const std::uint8_t> data, std::size_t& offset, std::uint8_t& value)
        {
            if (offset + 1 > data.size())
            {
                return false;
            }

            value = data[offset++];
            return true;
        }

        bool ReadUInt16(std::span<const std::uint8_t> data, std::size_t& offset, std::uint16_t& value)
        {
            if (offset + 2 > data.size())
            {
                return false;
            }

            value = static_cast<std::uint16_t>(data[offset]) << 8 |
                static_cast<std::uint16_t>(data[offset + 1]);
            offset += 2;
            return true;
        }

        bool ReadUInt32(std::span<const std::uint8_t> data, std::size_t& offset, std::uint32_t& value)
        {
            if (offset + 4 > data.size())
            {
                return false;
            }

            value = static_cast<std::uint32_t>(data[offset]) << 24 |
                static_cast<std::uint32_t>(data[offset + 1]) << 16 |
                static_cast<std::uint32_t>(data[offset + 2]) << 8 |
                static_cast<std::uint32_t>(data[offset + 3]);
            offset += 4;
            return true;
        }

        bool ReadFloat(std::span<const std::uint8_t> data, std::size_t& offset, float& value)
        {
            std::uint32_t bits = 0;
            if (!ReadUInt32(data, offset, bits))
            {
                return false;
            }

            value = std::bit_cast<float>(bits);
            return true;
        }

        template<typename T>
        bool ReadHeader(std::span<const std::uint8_t> data, std::size_t& offset, std::uint32_t& payloadSize)
        {
            std::uint16_t type = 0;
            std::uint16_t version = 0;

            if (!ReadUInt16(data, offset, type) || !ReadUInt16(data, offset, version) ||
                !ReadUInt32(data, offset, payloadSize))
            {
                return false;
            }

            return type == static_cast<std::uint16_t>(PacketTraits<T>::Type) && version == ProtocolVersion && data.size() == HeaderSize + payloadSize;
        }

        template<typename T>
        bool ReadHeader(std::span<const std::uint8_t> data, std::size_t& offset)
        {
            std::uint32_t payloadSize = 0;
            return ReadHeader<T>(data, offset, payloadSize) && payloadSize == PacketTraits<T>::PayloadSize;
        }
    }
#pragma endregion
    // ------------------------------ //

    std::optional<PacketType> ReadPacketType(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        std::uint16_t value = 0;
        if (!ReadUInt16(data, offset, value))
        {
            return std::nullopt;
        }

        const auto type = static_cast<PacketType>(value);
        switch (type)
        {
        case PacketType::AssignPlayer:
        case PacketType::PlayerReady:
        case PacketType::GameStart:
        case PacketType::GameClosed:
        case PacketType::PlayerInput:
        case PacketType::WorldState:
        case PacketType::BulletSpawn:
        case PacketType::BulletDestroy:
        case PacketType::ExpChanged:
        case PacketType::HpChanged:
        case PacketType::PlayerStatsChanged:
        case PacketType::PowerUpCollected:
        case PacketType::GameResult:
            return type;

        default:
            return std::nullopt;
        }
    }

    // ------------------------------ //
#pragma region Serilaize
    template<>
    ByteBuffer Serialize(const AssignPlayerPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<AssignPlayerPacket>::PayloadSize);
        WriteHeader<AssignPlayerPacket>(buffer);
        WriteUInt8(buffer, packet.playerId);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const PlayerReadyPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize);
        WriteHeader<PlayerReadyPacket>(buffer);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const GameStartPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<GameStartPacket>::PayloadSize);
        WriteHeader<GameStartPacket>(buffer);
        WriteUInt32(buffer, packet.randomSeed);
        WriteUInt32(buffer, packet.startTick);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const GameClosedPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<GameClosedPacket>::PayloadSize);
        WriteHeader<GameClosedPacket>(buffer);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const PlayerInputPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<PlayerInputPacket>::PayloadSize);
        WriteHeader<PlayerInputPacket>(buffer);
        WriteUInt32(buffer, packet.sequence);
        WriteFloat(buffer, packet.moveX);
        WriteFloat(buffer, packet.moveY);
        WriteUInt8(buffer, packet.fire ? 1 : 0);
        WriteUInt32(buffer, packet.fireSequence);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const WorldStatePacket& packet)
    {
        const std::uint32_t mobCount = packet.mobCount <= WorldStatePacket::MaxMobs ? packet.mobCount : static_cast<std::uint32_t>(WorldStatePacket::MaxMobs);
        const std::uint32_t payloadSize = PacketTraits<WorldStatePacket>::BasePayloadSize + mobCount * PacketTraits<WorldStatePacket>::MobStateSize;
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + payloadSize);
        WriteHeader<WorldStatePacket>(buffer, payloadSize);
        WriteUInt32(buffer, packet.serverTick);
        for (const PlayerState& player : packet.players)
        {
            WriteUInt8(buffer, player.playerId);
            WriteUInt16(buffer, player.positionX);
            WriteUInt16(buffer, player.positionY);
        }
        WriteUInt32(buffer, mobCount);
        for (std::uint32_t i = 0; i < mobCount; i++)
        {
            const MobState& mob = packet.mobs[i];
            WriteUInt32(buffer, mob.objectId);
            WriteUInt8(buffer, mob.targetPlayerId);
            WriteUInt16(buffer, mob.positionX);
            WriteUInt16(buffer, mob.positionY);
        }
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const BulletSpawnPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<BulletSpawnPacket>::PayloadSize);
        WriteHeader<BulletSpawnPacket>(buffer);
        WriteUInt32(buffer, packet.bulletId);
        WriteUInt32(buffer, packet.createdFrom);
        WriteUInt32(buffer, packet.fireSequence);
        WriteUInt16(buffer, packet.positionX);
        WriteUInt16(buffer, packet.positionY);
        WriteUInt8(buffer, static_cast<std::uint8_t>(packet.directionX));
        WriteUInt8(buffer, static_cast<std::uint8_t>(packet.directionY));
        WriteUInt16(buffer, packet.moveSpeed);
        WriteUInt16(buffer, packet.maxDistance);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const BulletDestroyPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<BulletDestroyPacket>::PayloadSize);
        WriteHeader<BulletDestroyPacket>(buffer);
        WriteUInt32(buffer, packet.bulletId);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const ExpChangedPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<ExpChangedPacket>::PayloadSize);
        WriteHeader<ExpChangedPacket>(buffer);
        WriteUInt8(buffer, packet.playerId);
        WriteUInt32(buffer, packet.newExp);
        WriteUInt32(buffer, packet.newLevel);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const HpChangedPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<HpChangedPacket>::PayloadSize);
        WriteHeader<HpChangedPacket>(buffer);
        WriteUInt32(buffer, packet.objectId);
        WriteFloat(buffer, packet.newHp);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const PlayerStatsChangedPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<PlayerStatsChangedPacket>::PayloadSize);
        WriteHeader<PlayerStatsChangedPacket>(buffer);
        WriteUInt8(buffer, packet.playerId);
        WriteFloat(buffer, packet.moveSpeed);
        WriteFloat(buffer, packet.bulletSpeed);
        WriteFloat(buffer, packet.bulletDistance);
        WriteFloat(buffer, packet.fireCooldown);
        WriteFloat(buffer, packet.attackPower);
        WriteUInt16(buffer, packet.moveSpeedMultiplier);
        WriteUInt16(buffer, packet.bulletSpeedMultiplier);
        WriteUInt16(buffer, packet.bulletDistanceMultiplier);
        WriteUInt16(buffer, packet.attackPowerMultiplier);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const PowerUpCollectedPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<PowerUpCollectedPacket>::PayloadSize);
        WriteHeader<PowerUpCollectedPacket>(buffer);
        WriteUInt32(buffer, packet.objectId);
        WriteUInt8(buffer, packet.playerId);
        return buffer;
    }

    template<>
    ByteBuffer Serialize(const GameResultPacket& packet)
    {
        ByteBuffer buffer;
        buffer.reserve(HeaderSize + PacketTraits<GameResultPacket>::PayloadSize);
        WriteHeader<GameResultPacket>(buffer);
        WriteUInt8(buffer, packet.winnerPlayerId);
        return buffer;
    }
#pragma endregion
    // ------------------------------ //
#pragma region Deserialize
    template<>
    std::optional<AssignPlayerPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<AssignPlayerPacket>(data, offset))
        {
            return std::nullopt;
        }

        AssignPlayerPacket packet{};
        if (!ReadUInt8(data, offset, packet.playerId))
        {
            return std::nullopt;
        }

        return packet;
    }

    template<>
    std::optional<PlayerReadyPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<PlayerReadyPacket>(data, offset))
        {
            return std::nullopt;
        }

        return PlayerReadyPacket{};
    }

    template<>
    std::optional<GameStartPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<GameStartPacket>(data, offset))
        {
            return std::nullopt;
        }

        GameStartPacket packet{};
        if (!ReadUInt32(data, offset, packet.randomSeed) ||
            !ReadUInt32(data, offset, packet.startTick))
        {
            return std::nullopt;
        }

        return packet;
    }

    template<>
    std::optional<GameClosedPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<GameClosedPacket>(data, offset))
        {
            return std::nullopt;
        }

        return GameClosedPacket{};
    }

    template<>
    std::optional<PlayerInputPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<PlayerInputPacket>(data, offset))
        {
            return std::nullopt;
        }

        PlayerInputPacket packet{};
        std::uint8_t fire = 0;
        if (!ReadUInt32(data, offset, packet.sequence) ||
            !ReadFloat(data, offset, packet.moveX) ||
            !ReadFloat(data, offset, packet.moveY) ||
            !ReadUInt8(data, offset, fire) || fire > 1 ||
            !ReadUInt32(data, offset, packet.fireSequence))
        {
            return std::nullopt;
        }

        packet.fire = fire == 1;
        return packet;
    }

    template<>
    std::optional<WorldStatePacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        std::uint32_t payloadSize = 0;
        if (!ReadHeader<WorldStatePacket>(data, offset, payloadSize))
            return std::nullopt;

        WorldStatePacket packet{};
        if (!ReadUInt32(data, offset, packet.serverTick))
            return std::nullopt;

        for (PlayerState& player : packet.players)
        {
            if (!ReadUInt8(data, offset, player.playerId) || !ReadUInt16(data, offset, player.positionX) || !ReadUInt16(data, offset, player.positionY))
                return std::nullopt;
        }

        if (!ReadUInt32(data, offset, packet.mobCount) || packet.mobCount > WorldStatePacket::MaxMobs)
            return std::nullopt;

        const std::uint32_t expectedPayloadSize = PacketTraits<WorldStatePacket>::BasePayloadSize + packet.mobCount * PacketTraits<WorldStatePacket>::MobStateSize;
        if (payloadSize != expectedPayloadSize)
            return std::nullopt;

        for (std::uint32_t i = 0; i < packet.mobCount; i++)
        {
            MobState& mob = packet.mobs[i];
            if (!ReadUInt32(data, offset, mob.objectId) || !ReadUInt8(data, offset, mob.targetPlayerId) || !ReadUInt16(data, offset, mob.positionX) || !ReadUInt16(data, offset, mob.positionY))
                return std::nullopt;
        }

        return packet;
    }

    template<>
    std::optional<BulletSpawnPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<BulletSpawnPacket>(data, offset))
            return std::nullopt;

        BulletSpawnPacket packet{};
        std::uint8_t directionX = 0;
        std::uint8_t directionY = 0;
        if (!ReadUInt32(data, offset, packet.bulletId) || !ReadUInt32(data, offset, packet.createdFrom) || !ReadUInt32(data, offset, packet.fireSequence) ||
            !ReadUInt16(data, offset, packet.positionX) || !ReadUInt16(data, offset, packet.positionY) ||
            !ReadUInt8(data, offset, directionX) || !ReadUInt8(data, offset, directionY) ||
            !ReadUInt16(data, offset, packet.moveSpeed) || !ReadUInt16(data, offset, packet.maxDistance))
            return std::nullopt;

        packet.directionX = static_cast<std::int8_t>(directionX);
        packet.directionY = static_cast<std::int8_t>(directionY);
        return packet;
    }

    template<>
    std::optional<BulletDestroyPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<BulletDestroyPacket>(data, offset))
            return std::nullopt;

        BulletDestroyPacket packet{};
        if (!ReadUInt32(data, offset, packet.bulletId))
            return std::nullopt;
        return packet;
    }

    template<>
    std::optional<ExpChangedPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<ExpChangedPacket>(data, offset))
            return std::nullopt;

        ExpChangedPacket packet{};
        if (!ReadUInt8(data, offset, packet.playerId) || !ReadUInt32(data, offset, packet.newExp) || !ReadUInt32(data, offset, packet.newLevel))
            return std::nullopt;
        return packet;
    }

    template<>
    std::optional<HpChangedPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<HpChangedPacket>(data, offset))
            return std::nullopt;

        HpChangedPacket packet{};
        if (!ReadUInt32(data, offset, packet.objectId) || !ReadFloat(data, offset, packet.newHp))
            return std::nullopt;
        return packet;
    }

    template<>
    std::optional<PlayerStatsChangedPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<PlayerStatsChangedPacket>(data, offset))
            return std::nullopt;

        PlayerStatsChangedPacket packet{};
        if (!ReadUInt8(data, offset, packet.playerId) || !ReadFloat(data, offset, packet.moveSpeed) || !ReadFloat(data, offset, packet.bulletSpeed) ||
            !ReadFloat(data, offset, packet.bulletDistance) || !ReadFloat(data, offset, packet.fireCooldown) || !ReadFloat(data, offset, packet.attackPower) ||
            !ReadUInt16(data, offset, packet.moveSpeedMultiplier) || !ReadUInt16(data, offset, packet.bulletSpeedMultiplier) ||
            !ReadUInt16(data, offset, packet.bulletDistanceMultiplier) || !ReadUInt16(data, offset, packet.attackPowerMultiplier))
            return std::nullopt;
        return packet;
    }

    template<>
    std::optional<PowerUpCollectedPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<PowerUpCollectedPacket>(data, offset))
            return std::nullopt;

        PowerUpCollectedPacket packet{};
        if (!ReadUInt32(data, offset, packet.objectId) || !ReadUInt8(data, offset, packet.playerId))
            return std::nullopt;
        return packet;
    }

    template<>
    std::optional<GameResultPacket> Deserialize(std::span<const std::uint8_t> data)
    {
        std::size_t offset = 0;
        if (!ReadHeader<GameResultPacket>(data, offset))
            return std::nullopt;

        GameResultPacket packet{};
        if (!ReadUInt8(data, offset, packet.winnerPlayerId) || packet.winnerPlayerId > WorldStatePacket::MaxPlayers)
            return std::nullopt;
        return packet;
    }
#pragma endregion
}
