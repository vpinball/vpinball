#include "common.h"
#include "Timer.h"

namespace B2SLegacy {

uint32_t Timer::TimerCallback(void* param, SDL_TimerID timerID, uint32_t interval)
{
   Timer* pTimer = static_cast<Timer*>(param);
   if (pTimer->m_elapsedListener)
      pTimer->m_elapsedListener(pTimer);
   return interval;
}

Timer::Timer()
{
   m_timerID = 0;
   m_enabled = false;

   m_interval = 0;
   m_elapsedListener = nullptr;
}

Timer::Timer(uint32_t interval, ElapsedListener elapsedListener, void* param)
{
   m_timerID = 0;
   m_enabled = false;

   m_interval = interval;
   m_elapsedListener = elapsedListener;
}

Timer::~Timer()
{
   Stop();
}

void Timer::Start()
{
   if (!m_enabled) {
      m_timerID = SDL_AddTimer(m_interval, TimerCallback, this);
      m_enabled = true;
   }
}

void Timer::Stop()
{
   if(m_enabled) {
      SDL_RemoveTimer(m_timerID);
      m_timerID = 0;
      m_enabled = false;
   }
}

void Timer::SetEnabled(bool enabled)
{
   if (enabled != m_enabled) {
      if (enabled)
         Start();
      else
         Stop();
   }
}

void Timer::SetInterval(uint32_t interval)
{
   if (interval != m_interval) {
      m_interval = interval;
      if(m_enabled) {
         Stop();
         Start();
      }
   }
}

void Timer::SetElapsedListener(ElapsedListener elapsedListener)
{
   m_elapsedListener = elapsedListener;
}

}
