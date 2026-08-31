#pragma once

#include "Component.h"
#include "Transform.h"
#include <raylib.h>
#include <string>

namespace Minigame::Components
{
	class PlayerMessage : public Component
	{
	public:
		explicit PlayerMessage(GameObject& owner);

		void Awake() override;
		void Update(float deltaTime) override;
		void DrawUI() override;

		void Show(const std::string& message, float duration = 2.0f);
		void SetFont(const Font& font, float fontSize);
		void SetOffset(Vector2 offset);

	private:
		Minigame::Components::Transform* transform = nullptr;
		const Font* font = nullptr;
		std::string message;
		float remainingTime = 0.0f;
		float fontSize = 20.0f;
		Vector2 offset{ 0.0f, -55.0f };
	};
}
