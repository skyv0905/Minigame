#include "ExpSystem.h"
#include "ServerWorld.h"
#include <algorithm>
#include <cmath>

namespace Minigame::Server
{
    void ExpSystem::AddExp(ServerPlayer& player, int amount) const
    {
        if (amount <= 0)
            return;

        player.exp.currentExp += amount;
        while (player.exp.currentExp >= player.exp.requiredExp)
        {
            player.exp.currentExp -= player.exp.requiredExp;
            player.exp.level++;
            player.exp.requiredExp = std::max(1, static_cast<int>(std::ceil(player.exp.requiredExp * player.exp.requiredExpGrowthRate)));
            OnLevelUp(player);
        }
    }

    void ExpSystem::OnLevelUp(ServerPlayer& player) const
    {
        const float m = player.exp.level % 5 == 0 ? 2.0f : 1.0f;
        player.attackPower += 5.0f * m;
        player.moveSpeed += 10.0f * m;
        player.bulletDistance += 5.0f * m;
        player.bulletSpeed += 10.0f * m;
    }
}
