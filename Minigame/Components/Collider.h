#pragma once

#include "Component.h"
#include "Transform.h"
#include <array>
#include <raylib.h>

struct GameServices;

namespace Minigame::Components
{
    struct CollisionInfo
    {
        GameObject& other;
        Vector2 collisionDirection;
        float collisionDepth;
    };

    class Collider : public Component
    {
    public:
        explicit Collider(GameObject& owner, GameServices& gameServices);

        void Start() override;
        void DrawUI() override;

        Rectangle GetBounds() const;
        std::array<Vector2, 4> GetCorners() const;
        bool CheckCollision(const Collider& other, Vector2& direction, float& depth) const;

        void SetSize(Vector2 size);
        void SetOffset(Vector2 origin);

        bool IsTrigger() const;
        void SetTrigger(bool trigger);

        void SetValid(bool valid);
        bool IsValid() const;

    private:
        GameServices& gameServices;
        Minigame::Components::Transform* transform = nullptr;

        Vector2 size{ 0.0f, 0.0f };
        Vector2 offset{ 0.0f, 0.0f };

        bool isTrigger = false;

        bool valid = true;
    };
}
