#pragma once

#include "BulletSystem.h"
#include "CollisionSystem.h"
#include "ExpSystem.h"
#include "HealthSystem.h"
#include "MobSystem.h"
#include "Network/Packets.h"
#include "PlayerSystem.h"
#include "PowerUpSystem.h"
#include "ServerEntities.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Minigame::Server
{
    class ServerWorld
    {
    public:
        void Reset();
        void AddPlayer(ServerPlayer player);
        void AddWall(ServerWall wall);
        void AddMob(ServerMob mob);
        void AddBullet(ServerBullet bullet);
        void AddPowerUp(ServerPowerUp powerUp);
        std::uint32_t AllocateObjectId();
        std::uint32_t GetNextObjectId() const;
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
