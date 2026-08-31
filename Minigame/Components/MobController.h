#pragma once
#include "Controller.h"
#include <raylib.h>

namespace Minigame::Components
{
	class MobController : public Controller
	{
	public:
		MobController(GameObject& owner, GameServices& gameServices);

		void Start() override;
		void Update(float deltaTime) override;
		void OnCollisionEnter(const CollisionInfo& info) override;

		void SetDetectionRange(float range);

	private:
		GameObject* target = nullptr;

		float detectionRangeSquare{ 10000.0f };
	};
}
