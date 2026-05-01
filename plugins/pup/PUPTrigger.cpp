// license:GPLv3+

#include "PUPTrigger.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"

namespace PUP {

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

PUPTrigger::PUPTrigger(bool active, const string& szDescript, const string& szTrigger, PUPScreen* pScreen, PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume,
   int priority, int length, int counter, int restSeconds, PlayAction playAction)
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
   switch (m_playAction) {
   case PlayAction::Normal:
      m_action = [&]() { m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, PlayAction::Normal, m_length); };
      break;

   case PlayAction::Loop:
      m_action = [&]()
      {
         m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, PlayAction::Loop, m_length);
         m_pScreen->SetLoop(true);
      };
      break;

   case PlayAction::SetBG:
      m_action = [&]() { m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, PlayAction::SetBG, m_length); };
      break;

   case PlayAction::SplashReset:
      m_action = [&]()
      {
         // Play splash video, background will restart from beginning when splash ends
         m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, PlayAction::SplashReset, m_length);
      };
      break;

   case PlayAction::SplashReturn:
      m_action = [&]()
      {
         // Play splash video, background will resume from where it left off when splash ends
         m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, PlayAction::SplashReturn, m_length);
      };
      break;

   case PlayAction::SkipSamePriority:
      m_action = [&]() { m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, PlayAction::SkipSamePriority, m_length); };
      break;

   case PlayAction::StopPlayer:
      m_action = [&]() { m_pScreen->Stop(m_priority); };
      break;

   case PlayAction::StopFile:
      m_action = [&]() { m_pScreen->Stop(m_pPlaylist, m_szPlayFile); };
      break;

   default:
      LOGE("Invalid play action: " + PlayActionToString(m_playAction));
      m_action = [](){};
      break;
   }
}

PUPTrigger* PUPTrigger::CreateFromCSV(PUPScreen* pScreen, const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 14) {
      LOGD("Invalid trigger: " + line);
      return nullptr;
   }

   const string& triggerPlaylist = parts[5];
   const string& triggerPlayAction = parts[12];

   int screen = string_to_int(parts[4], -1);
   if (screen != pScreen->GetScreenNum())
      return nullptr;

   bool active = (string_to_int(parts[1], 0) == 1);
   if (!active) {
      LOGD("Inactive trigger: " + line);
      return nullptr;
   }

   if (StrCompareNoCase(triggerPlayAction, "CustomFunc"s)) {
      // TODO parse the custom function and call PUPPinDisplay::SendMSG when triggered
      NOT_IMPLEMENTED("CustomFunc not implemented: " + line);
      return nullptr;
   }

   // Sometimes an empty playlist but with description is used as a comment/separator.
   if (triggerPlaylist.empty()) {
      // TODO A PuP Pack Audit should mark these as comment triggers if the trigger is active
      return nullptr;
   }

   PUPPlaylist* pPlaylist = pScreen->GetPlaylist(triggerPlaylist);
   if (!pPlaylist) {
      LOGE("Playlist not found: " + triggerPlaylist);
      return nullptr;
   }

   std::filesystem::path szPlayFile = parts[6];
   if (!szPlayFile.empty()) {
      szPlayFile = pPlaylist->GetPlayFile(szPlayFile);
      if (szPlayFile.empty()) {
         LOGE("PlayFile not found for playlist " + pPlaylist->GetFolder().string() + ": " + parts[6]);
         return nullptr;
      }
   }

   PlayAction playAction;
   if (StrCompareNoCase(triggerPlayAction, "Loop"s))
      playAction = PlayAction::Loop;
   else if (StrCompareNoCase(triggerPlayAction, "SplashReset"s))
      playAction = PlayAction::SplashReset;
   else if (StrCompareNoCase(triggerPlayAction, "SplashReturn"s))
      playAction = PlayAction::SplashReturn;
   else if (StrCompareNoCase(triggerPlayAction, "StopPlayer"s))
      playAction = PlayAction::StopPlayer;
   else if (StrCompareNoCase(triggerPlayAction, "StopFile"s))
      playAction = PlayAction::StopFile;
   else if (StrCompareNoCase(triggerPlayAction, "SetBG"s))
      playAction = PlayAction::SetBG;
   else if (StrCompareNoCase(triggerPlayAction, "PlaySSF"s))
      playAction = PlayAction::PlaySSF;
   else if (StrCompareNoCase(triggerPlayAction, "SkipSamePrty"s))
      playAction = PlayAction::SkipSamePriority;
   else if (StrCompareNoCase(triggerPlayAction, "CustomFunc"s))
      playAction = PlayAction::CustomFunction;
   else
      playAction = PlayAction::Normal;

   std::istringstream stream(parts[3]);
   string triggerString;
   while (std::getline(stream, triggerString, ','))
   {
      if (triggerString.empty())
      {
         LOGE("Empty token found in trigger string: " + parts[3]);
         continue;
      }
      const size_t equalPos = triggerString.find('=');
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
         LOGE("Invalid trigger name in trigger string: " + parts[3]);
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

   return (SDL_GetTicks() - m_lastTriggered) < ((uint64_t)m_restSeconds * 1000);
}

std::function<void()> PUPTrigger::Trigger() {
   if (IsResting()) {
      LOGD(std::format("Skipping resting trigger: trigger={{{}}}", ToString()));
      return [](){};
   }
   if (m_pScreen->GetMode() == PUPScreen::Mode::Off) {
      LOGD(std::format("Skipping trigger on Off screen: trigger={{{}}}", ToString()));
      return [](){};
   }
   m_lastTriggered = SDL_GetTicks();
   LOGD(std::format("Processing trigger: trigger={{{}}}", ToString()));
   return m_action;
}

void PUPTrigger::Invoke() {
   // Remember this as the most recent trigger so LabelShowPage "returnplay" can replay it.
   m_pScreen->m_lastPlayedTrigger = this;
   Trigger()();
}

string PUPTrigger::ToString() const {
   return "active="s + ((m_active == true) ? "true" : "false") +
      ", descript=" + m_szDescript +
      ", trigger=" + m_szTrigger +
      ", screen={" + m_pScreen->ToString() + '}' +
      ", playlist={" + m_pPlaylist->ToString() + '}' +
      ", playFile=" + m_szPlayFile.string() + // FIXME this may cause an exception
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", length=" + std::to_string(m_length) +
      ", count=" + std::to_string(m_counter) +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", playAction=" + PlayActionToString(m_playAction);
}

}
