#pragma once

#include "PacketType.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace Minigame::Network
{
    inline constexpr std::uint16_t ProtocolVersion = 4;
    inline constexpr float PositionUnitsPerPixel = 4.0f;

    inline constexpr float DecodePosition(std::uint16_t value)
    {
        return static_cast<float>(value) / PositionUnitsPerPixel;
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
        bool fire;
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
}
