#include "PlayerStatsUI.h"
#include "Health.h"
#include "HealthBar.h"
#include "PlayerController.h"
#include "Exp.h"
#include "../GameObject.h"
#include "../Scene.h"
#include <algorithm>

namespace Minigame::Components
{
	PlayerStatsUI::PlayerStatsUI(GameObject& owner) : Component(owner)
	{
	}

	void PlayerStatsUI::DrawUI()
	{
		auto* target = owner.GetScene().FindGameObjectWithTag(targetTag);
		auto* health = target ? target->GetComponent<Minigame::Components::Health>() : nullptr;
		auto* healthBar = target ? target->GetComponent<Minigame::Components::HealthBar>() : nullptr;
		auto* exp = target ? target->GetComponent<Minigame::Components::Exp>() : nullptr;
		auto* playerController = target ? target->GetComponent<Minigame::Components::PlayerController>() : nullptr;

		constexpr float panelWidth = 1366.0f;
		constexpr float panelHeight = 132.0f;
		const float panelY = static_cast<float>(GetScreenHeight()) - panelHeight;
		const Rectangle panel{ 0.0f, panelY, panelWidth, panelHeight };

		DrawRectangleRec(panel, Color{ 20, 25, 38, 245 });
		DrawRectangleGradientV(0, static_cast<int>(panelY), static_cast<int>(panelWidth), 6, Color{ 90, 150, 255, 255 }, Color{ 44, 66, 104, 255 });

		if (health)
		{
			const float maxHealth = health->GetMaxHealth();
			const float currentHealth = health->GetCurrentHealth();
			const float healthRatio = maxHealth > 0.0f ? std::clamp(currentHealth / maxHealth, 0.0f, 1.0f) : 0.0f;
			const Rectangle healthBackground{ 24.0f, panelY + 40.0f, 420.0f, 24.0f };
			Rectangle healthForeground = healthBackground;
			healthForeground.width *= healthRatio;

			DrawText(TextFormat("HP  %.0f / %.0f", currentHealth, maxHealth), Vector2{ 24.0f, panelY + 15.0f }, fontSize, WHITE);
			DrawRectangleRec(healthBackground, Color{ 8, 11, 18, 255 });
			DrawRectangleRec(healthForeground, healthBar ? healthBar->GetHealthColor() : healthColor);
			DrawRectangleLinesEx(healthBackground, 1.0f, Color{ 135, 155, 195, 255 });
		}

		if (exp)
		{
			const int maxExp = exp->GetRequiredExp();
			const int currentExp = exp->GetCurrentExp();
			const float expRatio = maxExp > 0 ? std::clamp(static_cast<float>(currentExp) / static_cast<float>(maxExp), 0.0f, 1.0f) : 0.0f;
			const Rectangle expBackground{ 24.0f, panelY + 95.0f, 420.0f, 24.0f };
			Rectangle expForeground = expBackground;
			expForeground.width *= expRatio;

			DrawText(TextFormat("EXP  %d / %d", currentExp, maxExp), Vector2{ 24.0f, panelY + 70.0f }, fontSize, WHITE);
			DrawRectangleRec(expBackground, Color{ 8, 11, 18, 255 });
			DrawRectangleRec(expForeground, Color{ 195, 224, 0, 255 });
			DrawRectangleLinesEx(expBackground, 1.0f, Color{ 135, 155, 195, 255 });

			DrawText(TextFormat("레벨 %d", exp->GetLevel()), Vector2{ 400.0f, panelY + 70.0f }, fontSize, Color{ 180, 205, 255, 255 });
		}

		if (!playerController)
			return;

		constexpr float statStartX = 480.0f;
		constexpr float statWidth = 160.0f;
		constexpr float statGap = 8.0f;
		const float statY = panelY + 24.0f;
		const Vector2 statSize{ statWidth, 84.0f };

		DrawStat("공격력", TextFormat("%.1f", playerController->GetFinalAttackPower()), playerController->GetAttackPowerDetail().c_str(), Rectangle{statStartX, statY, statSize.x, statSize.y});
		DrawStat("공격 속도", TextFormat("%.2f", 1.0f / std::max(playerController->GetFireCooldown(), 0.001f)), "", Rectangle{statStartX + (statWidth + statGap), statY, statSize.x, statSize.y});
		DrawStat("이동 속도", TextFormat("%.0f", playerController->GetFinalMoveSpeed()), playerController->GetMoveSpeedDetail().c_str(), Rectangle{ statStartX + (statWidth + statGap) * 2.0f, statY, statSize.x, statSize.y });
		DrawStat("발사체 속도", TextFormat("%.0f", playerController->GetFinalBulletSpeed()), playerController->GetBulletSpeedDetail().c_str(), Rectangle{ statStartX + (statWidth + statGap) * 3.0f, statY, statSize.x, statSize.y });
		DrawStat("사거리", TextFormat("%.0f", playerController->GetFinalBulletDistance()), playerController->GetBulletDistanceDetail().c_str(), Rectangle{ statStartX + (statWidth + statGap) * 4.0f, statY, statSize.x, statSize.y });
	}

	void PlayerStatsUI::SetFont(const Font& newFont, float newFontSize)
	{
		font = &newFont;
		fontSize = std::max(1.0f, newFontSize);
	}

	void PlayerStatsUI::SetTargetTag(const std::string& tag)
	{
		targetTag = tag;
	}

	void PlayerStatsUI::DrawText(const char* text, Vector2 position, float size, Color color) const
	{
		if (font)
		{
			DrawTextEx(*font, text, position, size, 1.0f, color);
		}
		else
		{
			::DrawText(text, static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size), color);
		}
	}

	void PlayerStatsUI::DrawStat(const char* label, const char* value, const char* value_detail, Rectangle bounds) const
	{
		DrawRectangleRec(bounds, Color{ 31, 39, 57, 255 });
		DrawRectangleLinesEx(bounds, 1.0f, Color{ 66, 82, 116, 255 });
		DrawText(label, Vector2{ bounds.x + 12.0f, bounds.y + 12.0f }, fontSize - 2.0f, Color{ 155, 170, 200, 255 });
		DrawText(value, Vector2{ bounds.x + 12.0f, bounds.y + 33.0f }, fontSize + 2.0f, WHITE);
		DrawText(value_detail, Vector2{ bounds.x + 12.0f, bounds.y + 53.0f }, fontSize - 2.0f, LIGHTGRAY);
	}
}
