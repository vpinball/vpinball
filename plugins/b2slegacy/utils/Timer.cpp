#include "common.h"
#include "Timer.h"

namespace B2SLegacy {

std::recursive_mutex Timer::s_timersMutex;
std::vector<Timer*> Timer::s_timers;

uint32_t Timer::TimerCallback(void* param, SDL_TimerID timerID, uint32_t interval)
{
   Timer* pTimer = static_cast<Timer*>(param);
   pTimer->m_pending = true;
   return interval;
}

void Timer::ServicePendingTimers()
{
   std::lock_guard<std::recursive_mutex> lock(s_timersMutex);
   for (size_t i = 0; i < s_timers.size(); i++) {
      Timer* pTimer = s_timers[i];
      if (pTimer->m_enabled && pTimer->m_pending.exchange(false) && pTimer->m_elapsedListener)
         pTimer->m_elapsedListener(pTimer);
   }
}

Timer::Timer()
{
   std::lock_guard<std::recursive_mutex> lock(s_timersMutex);
   s_timers.push_back(this);
}

Timer::Timer(uint32_t interval, ElapsedListener elapsedListener, void* param)
{
   m_interval = interval;
   m_elapsedListener = elapsedListener;

   std::lock_guard<std::recursive_mutex> lock(s_timersMutex);
   s_timers.push_back(this);
}

Timer::~Timer()
{
   Stop();

   std::lock_guard<std::recursive_mutex> lock(s_timersMutex);
   s_timers.erase(std::remove(s_timers.begin(), s_timers.end(), this), s_timers.end());
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
      m_pending = false;
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
