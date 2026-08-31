#pragma once
#include <raylib.h>
#include <string>
#include "Controller.h"
#include "Collider.h"
#include "Transform.h"
#include "PlayerMessage.h"
#include "Exp.h"
#include "../TimerManager.h"

namespace Minigame::Components
{
	class PlayerController : public Controller
	{
	public:
		PlayerController(GameObject& owner, GameServices& gameServices);
		
		void Awake() override;
		void Start() override;
		void Update(float deltaTime) override;
		void OnCollisionEnter(const CollisionInfo& info) override;
		
		void OnLevelUp(int level);

	private:
		Minigame::Components::PlayerMessage* playerMessage = nullptr;
		Minigame::Components::Exp* exp = nullptr;

		void Fire() override;
		void OnPowerUpCollected(const GameObject& powerUp);
	};
}

