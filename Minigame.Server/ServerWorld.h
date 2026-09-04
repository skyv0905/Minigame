#pragma once

#include "BulletSystem.h"
#include "CollisionSystem.h"
#include "ExpSystem.h"
#include "HealthSystem.h"
#include "MobSystem.h"
#include "Network/Packets.h"
#include "PlayerSystem.h"
#include "PowerUpSystem.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
        std::uint32_t fireSequence = 0;
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

    struct ServerHealth
    {
        float maxHealth = 0.0f;
        float currentHealth = 0.0f;
    };

    struct ServerExp
    {
        int currentExp = 0;
        int requiredExp = 100;
        int level = 1;
        float requiredExpGrowthRate = 1.05f;
    };

    struct ServerPlayer
    {
        std::uint32_t playerId = 0;
        Vector2 position;
        Vector2 forward{ 1.0f, 0.0f };
        ServerCollider collider;
        ServerCollider bulletCollider;
        ServerPlayerInput input;
        ServerHealth health;
        ServerExp exp;
        float moveSpeed = 150.0f;
        float bulletSpeed = 500.0f;
        float bulletDistance = 300.0f;
        float fireCooldown = 0.15f;
        float fireCooldownRemaining = 0.0f;
        float attackPower = 10.0f;
        int moveSpeedMultiplier = 100;
        int bulletSpeedMultiplier = 100;
        int bulletDistanceMultiplier = 100;
        int attackPowerMultiplier = 100;
        std::uint32_t lastProcessedFireSequence = 0;
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
        Vector2 forward{ 1.0f, 0.0f };
        ServerCollider collider;
        ServerCollider bulletCollider;
        float speed = 0.0f;
        float bulletSpeed = 0.0f;
        float bulletDistance = 0.0f;
        float fireCooldown = 0.0f;
        float fireCooldownRemaining = 0.0f;
        float attackPower = 0.0f;
        float detectionRange = 0.0f;
        ServerHealth health;
        int exp = 0;
    };

    struct ServerBullet
    {
        std::uint32_t objectId = 0;
        std::uint32_t createdFrom = 0;
        std::uint32_t fireSequence = 0;
        ColliderType createdFromType = ColliderType::None;
        Vector2 position;
        Vector2 direction;
        ServerCollider collider;
        float movedDistance = 0.0f;
        float maxDistance = 0.0f;
        float moveSpeed = 0.0f;
        float attackPower = 0.0f;
    };

    struct ServerPowerUp
    {
        std::uint32_t objectId = 0;
        std::string prefab;
        std::unordered_set<std::string> effects;
        Vector2 position;
        ServerCollider collider;
    };

    class ServerWorld
    {
    public:
        void Reset();
        void AddPlayer(ServerPlayer player);
        void AddWall(ServerWall wall);
        void AddMob(ServerMob mob);
        void AddBullet(ServerBullet bullet);
        void AddPowerUp(ServerPowerUp powerUp);
        void RemovePlayer(std::uint32_t playerId);
        void SetPlayerInput(std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet);
        void Update(float deltaTime);

        const std::unordered_map<std::uint32_t, ServerPlayer>& GetPlayers() const;
        const std::vector<ServerWall>& GetWalls() const;
        const std::unordered_map<std::uint32_t, ServerMob>& GetMobs() const;
        const std::unordered_map<std::uint32_t, ServerBullet>& GetBullets() const;
        const std::unordered_map<std::uint32_t, ServerPowerUp>& GetPowerUps() const;
        bool IsPlayerDead(std::uint32_t playerId) const;
        std::vector<ServerBullet> ConsumeSpawnedBullets();
        std::vector<Minigame::Network::BulletDestroyPacket> ConsumeDestroyedBulletPackets();
        std::vector<Minigame::Network::ExpChangedPacket> ConsumeExpChangedPackets();
        std::vector<Minigame::Network::HpChangedPacket> ConsumeHpChangedPackets();
        std::vector<Minigame::Network::PlayerStatsChangedPacket> ConsumePlayerStatsChangedPackets();
        std::vector<Minigame::Network::PowerUpCollectedPacket> ConsumePowerUpCollectedPackets();

    private:
        friend class PlayerSystem;
        friend class MobSystem;
        friend class BulletSystem;
        friend class CollisionSystem;
        friend class HealthSystem;
        friend class ExpSystem;
        friend class PowerUpSystem;

        PlayerSystem playerSystem;
        MobSystem mobSystem;
        BulletSystem bulletSystem;
        CollisionSystem collisionSystem;
        HealthSystem healthSystem;
        ExpSystem expSystem;
        PowerUpSystem powerUpSystem;

        std::unordered_map<std::uint32_t, ServerPlayer> players;
        std::vector<ServerWall> walls;
        std::unordered_map<std::uint32_t, ServerMob> mobs;
        std::unordered_map<std::uint32_t, ServerBullet> bullets;
        std::unordered_map<std::uint32_t, ServerPowerUp> powerUps;
        std::vector<ServerBullet> spawnedBullets;
        std::vector<Minigame::Network::BulletDestroyPacket> destroyedBulletPackets;
        std::vector<Minigame::Network::ExpChangedPacket> expChangedPackets;
        std::vector<Minigame::Network::HpChangedPacket> hpChangedPackets;
        std::vector<Minigame::Network::PlayerStatsChangedPacket> playerStatsChangedPackets;
        std::vector<Minigame::Network::PowerUpCollectedPacket> powerUpCollectedPackets;
        std::uint32_t nextObjectId = 1;

        void QueuePlayerStatsChanged(const ServerPlayer& player);
    };
}
