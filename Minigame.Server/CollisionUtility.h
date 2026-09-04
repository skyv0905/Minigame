#pragma once

#include <vector>

namespace Minigame::Server
{
    struct ServerCollider;
    struct ServerWall;
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
    bool SweepCollision(Vector2 startPosition, Vector2 endPosition, const ServerCollider& movingCollider, Vector2 targetPosition, const ServerCollider& targetCollider, float& hitTime);
    bool IsDiagonalMovementBlocked(Vector2 position, const ServerCollider& collider, Vector2 movement, const std::vector<ServerWall>& walls);
    bool CheckCollision(Vector2 positionA, const ServerCollider& colliderA, Vector2 positionB, const ServerCollider& colliderB, Vector2& direction, float& depth);
}
