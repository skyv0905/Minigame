#include "TextUI.h"
#include "../GameObject.h"
#include <algorithm>

namespace Minigame::Components
{
    TextUI::TextUI(GameObject& owner) : Component(owner)
    {
    }

    void TextUI::Awake()
    {
        transform = owner.GetComponent<Transform>();
    }

    void TextUI::DrawUI()
    {
        if (!transform || text.empty())
            return;

        const Font& drawFont = font ? *font : GetFontDefault();
        constexpr float spacing = 1.0f;
        const Vector2 textSize = MeasureTextEx(drawFont, text.c_str(), fontSize, spacing);
        const Vector2 position = { transform->GetPosition().x - textSize.x * 0.5f, transform->GetPosition().y - textSize.y * 0.5f };

        if (backgroundColor.a > 0)
        {
            DrawRectangleRec(Rectangle{ position.x, position.y, textSize.x, textSize.y }, backgroundColor);
        }

        DrawTextEx(drawFont, text.c_str(), position, fontSize, spacing, textColor);
    }

    void TextUI::SetText(const std::string& text)
    {
        this->text = text;
    }

    void TextUI::SetFont(const Font& font)
    {
        this->font = &font;
    }

    void TextUI::SetFontSize(float fontSize)
    {
        this->fontSize = std::max(1.0f, fontSize);
    }

    void TextUI::SetTextColor(Color color)
    {
        textColor = color;
    }

    void TextUI::SetBackgroundColor(Color color)
    {
        backgroundColor = color;
    }
}
