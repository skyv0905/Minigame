#include "PlayerMessage.h"
#include "../GameObject.h"
#include <algorithm>

namespace Minigame::Components
{
	PlayerMessage::PlayerMessage(GameObject& owner) : Component(owner)
	{
	}

	void PlayerMessage::Awake()
	{
		transform = owner.GetComponent<Minigame::Components::Transform>();
	}

	void PlayerMessage::Update(float deltaTime)
	{
		remainingTime = std::max(0.0f, remainingTime - deltaTime);
	}

	void PlayerMessage::DrawUI()
	{
		if (!transform || !font || remainingTime <= 0.0f || message.empty())
			return;

		constexpr float spacing = 1.0f;
		const Vector2 textSize = MeasureTextEx(*font, message.c_str(), fontSize, spacing);
		const Vector2 position = transform->GetPosition();
		const Vector2 textPosition
		{
			position.x + offset.x - textSize.x * 0.5f,
			position.y + offset.y - textSize.y
		};

		DrawTextEx(*font, message.c_str(), Vector2{ textPosition.x + 1.0f, textPosition.y + 1.0f }, fontSize, spacing, Color{ 0, 0, 0, 180 });
		DrawTextEx(*font, message.c_str(), textPosition, fontSize, spacing, WHITE);
	}

	void PlayerMessage::Show(const std::string& newMessage, float duration)
	{
		message = newMessage;
		remainingTime = std::max(0.0f, duration);
	}

	void PlayerMessage::SetFont(const Font& newFont, float newFontSize)
	{
		font = &newFont;
		fontSize = std::max(1.0f, newFontSize);
	}

	void PlayerMessage::SetOffset(Vector2 newOffset)
	{
		offset = newOffset;
	}
}
