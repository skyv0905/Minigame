#pragma once
#include <string>
#include <unordered_map>
#include <raylib.h>
#include "Component.h"
#include "SpriteRenderer.h"
#include "../GameServices.h"

namespace Minigame::Components
{
    struct AnimationFrame
    {
        const Texture2D* texture = nullptr;
        float delay = 0.0f;
        Vector2 origin{ 0.0f, 0.0f };
    };

    struct AnimationClip
    {
        std::string name;
        std::vector<AnimationFrame> frames;
        bool loop = true;
    };

    class Animator : public Component
    {
    public:
        Animator(GameObject& owner, GameServices& gameServices);

        void Start() override;
        void Update(float deltaTime) override;

        void Play(const std::string& state, bool restart = false);
        void PlayDefaultState(bool restart = false);
        bool IsFinished() const;

        void SetDefaultState(const std::string& state);
        void AddClip(const std::string& state, AnimationClip clip);

    private:
        GameServices& gameServices;
        Minigame::Components::SpriteRenderer* spriteRenderer = nullptr;

        std::unordered_map<std::string, AnimationClip> clips;

        AnimationClip* currentClip = nullptr;
        std::size_t currentFrame = 0;
        float elapsed = 0.0f;
        bool finished = false;

        std::string defaultState;

        void ApplyCurrentFrame();
    };
}
