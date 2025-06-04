#include "PUPTrigger.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"

namespace PUP {

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

static vector<PUPTrigger::PUPTriggerCondition> ParseConditions(const string& triggerString)
{
   vector<PUPTrigger::PUPTriggerCondition> triggers;
   std::istringstream stream(triggerString);
   string token;

   while (std::getline(stream, token, ','))
   {
      char type;
      int index, expected;
      size_t equalPos = token.find('=');
      if (equalPos != string::npos && equalPos > 1)
      {
         // Parse triggers with state (e.g., "W5=1")
         type = token[0];
         index = string_to_int(token.substr(1, equalPos - 1), -1);
         expected = string_to_int(token.substr(equalPos + 1), -1);
      }
      else if (token.length() >= 2)
      {
         // Parse triggers without state (e.g., "S10")
         type = token[0];
         index = string_to_int(token.substr(1), -1);
         expected = -1;
      }
      else
      {
         continue;
      }
      triggers.emplace_back(type, index, expected);
   }
   return triggers;
}

PUPTrigger::PUPTrigger(bool active, const string& szDescript, const string& szTrigger, PUPScreen* pScreen, PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, int length, int counter, int restSeconds, PUP_TRIGGER_PLAY_ACTION playAction)
   : m_szDescript(szDescript)
   , m_szTrigger(szTrigger)
   , m_pScreen(pScreen)
   , m_pPlaylist(pPlaylist)
   , m_active(active)
   , m_szPlayFile(szPlayFile)
   , m_volume(volume)
   , m_priority(priority)
   , m_length(length)
   , m_counter(counter)
   , m_restSeconds(restSeconds)
   , m_playAction(playAction)
   , m_conditions(ParseConditions(szTrigger))
{
}

PUPTrigger* PUPTrigger::CreateFromCSV(PUPScreen* pScreen, const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 14) {
      LOGD("Invalid trigger: %s", line.c_str());
      return nullptr;
   }

   const string& triggerPlaylist = parts[5];
   const string& triggerPlayAction = parts[12];

   int screen = string_to_int(parts[4], -1);
   if (screen != pScreen->GetScreenNum())
      return nullptr;

   bool active = (string_to_int(parts[1], 0) == 1);
   if (!active) {
      LOGD("Inactive trigger: %s", line.c_str());
      return nullptr;
   }

   if (StrCompareNoCase(triggerPlayAction, "CustomFunc")) {
      // TODO parse the custom function and call PUPPinDisplay::SendMSG when triggered
      LOGE("CustomFunc not implemented: %s", line.c_str());
      return nullptr;
   }

   // Sometimes an empty playlist but with description is used as a comment/separator.
   if (triggerPlaylist.empty()) {
      // TODO A PuP Pack Audit should mark these as comment triggers if the trigger is active
      return nullptr;
   }

   PUPPlaylist* pPlaylist = pScreen->GetPlaylist(triggerPlaylist);
   if (!pPlaylist) {
      LOGE("Playlist not found: %s", triggerPlaylist.c_str());
      return nullptr;
   }

   string szPlayFile = parts[6];
   if (!szPlayFile.empty()) {
      szPlayFile = pPlaylist->GetPlayFile(szPlayFile);
      if (szPlayFile.empty()) {
         LOGE("PlayFile not found for playlist %s: %s", pPlaylist->GetFolder().c_str(), parts[6].c_str());
         return nullptr;
      }
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

   std::istringstream stream(parts[3]);
   string triggerString;
   while (std::getline(stream, triggerString, ','))
   {
      if (triggerString.empty())
      {
         LOGE("Empty token found in trigger string: %s", parts[3].c_str());
         continue;
      }
      size_t equalPos = triggerString.find('=');
      string name;
      int value = -1; // Default state for the trigger
      if (equalPos != string::npos)
      {
         // Parse triggers with state (e.g., "W5=1")
         name = triggerString.substr(0, equalPos);
         value = string_to_int(triggerString.substr(equalPos + 1), -1);
      }
      else
      {
         // Parse triggers without state (e.g., "S10")
         name = triggerString;
      }
      if (name.empty())
      {
         LOGE("Invalid trigger name in trigger string: %s", parts[3].c_str());
         continue;
      }
   }

   return new PUPTrigger(
      active,
      parts[2], // descript
      parts[3], // trigger
      pScreen,
      pPlaylist,
      szPlayFile,
      parts[7].empty() ? pPlaylist->GetVolume() : string_to_float(parts[7], 0), // volume
      parts[8].empty() ? pPlaylist->GetPriority() : string_to_int(parts[8], 0), // priority
      string_to_int(parts[9], 0), // length
      string_to_int(parts[10], 0), // counter
      parts[11].empty() ? pPlaylist->GetRestSeconds() : string_to_int(parts[11], 0), // rest seconds
      playAction
   );
}

bool PUPTrigger::IsResting() const
{
   if (m_restSeconds <= 0)
      return false;

   if (m_lastTriggered == 0)
      return false;

   return (SDL_GetTicks() - m_lastTriggered) < ((Uint64)m_restSeconds * 1000);
}

void PUPTrigger::SetTriggered()
{
   m_lastTriggered = SDL_GetTicks();
}

string PUPTrigger::ToString() const {
   return string("active=") + ((m_active == true) ? "true" : "false") +
      ", descript=" + m_szDescript +
      ", trigger=" + m_szTrigger +
      ", screen={" + m_pScreen->ToString() + '}' +
      ", playlist={" + m_pPlaylist->ToString() + '}' +
      ", playFile=" + m_szPlayFile +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", length=" + std::to_string(m_length) +
      ", count=" + std::to_string(m_counter) +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", playAction=" + string(PUP_TRIGGER_PLAY_ACTION_TO_STRING(m_playAction));
}

}