#include "ServerWorld.h"
#include <algorithm>
#include <cmath>

namespace Minigame::Server
{
    void ServerWorld::Reset()
    {
        players.clear();
    }

    void ServerWorld::AddPlayer(std::uint32_t playerId, Vector2 spawnPosition)
    {
        ServerPlayer player{};
        player.playerId = playerId;
        player.position = spawnPosition;
        players.insert_or_assign(playerId, player);
    }

    void ServerWorld::RemovePlayer(std::uint32_t playerId)
    {
        players.erase(playerId);
    }

    void ServerWorld::SetPlayerInput(std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet)
    {
        auto player = players.find(playerId);
        if (player == players.end())
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
    }

    void ServerWorld::Update(float deltaTime)
    {
        for (auto& [playerId, player] : players)
        {
            player.position.x += player.input.moveX * PlayerSpeed * deltaTime;
            player.position.y += player.input.moveY * PlayerSpeed * deltaTime;
        }
    }

    const std::unordered_map<std::uint32_t, ServerPlayer>& ServerWorld::GetPlayers() const
    {
        return players;
    }
}
