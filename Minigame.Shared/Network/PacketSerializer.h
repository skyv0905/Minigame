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
        static constexpr std::uint32_t PayloadSize = 1;
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
        static constexpr std::uint32_t PayloadSize = 4 + 4 + 4;
    };
    template<>
    struct PacketTraits<PlayerFirePacket>
    {
        static constexpr PacketType Type = PacketType::PlayerFire;
        static constexpr std::uint32_t PayloadSize = 4 + 4 + 2 + 2 + 1 + 1;
    };
    template<>
    struct PacketTraits<WorldStatePacket>
    {
        static constexpr PacketType Type = PacketType::WorldState;
        static constexpr std::uint32_t BasePayloadSize = 4 + WorldStatePacket::MaxPlayers * (1 + 2 + 2) + 4;
        static constexpr std::uint32_t MobStateSize = 4 + 1 + 2 + 2;
    };
    template<>
    struct PacketTraits<BulletSpawnPacket>
    {
        static constexpr PacketType Type = PacketType::BulletSpawn;
        static constexpr std::uint32_t PayloadSize = 4 + 4 + 4 + 2 + 2 + 2 + 2 + 1 + 1 + 2 + 2;
    };
    template<>
    struct PacketTraits<BulletDestroyPacket>
    {
        static constexpr PacketType Type = PacketType::BulletDestroy;
        static constexpr std::uint32_t PayloadSize = 4 + 4 + 4 + 2 + 2 + 1;
    };
    template<>
    struct PacketTraits<ExpChangedPacket>
    {
        static constexpr PacketType Type = PacketType::ExpChanged;
        static constexpr std::uint32_t PayloadSize = 1 + 4 + 4;
    };
    template<>
    struct PacketTraits<HpChangedPacket>
    {
        static constexpr PacketType Type = PacketType::HpChanged;
        static constexpr std::uint32_t PayloadSize = 4 + 4;
    };
    template<>
    struct PacketTraits<PlayerStatsChangedPacket>
    {
        static constexpr PacketType Type = PacketType::PlayerStatsChanged;
        static constexpr std::uint32_t PayloadSize = 1 + 4 + 4 + 4 + 4 + 4 + 2 + 2 + 2 + 2;
    };
    template<>
    struct PacketTraits<PowerUpCollectedPacket>
    {
        static constexpr PacketType Type = PacketType::PowerUpCollected;
        static constexpr std::uint32_t PayloadSize = 4 + 1;
    };
    template<>
    struct PacketTraits<StageChangedPacket>
    {
        static constexpr PacketType Type = PacketType::StageChanged;
        static constexpr std::uint32_t PayloadSize = 2 + 4 + 2;
    };
    template<>
    struct PacketTraits<GameResultPacket>
    {
        static constexpr PacketType Type = PacketType::GameResult;
        static constexpr std::uint32_t PayloadSize = 1;
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
    template<>
    ByteBuffer Serialize(const PlayerFirePacket& packet);
    template<>
    ByteBuffer Serialize(const WorldStatePacket& packet);
    template<>
    ByteBuffer Serialize(const BulletSpawnPacket& packet);
    template<>
    ByteBuffer Serialize(const BulletDestroyPacket& packet);
    template<>
    ByteBuffer Serialize(const ExpChangedPacket& packet);
    template<>
    ByteBuffer Serialize(const HpChangedPacket& packet);
    template<>
    ByteBuffer Serialize(const PlayerStatsChangedPacket& packet);
    template<>
    ByteBuffer Serialize(const PowerUpCollectedPacket& packet);
    template<>
    ByteBuffer Serialize(const StageChangedPacket& packet);
    template<>
    ByteBuffer Serialize(const GameResultPacket& packet);

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
    template<>
    std::optional<PlayerFirePacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<WorldStatePacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<BulletSpawnPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<BulletDestroyPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<ExpChangedPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<HpChangedPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<PlayerStatsChangedPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<PowerUpCollectedPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<StageChangedPacket> Deserialize(std::span<const std::uint8_t> data);
    template<>
    std::optional<GameResultPacket> Deserialize(std::span<const std::uint8_t> data);
}
