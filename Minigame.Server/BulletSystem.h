#pragma once

namespace Minigame::Server
{
    class ServerWorld;

    class BulletSystem
    {
    public:
        void Update(ServerWorld& world, float deltaTime);
    };
}
