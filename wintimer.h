#pragma once

// call if msec,usec or uSleep, etc. should be more precise
void set_lowest_possible_win_timer_resolution();
// and then call this after being finished with precise measurements to allow the OS to better save power
void restore_win_timer_resolution();

// call before 1st use of msec,usec or uSleep
void wintimer_init();

U32 msec();
unsigned long long usec();

// needs timeBeginPeriod(1) before calling 1st time to make the Sleep(1) in here behave more or less accurately (and timeEndPeriod(1) after not needing that precision anymore)
void uSleep(const unsigned long long u);
void uOverSleep(const unsigned long long u);

double TheoreticRadiation(const unsigned int day, const unsigned int month, const unsigned int year, const double rlat);
double MaxTheoreticRadiation(const unsigned int year, const double rlat);
double SunsetSunriseLocalTime(const unsigned int day, const unsigned int month, const unsigned int year, const double rlong, const double rlat, const bool sunrise);


class FrameProfiler
{
public:
   enum ProfileSection
   {
      PROFILE_FRAME, // Overall frame length
      PROFILE_MISC, // Everything not covered below
      PROFILE_SCRIPT, // Time spent in script (all events)
      PROFILE_PHYSICS, // Time spent in the physics simulation
      PROFILE_GPU_COLLECT, // Time spent to build the render frame
      PROFILE_GPU_SUBMIT, // Time spent to submit the render frame to the GPU
      PROFILE_GPU_FLIP, // Time spent flipping the swap chain (flush the GPU render queue)
      PROFILE_SLEEP, // Time spent sleeping per frame (for synchronization)
      PROFILE_CUSTOM1, // Use in conjunction with PROFILE_FUNCTION to perform custom profiling
      PROFILE_CUSTOM2, // Use in conjunction with PROFILE_FUNCTION to perform custom profiling
      PROFILE_CUSTOM3, // Use in conjunction with PROFILE_FUNCTION to perform custom profiling
      PROFILE_COUNT
   };

   void Reset()
   { 
      m_frameIndex = -1;
      for (int i = 0; i < PROFILE_COUNT; i++)
      {
         m_profileMinData[i] = LONG_MAX;
         m_profileMaxData[i] = 0;
         m_profileTotalData[i] = 0;
      }
   }

   void NewFrame()
   {
      assert(m_profileSectionStackPos == 0);
      m_frameIndex++;
      m_profileTimeStamp = usec();
      if (m_frameIndex > 0)
      {
         unsigned long frameLength = (unsigned long)(m_profileTimeStamp - m_frameTimeStamp);
         m_profileData[m_profileIndex][PROFILE_FRAME] = frameLength;
         for (int i = 0; i < PROFILE_COUNT; i++)
         {
            unsigned long data = m_profileData[m_profileIndex][i];
            m_profileMinData[i] = min(m_profileMinData[i], data);
            m_profileMaxData[i] = max(m_profileMaxData[i], data);
            m_profileTotalData[i] += data;
         }
      }
      m_profileIndex = (m_profileIndex + 1) % N_SAMPLES;
      memset(m_profileData[m_profileIndex], 0, sizeof(m_profileData[0]));
      m_profileSection = FrameProfiler::PROFILE_MISC;
      m_frameTimeStamp = m_profileTimeStamp;
   }

   void SetProfileSection(ProfileSection section)
   {
      unsigned long long ts = usec();
      m_profileData[m_profileIndex][m_profileSection] += (unsigned long) (ts - m_profileTimeStamp);
      m_profileTimeStamp = ts;
      m_profileSection = section;
   }

   void EnterProfileSection(ProfileSection section)
   {
      assert(m_profileSectionStackPos < STACK_SIZE);
      m_profileSectionStack[m_profileSectionStackPos] = m_profileSection;
      m_profileSectionStackPos++;
      SetProfileSection(section);
   }

   void ExitProfileSection()
   {
      assert(m_profileSectionStackPos >= 0);
      m_profileSectionStackPos--;
      SetProfileSection(m_profileSectionStack[m_profileSectionStackPos]);
   }

   unsigned long Get(ProfileSection section) const { return m_profileData[m_profileIndex][section]; }
   unsigned long GetPrev(ProfileSection section) const { return m_profileData[(m_profileIndex + N_SAMPLES - 1) % N_SAMPLES][section]; }
   unsigned long GetMin(ProfileSection section) const { return m_profileMinData[section]; }
   unsigned long GetMax(ProfileSection section) const { return m_profileMaxData[section]; }
   double GetAvg(ProfileSection section) const { return m_frameIndex <= 0 ? 0. : ((double)m_profileTotalData[section] / (double)m_frameIndex); }
   double GetRatio(ProfileSection section) const { return m_profileTotalData[ProfileSection::PROFILE_FRAME] == 0 ? 0. : ((double)m_profileTotalData[section] / (double)m_profileTotalData[ProfileSection::PROFILE_FRAME]); }

   // (approximately) 1 second sliding average
   double GetSlidingAvg(ProfileSection section) const
   {
      unsigned int pos = m_profileIndex;
      unsigned long elapsed = 0ul;
      unsigned long sum = 0ul;
      unsigned int count = 0u;
      for (unsigned int i = 0u; i < N_SAMPLES; i++)
      {
         count++;
         if (count > m_frameIndex)
            break;
         pos = (pos + N_SAMPLES - 1) % N_SAMPLES;
         sum += m_profileData[pos][section];
         elapsed += m_profileData[pos][ProfileSection::PROFILE_FRAME];
         if (elapsed >= 1000000ul) // end of 1s sliding average
            break;
      }
      return count == 0 ? 0. : (double)sum / (double)count;
   }

private:
   constexpr static unsigned int N_SAMPLES = 1000; // Number of samples to store. Must be kept quite high to be able to do a 1s sliding average (so at 1000FPS, needs 100 samples)
   constexpr static unsigned int STACK_SIZE = 100;

   unsigned long long m_profileTimeStamp;

   unsigned int m_profileIndex = 0;
   unsigned long m_profileData[N_SAMPLES][PROFILE_COUNT];

   unsigned int m_frameIndex = -1;
   unsigned long long m_frameTimeStamp;

   unsigned long m_profileMaxData[PROFILE_COUNT];
   unsigned long m_profileMinData[PROFILE_COUNT];
   unsigned long m_profileTotalData[PROFILE_COUNT];

   int m_profileSectionStackPos = 0;
   ProfileSection m_profileSectionStack[STACK_SIZE];
   ProfileSection m_profileSection = PROFILE_MISC;
};

extern FrameProfiler g_frameProfiler;

#define PROFILE_FUNCTION(section) ScopedProfiler funcProfiler(section)
class ScopedProfiler
{
public:
   ScopedProfiler(const FrameProfiler::ProfileSection section) { g_frameProfiler.EnterProfileSection(section); }
   ~ScopedProfiler() { g_frameProfiler.ExitProfileSection(); }
};
