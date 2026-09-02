#pragma once

#include "Component.h"
#include <raylib.h>
#include <string>

namespace Minigame::Components
{
	class PlayerSpawner : public Component
	{
	public:
		explicit PlayerSpawner(GameObject& owner);

		void Update(float deltaTime) override;

		void SetPosition(Vector2 position);
		void SetAnimation(const std::string& name);
		void SetBulletTint(Color color);
		void SetHealthColor(Color color);

	private:
		bool spawned = false;

		Vector2 position{ 0.0f, 0.0f };
		std::string animation;
		Color bulletTint{ 220, 60, 60, 255 };
		Color healthColor{ 220, 60, 60, 255 };
	};
}

