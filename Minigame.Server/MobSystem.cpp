#include "MobSystem.h"
#include "CollisionUtility.h"
#include "ServerWorld.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Minigame::Server
{
    void MobSystem::Update(ServerWorld& world, float deltaTime)
    {
        for (auto& [objectId, mob] : world.mobs)
        {
            mob.targetPlayerId = 0;
            if (world.healthSystem.IsDead(mob.health))
                continue;

            if (mob.state != MobState::Idle)
            {
                const MobState previousState = mob.state;
                mob.stateRemaining = std::max(0.0f, mob.stateRemaining - deltaTime);
                if (mob.stateRemaining <= 0.0f)
                {
                    mob.state = MobState::Idle;
                }
                if (previousState == MobState::Regen)
                    continue;
            }

            const ServerPlayer* target = nullptr;
            std::uint32_t targetPlayerId = std::numeric_limits<std::uint32_t>::max();
            float closestDistanceSquared = mob.detectionRange * mob.detectionRange;
            for (const auto& [playerId, player] : world.players)
            {
                if (world.healthSystem.IsDead(player.health))
                    continue;

                const float dx = player.position.x - mob.position.x;
                const float dy = player.position.y - mob.position.y;
                const float distanceSquared = dx * dx + dy * dy;
                if (distanceSquared > closestDistanceSquared)
                    continue;
                if (target != nullptr && distanceSquared == closestDistanceSquared && playerId > targetPlayerId)
                    continue;

                target = &player;
                targetPlayerId = playerId;
                closestDistanceSquared = distanceSquared;
            }

            if (target == nullptr || closestDistanceSquared <= 0.0f)
                continue;

            mob.targetPlayerId = targetPlayerId;
            const float directionX = target->position.x - mob.position.x;
            const float directionY = target->position.y - mob.position.y;
            const float inverseDistance = 1.0f / std::sqrt(closestDistanceSquared);
            if (mob.state == MobState::Hit)
                continue;

            mob.forward = Vector2{ directionX * inverseDistance, directionY * inverseDistance };
            const Vector2 movement{ directionX * inverseDistance * mob.speed * deltaTime, directionY * inverseDistance * mob.speed * deltaTime };
            if (!IsDiagonalMovementBlocked(mob.position, mob.collider, movement, world.walls))
            {
                mob.position.x += movement.x;
                mob.position.y += movement.y;
            }
        }
    }
}
