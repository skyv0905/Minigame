#pragma once

#include "BulletSystem.h"
#include "CollisionSystem.h"
#include "MobSystem.h"
#include "Network/Packets.h"
#include "PlayerSystem.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

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

    enum class ColliderType
    {
        None,
        Player,
        Wall,
        Mob,
        Bullet,
        PowerUp
    };

    struct ServerCollider
    {
        Vector2 size;
        Vector2 offset;
        bool isTrigger = false;
        ColliderType type = ColliderType::None;
    };

    struct ServerPlayer
    {
        std::uint32_t playerId = 0;
        Vector2 position;
        ServerCollider collider;
        ServerPlayerInput input;
    };

    struct ServerWall
    {
        Vector2 position;
        ServerCollider collider;
    };

    struct ServerMob
    {
        std::uint32_t objectId = 0;
        std::uint32_t targetPlayerId = 0;
        std::string prefab;
        Vector2 position;
        ServerCollider collider;
        float speed = 0.0f;
        float bulletSpeed = 0.0f;
        float bulletDistance = 0.0f;
        float fireCooldown = 0.0f;
        float attackPower = 0.0f;
        float detectionRange = 0.0f;
        float health = 0.0f;
        int exp = 0;
    };

    struct ServerPowerUp
    {
        std::uint32_t objectId = 0;
        std::string prefab;
        std::string effect;
        Vector2 position;
        ServerCollider collider;
    };

    class ServerWorld
    {
    public:
        void Reset();
        void AddPlayer(std::uint32_t playerId, Vector2 spawnPosition, ServerCollider collider = {});
        void AddWall(ServerWall wall);
        void AddMob(ServerMob mob);
        void AddPowerUp(ServerPowerUp powerUp);
        void RemovePlayer(std::uint32_t playerId);
        void SetPlayerInput(std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet);
        void Update(float deltaTime);

        const std::unordered_map<std::uint32_t, ServerPlayer>& GetPlayers() const;
        const std::vector<ServerWall>& GetWalls() const;
        const std::unordered_map<std::uint32_t, ServerMob>& GetMobs() const;
        const std::unordered_map<std::uint32_t, ServerPowerUp>& GetPowerUps() const;

    private:
        static constexpr float PlayerSpeed = 150.0f;

        friend class PlayerSystem;
        friend class MobSystem;
        friend class BulletSystem;
        friend class CollisionSystem;

        PlayerSystem playerSystem;
        MobSystem mobSystem;
        BulletSystem bulletSystem;
        CollisionSystem collisionSystem;

        std::unordered_map<std::uint32_t, ServerPlayer> players;
        std::vector<ServerWall> walls;
        std::unordered_map<std::uint32_t, ServerMob> mobs;
        std::unordered_map<std::uint32_t, ServerPowerUp> powerUps;
    };
}
