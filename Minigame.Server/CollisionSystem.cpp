#include "CollisionSystem.h"
#include "CollisionUtility.h"
#include "ServerWorld.h"

namespace Minigame::Server
{
    namespace
    {
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
        objects.reserve(world.players.size() + world.walls.size() + world.mobs.size());
        for (auto& [playerId, player] : world.players)
            objects.push_back(CollisionObject{ &player.position, &player.collider });
        for (ServerWall& wall : world.walls)
            objects.push_back(CollisionObject{ &wall.position, &wall.collider });
        for (auto& [objectId, mob] : world.mobs)
        {
            if (mob.state != MobState::Regen)
            {
                objects.push_back(CollisionObject{ &mob.position, &mob.collider });
            }
        }
        return objects;
    }
}
