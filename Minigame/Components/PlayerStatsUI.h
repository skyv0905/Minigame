#pragma once

#include "Component.h"
#include <raylib.h>
#include <string>

namespace Minigame::Components
{
	class PlayerStatsUI : public Component
	{
	public:
		explicit PlayerStatsUI(GameObject& owner);

		void DrawUI() override;

		void SetFont(const Font& font, float fontSize);
		void SetTargetTag(const std::string& tag);

	private:
		const Font* font = nullptr;
		float fontSize = 18.0f;
		std::string targetTag = "LocalPlayer";

		void DrawText(const char* text, Vector2 position, float size, Color color) const;
		void DrawStat(const char* label, const char* value, const char* value_detail, Rectangle bounds) const;
	};
}
