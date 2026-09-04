#include "PlayerSystem.h"
#include "CollisionUtility.h"
#include "ServerWorld.h"
#include <algorithm>
#include <cmath>

namespace Minigame::Server
{
    void PlayerSystem::SetInput(ServerWorld& world, std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet)
    {
        auto player = world.players.find(playerId);
        if (player == world.players.end())
            return;

        float moveX = std::isfinite(packet.moveX) ? std::clamp(packet.moveX, -1.0f, 1.0f) : 0.0f;
        float moveY = std::isfinite(packet.moveY) ? std::clamp(packet.moveY, -1.0f, 1.0f) : 0.0f;
        const float lengthSquared = moveX * moveX + moveY * moveY;
        if (lengthSquared > 1.0f)
        {
            const float inverseLength = 1.0f / std::sqrt(lengthSquared);
            moveX *= inverseLength;
            moveY *= inverseLength;
        }

        player->second.input.moveX = moveX;
        player->second.input.moveY = moveY;
        player->second.input.fire = packet.fire;
        player->second.input.fireSequence = packet.fireSequence;
    }

    void PlayerSystem::Update(ServerWorld& world, float deltaTime)
    {
        for (auto& [playerId, player] : world.players)
        {
            if (world.healthSystem.IsDead(player.health))
                continue;

            if (player.input.moveX != 0.0f || player.input.moveY != 0.0f)
            {
                player.forward = Vector2{ player.input.moveX, player.input.moveY };
            }
            const float moveSpeed = std::min(player.moveSpeed * player.moveSpeedMultiplier / 100.0f, 700.0f);
            const Vector2 movement{ player.input.moveX * moveSpeed * deltaTime, player.input.moveY * moveSpeed * deltaTime };
            if (!IsDiagonalMovementBlocked(player.position, player.collider, movement, world.walls))
            {
                player.position.x += movement.x;
                player.position.y += movement.y;
            }
        }
    }
}
