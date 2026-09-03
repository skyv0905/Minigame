#include "ServerWorld.h"
#include <utility>

namespace Minigame::Server
{
    void ServerWorld::Reset()
    {
        players.clear();
        walls.clear();
        mobs.clear();
        powerUps.clear();
    }

    void ServerWorld::AddWall(ServerWall wall)
    {
        wall.collider.type = ColliderType::Wall;
        walls.push_back(std::move(wall));
    }

    void ServerWorld::AddMob(ServerMob mob)
    {
        mob.collider.type = ColliderType::Mob;
        mobs.insert_or_assign(mob.objectId, std::move(mob));
    }

    void ServerWorld::AddPowerUp(ServerPowerUp powerUp)
    {
        powerUp.collider.type = ColliderType::PowerUp;
        powerUps.insert_or_assign(powerUp.objectId, std::move(powerUp));
    }

    void ServerWorld::AddPlayer(std::uint32_t playerId, Vector2 spawnPosition, ServerCollider collider)
    {
        ServerPlayer player{};
        player.playerId = playerId;
        player.position = spawnPosition;
        player.collider = collider;
        player.collider.type = ColliderType::Player;
        players.insert_or_assign(playerId, player);
    }

    void ServerWorld::RemovePlayer(std::uint32_t playerId)
    {
        players.erase(playerId);
    }

    void ServerWorld::SetPlayerInput(std::uint32_t playerId, const Minigame::Network::PlayerInputPacket& packet)
    {
        playerSystem.SetInput(*this, playerId, packet);
    }

    void ServerWorld::Update(float deltaTime)
    {
        playerSystem.Update(*this, deltaTime);
        mobSystem.Update(*this, deltaTime);
        bulletSystem.Update(*this, deltaTime);
        collisionSystem.Update(*this);
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

    const std::unordered_map<std::uint32_t, ServerPowerUp>& ServerWorld::GetPowerUps() const
    {
        return powerUps;
    }
}
