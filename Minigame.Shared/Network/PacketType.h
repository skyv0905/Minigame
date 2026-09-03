#pragma once

#include <cstdint>

namespace Minigame::Network
{
    enum class PacketType : std::uint16_t
    {
        AssignPlayer,
        PlayerReady,
        GameStart,
        GameClosed,
        PlayerInput,
        WorldState,
        BulletSpawn,
        BulletDestroy,
        ExpChanged,
        HpChanged,
        GameResult
    };

    enum class PacketSendType
    {
        Unreliable,
        Reliable
    };

	enum class PacketChannelType : std::uint8_t
    {
        Control = 0,
        Gameplay = 1
    };
}
