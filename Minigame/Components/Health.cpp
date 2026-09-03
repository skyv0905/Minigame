#include "Health.h"
#include <algorithm>

namespace Minigame::Components
{
    Health::Health(GameObject& owner, float maxHealth) : Component(owner),
        maxHealth(std::max(0.0f, maxHealth)),
        currentHealth(this->maxHealth)
    {
    }

    void Health::Hit(float damage)
    {
        if (damage <= 0.0f || IsDead())
            return;

        currentHealth = std::max(0.0f, currentHealth - damage);
    }

    void Health::Kill()
    {
        currentHealth = 0.0f;
    }

    void Health::Heal(float rate)
    {
        if (rate <= 0.0f || IsDead())
        {
            return;
        }
        if (rate > 1.0f)
        {
            rate /= 100.0f;
        }

        float newHealth = currentHealth + maxHealth * rate;
        currentHealth = std::min(newHealth, maxHealth);
    }

    void Health::SetCurrentHealth(float health)
    {
        currentHealth = std::clamp(health, 0.0f, maxHealth);
    }

    bool Health::IsDead() const
    {
        return currentHealth <= 0.0f;
    }

    float Health::GetCurrentHealth() const
    {
        return currentHealth;
    }

    float Health::GetMaxHealth() const
    {
        return maxHealth;
    }
}
