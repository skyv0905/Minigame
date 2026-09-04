#pragma once

namespace Minigame::Server
{
    class ServerWorld;
    struct ServerPlayer;
    struct ServerPowerUp;

    class PowerUpSystem
    {
    public:
        void Update(ServerWorld& world);

    private:
        void ApplyEffects(ServerWorld& world, ServerPlayer& player, const ServerPowerUp& powerUp);
    };
}
