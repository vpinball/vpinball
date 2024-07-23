#include "stdafx.h"

#include "PUPTrigger.h"
#include "PUPTriggerCondition.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"

const char* PUP_TRIGGER_PLAY_ACTION_STRINGS[] = {
   "PUP_TRIGGER_PLAY_ACTION_NORMAL",
   "PUP_TRIGGER_PLAY_ACTION_LOOP",
   "PUP_TRIGGER_PLAY_ACTION_SPLASH_RESET",
   "PUP_TRIGGER_PLAY_ACTION_SPLASH_RETURN",
   "PUP_TRIGGER_PLAY_ACTION_STOP_PLAYER",
   "PUP_TRIGGER_PLAY_ACTION_STOP_FILE",
   "PUP_TRIGGER_PLAY_ACTION_SET_BG",
   "PUP_TRIGGER_PLAY_ACTION_PLAY_SSF",
   "PUP_TRIGGER_PLAY_ACTION_SKIP_SAME_PRTY",
   "PUP_TRIGGER_PLAY_ACTION_CUSTOM_FUNC"
};

const char* PUP_TRIGGER_PLAY_ACTION_TO_STRING(PUP_TRIGGER_PLAY_ACTION value)
{
   if ((size_t)value >= std::size(PUP_TRIGGER_PLAY_ACTION_STRINGS))
      return "UNKNOWN";
   return PUP_TRIGGER_PLAY_ACTION_STRINGS[value];
}

/*
   triggers.pup: ID,Active,Descript,Trigger,ScreenNum,PlayList,PlayFile,Volume,Priority,Length,Counter,RestSeconds,Loop,Defaults
   PuP Pack Editor: Descript,Trigger,Screen,Playlist,PlayFile,Volume,RestSeconds,Priority,Counter,Length(s),PlayAction,Active

   mappings:

     ID = ?
     Active = Active
     Descript = Descript
     Trigger = Trigger
     ScreenNum = Screen
     PlayList = Playlist
     PlayFile = PlayFile
     Volume = Volume
     Priority = Priority
     Length = Length(s)
     Counter = Counter
     RestSeconds = RestSeconds
     Loop = PlayAction
     Defaults = ?

   trigger names:

     S = Solenoid
     W = Switches
     L = Lights
     G = GIs
     E = DOFs
     M = Mechs
     D = PupCap DMD Match or B2S LED
*/

PUPTrigger::PUPTrigger(bool active, const string& szDescript, const vector<PUPTriggerCondition*>& conditions, PUPScreen* pScreen, PUPPlaylist* pPlaylist, const string& szPlayFile, float volume,
   int priority, int length, int counter, int restMs, PUP_TRIGGER_PLAY_ACTION playAction)
{
   m_active = active;
   m_szDescript = szDescript;
   m_conditions = conditions;
   m_pScreen = pScreen;
   m_pPlaylist = pPlaylist;
   m_szPlayFile = szPlayFile;
   m_volume = volume;
   m_priority = priority;
   m_length = length;
   m_counter = counter;
   m_restMs = restMs;
   m_playAction = playAction;
   m_lastTriggered = 0;
}

PUPTrigger::~PUPTrigger()
{
   for (auto* pCondition : m_conditions)
      delete pCondition;
}

