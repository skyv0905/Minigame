#pragma once

#include "Component.h"
#include "Transform.h"
#include "Health.h"
#include <raylib.h>

namespace Minigame::Components
{
	class HealthBar : public Component
	{
	public:
		explicit HealthBar(GameObject& owner);

		void Awake() override;
		void DrawUI() override;

		void SetOffset(Vector2 offset);
		void SetSize(Vector2 size);
		void SetHealthColor(Color color);
		Color GetHealthColor() const;

	private:
		Minigame::Components::Transform* transform = nullptr;
		Minigame::Components::Health* health = nullptr;

		Vector2 offset{ 0.0f, -50.0f };
		Vector2 size{ 55.0f, 6.0f };

		Color healthColor{ 220, 60, 60, 255 };
	};
}
