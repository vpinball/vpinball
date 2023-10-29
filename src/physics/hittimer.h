#pragma once

class HitTimer
{
public:
   IFireEvents *m_pfe;
   unsigned int m_nextfire;
   int m_interval;
};
