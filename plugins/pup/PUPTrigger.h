// license:GPLv3+

#pragma once

#include "PUPManager.h"

#include <algorithm>

namespace PUP {

class PUPPlaylist;
class PUPScreen;

class PUPTrigger final
{
public:
   static PUPTrigger* CreateFromCSV(PUPScreen* pScreen, const string& line);

   class PUPTriggerCondition
   {
   public:
      PUPTriggerCondition(char type, int number, int expected)
         : m_type(type), m_number(number), m_expected(expected) {}
      const char m_type; // Trigger type
      const int m_number; // Trigger id
      const int m_expected; // Expected value, 1 when not given

      bool IsTriggered() const { return m_value == m_expected; }
      int m_value = 0; // Current value
   };

   const string& GetDescription() const { return m_szDescript; }
   PUPScreen* GetScreen() const { return m_pScreen; }
   PUPPlaylist* GetPlaylist() const { return m_pPlaylist; }
   const string& GetTrigger() const { return m_szTrigger; }
   vector<PUPTriggerCondition>& GetTriggers() { return m_conditions; }
   bool IsTriggered() const { return std::ranges::all_of(m_conditions, [](const PUPTriggerCondition& x) { return x.IsTriggered(); }); }
   float GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   string ToString() const;

   bool IsResting() const;

   void Invoke();

private:
   PUPTrigger(const string& szDescript, const string& szTrigger, PUPScreen* pScreen, PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, int length, int counter, int restSeconds, PlayAction playAction, bool useDefaults, const string& szCustomFunction = ""s);
   void Fire(bool checkRest);
   void Run();
   const string m_szDescript;
   const string m_szTrigger;
   const string m_szCustomFunction;
   vector<PUPTriggerCondition> m_conditions;
   PUPScreen* const m_pScreen;
   PUPPlaylist* const m_pPlaylist;
   const std::filesystem::path m_szPlayFile;
   const float m_volume;
   const int m_priority;
   const int m_length;
   const int m_counter;
   const int m_restSeconds;
   const PlayAction m_playAction;
   const bool m_useDefaults;

   uint64_t m_lastTriggered = 0;
   int m_hitCount = 0;
   bool m_delayPending = false;
};

}
