#include "BulletSystem.h"
#include "CollisionUtility.h"
#include "ServerWorld.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace Minigame::Server
{
    namespace
    {
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
        std::vector<std::uint32_t> mobsToRemove;
        for (auto& [objectId, bullet] : world.bullets)
        {
            const float moveDistance = bullet.moveSpeed * deltaTime;
            bullet.position.x += bullet.direction.x * moveDistance;
            bullet.position.y += bullet.direction.y * moveDistance;
            bullet.movedDistance += std::abs(moveDistance);
            bool hit = bullet.movedDistance >= bullet.maxDistance;

            for (const ServerWall& wall : world.walls)
            {
                if (IsOverlapping(bullet.position, bullet.collider, wall.position, wall.collider))
                {
                    hit = true;
                    break;
                }
            }

            if (!hit && bullet.createdFromType == ColliderType::Player)
            {
                for (auto& [mobId, mob] : world.mobs)
                {
                    if (world.healthSystem.IsDead(mob.health) || !IsOverlapping(bullet.position, bullet.collider, mob.position, mob.collider))
                        continue;

                    const float previousHealth = mob.health.currentHealth;
                    world.healthSystem.Hit(mob.health, bullet.attackPower);
                    if (mob.health.currentHealth != previousHealth)
                    {
                        world.hpChangedPackets.push_back({ mobId, mob.health.currentHealth });
                    }
                    if (world.healthSystem.IsDead(mob.health))
                    {
                        mobsToRemove.push_back(mobId);
                        const auto player = world.players.find(bullet.createdFrom);
                        if (player != world.players.end())
                        {
                            const int previousExp = player->second.exp.currentExp;
                            const int previousLevel = player->second.exp.level;
                            world.expSystem.AddExp(player->second, mob.exp);
                            if (player->second.exp.currentExp != previousExp || player->second.exp.level != previousLevel)
                            {
                                world.expChangedPackets.push_back(
                                    {
                                        static_cast<std::uint8_t>(player->first),
                                        static_cast<std::uint32_t>(player->second.exp.currentExp),
                                        static_cast<std::uint32_t>(player->second.exp.level)
                                    });
                            }
                            if (player->second.exp.level != previousLevel)
                            {
                                world.QueuePlayerStatsChanged(player->second);
                            }
                        }
                    }
                    hit = true;
                    break;
                }

                if (!hit)
                {
                    for (auto& [playerId, player] : world.players)
                    {
                        if (playerId == bullet.createdFrom || world.healthSystem.IsDead(player.health) || !IsOverlapping(bullet.position, bullet.collider, player.position, player.collider))
                            continue;

                        const float previousHealth = player.health.currentHealth;
                        world.healthSystem.Hit(player.health, bullet.attackPower * 0.08f);
                        if (player.health.currentHealth != previousHealth)
                        {
                            world.hpChangedPackets.push_back({ playerId, player.health.currentHealth });
                        }
                        hit = true;
                        break;
                    }
                }
            }
            else if (!hit && bullet.createdFromType == ColliderType::Mob)
            {
                for (auto& [playerId, player] : world.players)
                {
                    if (world.healthSystem.IsDead(player.health) || !IsOverlapping(bullet.position, bullet.collider, player.position, player.collider))
                        continue;

                    const float previousHealth = player.health.currentHealth;
                    world.healthSystem.Hit(player.health, bullet.attackPower);
                    if (player.health.currentHealth != previousHealth)
                    {
                        world.hpChangedPackets.push_back({ playerId, player.health.currentHealth });
                    }
                    hit = true;
                    break;
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
        for (const std::uint32_t objectId : mobsToRemove)
        {
            world.mobs.erase(objectId);
        }

        for (auto& [playerId, player] : world.players)
        {
            player.fireCooldownRemaining = std::max(0.0f, player.fireCooldownRemaining - deltaTime);
            if (world.healthSystem.IsDead(player.health) || player.input.fireSequence <= player.lastProcessedFireSequence || player.fireCooldownRemaining > 0.0f)
            {
                continue;
            }

            world.AddBullet(CreateBullet(world.nextObjectId++, playerId, ColliderType::Player,
                player.position, player.forward, player.bulletCollider, std::min(player.bulletSpeed * player.bulletSpeedMultiplier / 100.0f, 900.0f),
                player.bulletDistance * player.bulletDistanceMultiplier / 100.0f, player.attackPower * player.attackPowerMultiplier / 100.0f, player.input.fireSequence));
            player.lastProcessedFireSequence = player.input.fireSequence;
            player.fireCooldownRemaining = std::max(0.0f, player.fireCooldown);
        }

        for (auto& [mobId, mob] : world.mobs)
        {
            mob.fireCooldownRemaining = std::max(0.0f, mob.fireCooldownRemaining - deltaTime);
            if (world.healthSystem.IsDead(mob.health) || mob.targetPlayerId == 0 || mob.fireCooldownRemaining > 0.0f)
            {
                continue;
            }

            world.AddBullet(CreateBullet(world.nextObjectId++, mobId, ColliderType::Mob,
                mob.position, mob.forward, mob.bulletCollider, mob.bulletSpeed, mob.bulletDistance, mob.attackPower, 0));
            mob.fireCooldownRemaining = std::max(0.0f, mob.fireCooldown);
        }
    }
}
