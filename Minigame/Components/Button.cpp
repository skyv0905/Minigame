#include "Button.h"
#include "../GameObject.h"

namespace Minigame::Components
{
    void Button::Awake()
    {
        transform = owner.GetComponent<Minigame::Components::Transform>();
    }

    void Button::Update(float deltaTime)
    {
        Vector2 position{ 0.0f, 0.0f };
        if (transform)
        {
            position = transform->GetPosition();
        }
        bounds = { position.x, position.y, size.x, size.y };

        Vector2 mousePosition = GetMousePosition();
        hovered = CheckCollisionPointRec(mousePosition, bounds);
        pressed = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if (hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (onClick)
            {
                onClick();
            }
        }
    }

    void Button::DrawUI()
    {
        Color color = LIGHTGRAY;

        if (pressed)
        {
            color = GRAY;
        }
        else if (hovered)
        {
            color = RAYWHITE;
        }

        DrawRectangleRec(bounds, color);
        DrawRectangleLinesEx(bounds, 2.0f, BLACK);

        int textWidth = 0;
        float spacing = 1.0f;
        if (font)
        {
            textWidth = MeasureTextEx(*font, text.c_str(), fontSize, spacing).x;
        }
        else
        {
            textWidth = MeasureText(text.c_str(), fontSize);
        }

        float x = bounds.x + (bounds.width - textWidth) / 2.0f;
        float y = bounds.y + (bounds.height - fontSize) / 2.0f;
        if (font)
        {
            Vector2 textPos{ x, y };
            DrawTextEx(*font, text.c_str(), textPos, fontSize, spacing, BLACK);
        }
        else
        {
            DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), fontSize, BLACK);
        }
    }

    void Button::SetSize(float width, float height)
    {
        size.x = width;
        size.y = height;
    }

    void Button::SetSize(Vector2 size)
    {
        this->size = size;
    }

    void Button::SetText(const std::string& text)
    {
        this->text = text;
    }

    void Button::SetFont(const Font& font, float fontSize)
    {
        this->font = &font;
        this->fontSize = fontSize;
    }

    void Button::SetOnClick(std::function<void()> callback)
    {
        onClick = std::move(callback);
    }

    bool Button::IsHovered() const
    {
        return hovered;
    }

    bool Button::IsPressed() const
    {
        return pressed;
    }
}