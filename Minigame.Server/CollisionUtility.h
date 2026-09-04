#pragma once

namespace Minigame::Server
{
    struct ServerCollider;
    struct Vector2;

    struct Bounds
    {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
    };

    Bounds GetBounds(Vector2 position, const ServerCollider& collider);
    bool IsOverlapping(Vector2 positionA, const ServerCollider& colliderA, Vector2 positionB, const ServerCollider& colliderB);
    bool CheckCollision(Vector2 positionA, const ServerCollider& colliderA, Vector2 positionB, const ServerCollider& colliderB, Vector2& direction, float& depth);
}
