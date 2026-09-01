#pragma once

#include "PacketType.h"
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

    struct PlayerInputPacket
    {
        std::uint32_t sequence;
        float moveX;
        float moveY;
        bool fire;
    };
}
