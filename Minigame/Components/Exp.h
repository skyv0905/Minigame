#pragma once

#include "Component.h"
#include <functional>

namespace Minigame::Components
{
	class Exp : public Component
	{
	public:
		Exp(GameObject& owner, int initialLevel, int requiredExp, float requiredExpGrowthRate);

		void SetOnLevelUp(std::function<void(int)> callback);

		void AddExp(int amount);

		int GetCurrentExp() const;
		int GetRequiredExp() const;
		int GetLevel() const;
		float GetProgress() const;

	private:
		int currentExp = 0;
		int requiredExp = 100;
		int level = 1;
		float requiredExpGrowthRate = 1.05f;

		std::function<void(int)> onLevelUp;
	};
}
