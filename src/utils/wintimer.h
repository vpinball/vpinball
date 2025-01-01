// license:GPLv3+

#pragma once

#ifdef __STANDALONE__
#include <climits>
#endif
#include <thread>

#include "robin_hood.h"

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
      // Logic thread
      PROFILE_MISC,          // Everything not covered below
      PROFILE_SCRIPT,        // Time spent in script (all events)
      PROFILE_PHYSICS,       // Time spent in the physics simulation
      PROFILE_SLEEP,         // Time spent sleeping per frame
      PROFILE_PREPARE_FRAME, // Time spent to build the render frame
      PROFILE_CUSTOM1,       // Use in conjunction with PROFILE_FUNCTION to perform custom profiling of sub sections of frames
      PROFILE_CUSTOM2,       // Use in conjunction with PROFILE_FUNCTION to perform custom profiling of sub sections of frames
      PROFILE_CUSTOM3,       // Use in conjunction with PROFILE_FUNCTION to perform custom profiling of sub sections of frames
      // Render thread
      PROFILE_RENDER_WAIT,   // Time spent waiting for a frame to be ready to be submitted (when CPU bounded)
      PROFILE_RENDER_SUBMIT, // Time spent to submit the render frame to the GPU
      PROFILE_RENDER_FLIP,   // Time spent flipping the swap chain (flush the GPU render queue)
      PROFILE_RENDER_SLEEP,  // Time spent sleeping per frame (for user setting FPS synchronization)
      // Dedicated counters
      PROFILE_FRAME,             // Overall frame length
      PROFILE_INPUT_POLL_PERIOD, // Time spent between 2 input processings (not tied to frame timings)
      PROFILE_INPUT_TO_PRESENT,  // Time spent between the last input polling in a frame to the presentation of this frame (note that present correspond to queueing a present operation, and not actually presenting the frame)
      // Latency considerations:
      // - The overall input-physic lag is the time between physical device to OS input (including USB latency), then input polling, then controller syncing, and finally physics
      // - The overall physic-visual lag is the time between last physic simulation to the presentation of the corresponding frame (including display lag obtained using PresentMon tool f.e.)
      PROFILE_COUNT
   };

   void Reset()
   { 
      m_logWorstFrame = false;
      m_frameIndex = -1;
      m_processInputCount = 0;
      m_processInputTimeStamp = 0;
      m_presentedCount = 0;
      m_processInputTimeStampOnPrepare = 0;
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

   void EnableWorstFrameLogging(const bool enable) { m_logWorstFrame = enable; }

   void LogWorstFrame()
   {
      if (!m_logWorstFrame)
         return;
      m_logWorstFrame = false;
      static const string labels[] = { 
         "Misc:          "s, 
         "Script:        "s, 
         "Physics:       "s, 
         "Sleep:         "s,
         "Prepare Frame: "s, 
         "Custom 1:      "s,
         "Custom 2:      "s,
         "Custom 3:      "s,
         // Render thread
         "Render Wait:   "s, 
         "Render Submit: "s,
         "Render Flip:   "s,
         "Render Sleep:  "s,
      };
      for (int i = 0; i < N_WORST; i++)
      {
         if (m_profileWorstData[i][PROFILE_FRAME] == 0)
            break;
         PLOGI << "Long Frame of " << std::setw(5) << std::fixed << std::setprecision(1) << (m_profileWorstData[i][PROFILE_FRAME] * 1e-3) << "ms happened after " 
               << std::setw(5) << std::fixed << (m_profileWorstGameTime[i] * 1e-3) << "s:";
         for (int j = PROFILE_MISC; j <= PROFILE_CUSTOM3; j++)
            if (m_profileWorstData[i][j] > 100) // only log impacting timings (above 0.1 ms)
            {
               PLOGI << "  . " << labels[j] << std::setw(6) << std::fixed << std::setprecision(1) << (m_profileWorstData[i][j] * 1e-3) << "ms";
               if (j == PROFILE_SCRIPT)
               {
                  std::istringstream iss(EventDataToLog(m_worstScriptEventData[i], m_profileWorstProfileTimers[i], m_profileWorstProfileTimersLen[i]));
                  std::string item;
                  while (std::getline(iss, item, '\n'))
                     if (!item.empty())
                     {
                        PLOGI << item;
                     }
               }
            }
      }
   }

   void NewFrame(U32 gametime)
   {
      // assert(m_threadLock == std::this_thread::get_id()); // Not asserted as NewFrame happens in a critical section (guarded by frameMutex)
      assert(m_profileSectionStackPos == 0);
      m_frameIndex++;
      if ((m_processInputTimeStampOnPrepare != 0) && (m_lastPresentedTimeStamp > m_processInputTimeStampOnPrepare))
      {
         // Processed asynchronously here since input events are from game logic thread while present events are from rendering thread
         unsigned int elapsed = (unsigned int) (m_lastPresentedTimeStamp - m_processInputTimeStampOnPrepare);
         m_profileData[m_presentedIndex][PROFILE_INPUT_TO_PRESENT] = elapsed;
         m_profileMinData[PROFILE_INPUT_TO_PRESENT] = min(m_profileMinData[PROFILE_INPUT_TO_PRESENT], elapsed);
         m_profileMaxData[PROFILE_INPUT_TO_PRESENT] = max(m_profileMaxData[PROFILE_INPUT_TO_PRESENT], elapsed);
         m_profileTotalData[PROFILE_INPUT_TO_PRESENT] += elapsed;
         m_presentedIndex = (m_presentedIndex + 1) % N_SAMPLES;
         m_presentedCount++;
      }
      m_processInputTimeStampOnPrepare = m_processInputTimeStamp;
      m_profileTimeStamp = usec();
      if (m_frameIndex > 0)
      {
         unsigned int frameLength = (unsigned int)(m_profileTimeStamp - m_frameTimeStamp);
         m_profileData[m_profileIndex][PROFILE_FRAME] = frameLength;
         // Keep worst frames for easier inspection of stutter causes
         if ((m_frameIndex > 100) && (frameLength >= m_leastWorstFrameLength))
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
                     m_worstScriptEventData[i].clear();
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
      memset(m_profileDataStart[m_profileIndex], 0, sizeof(m_profileDataStart[0]));
      m_scriptEventData.clear();
      m_profileTimersPos = 0;
      m_profileTimers[m_profileTimersPos] = 0;
      m_profileSection = FrameProfiler::PROFILE_MISC;
      m_frameTimeStamp = m_profileTimeStamp;
   }

   void SetProfileSection(ProfileSection section)
   {
      assert(m_threadLock == std::this_thread::get_id());
      assert(0 <= section && section < PROFILE_COUNT);
      const unsigned long long ts = usec();
      m_profileDataEnd[m_profileIndex][m_profileSection] = ts;
      m_profileData[m_profileIndex][m_profileSection] += (unsigned int)(ts - m_profileTimeStamp);
      m_profileTimeStamp = ts;
      m_profileSection = section;
      if (m_profileDataStart[m_profileIndex][m_profileSection] == 0)
         m_profileDataStart[m_profileIndex][m_profileSection] = ts;
   }

   void AdjustBGFXSubmit(U32 us)
   {
      assert(m_threadLock == std::this_thread::get_id());
      //m_profileDataEnd[m_profileIndex][PROFILE_RENDER_SUBMIT] += us;
      m_profileData[m_profileIndex][PROFILE_RENDER_SUBMIT] += us;
      /* if (m_profileDataStart[m_profileIndex][PROFILE_RENDER_SLEEP] != 0)
      {
         m_profileDataStart[m_profileIndex][PROFILE_RENDER_SLEEP] += us;
         m_profileDataEnd[m_profileIndex][PROFILE_RENDER_SLEEP] += us;
      }*/
      //m_profileDataStart[m_profileIndex][PROFILE_RENDER_FLIP] += us;
      m_profileData[m_profileIndex][PROFILE_RENDER_FLIP] -= us;
   }

   void EnterProfileSection(ProfileSection section)
   {
      assert(m_threadLock == std::this_thread::get_id());
      assert(0 <= section && section < PROFILE_COUNT);
      assert(m_profileSectionStackPos < STACK_SIZE);
      m_profileSectionStack[m_profileSectionStackPos] = m_profileSection;
      m_profileSectionStackPos++;
      SetProfileSection(section);
   }

   void ExitProfileSection()
   {
      assert(m_threadLock == std::this_thread::get_id());
      assert(m_profileSectionStackPos >= 0);
      m_profileSectionStackPos--;
      SetProfileSection(m_profileSectionStack[m_profileSectionStackPos]);
   }

   void EnterScriptSection(DISPID id, const char* timer_name = nullptr)
   {
      assert(m_threadLock == std::this_thread::get_id());
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
      assert(m_threadLock == std::this_thread::get_id());
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
           : section == PROFILE_INPUT_TO_PRESENT  ? m_profileData[m_presentedIndex][PROFILE_INPUT_TO_PRESENT]
                                                  : m_profileData[m_profileIndex][section];
   }

   unsigned int GetPrev(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      return section == PROFILE_INPUT_POLL_PERIOD ? m_profileData[(m_processInputIndex + N_SAMPLES - 1) % N_SAMPLES][PROFILE_INPUT_POLL_PERIOD]
           : section == PROFILE_INPUT_TO_PRESENT  ? m_profileData[(m_presentedIndex + N_SAMPLES - 1) % N_SAMPLES][PROFILE_INPUT_TO_PRESENT]
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
      return section == PROFILE_INPUT_POLL_PERIOD ? (m_processInputCount <= 0 ? 0. : (static_cast<double>(m_profileTotalData[PROFILE_INPUT_POLL_PERIOD]) / static_cast<double>(m_processInputCount)))
           : section == PROFILE_INPUT_TO_PRESENT  ? (m_presentedCount <= 0    ? 0. : (static_cast<double>(m_profileTotalData[PROFILE_INPUT_TO_PRESENT])  / static_cast<double>(m_presentedCount)))
                                                  : (m_frameIndex <= 0        ? 0. : (static_cast<double>(m_profileTotalData[section])                   / static_cast<double>(m_frameIndex)));
   }
   
   double GetRatio(ProfileSection section) const
   {
      assert(0 <= section && section <= PROFILE_FRAME); // Unimplemented and not meaningful for other sections 
      return m_profileTotalData[ProfileSection::PROFILE_FRAME] == 0 ? 0. : (static_cast<double>(m_profileTotalData[section]) / static_cast<double>(m_profileTotalData[ProfileSection::PROFILE_FRAME]));
   }
   
   double GetSlidingRatio(ProfileSection section) const
   {
      assert(0 <= section && section <= PROFILE_FRAME); // Unimplemented and not meaningful for other sections
      double frame = GetSlidingAvg(PROFILE_FRAME);
      return frame <= 1e-9 ? 0. : GetSlidingAvg(section) / frame;
   }
   
   // (approximately) 1 second sliding average of frame sections
   double GetSlidingAvg(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
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
         if (section <= PROFILE_FRAME && elapsed >= 1000000u) // end of 1s sliding average
            break;
         pos = (pos + N_SAMPLES - 1) % N_SAMPLES;
      }
      return count == 0 ? 0. : static_cast<double>(sum) / static_cast<double>(count);
   }

   // (approximately) 1 second sliding minimum of frame sections
   unsigned int GetSlidingMin(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      unsigned int pos = (m_profileIndex + N_SAMPLES - 1) % N_SAMPLES; // Start from last frame
      unsigned int elapsed = 0u;
      unsigned int sum = UINT_MAX;
      unsigned int count = 0u;
      for (unsigned int i = 0u; i < N_SAMPLES; i++)
      {
         if (count >= m_frameIndex)
            break;
         count++;
         sum = min(sum, m_profileData[pos][section]);
         elapsed += m_profileData[pos][ProfileSection::PROFILE_FRAME];
         if (section <= PROFILE_FRAME && elapsed >= 1000000u) // end of 1s sliding average
            break;
         pos = (pos + N_SAMPLES - 1) % N_SAMPLES;
      }
      return count == 0 ? 0 : sum;
   }

   // (approximately) 1 second sliding maximum of frame sections
   unsigned int GetSlidingMax(ProfileSection section) const
   {
      assert(0 <= section && section < PROFILE_COUNT);
      unsigned int pos = (m_profileIndex + N_SAMPLES - 1) % N_SAMPLES; // Start from last frame
      unsigned int elapsed = 0u;
      unsigned int sum = 0;
      unsigned int count = 0u;
      for (unsigned int i = 0u; i < N_SAMPLES; i++)
      {
         if (count >= m_frameIndex)
            break;
         count++;
         sum = max(sum, m_profileData[pos][section]);
         elapsed += m_profileData[pos][ProfileSection::PROFILE_FRAME];
         if (section <= PROFILE_FRAME && elapsed >= 1000000u) // end of 1s sliding average
            break;
         pos = (pos + N_SAMPLES - 1) % N_SAMPLES;
      }
      return count == 0 ? 0 : sum;
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
         const unsigned int period = m_profileData[pos][PROFILE_INPUT_POLL_PERIOD];
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
         return sum <= 0 ? 0. : static_cast<double>(latency) / static_cast<double>(sum);
   }

   unsigned long long GetPrevStart(ProfileSection section) const
   {
      assert(0 <= section && section <= PROFILE_FRAME); // Unimplemented and not really meaningful for other sections
      return m_profileDataStart[(m_profileIndex + N_SAMPLES - 1) % N_SAMPLES][section];
   }

   unsigned long long GetPrevEnd(ProfileSection section) const
   {
      assert(0 <= section && section <= PROFILE_FRAME); // Unimplemented and not really meaningful for other sections
      return m_profileDataEnd[(m_profileIndex + N_SAMPLES - 1) % N_SAMPLES][section];
   }

   string GetWorstScriptInfo() const
   {
      return EventDataToLog(m_worstScriptEventData[0], m_profileWorstProfileTimers[0], m_profileWorstProfileTimersLen[0]);
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
   
   void OnPresented(U64 when) // May be called from any thread
   {
      m_lastPresentedTimeStamp = when;
   }

   void SetThreadLock()
   {
      m_threadLock = std::this_thread::get_id();
   }

private:
   constexpr static unsigned int N_SAMPLES = 1000; // Number of samples to store. Must be kept quite high to be able to do a 1s sliding average (so at 1000FPS, needs 100 samples)
   constexpr static unsigned int N_WORST = 10; // Number of longest frames to keep detailed profile timing
   constexpr static unsigned int STACK_SIZE = 100;
   constexpr static unsigned int MAX_TIMER_LOG = 1024;

   bool m_logWorstFrame = false;

   std::thread::id m_threadLock = std::this_thread::get_id();

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
   unsigned int m_presentedIndex = 0;
   unsigned int m_presentedCount = 0;
   unsigned long long m_processInputTimeStampOnPrepare = 0;
   unsigned long long m_lastPresentedTimeStamp = 0;

   // Raw data
   unsigned int m_profileData[N_SAMPLES][PROFILE_COUNT];
   unsigned long long m_profileDataStart[N_SAMPLES][PROFILE_COUNT];
   unsigned long long m_profileDataEnd[N_SAMPLES][PROFILE_COUNT];
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

   string EventDataToLog(const robin_hood::unordered_map<DISPID, EventTick>& eventData, const char* profileTimers, const size_t profileTimersLen) const
   {
      std::stringstream ss;
      for (auto v : eventData)
      {
         string name;
         switch (v.first)
         {
         case 1000: name = "GameEvents:KeyDown"s; break;
         case 1001: name = "GameEvents:KeyUp"s; break;
         case 1002: name = "GameEvents:Init"s; break;
         case 1003: name = "GameEvents:MusicDone"s; break;
         case 1004: name = "GameEvents:Exit"s; break;
         case 1005: name = "GameEvents:Paused"s; break;
         case 1006: name = "GameEvents:UnPaused"s; break;
         case 1007: name = "GameEvents:OptionEvent"s; break;
         case 1101: name = "SurfaceEvents:Slingshot"s; break;
         case 1200: name = "FlipperEvents:Collide"s; break;
         case 1300: name = "TimerEvents:Timer"s; break;
         case 1301: name = "SpinnerEvents:Spin"s; break;
         case 1302: name = "TargetEvents:Dropped"s; break;
         case 1303: name = "TargetEvents:Raised"s; break;
         case 1320: name = "LightSeqEvents:PlayDone"s; break;
         case 1400: name = "HitEvents:Hit"s; break;
         case 1401: name = "HitEvents:Unhit"s; break;
         case 1402: name = "LimitEvents:EOS"s; break;
         case 1403: name = "LimitEvents:BOS"s; break;
         case 1404: name = "AnimateEvents:Animate"s; break;
         default: name = "DispID[" + std::to_string(v.first) + ']';
         }
         // ss << " spent in " << std::setw(3) << v.second.callCount << " calls of " << name;
         if (v.first == 1300)
         {
            struct info
            {
               int calls;
               U32 lengths;
            };
            robin_hood::unordered_map<string, info> infos;
            size_t pos = 0;
            while (pos < profileTimersLen)
            {
               string nameip(&profileTimers[pos]);
               pos += nameip.length() + 1;
               U32 length = *((U32*)&profileTimers[pos]);
               pos += 4;
               auto it = infos.find(nameip);
               if (it == infos.end())
               {
                  infos[nameip] = { 1, length };
               }
               else
               {
                  it->second.calls++;
                  it->second.lengths += length;
               }
            }
            for (const auto& pair : infos)
               ss << "   . " << std::setw(3) << pair.second.calls << " calls of " << pair.first << " lasting " << std::setw(4) << std::fixed << std::setprecision(1)
                  << (pair.second.lengths * 1e-3) << "ms (total)\n";
         }
         else
            ss << "   . " << std::setw(3) << v.second.callCount << " calls of " << name << " lasting " << std::setw(4) << std::fixed << std::setprecision(1) << v.second.totalLength
               << "ms (total)\n";
      }
      return ss.str();
   }
};

extern FrameProfiler* g_frameProfiler;

// Helper macro to profile custom function on the game logic thread
#define PROFILE_FUNCTION(section) ScopedProfiler funcProfiler(section)
class ScopedProfiler
{
public:
   ScopedProfiler(const FrameProfiler::ProfileSection section) { g_frameProfiler->EnterProfileSection(section); }
   ~ScopedProfiler() { g_frameProfiler->ExitProfileSection(); }
};
