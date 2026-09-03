#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <raylib.h>

namespace Minigame::Components
{
    class NetworkPositionInterpolator
    {
    public:
        void AddSnapshot(std::uint32_t serverTick, Vector2 position);
        bool Update(float deltaTime, Vector2& position);

    private:
        struct PositionSnapshot
        {
            std::uint32_t serverTick;
            Vector2 position;
        };

        static constexpr double ServerTickRate = 30.0;
        static constexpr double InterpolationDelayTicks = 3.0;
        static constexpr std::size_t RequiredSnapshotCount = 4;
        static constexpr std::size_t MaxSnapshotCount = 8;

        std::deque<PositionSnapshot> snapshots;
        double renderTick = 0.0;
        bool started = false;
    };
}
