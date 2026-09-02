#pragma once

#include "Component.h"
#include <cstdint>
#include <raylib.h>
#include <string>
#include <vector>

struct GameServices;

namespace Minigame::Components
{
	struct PlayerSpawnInfo
	{
		std::uint32_t playerId = 0;
		Vector2 position{ 0.0f, 0.0f };
		std::string animation;
		Color bulletTint{ 220, 60, 60, 255 };
		Color healthColor{ 220, 60, 60, 255 };
	};

	class PlayerSpawner : public Component
	{
	public:
		PlayerSpawner(GameObject& owner, GameServices& gameServices);

		void Update(float deltaTime) override;

		void SetPlayerCount(std::uint32_t count);
		void AddPlayer(PlayerSpawnInfo&& player);

	private:
		GameServices& gameServices;
		bool spawned = false;
		std::uint32_t playerCount = 0;
		std::vector<PlayerSpawnInfo> players;
	};
}

