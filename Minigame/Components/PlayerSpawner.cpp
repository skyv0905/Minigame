#include "PlayerSpawner.h"
#include "Transform.h"
#include "PlayerController.h"
#include "HealthBar.h"
#include "Animator.h"
#include "../GameObject.h"
#include "../Scene.h"

namespace Minigame::Components
{
	PlayerSpawner::PlayerSpawner(GameObject& owner) : Component(owner)
	{
	}

	void PlayerSpawner::Update(float deltaTime)
	{
		if (spawned)
			return;

		auto* player = owner.GetScene().Instantiate("Player");
		if (player == nullptr)
			return;

		auto* transform = player->GetComponent<Minigame::Components::Transform>();
		auto* playerController = player->GetComponent<Minigame::Components::PlayerController>();
		auto* healthBar = player->GetComponent<Minigame::Components::HealthBar>();
		auto* animator = player->GetComponent<Minigame::Components::Animator>();

		if (transform)
		{
			transform->SetPosition(position);
		}
		if (playerController)
		{
			playerController->SetBulletTint(bulletTint);
		}
		if (healthBar)
		{
			healthBar->SetHealthColor(healthColor);
		}
		if (animator && !animation.empty())
		{
			animator->SetDefaultState(animation);
		}

		player->AddTag("LocalPlayer");

		spawned = true;
	}

	void PlayerSpawner::SetPosition(Vector2 position)
	{
		this->position = position;
	}

	void PlayerSpawner::SetAnimation(const std::string& name)
	{
		this->animation = name;
	}

	void PlayerSpawner::SetBulletTint(Color color)
	{
		bulletTint = color;
	}

	void PlayerSpawner::SetHealthColor(Color color)
	{
		healthColor = color;
	}
}
