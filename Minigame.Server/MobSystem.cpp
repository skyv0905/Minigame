#include "MobSystem.h"
#include "ServerWorld.h"
#include <cmath>
#include <limits>

namespace Minigame::Server
{
    void MobSystem::Update(ServerWorld& world, float deltaTime)
    {
        if (world.players.empty())
            return;

        for (auto& [objectId, mob] : world.mobs)
        {
            mob.targetPlayerId = 0;
            const ServerPlayer* target = nullptr;
            std::uint32_t targetPlayerId = std::numeric_limits<std::uint32_t>::max();
            float closestDistanceSquared = mob.detectionRange * mob.detectionRange;
            for (const auto& [playerId, player] : world.players)
            {
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
            mob.forward = Vector2{ directionX * inverseDistance, directionY * inverseDistance };
            mob.position.x += directionX * inverseDistance * mob.speed * deltaTime;
            mob.position.y += directionY * inverseDistance * mob.speed * deltaTime;
        }
    }
}
