#include "ServerWorld.h"
#include <algorithm>
#include <utility>

namespace Minigame::Server
{
    void ServerWorld::Reset()
    {
        players.clear();
        walls.clear();
        mobs.clear();
        bullets.clear();
        powerUps.clear();
        spawnedBullets.clear();
        destroyedBulletPackets.clear();
        expChangedPackets.clear();
        hpChangedPackets.clear();
        playerStatsChangedPackets.clear();
        powerUpCollectedPackets.clear();
        nextObjectId = 1;
    }

    void ServerWorld::AddWall(ServerWall wall)
    {
        wall.collider.type = ColliderType::Wall;
        walls.push_back(std::move(wall));
    }

    void ServerWorld::AddMob(ServerMob mob)
    {
        mob.collider.type = ColliderType::Mob;
        mob.bulletCollider.type = ColliderType::Bullet;
        mob.fireCooldownRemaining = mob.fireCooldown;
        nextObjectId = std::max(nextObjectId, mob.objectId + 1);
        mobs.insert_or_assign(mob.objectId, std::move(mob));
    }

    void ServerWorld::AddBullet(ServerBullet bullet)
    {
        bullet.collider.type = ColliderType::Bullet;
        nextObjectId = std::max(nextObjectId, bullet.objectId + 1);
        bullets.insert_or_assign(bullet.objectId, bullet);
        spawnedBullets.push_back(std::move(bullet));
    }

    void ServerWorld::AddPowerUp(ServerPowerUp powerUp)
    {
        powerUp.collider.type = ColliderType::PowerUp;
        nextObjectId = std::max(nextObjectId, powerUp.objectId + 1);
        powerUps.insert_or_assign(powerUp.objectId, std::move(powerUp));
    }

    std::uint32_t ServerWorld::AllocateObjectId()
    {
        return nextObjectId++;
    }

    std::uint32_t ServerWorld::GetNextObjectId() const
    {
        return nextObjectId;
    }

    void ServerWorld::AddPlayer(ServerPlayer player)
    {
        player.collider.type = ColliderType::Player;
        player.bulletCollider.type = ColliderType::Bullet;
        nextObjectId = std::max(nextObjectId, player.playerId + 1);
        players.insert_or_assign(player.playerId, std::move(player));
    }

    void ServerWorld::RemovePlayer(std::uint32_t playerId)
    {
        players.erase(playerId);
    }

    void ServerWorld::SetPlayerInput(std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet)
    {
        playerSystem.SetInput(*this, playerId, packet);
    }

    void ServerWorld::QueuePlayerFire(std::uint32_t playerId, const Minigame::Network::PlayerFirePacket& packet)
    {
        auto player = players.find(playerId);
        if (player == players.end() || player->second.fireRequests.size() >= 8)
            return;

        player->second.fireRequests.push_back(
            {
                packet.fireSequence,
                packet.clientTick,
                Vector2{ Minigame::Network::DecodePosition(packet.positionX), Minigame::Network::DecodePosition(packet.positionY) },
                Vector2{ Minigame::Network::DecodeDirection(packet.directionX), Minigame::Network::DecodeDirection(packet.directionY) }
            });
    }

    void ServerWorld::Update(float deltaTime, std::uint32_t serverTick)
    {
        playerSystem.Update(*this, deltaTime);
        mobSystem.Update(*this, deltaTime);
        bulletSystem.Update(*this, deltaTime, serverTick);
        collisionSystem.Update(*this);
        powerUpSystem.Update(*this);
        for (auto& [playerId, player] : players)
        {
            player.positionHistory.push_back({ serverTick, player.position });
            while (player.positionHistory.size() > 32)
            {
                player.positionHistory.pop_front();
            }
        }
    }

    const std::unordered_map<std::uint32_t, ServerPlayer>& ServerWorld::GetPlayers() const
    {
        return players;
    }

    const std::vector<ServerWall>& ServerWorld::GetWalls() const
    {
        return walls;
    }

    const std::unordered_map<std::uint32_t, ServerMob>& ServerWorld::GetMobs() const
    {
        return mobs;
    }

    const std::unordered_map<std::uint32_t, ServerBullet>& ServerWorld::GetBullets() const
    {
        return bullets;
    }

    const std::unordered_map<std::uint32_t, ServerPowerUp>& ServerWorld::GetPowerUps() const
    {
        return powerUps;
    }

    bool ServerWorld::IsPlayerDead(std::uint32_t playerId) const
    {
        const auto player = players.find(playerId);
        return player != players.end() && healthSystem.IsDead(player->second.health);
    }

    std::vector<ServerBullet> ServerWorld::ConsumeSpawnedBullets()
    {
        std::vector<ServerBullet> result = std::move(spawnedBullets);
        spawnedBullets.clear();
        return result;
    }

    std::vector<Minigame::Network::BulletDestroyPacket> ServerWorld::ConsumeDestroyedBulletPackets()
    {
        std::vector<Minigame::Network::BulletDestroyPacket> result = std::move(destroyedBulletPackets);
        destroyedBulletPackets.clear();
        return result;
    }

    std::vector<Minigame::Network::ExpChangedPacket> ServerWorld::ConsumeExpChangedPackets()
    {
        std::vector<Minigame::Network::ExpChangedPacket> result = std::move(expChangedPackets);
        expChangedPackets.clear();
        return result;
    }

    std::vector<Minigame::Network::HpChangedPacket> ServerWorld::ConsumeHpChangedPackets()
    {
        std::vector<Minigame::Network::HpChangedPacket> result = std::move(hpChangedPackets);
        hpChangedPackets.clear();
        return result;
    }

    std::vector<Minigame::Network::PlayerStatsChangedPacket> ServerWorld::ConsumePlayerStatsChangedPackets()
    {
        std::vector<Minigame::Network::PlayerStatsChangedPacket> result = std::move(playerStatsChangedPackets);
        playerStatsChangedPackets.clear();
        return result;
    }

    std::vector<Minigame::Network::PowerUpCollectedPacket> ServerWorld::ConsumePowerUpCollectedPackets()
    {
        std::vector<Minigame::Network::PowerUpCollectedPacket> result = std::move(powerUpCollectedPackets);
        powerUpCollectedPackets.clear();
        return result;
    }

    void ServerWorld::QueuePlayerStatsChanged(const ServerPlayer& player)
    {
        playerStatsChangedPackets.push_back(
            {
                static_cast<std::uint8_t>(player.playerId),
                player.moveSpeed, player.bulletSpeed, player.bulletDistance, player.fireCooldown, player.attackPower,
                static_cast<std::uint16_t>(player.moveSpeedMultiplier), static_cast<std::uint16_t>(player.bulletSpeedMultiplier),
                static_cast<std::uint16_t>(player.bulletDistanceMultiplier), static_cast<std::uint16_t>(player.attackPowerMultiplier)
            });
    }
}
