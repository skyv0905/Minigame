#pragma once

#include "Component.h"
#include <raylib.h>

namespace Minigame::Components
{
    class Transform : public Component
    {
    public:
        explicit Transform(GameObject& owner) : Component(owner)
        {
        }

        void SetPosition(float x, float y);
        void SetPosition(Vector2 pos);
        void SetRotation(float angle);
        void SetScale(float scale);

        Vector2 GetPosition() const;
        float GetRotation() const;
        float GetScale() const;

    private:
        Vector2 position{ 0.0f, 0.0f };
        float rotation = 0.0f;
        float scale = 1.0f;
    };
}