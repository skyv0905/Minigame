#pragma once

#include <cstdint>

namespace Minigame::Server
{
    class ServerWorld;

    class BulletSystem
    {
    public:
        void Update(ServerWorld& world, float deltaTime, std::uint32_t serverTick);
    };
}
