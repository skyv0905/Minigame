#pragma once

#include "Component.h"
#include <cstdint>

struct GameServices;

namespace Minigame::Components
{
    class Transform;

    class PlayerControllerNetwork : public Component
    {
    public:
        PlayerControllerNetwork(GameObject& owner, GameServices& gameServices);

        void Awake() override;
        void Update(float deltaTime) override;

        void SetPlayerId(std::uint32_t id);

    private:
        GameServices& gameServices;
        Transform* transform = nullptr;
        std::uint32_t playerId = 0;
        std::uint32_t lastAppliedServerTick = 0;
        bool hasAppliedState = false;
    };
}
