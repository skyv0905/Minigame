#pragma once

#include "Component.h"
#include "NetworkPositionInterpolator.h"
#include <cstdint>

struct GameServices;

namespace Minigame::Components
{
    class Animator;
    class SpriteRenderer;
    class Transform;

    class MobControllerNetwork : public Component
    {
    public:
        MobControllerNetwork(GameObject& owner, GameServices& gameServices);

        void Awake() override;
        void Start() override;
        void Update(float deltaTime) override;

        void SetObjectId(std::uint32_t id);

    private:
        GameServices& gameServices;
        Transform* transform = nullptr;
        Animator* animator = nullptr;
        SpriteRenderer* spriteRenderer = nullptr;
        NetworkPositionInterpolator positionInterpolator;
        std::uint32_t objectId = 0;
        std::uint32_t lastAppliedServerTick = 0;
        bool hasAppliedState = false;
        bool isOnRegen = true;
    };
}
