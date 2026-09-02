#pragma once

#include "PacketType.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace Minigame::Network
{
    inline constexpr std::uint16_t ProtocolVersion = 1;

    struct PacketHeader
    {
        PacketType type;
        std::uint16_t protocolVersion;
        std::uint32_t payloadSize;
    };

    struct AssignPlayerPacket
    {
        std::uint32_t playerId;
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
        std::uint32_t playerId;
        float positionX;
        float positionY;
    };

    struct WorldStatePacket
    {
        static constexpr std::size_t MaxPlayers = 2;

        std::uint32_t serverTick;
        std::uint32_t playerCount;
        std::array<PlayerState, MaxPlayers> players;
    };
}
