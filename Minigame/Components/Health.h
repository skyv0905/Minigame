#pragma once

#include "Component.h"

namespace Minigame::Components
{
    class Health : public Component
    {
    public:
        Health(GameObject& owner, float maxHealth);

        void Hit(float damage);
        void Kill();
        void Heal(float rate);
        bool IsDead() const;

        float GetCurrentHealth() const;
        float GetMaxHealth() const;

    private:
        float maxHealth = 0.0f;
        float currentHealth = 0.0f;
    };
}
