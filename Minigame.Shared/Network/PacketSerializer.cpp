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
        bool ReadHeader(std::span<const std::uint8_t> data, std::size_t& offset)
        {
            std::uint16_t type = 0;
            std::uint16_t version = 0;
            std::uint32_t payloadSize = 0;

            if (!ReadUInt16(data, offset, type) || !ReadUInt16(data, offset, version) ||
                !ReadUInt32(data, offset, payloadSize))
            {
                return false;
            }

            return type == static_cast<std::uint16_t>(PacketTraits<T>::Type) && version == ProtocolVersion &&
                payloadSize == PacketTraits<T>::PayloadSize && data.size() == HeaderSize + payloadSize;
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
        case PacketType::PlayerInput:
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
        WriteUInt32(buffer, packet.playerId);
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
        if (!ReadUInt32(data, offset, packet.playerId))
        {
            return std::nullopt;
        }

        return packet;
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
            !ReadUInt8(data, offset, fire) || fire > 1)
        {
            return std::nullopt;
        }

        packet.fire = fire == 1;
        return packet;
    }
#pragma endregion
}
