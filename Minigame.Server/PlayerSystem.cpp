#include "PlayerSystem.h"
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
            player.position.x += player.input.moveX * player.moveSpeed * deltaTime;
            player.position.y += player.input.moveY * player.moveSpeed * deltaTime;
        }
    }
}
