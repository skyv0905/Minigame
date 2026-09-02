#pragma once

#include "Component.h"
#include <cstdint>

struct GameServices;

namespace Minigame::Components
{
    class NetworkInputSender : public Component
    {
    public:
        NetworkInputSender(GameObject& owner, GameServices& gameServices);

        void Update(float deltaTime) override;

    private:
        static constexpr float SendInterval = 1.0f / 30.0f;
        static constexpr int MaxSendsPerFrame = 2;

        GameServices& gameServices;
        std::uint32_t nextSequence = 1;
        float sendAccumulator = 0.0f;
        bool fireBuffered = false;

        bool SendInput();
    };
}
