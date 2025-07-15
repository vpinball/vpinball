// license:GPLv3+

#pragma once

class HitTimer final
{
public:
   HitTimer(const string& name, int interval, IFireEvents* handler)
      : m_name(name)
      , m_pfe(handler)
      , m_interval(interval >= 0 ? max(interval, MAX_TIMER_MSEC_INTERVAL) : max(-2, interval))
   {
      m_nextfire = m_interval;
   }
   ~HitTimer() { }

   string m_name;
   IFireEvents * const m_pfe;

   int m_interval;
   unsigned int m_nextfire = 0;
};
