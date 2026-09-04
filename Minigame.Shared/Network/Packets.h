#pragma once

#include "PacketType.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace Minigame::Network
{
    inline constexpr std::uint16_t ProtocolVersion = 1;
    inline constexpr float PositionUnitsPerPixel = 4.0f;

    inline constexpr float DecodePosition(std::uint16_t value)
    {
        return static_cast<float>(value) / PositionUnitsPerPixel;
    }

    inline std::uint16_t EncodePosition(float value)
    {
        if (!std::isfinite(value))
            return 0;

        const long maximum = static_cast<long>((std::numeric_limits<std::uint16_t>::max)());
        const double scaled = static_cast<double>(value) * PositionUnitsPerPixel;
        const long rounded = std::lround(std::clamp(scaled, 0.0, static_cast<double>(maximum)));
        return static_cast<std::uint16_t>(rounded);
    }

    inline std::int8_t EncodeDirection(float value)
    {
        const long rounded = std::lround(std::clamp(value, -1.0f, 1.0f) * 127.0f);
        return static_cast<std::int8_t>(rounded);
    }

    inline constexpr float DecodeDirection(std::int8_t value)
    {
        return static_cast<float>(value) / 127.0f;
    }

    struct PacketHeader
    {
        PacketType type;
        std::uint16_t protocolVersion;
        std::uint32_t payloadSize;
    };

    struct AssignPlayerPacket
    {
        std::uint8_t playerId;
    };

    struct PlayerReadyPacket
    {
    };

    struct GameStartPacket
    {
        std::uint32_t randomSeed;
        std::uint32_t startTick;
    };

    struct GameClosedPacket
    {
    };

    struct PlayerInputPacket
    {
        std::uint32_t sequence;
        float moveX;
        float moveY;
    };

    struct PlayerFirePacket
    {
        std::uint32_t fireSequence;
        std::uint32_t clientTick;
        std::uint16_t positionX;
        std::uint16_t positionY;
        std::int8_t directionX;
        std::int8_t directionY;
    };

    struct PlayerState
    {
        std::uint8_t playerId;
        std::uint16_t positionX;
        std::uint16_t positionY;
    };

    struct MobState
    {
        std::uint32_t objectId;
        std::uint8_t targetPlayerId;
        std::uint16_t positionX;
        std::uint16_t positionY;
    };

    struct WorldStatePacket
    {
        static constexpr std::size_t MaxPlayers = 2;
        static constexpr std::size_t MaxMobs = 64;

        std::uint32_t serverTick;
        std::array<PlayerState, MaxPlayers> players;
        std::uint32_t mobCount;
        std::array<MobState, MaxMobs> mobs;
    };

    struct BulletSpawnPacket
    {
        std::uint32_t bulletId;
        std::uint32_t createdFrom;
        std::uint32_t fireSequence;
        std::uint16_t positionX;
        std::uint16_t positionY;
        std::uint16_t serverPositionX;
        std::uint16_t serverPositionY;
        std::int8_t directionX;
        std::int8_t directionY;
        std::uint16_t moveSpeed;
        std::uint16_t maxDistance;
    };

    enum class BulletDestroyReason : std::uint8_t
    {
        MaxDistance,
        Collision
    };

    struct BulletDestroyPacket
    {
        std::uint32_t bulletId;
        std::uint32_t createdFrom;
        std::uint32_t hitObjectId;
        std::uint16_t positionX;
        std::uint16_t positionY;
        BulletDestroyReason reason;
    };

    struct ExpChangedPacket
    {
        std::uint8_t playerId;
        std::uint32_t newExp;
        std::uint32_t newLevel;
    };

    struct HpChangedPacket
    {
        std::uint32_t objectId;
        float newHp;
    };

    struct PlayerStatsChangedPacket
    {
        std::uint8_t playerId;
        float moveSpeed;
        float bulletSpeed;
        float bulletDistance;
        float fireCooldown;
        float attackPower;
        std::uint16_t moveSpeedMultiplier;
        std::uint16_t bulletSpeedMultiplier;
        std::uint16_t bulletDistanceMultiplier;
        std::uint16_t attackPowerMultiplier;
    };

    struct PowerUpCollectedPacket
    {
        std::uint32_t objectId;
        std::uint8_t playerId;
    };

    struct StageChangedPacket
    {
        std::uint16_t stage;
        std::uint32_t firstObjectId;
        std::uint16_t objectCount;
    };

    struct GameResultPacket
    {
        std::uint8_t winnerPlayerId;
    };
}
