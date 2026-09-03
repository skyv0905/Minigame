#pragma once

#include <cstdint>

namespace Minigame::Network
{
    struct PlayerInputPacket;
}

namespace Minigame::Server
{
    class ServerWorld;

    class PlayerSystem
    {
    public:
        void SetInput(ServerWorld& world, std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet);
        void Update(ServerWorld& world, float deltaTime);
    };
}
