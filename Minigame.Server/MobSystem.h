#pragma once

namespace Minigame::Server
{
    class ServerWorld;

    class MobSystem
    {
    public:
        void Update(ServerWorld& world, float deltaTime);
    };
}
