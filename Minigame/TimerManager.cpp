#include "TimerManager.h"
#include <algorithm>
#include <utility>

TimerId TimerManager::SetTimeout(float seconds, std::function<void()> callback)
{
    const TimerId id = nextId++;
    auto& target = updating ? pendingTimers : timers;
    target.push_back({ id, seconds, seconds, false, false, std::move(callback) });

    return id;
}

TimerId TimerManager::SetInterval(float seconds, std::function<void()> callback)
{
    const TimerId id = nextId++;
    auto& target = updating ? pendingTimers : timers;
    target.push_back({ id, seconds, seconds, true, false, std::move(callback) });

    return id;
}

void TimerManager::Cancel(TimerId id)
{
    const auto cancelTimer = [id](std::vector<TimerEntry>& entries)
    {
        for (auto& timer : entries)
        {
            if (timer.id == id)
            {
                timer.cancelled = true;
                return true;
            }
        }

        return false;
    };

    if (!cancelTimer(timers))
    {
        cancelTimer(pendingTimers);
    }
}

void TimerManager::Update(float deltaTime)
{
    updating = true;
    const std::size_t count = timers.size();

    for (std::size_t i = 0; i < count; i++)
    {
        auto& timer = timers[i];
        if (timer.cancelled)
            continue;

        timer.remaining -= deltaTime;

        if (timer.remaining <= 0.0f)
        {
            timer.callback();

            if (timer.repeating && !timer.cancelled)
                timer.remaining += timer.duration;
            else
                timer.cancelled = true;
        }
    }

    std::erase_if(timers, [](const TimerEntry& timer)
        {
            return timer.cancelled;
        });

    updating = false;

    for (auto& timer : pendingTimers)
    {
        if (!timer.cancelled)
        {
            timers.push_back(std::move(timer));
        }
    }

    pendingTimers.clear();
}
