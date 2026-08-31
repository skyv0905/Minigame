#pragma once

#include "Component.h"
#include "Transform.h"
#include <raylib.h>

namespace Minigame::Components
{
	class SpriteRenderer : public Component
	{
	public:
		explicit SpriteRenderer(GameObject& owner) : Component(owner)
		{
		}
		
		void Awake() override;
		void Start() override;
		void Draw() override;

		void SetTexture(const Texture2D& texture);
		void SetOrigin(Vector2 origin);
		void SetTint(Color tint);

		void SetFlipX(bool flip);
		void SetFlipY(bool flip);
		bool GetFlipX();
		bool GetFlipY();
		
		void SetDrawBounds(bool draw);


	private:
		Minigame::Components::Transform* transform = nullptr;
		const Texture2D* texture = nullptr;
		Vector2 origin{ 0.0f, 0.0f };
		Color tint = WHITE;
		bool flipX = false;
		bool flipY = false;

		bool drawBounds = false;
	};
}

