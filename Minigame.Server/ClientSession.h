#pragma once
#include <enet/enet.h>
#include <cstdint>

namespace Minigame::Server
{
    struct ClientSession
    {
        ENetPeer* peer = nullptr;
        std::uint32_t playerId = 0;
        bool ready = false;
    };
}
