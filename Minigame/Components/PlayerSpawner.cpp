#include "PlayerSpawner.h"
#include "Transform.h"
#include "PlayerController.h"
#include "PlayerControllerNetwork.h"
#include "HealthBar.h"
#include "Animator.h"
#include "../GameServices.h"
#include "../GameObject.h"
#include "../Network/NetworkClient.h"
#include "../Scene.h"
#include <algorithm>
#include <utility>

namespace Minigame::Components
{
	PlayerSpawner::PlayerSpawner(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
	{
	}

	void PlayerSpawner::Update(float deltaTime)
	{
		if (spawned)
			return;

		const std::size_t spawnCount = std::min<std::size_t>(playerCount, players.size());
		for (std::size_t i = 0; i < spawnCount; i++)
		{
			const PlayerSpawnInfo& spawnInfo = players[i];
			auto* player = owner.GetScene().Instantiate("Player");
			if (player == nullptr)
				continue;

			if (auto* transform = player->GetComponent<Minigame::Components::Transform>())
				transform->SetPosition(spawnInfo.position);
			if (auto* playerController = player->GetComponent<Minigame::Components::PlayerController>())
				playerController->SetBulletTint(spawnInfo.bulletTint);
			if (auto* networkController = player->GetComponent<Minigame::Components::PlayerControllerNetwork>())
				networkController->SetPlayerId(spawnInfo.playerId);
			if (auto* healthBar = player->GetComponent<Minigame::Components::HealthBar>())
				healthBar->SetHealthColor(spawnInfo.healthColor);
			if (auto* animator = player->GetComponent<Minigame::Components::Animator>(); animator && !spawnInfo.animation.empty())
				animator->SetDefaultState(spawnInfo.animation);

			player->AddTag("Player" + std::to_string(spawnInfo.playerId));
			if (playerCount == 1 || gameServices.network.GetPlayerId() == spawnInfo.playerId)
				player->AddTag("LocalPlayer");
		}

		spawned = true;
	}

	void PlayerSpawner::SetPlayerCount(std::uint32_t count)
	{
		playerCount = count;
	}

	void PlayerSpawner::AddPlayer(PlayerSpawnInfo&& player)
	{
		players.push_back(std::move(player));
	}
}
