#include "HealthSystem.h"
#include "ServerWorld.h"
#include <algorithm>

namespace Minigame::Server
{
    void HealthSystem::Hit(ServerHealth& health, float damage) const
    {
        if (damage <= 0.0f || IsDead(health))
            return;

        health.currentHealth = std::max(0.0f, health.currentHealth - damage);
    }

    void HealthSystem::Kill(ServerHealth& health) const
    {
        health.currentHealth = 0.0f;
    }

    void HealthSystem::Heal(ServerHealth& health, float rate) const
    {
        if (rate <= 0.0f || IsDead(health))
        {
            return;
        }
        if (rate > 1.0f)
        {
            rate /= 100.0f;
        }

        float newHealth = health.currentHealth + health.maxHealth * rate;
        health.currentHealth = std::min(newHealth, health.maxHealth);
    }

    bool HealthSystem::IsDead(const ServerHealth& health) const
    {
        return health.currentHealth <= 0.0f;
    }
}
