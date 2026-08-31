#pragma once

#include "Component.h"
#include "Transform.h"
#include <raylib.h>
#include <string>

namespace Minigame::Components
{
    class TextUI : public Component
    {
    public:
        explicit TextUI(GameObject& owner);

        void Awake() override;
        void DrawUI() override;

        void SetText(const std::string& text);
        void SetFont(const Font& font);
        void SetFontSize(float fontSize);
        void SetTextColor(Color color);
        void SetBackgroundColor(Color color);

    private:
        Transform* transform = nullptr;
        std::string text;
        const Font* font = nullptr;
        float fontSize = 50.0f;
        Color textColor = BLACK;
        Color backgroundColor = BLANK;
    };
}
