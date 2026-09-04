#include "CollisionUtility.h"
#include "ServerWorld.h"
#include <algorithm>
#include <limits>

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

    bool SweepCollision(Vector2 startPosition, Vector2 endPosition, const ServerCollider& movingCollider, Vector2 targetPosition, const ServerCollider& targetCollider, float& hitTime)
    {
        const Bounds moving = GetBounds(startPosition, movingCollider);
        const Bounds target = GetBounds(targetPosition, targetCollider);
        if (moving.left < target.right && moving.right > target.left && moving.top < target.bottom && moving.bottom > target.top)
        {
            hitTime = 0.0f;
            return true;
        }

        const float deltaX = endPosition.x - startPosition.x;
        const float deltaY = endPosition.y - startPosition.y;
        const float infinity = (std::numeric_limits<float>::infinity)();
        float entryX = -infinity;
        float exitX = infinity;
        float entryY = -infinity;
        float exitY = infinity;

        if (deltaX > 0.0f)
        {
            entryX = (target.left - moving.right) / deltaX;
            exitX = (target.right - moving.left) / deltaX;
        }
        else if (deltaX < 0.0f)
        {
            entryX = (target.right - moving.left) / deltaX;
            exitX = (target.left - moving.right) / deltaX;
        }
        else if (moving.right < target.left || moving.left > target.right)
        {
            return false;
        }

        if (deltaY > 0.0f)
        {
            entryY = (target.top - moving.bottom) / deltaY;
            exitY = (target.bottom - moving.top) / deltaY;
        }
        else if (deltaY < 0.0f)
        {
            entryY = (target.bottom - moving.top) / deltaY;
            exitY = (target.top - moving.bottom) / deltaY;
        }
        else if (moving.bottom < target.top || moving.top > target.bottom)
        {
            return false;
        }

        const float entryTime = (std::max)(entryX, entryY);
        const float exitTime = (std::min)(exitX, exitY);
        if (entryTime > exitTime || entryTime < 0.0f || entryTime > 1.0f)
            return false;

        hitTime = entryTime;
        return true;
    }

    bool IsDiagonalMovementBlocked(Vector2 position, const ServerCollider& collider, Vector2 movement, const std::vector<ServerWall>& walls)
    {
        if (movement.x == 0.0f || movement.y == 0.0f)
            return false;

        bool blockedX = false;
        bool blockedY = false;
        const Vector2 positionX{ position.x + movement.x, position.y };
        const Vector2 positionY{ position.x, position.y + movement.y };
        for (const ServerWall& wall : walls)
        {
            blockedX = blockedX || IsOverlapping(positionX, collider, wall.position, wall.collider);
            blockedY = blockedY || IsOverlapping(positionY, collider, wall.position, wall.collider);
            if (blockedX && blockedY)
                return true;
        }

        return false;
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
