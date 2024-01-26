#pragma once

namespace VP {

class Timer
{
public:
    static Uint32 TimerCallback(Uint32 interval, void* param);

    using ElapsedListener = std::function<void(Timer* pTimer)>;

    Timer();
    Timer(Uint32 interval, ElapsedListener listener, void* param = nullptr);
    ~Timer();
    
    virtual void Start();
    virtual void Stop();
    bool IsEnabled() const { return m_enabled; }
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
