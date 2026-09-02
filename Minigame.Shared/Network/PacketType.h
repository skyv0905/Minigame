#pragma once

#include <cstdint>

namespace Minigame::Network
{
    enum class PacketType : std::uint16_t
    {
        AssignPlayer,
        PlayerInput,
        GameResult
    };

    enum class PacketSendType
    {
        Unreliable,
        Reliable
    };
}