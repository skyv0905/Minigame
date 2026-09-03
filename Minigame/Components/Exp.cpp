#include "Exp.h"
#include <algorithm>
#include <cmath>

namespace Minigame::Components
{
	Exp::Exp(GameObject& owner, int initialLevel, int requiredExp, float requiredExpGrowthRate) :
		Component(owner),
		requiredExp(std::max(1, requiredExp)),
		level(std::max(1, initialLevel)),
		requiredExpGrowthRate(std::max(1.0f, requiredExpGrowthRate))
	{
	}

	void Exp::SetOnLevelUp(std::function<void(int)> callback)
	{
		onLevelUp = std::move(callback);
	}

	void Exp::AddExp(int amount)
	{
		if (amount <= 0)
			return;

		currentExp += amount;
		while (currentExp >= requiredExp)
		{
			currentExp -= requiredExp;
			level++;
			requiredExp = std::max(1, static_cast<int>(std::ceil(requiredExp * requiredExpGrowthRate)));
			if (onLevelUp)
			{
				onLevelUp(level);
			}
		}
	}

	void Exp::SetNetworkState(int newExp, int newLevel)
	{
		newLevel = std::max(1, newLevel);
		while (level < newLevel)
		{
			level++;
			requiredExp = std::max(1, static_cast<int>(std::ceil(requiredExp * requiredExpGrowthRate)));
			if (onLevelUp)
			{
				onLevelUp(level);
			}
		}
		currentExp = std::clamp(newExp, 0, requiredExp - 1);
	}

	int Exp::GetCurrentExp() const
	{
		return currentExp;
	}

	int Exp::GetRequiredExp() const
	{
		return requiredExp;
	}

	int Exp::GetLevel() const
	{
		return level;
	}

	float Exp::GetProgress() const
	{
		return static_cast<float>(currentExp) / static_cast<float>(requiredExp);
	}
}
