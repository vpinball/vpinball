// license:GPLv3+

#pragma once

class HitTimer final
{
public:
   HitTimer(const string& name, int interval, IFireEvents* handler);
   ~HitTimer() = default;

   int GetInterval() const { return m_interval; }
   void SetInterval(int intervalMs);
   void Defer();
   void Update(const unsigned int simulationTime);
   void OnNewFrame() { if (m_interval == -1) Fire(); }
   void OnGameSync() { if (m_interval == -2) Fire(); }

private:
   void Fire();

   const string m_name;
   IFireEvents * const m_pfe;
   int m_interval;
   unsigned int m_nextfire = 0;

   // amount of msecs to wait (at least) until same timer can be triggered again (e.g. they can fall behind, if set to > 1, as update cycle is 1000Hz)   
   const long MAX_TIMER_MSEC_INTERVAL = 1;
};
