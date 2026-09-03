#include "NetworkPositionInterpolator.h"

#include <algorithm>

namespace Minigame::Components
{
    void NetworkPositionInterpolator::AddSnapshot(std::uint32_t serverTick, Vector2 position)
    {
        if (!snapshots.empty() && serverTick <= snapshots.back().serverTick)
            return;

        snapshots.push_back(PositionSnapshot{ serverTick, position });

        while (snapshots.size() > MaxSnapshotCount)
            snapshots.pop_front();

        if (!started && snapshots.size() >= RequiredSnapshotCount)
        {
            renderTick = static_cast<double>(snapshots.back().serverTick) - InterpolationDelayTicks;
            started = true;
        }

        if (started && renderTick < snapshots.front().serverTick)
            renderTick = snapshots.front().serverTick;
    }

    bool NetworkPositionInterpolator::Update(float deltaTime, Vector2& position)
    {
        if (snapshots.empty())
            return false;

        if (!started)
        {
            position = snapshots.front().position;
            return true;
        }

        renderTick = std::min(renderTick + static_cast<double>(std::max(deltaTime, 0.0f)) * ServerTickRate, static_cast<double>(snapshots.back().serverTick));

        while (snapshots.size() >= 2 && static_cast<double>(snapshots[1].serverTick) <= renderTick)
            snapshots.pop_front();

        if (snapshots.size() == 1 || renderTick <= snapshots.front().serverTick)
        {
            position = snapshots.front().position;
            return true;
        }

        const PositionSnapshot& from = snapshots[0];
        const PositionSnapshot& to = snapshots[1];
        const double tickRange = static_cast<double>(to.serverTick - from.serverTick);
        const float amount = static_cast<float>((renderTick - from.serverTick) / tickRange);
        const float clampedAmount = std::clamp(amount, 0.0f, 1.0f);
        position.x = from.position.x + (to.position.x - from.position.x) * clampedAmount;
        position.y = from.position.y + (to.position.y - from.position.y) * clampedAmount;
        return true;
    }
}
