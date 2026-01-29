// license:GPLv3+

#include "core/stdafx.h"

#include "hittimer.h"


HitTimer::HitTimer(const string& name, int interval, IFireEvents* handler)
   : m_name(name)
   , m_pfe(handler)
{
   SetInterval(m_interval);
}

void HitTimer::SetInterval(int intervalMs)
{
   m_interval = intervalMs >= 0 ? max(intervalMs, (int)MAX_TIMER_MSEC_INTERVAL) : max(-2, intervalMs);
   m_nextfire = g_pplayer ? (g_pplayer->m_time_msec + m_interval) : m_interval;
}

void HitTimer::Defer()
{
   // Fakes the disabling of the timer, until it will be catched by the cleanup via m_changed_vht
   m_nextfire = g_pplayer->m_time_msec + 0xFFFFFFFF;
}

void HitTimer::Update(const unsigned int simulationTime)
{
   if (m_interval >= 0 && m_nextfire <= simulationTime)
   {
      const unsigned int curnextfire = m_nextfire;
      Fire();
      // Only add interval if the next fire time hasn't changed since the event was run. 
      // Handles corner case:
      //Timer1.Enabled = False
      //Timer1.Interval = 1000
      //Timer1.Enabled = True
      if (curnextfire == m_nextfire && m_interval > 0)
         while (m_nextfire <= simulationTime)
            m_nextfire += m_interval;
   }
}

void HitTimer::Fire()
{
   g_pplayer->m_logicProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, m_name);
   m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
   g_pplayer->m_logicProfiler.ExitScriptSection(m_name);
}
