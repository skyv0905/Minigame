#pragma once

#include "Component.h"
#include "NetworkPositionInterpolator.h"
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
        static constexpr float CorrectionRate = 12.0f;
        static constexpr float CorrectionDeadZone = 8.0f;
        static constexpr float SnapDistance = 50.0f;

        GameServices& gameServices;
        Transform* transform = nullptr;
        NetworkPositionInterpolator positionInterpolator;
        Vector2 pendingCorrection{};
        std::uint32_t playerId = 0;
        std::uint32_t lastAppliedServerTick = 0;
        bool hasAppliedState = false;
    };
}
