#pragma once

#ifdef __STANDALONE__
#include <climits>
#endif

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
      // Sections of a frame. Sum of the following sections should give the same as PROFILE_FRAME
      PROFILE_MISC,        // Everything not covered below
      PROFILE_SCRIPT,      // Time spent in script (all events)
      PROFILE_PHYSICS,     // Time spent in the physics simulation
      PROFILE_GPU_COLLECT, // Time spent to build the render frame
      PROFILE_GPU_SUBMIT,  // Time spent to submit the render frame to the GPU
      PROFILE_GPU_FLIP,    // Time spent flipping the swap chain (flush the GPU render queue)
      PROFILE_SLEEP,       // Time spent sleeping per frame (for synchronization)
      PROFILE_CUSTOM1,     // Use in conjunction with PROFILE_FUNCTION to perform custom profiling of sub sections of frames
      PROFILE_CUSTOM2,     // Use in conjunction with PROFILE_FUNCTION to perform custom profiling of sub sections of frames
      PROFILE_CUSTOM3,     // Use in conjunction with PROFILE_FUNCTION to perform custom profiling of sub sections of frames
      // Dedicated counters
      PROFILE_FRAME,             // Overall frame length
      PROFILE_INPUT_POLL_PERIOD, // Time spent between 2 input processings.
      PROFILE_INPUT_TO_PRESENT,  // Time spent between the last input taen in account in a frame to the presentation of this frame
                                 // The overall game lag is the sum of this lag with the present to display lag obtained using PresentMon tool
      PROFILE_COUNT
   };

   void Reset()
   { 
      m_logWorstFrame = false;
      m_frameIndex = -1;
      m_processInputCount = 0;
      m_processInputTimeStamp = 0;
      m_prepareCount = 0;
      m_prepareTimeStamp = 0;
      for (int i = 0; i < N_SAMPLES; i++)
         memset(m_profileData[i], 0, sizeof(m_profileData[0]));
      for (int i = 0; i < PROFILE_COUNT; i++)
      {
         m_profileMinData[i] = ~0u;
         m_profileMaxData[i] = 0;
         m_profileTotalData[i] = 0;
      }
      m_scriptEventData.clear();
      // Clear worst frames
      m_leastWorstFrameLength = 0;
      for (int i = 0; i < N_WORST; i++)
      {
         memset(m_profileWorstData[i], 0, sizeof(m_profileWorstData[0]));
         m_worstScriptEventData[i].clear();
      }
   }

   void EnableWorstFrameLogging(bool enable) { m_logWorstFrame = enable; }

   void LogWorstFrame()
   {
      if (!m_logWorstFrame)
         return;
      m_logWorstFrame = false;
      const string labels[] = { 
         "Misc:          ", 
         "Script:        ", 
         "Physics:       ", 
         "Prepare Frame: ", 
         "Submit Frame:  ", 
         "GPU Flip:      ", 
         "Sleep:         ", 
         "Custom 1:      ", 
         "Custom 2:      ", 
         "Custom 3:      " };
      for (int i = 0; i < N_WORST; i++)
      {
         if (m_profileWorstData[i][PROFILE_FRAME] == 0)
            break;
         PLOGI << "Long Frame of " << std::setw(5) << std::fixed << std::setprecision(1) << (m_profileWorstData[i][PROFILE_FRAME] * 1e-3) << "ms happened after " 
               << std::setw(5) << std::fixed << (m_profileWorstGameTime[i] * 1e-3) << "s:";
         for (int j = PROFILE_MISC; j <= PROFILE_CUSTOM3; j++)
            if (m_profileWorstData[i][j] > 100) // only log impacting timings (above 0.1 ms)
            {
               std::stringstream ss;
               ss << "  . " << labels[j] << std::setw(6) << std::fixed << std::setprecision(1) << (m_profileWorstData[i][j] * 1e-3) << "ms";
               if (j == PROFILE_SCRIPT)
               {
                  for (auto v : m_worstScriptEventData[i])
                  {
                     if (m_worstScriptEventData[i].size() > 1)
                     {
                        PLOGI << ss.str();
                        ss.str("");
                        ss.clear();
                        ss << "    . " << std::setw(4) << std::fixed << std::setprecision(1) << (v.second.totalLength * 1e-3) << "ms";
                     }
                     string name;
                     switch (v.first)
                     {
                     case 1000: name = "GameEvents:KeyDown"; break;
                     case 1001: name = "GameEvents:KeyUp"; break;
                     case 1002: name = "GameEvents:Init"; break;
                     case 1003: name = "GameEvents:MusicDone"; break;
                     case 1004: name = "GameEvents:Exit"; break;
                     case 1005: name = "GameEvents:Paused"; break;
                     case 1006: name = "GameEvents:UnPaused"; break;
                     case 1007: name = "GameEvents:OptionEvent"; break;
                     case 1101: name = "SurfaceEvents:Slingshot"; break;
                     case 1200: name = "FlipperEvents:Collide"; break;
                     case 1300: name = "TimerEvents:Timer"; break;
                     case 1301: name = "SpinnerEvents:Spin"; break;
                     case 1302: name = "TargetEvents:Dropped"; break;
                     case 1303: name = "TargetEvents:Raised"; break;
                     case 1320: name = "LightSeqEvents:PlayDone"; break;
                     case 1400: name = "HitEvents:Hit"; break;
                     case 1401: name = "HitEvents:Unhit"; break;
                     case 1402: name = "LimitEvents:EOS"; break;
                     case 1403: name = "LimitEvents:BOS"; break;
                     case 1404: name = "AnimateEvents:Animate"; break;
                     default: name = "DispID["s + std::to_string(v.first) + "]";
                     }
                     ss << " spent in " << std::setw(3) << v.second.callCount << " calls of " << name;
                     if (v.first == 1300)
                     {
                        struct info { int calls; U32 lengths; };
                        robin_hood::unordered_map<string, info> infos;
                        size_t pos = 0;
                        while (pos < m_profileWorstProfileTimersLen[i])
                        {
                           string name(&m_profileWorstProfileTimers[i][pos]);
                           pos += name.length() + 1;
                           U32 length = *((U32*)&m_profileWorstProfileTimers[i][pos]);
                           pos += 4;
                           auto it = infos.find(name);
                           if (it == infos.end())
                           {
                              infos[name] = { 1, length };
                           }
                           else
                           {
                              it->second.calls++;
                              it->second.lengths += length;
                           }
                        }
                        ss << " (";
                        for (auto pair : infos)
                           ss << pair.second.calls << " x " << pair.first << " => " << std::setw(4) << std::fixed << std::setprecision(1) << (pair.second.lengths * 1e-3) << "ms, ";
                        ss.seekp(-2, ss.cur);
                        ss << ") ";
                     }
                  }
               }
               PLOGI << ss.str();
            }
      }
   }

   void NewFrame(U32 gametime)
   {
      assert(m_profileSectionStackPos == 0);
      m_frameIndex++;
      m_profileTimeStamp = usec();
      if (m_frameIndex > 0)
      {
         unsigned int frameLength = (unsigned int)(m_profileTimeStamp - m_frameTimeStamp);
         m_profileData[m_profileIndex][PROFILE_FRAME] = frameLength;
         // Keep worst frames for easier inspection of stutter causes
         if (frameLength >= m_leastWorstFrameLength)
         {
            unsigned int least_worst = INT_MAX;
            for (int i = 0; i < N_WORST; i++)
            {
               if (m_profileWorstData[i][PROFILE_FRAME] < least_worst)
               {
                  least_worst = m_profileWorstData[i][PROFILE_FRAME];
                  if (least_worst <= m_leastWorstFrameLength)
                  {
                     m_leastWorstFrameLength = 0;
                     m_profileWorstGameTime[i] = gametime;
                     memcpy(m_profileWorstData[i], m_profileData[m_profileIndex], sizeof(m_profileWorstData[0]));
                     m_worstScriptEventData[i] = m_scriptEventData;
                     memcpy(m_profileWorstProfileTimers[i], m_profileTimers, m_profileTimersPos);
                     m_profileWorstProfileTimersLen[i] = m_profileTimersPos;
                  }
               }
            }
            m_leastWorstFrameLength = least_worst;
         }
         for (int i = 0; i < PROFILE_COUNT; i++)
         {
            unsigned int data = m_profileData[m_profileIndex][i];
            m_profileMinData[i] = min(m_profileMinData[i], data);
            m_profileMaxData[i] = max(m_profileMaxData[i], data);
            m_profileTotalData[i] += data;
         }
      }
      m_profileIndex = (m_profileIndex + 1) % N_SAMPLES;
      memset(m_profileData[m_profileIndex], 0, sizeof(m_profileData[0]));
      m_scriptEventData.clear();
      m_profileTimersPos = 0;
      m_profileTimers[m_profileTimersPos] = 0;
      m_profileSection = FrameProfiler::PROFILE_MISC;
      m_frameTimeStamp = m_profileTimeStamp;
   }

   void SetProfileSection(ProfileSection section)
   {
      assert(0 <= section && section < PROFILE_COUNT);
      const unsigned long long ts = usec();
      m_profileData[m_profileIndex][m_profileSection] += (unsigned int) (ts - m_profileTimeStamp);
      m_profileTimeStamp = ts;
      m_profileSection = section;
   }

   void EnterProfileSection(ProfileSection section)
   {
      assert(0 <= section && section < PROFILE_COUNT);
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

   void EnterScriptSection(DISPID id, const char* timer_name = nullptr)
   {
      EnterProfileSection(PROFILE_SCRIPT);
      m_scriptEventDispID = id;
      // For the time being, just store a list of the timer called during the script profile section
      if (timer_name)
      {
         m_profileTimerTimeStamp = m_profileTimeStamp;
         size_t len = strlen(timer_name) + 1;
         if (m_profileTimersPos + len < MAX_TIMER_LOG - 8)
         {
            strcpy_s(&m_profileTimers[m_profileTimersPos], len, timer_name);
            m_profileTimersPos += len;
         }
         else if (m_profileTimersPos + 8 < MAX_TIMER_LOG)
         {
            strcpy_s(&m_profileTimers[m_profileTimersPos], 4, "...");
            m_profileTimersPos += 4;
         }
      }
   }

   void ExitScriptSection(const char* timer_name = nullptr)
   {
      unsigned long long profileTimeStamp = m_profileTimeStamp;
      ExitProfileSection();
      EventTick& et = m_scriptEventData[m_scriptEventDispID];
      et.totalLength += (unsigned int)(m_profileTimeStamp - profileTimeStamp);
      if (m_profileSection != PROFILE_SCRIPT)
         et.callCount++;
      if (timer_name && (m_profileTimersPos + 4 < MAX_TIMER_LOG))
      {
         *((U32*)(&m_profileTimers[m_profileTimersPos])) = (U32)(m_profileTimeStamp - m_profileTimerTimeStamp);
         m_profileTimersPos += 4;
      }
   }

   unsigned int Get(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      return section == PROFILE_INPUT_POLL_PERIOD ? m_profileData[m_processInputIndex][PROFILE_INPUT_POLL_PERIOD]
           : section == PROFILE_INPUT_TO_PRESENT  ? m_profileData[m_prepareIndex][PROFILE_INPUT_TO_PRESENT]
                                                  : m_profileData[m_profileIndex][section];
   }
   
   unsigned int GetPrev(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      return section == PROFILE_INPUT_POLL_PERIOD ? m_profileData[(m_processInputIndex + N_SAMPLES - 1) % N_SAMPLES][PROFILE_INPUT_POLL_PERIOD]
           : section == PROFILE_INPUT_TO_PRESENT  ? m_profileData[(m_prepareIndex + N_SAMPLES - 1) % N_SAMPLES][PROFILE_INPUT_TO_PRESENT]
                                                  : m_profileData[(m_profileIndex + N_SAMPLES - 1) % N_SAMPLES][section];
   }
   
   unsigned int GetMin(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      return m_profileMinData[section];
   }
   
   unsigned int GetMax(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      return m_profileMaxData[section];
   }
   
   double GetAvg(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      return section == PROFILE_INPUT_POLL_PERIOD ? (m_processInputCount <= 0 ? 0. : ((double)m_profileTotalData[PROFILE_INPUT_POLL_PERIOD] / (double)m_processInputCount))
           : section == PROFILE_INPUT_TO_PRESENT  ? (m_prepareCount <= 0      ? 0. : ((double)m_profileTotalData[PROFILE_INPUT_TO_PRESENT] / (double)m_prepareCount))
                                                  : (m_frameIndex <= 0        ? 0. : ((double)m_profileTotalData[section] / (double)m_frameIndex));
   }
   
   double GetRatio(ProfileSection section) const
   {
      assert(0 <= section && section <= PROFILE_FRAME); // Unimplemented and not meaningful for other sections 
      return m_profileTotalData[ProfileSection::PROFILE_FRAME] == 0 ? 0. : ((double)m_profileTotalData[section] / (double)m_profileTotalData[ProfileSection::PROFILE_FRAME]);
   }
   
   // (approximately) 1 second sliding average of frame sections
   double GetSlidingAvg(ProfileSection section) const
   {
      assert(0 <= section && section <= PROFILE_FRAME); // Unimplemented and not really meaningful for other sections
      unsigned int pos = (m_profileIndex + N_SAMPLES - 1) % N_SAMPLES; // Start from last frame
      unsigned int elapsed = 0u;
      unsigned int sum = 0u;
      unsigned int count = 0u;
      for (unsigned int i = 0u; i < N_SAMPLES; i++)
      {
         if (count >= m_frameIndex)
            break;
         count++;
         sum += m_profileData[pos][section];
         elapsed += m_profileData[pos][ProfileSection::PROFILE_FRAME];
         if (elapsed >= 1000000u) // end of 1s sliding average
            break;
         pos = (pos + N_SAMPLES - 1) % N_SAMPLES;
      }
      return count == 0 ? 0. : (double)sum / (double)count;
   }

   double GetSlidingInputLag(const bool isMax) const
   {
      unsigned int pos = (m_processInputIndex + N_SAMPLES - 1) % N_SAMPLES; // Start from last frame
      unsigned int latency = 0u;
      unsigned int sum = 0u;
      unsigned int count = 0u;
      for (unsigned int i = 0u; i < N_SAMPLES; i++)
      {
         count++;
         pos = (pos + N_SAMPLES - 1) % N_SAMPLES;
         unsigned int period = m_profileData[pos][PROFILE_INPUT_POLL_PERIOD];
         sum += period;
         if (isMax)
            latency = max(latency, period);
         else
            latency += period * period / 2;
         if (sum >= 1000000u) // end of 1s sliding average
            break;
      }
      if (isMax)
         return latency;
      else
         return sum <= 0 ? 0. : (double) latency / (double) sum;
   }
   
   void OnProcessInput()
   {
      unsigned long long ts = usec();
      if (m_processInputTimeStamp != 0)
      {
         unsigned int elapsed = (unsigned int)(ts - m_processInputTimeStamp);
         m_profileData[m_processInputIndex][PROFILE_INPUT_POLL_PERIOD] = elapsed;
         m_profileMinData[PROFILE_INPUT_POLL_PERIOD] = min(m_profileMinData[PROFILE_INPUT_POLL_PERIOD], elapsed);
         m_profileMaxData[PROFILE_INPUT_POLL_PERIOD] = max(m_profileMaxData[PROFILE_INPUT_POLL_PERIOD], elapsed);
         m_profileTotalData[PROFILE_INPUT_POLL_PERIOD] += elapsed;
         m_processInputIndex = (m_processInputIndex + 1) % N_SAMPLES;
         m_processInputCount++;
      }
      m_processInputTimeStamp = ts;
   }
   
   void OnPrepare()
   {
      m_prepareTimeStamp = m_processInputTimeStamp;
   }
   
   void OnPresent()
   {
      if (m_prepareTimeStamp == 0)
         return;
      unsigned int elapsed = (unsigned int) (usec() - m_prepareTimeStamp);
      m_profileData[m_prepareIndex][PROFILE_INPUT_TO_PRESENT] = elapsed;
      m_profileMinData[PROFILE_INPUT_TO_PRESENT] = min(m_profileMinData[PROFILE_INPUT_TO_PRESENT], elapsed);
      m_profileMaxData[PROFILE_INPUT_TO_PRESENT] = max(m_profileMaxData[PROFILE_INPUT_TO_PRESENT], elapsed);
      m_profileTotalData[PROFILE_INPUT_TO_PRESENT] += elapsed;
      m_prepareIndex = (m_prepareIndex + 1) % N_SAMPLES;
      m_prepareCount++;
   }

private:
   constexpr static unsigned int N_SAMPLES = 1000; // Number of samples to store. Must be kept quite high to be able to do a 1s sliding average (so at 1000FPS, needs 100 samples)
   constexpr static unsigned int N_WORST = 10; // Number of longest frames to keep detailed profile timing
   constexpr static unsigned int STACK_SIZE = 100;
   constexpr static unsigned int MAX_TIMER_LOG = 1024;

   bool m_logWorstFrame = false;

   // Frame profiling (sequence of section)
   unsigned int m_profileIndex = 0;
   unsigned long long m_profileTimeStamp;
   int m_profileSectionStackPos = 0;
   ProfileSection m_profileSectionStack[STACK_SIZE];
   ProfileSection m_profileSection = PROFILE_MISC;
   struct EventTick
   {
      unsigned int callCount = 0;
      unsigned int totalLength = 0;
   };
   DISPID m_scriptEventDispID = 0;
   robin_hood::unordered_map<DISPID, EventTick> m_scriptEventData;

   // Overall frame
   unsigned int m_frameIndex = -1;
   unsigned long long m_frameTimeStamp;

   // Input lag (polling frequency)
   unsigned int m_processInputIndex = 0;
   unsigned int m_processInputCount = 0;
   unsigned long long m_processInputTimeStamp;
   
   // Present lag
   unsigned int m_prepareIndex = 0;
   unsigned int m_prepareCount = 0;
   unsigned long long m_prepareTimeStamp;

   // Raw data
   unsigned int m_profileData[N_SAMPLES][PROFILE_COUNT];
   unsigned int m_profileMaxData[PROFILE_COUNT];
   unsigned int m_profileMinData[PROFILE_COUNT];
   unsigned int m_profileTotalData[PROFILE_COUNT];
   char m_profileTimers[MAX_TIMER_LOG];
   size_t m_profileTimersPos = 0;
   unsigned long long m_profileTimerTimeStamp;

   // Worst frames data
   unsigned int m_leastWorstFrameLength;
   unsigned int m_profileWorstData[N_WORST][PROFILE_COUNT];
   unsigned int m_profileWorstGameTime[N_WORST];
   char m_profileWorstProfileTimers[N_WORST][MAX_TIMER_LOG];
   size_t m_profileWorstProfileTimersLen[N_WORST];
   robin_hood::unordered_map<DISPID, EventTick> m_worstScriptEventData[N_WORST];
};

extern FrameProfiler g_frameProfiler;

#define PROFILE_FUNCTION(section) ScopedProfiler funcProfiler(section)
class ScopedProfiler
{
public:
   ScopedProfiler(const FrameProfiler::ProfileSection section) { g_frameProfiler.EnterProfileSection(section); }
   ~ScopedProfiler() { g_frameProfiler.ExitProfileSection(); }
};