PUPTrigger* PUPTrigger::CreateFromCSV(PUPScreen* pScreen, const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 14) {
      PLOGD.printf("Invalid trigger: %s", line.c_str());
      return nullptr;
   }

   const string& triggerPlaylist = parts[5];
   const string& triggerPlayAction = parts[12];

   int screen = string_to_int(parts[4], -1);
   if (screen != pScreen->GetScreenNum())
      return nullptr;

   bool active = (string_to_int(parts[1], 0) == 1);
   if (!active) {
      PLOGD.printf("Inactive trigger: %s", line.c_str());
      return nullptr;
   }

   if (StrCompareNoCase(triggerPlayAction, "CustomFunc")) {
      // TODO parse the custom function and call PUPPinDisplay::SendMSG when triggered
      PLOGW.printf("CustomFunc not implemented: %s", line.c_str());
      return nullptr;
   }

   // Sometimes an empty playlist but with description is used as a comment/separator.
   if (triggerPlaylist.empty()) {
      // TODO A PuP Pack Audit should mark these as comment triggers if the trigger is active
      return nullptr;
   }

   PUPPlaylist* pPlaylist = pScreen->GetPlaylist(triggerPlaylist);
   if (!pPlaylist) {
      PLOGW.printf("Playlist not found: %s", triggerPlaylist.c_str());
      return nullptr;
   }

   string szPlayFile = parts[6];
   if (!szPlayFile.empty()) {
      szPlayFile = pPlaylist->GetPlayFile(szPlayFile);
      if (szPlayFile.empty()) {
         PLOGW.printf("PlayFile not found for playlist %s: %s", pPlaylist->GetFolder().c_str(), parts[6].c_str());
         return nullptr;
      }
   }

   vector<PUPTriggerCondition*> conditions = PUPTriggerCondition::CreateFromCSV(parts[3]);
   if (conditions.empty()) {
      PLOGD.printf("No conditions: %s", line.c_str());
      return nullptr;
   }

   PUP_TRIGGER_PLAY_ACTION playAction;
   if (StrCompareNoCase(triggerPlayAction, "Loop"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_LOOP;
   else if (StrCompareNoCase(triggerPlayAction, "SplashReset"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_SPLASH_RESET;
   else if (StrCompareNoCase(triggerPlayAction, "SplashReturn"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_SPLASH_RETURN;
   else if (StrCompareNoCase(triggerPlayAction, "StopPlayer"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_STOP_PLAYER;
   else if (StrCompareNoCase(triggerPlayAction, "StopFile"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_STOP_FILE;
   else if (StrCompareNoCase(triggerPlayAction, "SetBG"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_SET_BG;
   else if (StrCompareNoCase(triggerPlayAction, "PlaySSF"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_PLAY_SSF;
   else if (StrCompareNoCase(triggerPlayAction, "SkipSamePrty"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_SKIP_SAME_PRTY;
   else if (StrCompareNoCase(triggerPlayAction, "CustomFunc"s))
      playAction = PUP_TRIGGER_PLAY_ACTION_CUSTOM_FUNC;
   else
      playAction = PUP_TRIGGER_PLAY_ACTION_NORMAL;

   int restMs = string_to_int(parts[11], pPlaylist->GetRestSeconds());
   if (abs(restMs) <= 99)
      restMs *= 1000;

   return new PUPTrigger(active,
      parts[2], // descript
      conditions,
      pScreen, pPlaylist, szPlayFile, parts[7].empty() ? pPlaylist->GetVolume() : string_to_int(parts[7], 0), // volume
      parts[8].empty() ? pPlaylist->GetPriority() : string_to_int(parts[8], 0), // priority
      string_to_int(parts[9], 0), // length
      string_to_int(parts[10], 0), // counter
      restMs,
      playAction);
}

bool PUPTrigger::IsResting() const
{
   if (m_restMs <= 0)
      return false;

   if (m_lastTriggered == 0)
      return false;

   return (SDL_GetTicks() - m_lastTriggered) < m_restMs;
}

bool PUPTrigger::Evaluate(PUPManager* pManager, const PUPTriggerData& data)
{
   if (IsResting())
      return false;

   // Make sure all conditions match, and at least one condition id matches the trigger id

   bool foundId = false;
   bool idMatch;

   for (auto& pCondition : m_conditions) {
      if (!pCondition->Evaluate(pManager, data, idMatch))
         return false;

      if (!foundId && idMatch)
         foundId = true;
   }

   if (!foundId)
      return false;

   m_lastTriggered = SDL_GetTicks();

   return true;
}

string PUPTrigger::ToString() const
{
   return string("active=") + ((m_active == true) ? "true" : "false") +
      ", descript=" + m_szDescript +
      ", conditions=[" + [&]() {
            string result;
            for (const auto& pCondition : m_conditions) {
               if (!result.empty())
                  result += ", ";
               result += pCondition->ToString();
            }
            return result;
      }() + "]" +
      ", screen={" + m_pScreen->ToString() + '}' +
      ", playlist={" + m_pPlaylist->ToString() + '}' +
      ", playFile=" + m_szPlayFile +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", length=" + std::to_string(m_length) +
      ", count=" + std::to_string(m_counter) +
      ", restMs=" + std::to_string(m_restMs) +
      ", playAction=" + string(PUP_TRIGGER_PLAY_ACTION_TO_STRING(m_playAction));
}
