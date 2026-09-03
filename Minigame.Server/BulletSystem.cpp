#include "BulletSystem.h"
#include "ServerWorld.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace Minigame::Server
{
    namespace
    {
        struct Bounds
        {
            float left;
            float top;
            float right;
            float bottom;
        };

        Bounds GetBounds(Vector2 position, const ServerCollider& collider)
        {
            return Bounds
            {
                position.x + collider.offset.x,
                position.y + collider.offset.y,
                position.x + collider.offset.x + collider.size.x,
                position.y + collider.offset.y + collider.size.y
            };
        }

        bool IsOverlapping(Vector2 firstPosition, const ServerCollider& first, Vector2 secondPosition, const ServerCollider& second)
        {
            const Bounds a = GetBounds(firstPosition, first);
            const Bounds b = GetBounds(secondPosition, second);

            return a.left < b.right && a.right > b.left && a.top < b.bottom && a.bottom > b.top;
        }

        ServerBullet CreateBullet(std::uint32_t objectId, std::uint32_t creatorId, ColliderType creatorType,
            Vector2 position, Vector2 direction, const ServerCollider& collider, float speed, float distance, float attackPower, std::uint32_t fireSequence)
        {
            ServerBullet bullet{};
            bullet.objectId = objectId;
            bullet.createdFrom = creatorId;
            bullet.createdFromType = creatorType;
            bullet.fireSequence = fireSequence;
            bullet.position = position;
            bullet.direction = direction;
            bullet.collider = collider;
            bullet.moveSpeed = speed;
            bullet.maxDistance = distance;
            bullet.attackPower = attackPower;
            return bullet;
        }
    }

    void BulletSystem::Update(ServerWorld& world, float deltaTime)
    {
        std::vector<std::uint32_t> bulletsToRemove;
        for (auto& [objectId, bullet] : world.bullets)
        {
            const float moveDistance = bullet.moveSpeed * deltaTime;
            bullet.position.x += bullet.direction.x * moveDistance;
            bullet.position.y += bullet.direction.y * moveDistance;
            bullet.movedDistance += std::abs(moveDistance);
            bool hit = bullet.movedDistance >= bullet.maxDistance;

            for (const ServerWall& wall : world.walls)
            {
                hit = hit || IsOverlapping(bullet.position, bullet.collider, wall.position, wall.collider);
            }

            if (bullet.createdFromType == ColliderType::Player)
            {
                for (const auto& [mobId, mob] : world.mobs)
                {
                    hit = hit || IsOverlapping(bullet.position, bullet.collider, mob.position, mob.collider);
                }
            }
            else if (bullet.createdFromType == ColliderType::Mob)
            {
                for (const auto& [playerId, player] : world.players)
                {
                    hit = hit || IsOverlapping(bullet.position, bullet.collider, player.position, player.collider);
                }
            }

            if (hit)
            {
                bulletsToRemove.push_back(objectId);
            }
        }
        for (const std::uint32_t objectId : bulletsToRemove)
        {
            world.bullets.erase(objectId);
            world.destroyedBulletIds.push_back(objectId);
        }

        for (auto& [playerId, player] : world.players)
        {
            player.fireCooldownRemaining = std::max(0.0f, player.fireCooldownRemaining - deltaTime);
            if (player.input.fireSequence <= player.lastProcessedFireSequence || player.fireCooldownRemaining > 0.0f)
            {
                continue;
            }

            world.AddBullet(CreateBullet(world.nextObjectId++, playerId, ColliderType::Player,
                player.position, player.forward, player.bulletCollider, player.bulletSpeed, player.bulletDistance, player.attackPower, player.input.fireSequence));
            player.lastProcessedFireSequence = player.input.fireSequence;
            player.fireCooldownRemaining = std::max(0.0f, player.fireCooldown);
        }

        for (auto& [mobId, mob] : world.mobs)
        {
            mob.fireCooldownRemaining = std::max(0.0f, mob.fireCooldownRemaining - deltaTime);
            if (mob.targetPlayerId == 0 || mob.fireCooldownRemaining > 0.0f)
            {
                continue;
            }

            world.AddBullet(CreateBullet(world.nextObjectId++, mobId, ColliderType::Mob,
                mob.position, mob.forward, mob.bulletCollider, mob.bulletSpeed, mob.bulletDistance, mob.attackPower, 0));
            mob.fireCooldownRemaining = std::max(0.0f, mob.fireCooldown);
        }
    }
}
