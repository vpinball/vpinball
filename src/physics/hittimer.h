#pragma once

class HitTimer
{
public:
   HitTimer(const char* name, int interval, IFireEvents* handler)
      : m_pfe(handler)
      , m_interval(interval >= 0 ? max(interval, MAX_TIMER_MSEC_INTERVAL) : max(-2, interval))
   {
      size_t len = strlen(name);
      char* nameCopy = new char[len + 1];
      strcpy_s(nameCopy, len + 1, name); 
      m_name = nameCopy;
      m_nextfire = m_interval;
   }
   ~HitTimer()
   {
      delete[] m_name;
   }

   const char* m_name;
   IFireEvents *m_pfe;

   int m_interval;
   unsigned int m_nextfire = 0;
};
