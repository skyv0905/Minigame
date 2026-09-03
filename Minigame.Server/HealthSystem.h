#pragma once

namespace Minigame::Server
{
    struct ServerHealth;

    class HealthSystem
    {
    public:
        void Hit(ServerHealth& health, float damage) const;
        void Kill(ServerHealth& health) const;
        void Heal(ServerHealth& health, float rate) const;
        bool IsDead(const ServerHealth& health) const;
    };
}
