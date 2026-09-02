#pragma once

#include "Network/Packets.h"
#include <cstdint>
#include <unordered_map>

namespace Minigame::Server
{
    struct Vector2
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct ServerPlayerInput
    {
        float moveX = 0.0f;
        float moveY = 0.0f;
        bool fire = false;
    };

    struct ServerPlayer
    {
        std::uint32_t playerId = 0;
        Vector2 position;
        ServerPlayerInput input;
    };

    class ServerWorld
    {
    public:
        void Reset();
        void AddPlayer(std::uint32_t playerId, Vector2 spawnPosition);
        void RemovePlayer(std::uint32_t playerId);
        void SetPlayerInput(std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet);
        void Update(float deltaTime);

        const std::unordered_map<std::uint32_t, ServerPlayer>& GetPlayers() const;

    private:
        static constexpr float PlayerSpeed = 150.0f;

        std::unordered_map<std::uint32_t, ServerPlayer> players;
    };
}
