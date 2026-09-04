#include "CollisionUtility.h"
#include "ServerWorld.h"
#include <algorithm>

namespace Minigame::Server
{
    Bounds GetBounds(Vector2 position, const ServerCollider& collider)
    {
        const float left = position.x + collider.offset.x;
        const float top = position.y + collider.offset.y;

        return Bounds
        {
            left,
            top,
            left + collider.size.x,
            top + collider.size.y
        };
    }

    bool IsOverlapping(Vector2 positionA, const ServerCollider& colliderA, Vector2 positionB, const ServerCollider& colliderB)
    {
        const Bounds boundsA = GetBounds(positionA, colliderA);
        const Bounds boundsB = GetBounds(positionB, colliderB);

        return boundsA.left < boundsB.right && boundsA.right > boundsB.left && boundsA.top < boundsB.bottom && boundsA.bottom > boundsB.top;
    }

    bool CheckCollision(Vector2 positionA, const ServerCollider& colliderA, Vector2 positionB, const ServerCollider& colliderB, Vector2& direction, float& depth)
    {
        const Bounds boundsA = GetBounds(positionA, colliderA);
        const Bounds boundsB = GetBounds(positionB, colliderB);
        const float overlapX = std::min(boundsA.right, boundsB.right) - std::max(boundsA.left, boundsB.left);
        const float overlapY = std::min(boundsA.bottom, boundsB.bottom) - std::max(boundsA.top, boundsB.top);
        if (overlapX <= 0.0f || overlapY <= 0.0f)
            return false;

        const float centerAX = (boundsA.left + boundsA.right) * 0.5f;
        const float centerAY = (boundsA.top + boundsA.bottom) * 0.5f;
        const float centerBX = (boundsB.left + boundsB.right) * 0.5f;
        const float centerBY = (boundsB.top + boundsB.bottom) * 0.5f;
        if (overlapX < overlapY)
        {
            direction = Vector2{ centerAX < centerBX ? -1.0f : 1.0f, 0.0f };
            depth = overlapX;
        }
        else
        {
            direction = Vector2{ 0.0f, centerAY < centerBY ? -1.0f : 1.0f };
            depth = overlapY;
        }
        return true;
    }
}
