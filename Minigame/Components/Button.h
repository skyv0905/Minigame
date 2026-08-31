#pragma once

#include "Component.h"
#include "Transform.h"
#include <raylib.h>
#include <functional>
#include <string>

namespace Minigame::Components
{
    class Button : public Component
    {
    public:
        explicit Button(GameObject& owner) : Component(owner)
        {
        }

        void Awake() override;
        void Update(float deltaTime) override;
        void DrawUI() override;

        void SetSize(float width, float height);
        void SetSize(Vector2 size);
        void SetText(const std::string& text);
        void SetFont(const Font& font, float fontSize);
        void SetOnClick(std::function<void()> callback);

        bool IsHovered() const;
        bool IsPressed() const;

    private:
        Minigame::Components::Transform* transform = nullptr;

        Vector2 size{ 100.0f, 50.0f };
        Rectangle bounds{ 0.0, 0.0, 100.0, 50.f };
        std::string text;
        
        const Font* font = nullptr;
        float fontSize = 20.0f;

        std::function<void()> onClick;

        bool hovered = false;
        bool pressed = false;
    };
}