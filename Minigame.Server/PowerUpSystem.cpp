#include "PowerUpSystem.h"
#include "CollisionUtility.h"
#include "ServerWorld.h"
#include <algorithm>
#include <vector>

namespace Minigame::Server
{
    void PowerUpSystem::Update(ServerWorld& world)
    {
        std::vector<std::uint32_t> powerUpsToRemove;
        for (const auto& [powerUpId, powerUp] : world.powerUps)
        {
            for (auto& [playerId, player] : world.players)
            {
                if (world.healthSystem.IsDead(player.health) || !IsOverlapping(player.position, player.collider, powerUp.position, powerUp.collider))
                    continue;

                ApplyEffects(world, player, powerUp);
                world.powerUpCollectedPackets.push_back({ powerUpId, static_cast<std::uint8_t>(playerId) });
                powerUpsToRemove.push_back(powerUpId);
                break;
            }
        }

        for (const std::uint32_t powerUpId : powerUpsToRemove)
        {
            world.powerUps.erase(powerUpId);
        }
    }

    void PowerUpSystem::ApplyEffects(ServerWorld& world, ServerPlayer& player, const ServerPowerUp& powerUp)
    {
        bool playerStatsChanged = false;
        if (powerUp.effects.contains("incSpeed"))
        {
            player.moveSpeedMultiplier += 25;
            playerStatsChanged = true;
        }
        if (powerUp.effects.contains("incAttackPower"))
        {
            player.attackPowerMultiplier += 5;
            playerStatsChanged = true;
        }
        if (powerUp.effects.contains("incAttackSpeed") && player.fireCooldown > 0.1f)
        {
			const float previousFireCooldown = player.fireCooldown;
            player.fireCooldown = std::max(player.fireCooldown * 0.82f, 0.1f);
            player.attackPowerMultiplier = std::max(player.attackPowerMultiplier - 5, 10);
            playerStatsChanged |= previousFireCooldown != player.fireCooldown;
        }
        if (powerUp.effects.contains("incBulletDistance"))
        {
            player.bulletDistanceMultiplier += 25;
            playerStatsChanged = true;
        }
        if (powerUp.effects.contains("incBulletSpeed"))
        {
            player.bulletSpeedMultiplier += 25;
            playerStatsChanged = true;
        }

        const float previousHealth = player.health.currentHealth;
        if (powerUp.effects.contains("heal1"))
        {
            world.healthSystem.Heal(player.health, 0.15f);
        }
        if (powerUp.effects.contains("heal2"))
        {
            world.healthSystem.Heal(player.health, 0.3f);
        }
        if (player.health.currentHealth != previousHealth)
        {
            world.hpChangedPackets.push_back({ player.playerId, player.health.currentHealth });
        }

        if (playerStatsChanged)
        {
            world.QueuePlayerStatsChanged(player);
        }
    }
}
