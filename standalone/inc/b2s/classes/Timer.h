#pragma once

namespace B2S {

class Timer
{
public:
    static Uint32 TimerCallback(void* param, SDL_TimerID timerID, Uint32 interval);

    using ElapsedListener = std::function<void(Timer* pTimer)>;

    Timer();
    Timer(Uint32 interval, ElapsedListener listener, void* param = NULL);
    ~Timer();
    
    virtual void Start();
    virtual void Stop();
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled);
    Uint32 GetInterval() const { return m_interval; }
    void SetInterval(Uint32 interval);
    void SetElapsedListener(ElapsedListener listener);

private:
    SDL_TimerID m_timerID;
    bool m_enabled;
    Uint32 m_interval;
    ElapsedListener m_elapsedListener;
};

}