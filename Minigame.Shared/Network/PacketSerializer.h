#pragma once

#include "Packets.h"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace Minigame::Network
{
    using ByteBuffer = std::vector<std::uint8_t>;

    // ------------------------------ //

    std::optional<PacketType> ReadPacketType(std::span<const std::uint8_t> data);

    // ------------------------------ //

    template<typename T>
    struct PacketTraits;

    template<>
    struct PacketTraits<AssignPlayerPacket>
    {
        static constexpr PacketType Type = PacketType::AssignPlayer;
        static constexpr std::uint32_t PayloadSize = 4;
    };
    template<>
    struct PacketTraits<PlayerReadyPacket>
    {
        static constexpr PacketType Type = PacketType::PlayerReady;
        static constexpr std::uint32_t PayloadSize = 0;
    };
    template<>
    struct PacketTraits<GameStartPacket>
    {
        static constexpr PacketType Type = PacketType::GameStart;
        static constexpr std::uint32_t PayloadSize = 4 + 4;
    };
    template<>
    struct PacketTraits<GameClosedPacket>
    {
        static constexpr PacketType Type = PacketType::GameClosed;
        static constexpr std::uint32_t PayloadSize = 0;
    };
    template<>
    struct PacketTraits<PlayerInputPacket>
    {
        static constexpr PacketType Type = PacketType::PlayerInput;
        static constexpr std::uint32_t PayloadSize = 4 + 4 + 4 + 1;
    };

    // ------------------------------ //

    template<typename T>
    ByteBuffer Serialize(const T& packet) = delete;

    template<>
    ByteBuffer Serialize(const AssignPlayerPacket& packet);
    template<>
    ByteBuffer Serialize(const PlayerReadyPacket& packet);
    template<>
    ByteBuffer Serialize(const GameStartPacket& packet);
    template<>
    ByteBuffer Serialize(const GameClosedPacket& packet);
    template<>
    ByteBuffer Serialize(const PlayerInputPacket& packet);

    // ------------------------------ //

    template<typename T>
    std::optional<T> Deserialize(std::span<const std::uint8_t> data) = delete;

    template<>
    std::optional<AssignPlayerPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<PlayerReadyPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<GameStartPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<GameClosedPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<PlayerInputPacket> Deserialize(std::span<const std::uint8_t> data);
}
