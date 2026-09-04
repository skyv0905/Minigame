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
            Vector2 position, Vector2 direction, const ServerCollider& collider, float speed, float distance, float attackPower, std::uint32_t fireSequence, float pendingMoveDistance = -1.0f)
        {
            ServerBullet bullet{};
            bullet.objectId = objectId;
            bullet.createdFrom = creatorId;
            bullet.createdFromType = creatorType;
            bullet.fireSequence = fireSequence;
            bullet.position = position;
            bullet.spawnPosition = position;
            bullet.direction = direction;
            bullet.collider = collider;
            bullet.moveSpeed = speed;
            bullet.pendingMoveDistance = pendingMoveDistance;
            bullet.maxDistance = distance;
            bullet.attackPower = attackPower;
            return bullet;
        }
    }

    void BulletSystem::Update(ServerWorld& world, float deltaTime, std::uint32_t serverTick)
    {
        // 플레이어 불릿 발사 요청 처리
        for (auto& [playerId, player] : world.players)
        {
            player.fireCooldownRemaining = std::max(0.0f, player.fireCooldownRemaining - deltaTime);
            if (world.healthSystem.IsDead(player.health))
            {
                player.fireRequests.clear();
                continue;
            }

            while (!player.fireRequests.empty())
            {
                const ServerPlayerFireRequest request = player.fireRequests.front();

                // 발사 순서 검증
                player.fireRequests.pop_front();
                if (request.fireSequence <= player.lastProcessedFireSequence)
                    continue;
                player.lastProcessedFireSequence = request.fireSequence;

                // 클라-서버 쿨타임 차이 1틱 허용
                constexpr float fireCooldownTolerance = 1.0f / 30.0f;
                if (player.fireCooldownRemaining > fireCooldownTolerance)
                    continue;

                // 네트워크 지연 -2~15틱 허용
                constexpr std::uint32_t maxRewindTicks = 15;
                constexpr std::uint32_t maxFutureTicks = 2;
                if (request.clientTick > serverTick + maxFutureTicks || (serverTick > request.clientTick && serverTick - request.clientTick > maxRewindTicks))
                    continue;

                // 불릿 위치 결정
                // 클라에서 발사 요청 시 위치 확인
                const ServerPositionSnapshot* historicalPosition = nullptr;
                for (auto history = player.positionHistory.rbegin(); history != player.positionHistory.rend(); ++history)
                {
                    if (history->serverTick <= request.clientTick)
                    {
                        historicalPosition = &*history;
                        break;
                    }
                }

                const Vector2 verifiedPosition = historicalPosition ? historicalPosition->position : player.position;

                // 서버-클라 위치 차이; 3틱 + 2px까지 허용
                const float moveSpeed = std::min(player.moveSpeed * player.moveSpeedMultiplier / 100.0f, 700.0f);
                const float maxPositionError = moveSpeed * 3.0f / 30.0f + 2.0f;

                const float positionDeltaX = request.position.x - verifiedPosition.x;
                const float positionDeltaY = request.position.y - verifiedPosition.y;
                const bool positionAllowed = positionDeltaX * positionDeltaX + positionDeltaY * positionDeltaY <= maxPositionError * maxPositionError;

                // 발사 시작 위치 결정
                const Vector2 spawnPosition = positionAllowed ? request.position : verifiedPosition;

                // 발사 방향 검증
                const float directionLengthSquared = request.direction.x * request.direction.x + request.direction.y * request.direction.y;
                if (directionLengthSquared < 0.01f || !std::isfinite(directionLengthSquared))
                    continue;

                const float inverseDirectionLength = 1.0f / std::sqrt(directionLengthSquared);
                const Vector2 direction{ request.direction.x * inverseDirectionLength, request.direction.y * inverseDirectionLength };

                // 서버 입력 지연간 이동 거리 조정
                const float bulletSpeed = std::min(player.bulletSpeed * player.bulletSpeedMultiplier / 100.0f, 900.0f);
                const std::uint32_t elapsedTicks = serverTick > request.clientTick ? serverTick - request.clientTick : 0;
                const float catchUpDistance = bulletSpeed * static_cast<float>(elapsedTicks) / 30.0f;

                world.AddBullet(CreateBullet(world.nextObjectId++, playerId, ColliderType::Player,
                    spawnPosition, direction, player.bulletCollider, bulletSpeed,
                    player.bulletDistance * player.bulletDistanceMultiplier / 100.0f, player.attackPower * player.attackPowerMultiplier / 100.0f,
                    request.fireSequence, catchUpDistance));
                player.fireCooldownRemaining = std::max(0.0f, player.fireCooldown);
            }
        }

        std::vector<Minigame::Network::BulletDestroyPacket> bulletsToDestroy;
        std::vector<std::uint32_t> mobsToRemove;

        // 불릿 충돌 처리
        for (auto& [objectId, bullet] : world.bullets)
        {
            const float remainingDistance = (std::max)(0.0f, bullet.maxDistance - bullet.movedDistance);
            const float requestedMoveDistance = bullet.pendingMoveDistance >= 0.0f ? bullet.pendingMoveDistance : std::abs(bullet.moveSpeed * deltaTime);
            const float moveDistance = (std::min)(requestedMoveDistance, remainingDistance);
            bullet.pendingMoveDistance = -1.0f;
            const bool reachedMaxDistance = moveDistance >= remainingDistance;
            const Vector2 startPosition = bullet.position;
            const Vector2 endPosition{ startPosition.x + bullet.direction.x * moveDistance, startPosition.y + bullet.direction.y * moveDistance };
            float nearestHitTime = 2.0f;
            ColliderType hitType = ColliderType::None;
            std::uint32_t hitObjectId = 0;

            // 벽 충돌 체크
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

            // 플레이어가 발사한 불릿 충돌 체크
            if (bullet.createdFromType == ColliderType::Player)
            {
                // 플레이어 -> 몹
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

                // 플레이어 -> 플레이어
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
            // 몹이 발사한 불릿 충돌 체크
            else if (bullet.createdFromType == ColliderType::Mob)
            {
                // 몹 -> 플레이어
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

            // 충돌 여부 따라 불릿 위치 처리
            const bool hit = hitType != ColliderType::None;
            bullet.position.x = startPosition.x + (endPosition.x - startPosition.x) * (hit ? nearestHitTime : 1.0f);
            bullet.position.y = startPosition.y + (endPosition.y - startPosition.y) * (hit ? nearestHitTime : 1.0f);
            bullet.movedDistance += moveDistance * (hit ? nearestHitTime : 1.0f);
            const auto spawnedBullet = std::find_if(world.spawnedBullets.begin(), world.spawnedBullets.end(), [objectId](const ServerBullet& spawned) { return spawned.objectId == objectId; });
            if (spawnedBullet != world.spawnedBullets.end())
            {
                spawnedBullet->position = bullet.position;
            }

            // 몹과 충돌
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
            // 플레이어와 충돌
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

            // 최대 이동 거리 체크
            if (hit || reachedMaxDistance)
            {
                bulletsToDestroy.push_back(
                    {
                        objectId,
                        bullet.createdFrom,
                        hitObjectId,
                        Minigame::Network::EncodePosition(bullet.position.x),
                        Minigame::Network::EncodePosition(bullet.position.y),
                        hit ? Minigame::Network::BulletDestroyReason::Collision : Minigame::Network::BulletDestroyReason::MaxDistance
                    });
            }
        }
        // 불릿 파괴
        for (const Minigame::Network::BulletDestroyPacket& packet : bulletsToDestroy)
        {
            world.bullets.erase(packet.bulletId);
            world.destroyedBulletPackets.push_back(packet);
        }
        // 사망한 몹 제거
        for (const std::uint32_t objectId : mobsToRemove)
        {
            world.mobs.erase(objectId);
        }

        // 몹 불릿 발사 처리
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
