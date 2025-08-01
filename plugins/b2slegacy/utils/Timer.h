#pragma once

#include <SDL3/SDL.h>
#include <functional>

namespace B2SLegacy {

class Timer
{
public:
    static uint32_t TimerCallback(void* param, SDL_TimerID timerID, uint32_t interval);

    using ElapsedListener = std::function<void(Timer* pTimer)>;

    Timer();
    Timer(uint32_t interval, ElapsedListener listener, void* param = nullptr);
    ~Timer();

    virtual void Start();
    virtual void Stop();
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled);
    uint32_t GetInterval() const { return m_interval; }
    void SetInterval(uint32_t interval);
    void SetElapsedListener(ElapsedListener listener);

private:
    SDL_TimerID m_timerID;
    bool m_enabled;
    uint32_t m_interval;
    ElapsedListener m_elapsedListener;
};

}
