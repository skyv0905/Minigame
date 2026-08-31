#pragma once

#include <cstdint>
#include <functional>
#include <vector>

using TimerId = std::uint64_t;

class TimerManager
{
public:
    TimerId SetTimeout(float seconds, std::function<void()> callback);
    TimerId SetInterval(float seconds, std::function<void()> callback);

    void Cancel(TimerId id);
    void Update(float deltaTime);

private:
    struct TimerEntry
    {
        TimerId id;
        float duration;
        float remaining;
        bool repeating;
        bool cancelled;
        std::function<void()> callback;
    };

    std::vector<TimerEntry> timers;
    std::vector<TimerEntry> pendingTimers;
    TimerId nextId = 1;
    bool updating = false;
};
