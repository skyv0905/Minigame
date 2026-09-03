#include "CollisionSystem.h"
#include "ServerWorld.h"
#include <algorithm>

namespace Minigame::Server
{
    namespace
    {
        struct Bounds
        {
            float left = 0.0f;
            float top = 0.0f;
            float right = 0.0f;
            float bottom = 0.0f;
        };

        Bounds GetBounds(Vector2 position, const ServerCollider& collider)
        {
            const float left = position.x + collider.offset.x;
            const float top = position.y + collider.offset.y;
            return Bounds{ left, top, left + collider.size.x, top + collider.size.y };
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

        void MoveOutOfCollision(Vector2& position, Vector2 direction, float depth)
        {
            position.x += direction.x * depth;
            position.y += direction.y * depth;
        }
    }

    void CollisionSystem::Update(ServerWorld& world)
    {
        std::vector<CollisionObject> objects = CollectCollisionObjects(world);
        for (std::size_t i = 0; i < objects.size(); i++)
        {
            for (std::size_t j = i + 1; j < objects.size(); j++)
            {
                Vector2 direction{};
                float depth = 0.0f;
                if (CheckCollision(*objects[i].position, *objects[i].collider, *objects[j].position, *objects[j].collider, direction, depth))
                {
                    HandleCollision(objects[i], objects[j], direction, depth);
                }
            }
        }
    }

    void CollisionSystem::HandleCollision(CollisionObject& objectA, CollisionObject& objectB, Vector2 direction, float depth)
    {
        const ColliderType typeA = objectA.collider->type;
        const ColliderType typeB = objectB.collider->type;
        if (typeA == ColliderType::Player && typeB == ColliderType::Player)
        {
            MoveOutOfCollision(*objectA.position, direction, depth);
            MoveOutOfCollision(*objectB.position, Vector2{ -direction.x, -direction.y }, depth);
            return;
        }

        if ((typeA == ColliderType::Player && typeB == ColliderType::Mob) ||
            (typeA == ColliderType::Mob && typeB == ColliderType::Player) ||
            (typeA == ColliderType::Mob && typeB == ColliderType::Mob))
        {
            MoveOutOfCollision(*objectA.position, direction, depth);
            MoveOutOfCollision(*objectB.position, Vector2{ -direction.x, -direction.y }, depth);
            return;
        }

        if ((typeA == ColliderType::Player || typeA == ColliderType::Mob) && typeB == ColliderType::Wall)
        {
            MoveOutOfCollision(*objectA.position, direction, depth);
        }
        else if ((typeB == ColliderType::Player || typeB == ColliderType::Mob) && typeA == ColliderType::Wall)
        {
            MoveOutOfCollision(*objectB.position, Vector2{ -direction.x, -direction.y }, depth);
        }
    }

    std::vector<CollisionSystem::CollisionObject> CollisionSystem::CollectCollisionObjects(ServerWorld& world)
    {
        std::vector<CollisionObject> objects;
        objects.reserve(world.players.size() + world.walls.size() + world.mobs.size() + world.powerUps.size());
        for (auto& [playerId, player] : world.players)
            objects.push_back(CollisionObject{ &player.position, &player.collider });
        for (ServerWall& wall : world.walls)
            objects.push_back(CollisionObject{ &wall.position, &wall.collider });
        for (auto& [objectId, mob] : world.mobs)
            objects.push_back(CollisionObject{ &mob.position, &mob.collider });
        for (auto& [objectId, powerUp] : world.powerUps)
            objects.push_back(CollisionObject{ &powerUp.position, &powerUp.collider });

        return objects;
    }
}
