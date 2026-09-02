#include "HealthBar.h"
#include "../GameObject.h"
#include <algorithm>

namespace Minigame::Components
{
	HealthBar::HealthBar(GameObject& owner) : Component(owner)
	{
	}

	void HealthBar::Awake()
	{
		transform = owner.GetComponent<Minigame::Components::Transform>();
		health = owner.GetComponent<Minigame::Components::Health>();
	}

	void HealthBar::DrawUI()
	{
		if (!transform || !health || health->IsDead())
			return;

		const float maxHealth = health->GetMaxHealth();
		if (maxHealth <= 0.0f)
			return;

		const float ratio = std::clamp(health->GetCurrentHealth() / maxHealth, 0.0f, 1.0f);
		const Vector2 position = transform->GetPosition();
		const Rectangle background
		{
			position.x + offset.x - size.x * 0.5f,
			position.y + offset.y,
			size.x,
			size.y
		};
		Rectangle foreground = background;
		foreground.width *= ratio;

		DrawRectangleRec(background, Color{ 40, 40, 40, 220 });
		DrawRectangleRec(foreground, healthColor);
		DrawRectangleLinesEx(background, 1.0f, BLACK);

	}

	void HealthBar::SetOffset(Vector2 offset)
	{
		this->offset = offset;
	}

	void HealthBar::SetSize(Vector2 size)
	{
		this->size = size;
	}

	void HealthBar::SetHealthColor(Color color)
	{
		this->healthColor = color;
	}

	Color HealthBar::GetHealthColor() const
	{
		return healthColor;
	}
}
