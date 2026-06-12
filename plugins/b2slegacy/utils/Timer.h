#pragma once

#include <SDL3/SDL.h>
#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

namespace B2SLegacy {

class Timer
{
public:
    static uint32_t TimerCallback(void* param, SDL_TimerID timerID, uint32_t interval);
    static void ServicePendingTimers();

    using ElapsedListener = std::function<void(Timer* pTimer)>;

    Timer();
    Timer(uint32_t interval, ElapsedListener listener, void* param = nullptr);
    virtual ~Timer();

    virtual void Start();
    virtual void Stop();
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled);
    uint32_t GetInterval() const { return m_interval; }
    void SetInterval(uint32_t interval);
    void SetElapsedListener(ElapsedListener listener);

private:
    SDL_TimerID m_timerID = 0;
    bool m_enabled = false;
    uint32_t m_interval = 0;
    ElapsedListener m_elapsedListener;
    std::atomic<bool> m_pending { false };

    static std::recursive_mutex s_timersMutex;
    static std::vector<Timer*> s_timers;
};

}
