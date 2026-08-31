#include "SpriteRenderer.h"
#include "Transform.h"
#include "../GameObject.h"
#include <array>
#include <cmath>

namespace Minigame::Components
{
    void SpriteRenderer::Awake()
    {
        transform = owner.GetComponent<Minigame::Components::Transform>();
    }

    void SpriteRenderer::Start()
    {
    }

    void SpriteRenderer::Draw()
    {
        if (texture == nullptr || transform == nullptr)
            return;

        const float scale = transform->GetScale();

        Rectangle srcRect{ 0.0f, 0.0f, static_cast<float>(texture->width) * (flipX ? -1.0f : 1.0f), static_cast<float>(texture->height) * (flipY ? -1.0f : 1.0f) };
        Rectangle dstRect{ transform->GetPosition().x, transform->GetPosition().y, texture->width * scale, texture->height * scale };

        const float originX = flipX
            ? static_cast<float>(texture->width) - origin.x
            : origin.x;
        const float originY = flipY
            ? static_cast<float>(texture->height) - origin.y
            : origin.y;
        Vector2 scaledOrigin{ originX * scale, originY * scale };

        DrawTexturePro(*texture, srcRect, dstRect, scaledOrigin, transform->GetRotation(), tint);

        if (drawBounds)
        //if (true)
        {
            if (transform->GetRotation() == 0.0f)
            {
                DrawRectangleLinesEx(Rectangle{dstRect.x - scaledOrigin.x, dstRect.y - scaledOrigin.y, dstRect.width, dstRect.height }, 2.0f, BLACK);
            }
            else
            {
                const float radians = transform->GetRotation() * DEG2RAD;
                const float cosine = std::cos(radians);
                const float sine = std::sin(radians);
                const std::array<Vector2, 4> localCorners
                {
                    Vector2{ -scaledOrigin.x, -scaledOrigin.y },
                    Vector2{ dstRect.width - scaledOrigin.x, -scaledOrigin.y },
                    Vector2{ dstRect.width - scaledOrigin.x, dstRect.height - scaledOrigin.y },
                    Vector2{ -scaledOrigin.x, dstRect.height - scaledOrigin.y }
                };
                std::array<Vector2, 4> corners;
                for (size_t i = 0; i < corners.size(); ++i)
                {
                    corners[i] = Vector2
                    {
                        dstRect.x + localCorners[i].x * cosine - localCorners[i].y * sine,
                        dstRect.y + localCorners[i].x * sine + localCorners[i].y * cosine
                    };
                }
                for (size_t i = 0; i < corners.size(); ++i)
                {
                    DrawLineEx(corners[i], corners[(i + 1) % corners.size()], 2.0f, BLACK);
                }
            }
        }
    }

	void SpriteRenderer::SetTexture(const Texture2D& texture)
	{
		this->texture = &texture;
	}

    void SpriteRenderer::SetOrigin(Vector2 origin)
    {
        this->origin = origin;
    }

    void SpriteRenderer::SetTint(Color tint)
    {
        this->tint = tint;
    }

    void SpriteRenderer::SetFlipX(bool flip)
    {
        flipX = flip;
    }

    void SpriteRenderer::SetFlipY(bool flip)
    {
        flipY = flip;
    }

    bool SpriteRenderer::GetFlipX()
    {
        return flipX;
    }

    bool SpriteRenderer::GetFlipY()
    {
        return flipY;
    }

    void SpriteRenderer::SetDrawBounds(bool draw)
    {
        drawBounds = draw;
    }
}
