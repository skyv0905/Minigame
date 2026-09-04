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
        std::vector<Minigame::Network::BulletDestroyPacket> bulletsToDestroy;
        std::vector<std::uint32_t> mobsToRemove;
        for (auto& [objectId, bullet] : world.bullets)
        {
            const float remainingDistance = (std::max)(0.0f, bullet.maxDistance - bullet.movedDistance);
            const float moveDistance = (std::min)(std::abs(bullet.moveSpeed * deltaTime), remainingDistance);
            const bool reachedMaxDistance = moveDistance >= remainingDistance;
            const Vector2 startPosition = bullet.position;
            const Vector2 endPosition{ startPosition.x + bullet.direction.x * moveDistance, startPosition.y + bullet.direction.y * moveDistance };
            float nearestHitTime = 2.0f;
            ColliderType hitType = ColliderType::None;
            std::uint32_t hitObjectId = 0;

            for (const ServerWall& wall : world.walls)
            {
                float hitTime = 0.0f;
                if (SweepCollision(startPosition, endPosition, bullet.collider, wall.position, wall.collider, hitTime) && hitTime < nearestHitTime)
                {
                    nearestHitTime = hitTime;
                    hitType = ColliderType::Wall;
                    hitObjectId = 0;
                }
            }

            if (bullet.createdFromType == ColliderType::Player)
            {
                for (auto& [mobId, mob] : world.mobs)
                {
                    if (mob.state == MobState::Regen || world.healthSystem.IsDead(mob.health))
                        continue;

                    float hitTime = 0.0f;
                    if (SweepCollision(startPosition, endPosition, bullet.collider, mob.position, mob.collider, hitTime) && hitTime < nearestHitTime)
                    {
                        nearestHitTime = hitTime;
                        hitType = ColliderType::Mob;
                        hitObjectId = mobId;
                    }
                }

                for (auto& [playerId, player] : world.players)
                {
                    if (playerId == bullet.createdFrom || world.healthSystem.IsDead(player.health))
                        continue;

                    float hitTime = 0.0f;
                    if (SweepCollision(startPosition, endPosition, bullet.collider, player.position, player.collider, hitTime) && hitTime < nearestHitTime)
                    {
                        nearestHitTime = hitTime;
                        hitType = ColliderType::Player;
                        hitObjectId = playerId;
                    }
                }
            }
            else if (bullet.createdFromType == ColliderType::Mob)
            {
                for (auto& [playerId, player] : world.players)
                {
                    if (world.healthSystem.IsDead(player.health))
                        continue;

                    float hitTime = 0.0f;
                    if (SweepCollision(startPosition, endPosition, bullet.collider, player.position, player.collider, hitTime) && hitTime < nearestHitTime)
                    {
                        nearestHitTime = hitTime;
                        hitType = ColliderType::Player;
                        hitObjectId = playerId;
                    }
                }
            }

            const bool hit = hitType != ColliderType::None;
            bullet.position.x = startPosition.x + (endPosition.x - startPosition.x) * (hit ? nearestHitTime : 1.0f);
            bullet.position.y = startPosition.y + (endPosition.y - startPosition.y) * (hit ? nearestHitTime : 1.0f);
            bullet.movedDistance += moveDistance * (hit ? nearestHitTime : 1.0f);

            if (hitType == ColliderType::Mob)
            {
                auto mob = world.mobs.find(hitObjectId);
                if (mob != world.mobs.end())
                {
                    const float previousHealth = mob->second.health.currentHealth;
                    world.healthSystem.Hit(mob->second.health, bullet.attackPower);
                    if (mob->second.health.currentHealth != previousHealth)
                    {
                        world.hpChangedPackets.push_back({ hitObjectId, mob->second.health.currentHealth });
                    }
                    if (world.healthSystem.IsDead(mob->second.health))
                    {
                        mobsToRemove.push_back(hitObjectId);
                        const auto player = world.players.find(bullet.createdFrom);
                        if (player != world.players.end())
                        {
                            const int previousExp = player->second.exp.currentExp;
                            const int previousLevel = player->second.exp.level;
                            world.expSystem.AddExp(player->second, mob->second.exp);
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
                    else
                    {
                        mob->second.state = MobState::Hit;
                        mob->second.stateRemaining = ServerMob::HitDuration;
                    }
                }
            }
            else if (hitType == ColliderType::Player)
            {
                auto player = world.players.find(hitObjectId);
                if (player != world.players.end())
                {
                    const float attackPower = bullet.createdFromType == ColliderType::Player ? bullet.attackPower * 0.08f : bullet.attackPower;
                    const float previousHealth = player->second.health.currentHealth;
                    world.healthSystem.Hit(player->second.health, attackPower);
                    if (player->second.health.currentHealth != previousHealth)
                    {
                        world.hpChangedPackets.push_back({ hitObjectId, player->second.health.currentHealth });
                    }
                }
            }

            if (hit || reachedMaxDistance)
            {
                bulletsToDestroy.push_back({ objectId, bullet.createdFrom, hitObjectId });
            }
        }
        for (const Minigame::Network::BulletDestroyPacket& packet : bulletsToDestroy)
        {
            world.bullets.erase(packet.bulletId);
            world.destroyedBulletPackets.push_back(packet);
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
            if (mob.state == MobState::Regen || world.healthSystem.IsDead(mob.health) || mob.targetPlayerId == 0 || mob.fireCooldownRemaining > 0.0f)
            {
                continue;
            }

            world.AddBullet(CreateBullet(world.nextObjectId++, mobId, ColliderType::Mob,
                mob.position, mob.forward, mob.bulletCollider, mob.bulletSpeed, mob.bulletDistance, mob.attackPower, 0));
            mob.fireCooldownRemaining = std::max(0.0f, mob.fireCooldown);
        }
    }
}
