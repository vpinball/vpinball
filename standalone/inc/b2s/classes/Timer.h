#pragma once

namespace B2S {

class Timer
{
public:
    typedef void (*Callback)(void*);

    static Uint32 TimerCallback(Uint32 interval, void* param);

public:
    Timer(Uint32 interval, Callback callback, void* param = nullptr);
    ~Timer();
    
    void Start();
    void Stop();
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled);
    Uint32 GetInterval() const { return m_interval; }
    void SetInterval(Uint32 interval);

private:
    SDL_TimerID m_timerID;
    bool m_enabled;
    Uint32 m_interval;
    Callback m_callback;
    void* m_param;
};

}