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
   int priority, int length, int counter, int restSeconds, PUPTrigger::Action playAction)
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
   case PUPTrigger::Action::Normal:
      m_action = [&]() { m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, false, m_length, false); };
      break;

   case PUPTrigger::Action::Loop:
      m_action = [&]()
      {
         m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, false, m_length, false);
         m_pScreen->SetLoop(true);
      };
      break;

   case PUPTrigger::Action::SetBG:
      m_action = [&]() { m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, false, m_length, true); };
      break;

   case PUPTrigger::Action::SkipSamePriority:
      m_action = [&]() { m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, true, m_length, false); };
      break;

   case PUPTrigger::Action::StopPlayer:
      m_action = [&]() { m_pScreen->Stop(m_priority); };
      break;

   case PUPTrigger::Action::StopFile:
      m_action = [&]() { m_pScreen->Stop(m_pPlaylist, m_szPlayFile); };
      break;

   default:
      LOGE("Invalid play action: %s", PUPTrigger::ToString(m_playAction).c_str());
      m_action = [](){};
      break;
   }
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

   if (StrCompareNoCase(triggerPlayAction, "CustomFunc"s)) {
      // TODO parse the custom function and call PUPPinDisplay::SendMSG when triggered
      NOT_IMPLEMENTED("CustomFunc not implemented: %s", line.c_str());
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

   std::filesystem::path szPlayFile = parts[6];
   if (!szPlayFile.empty()) {
      szPlayFile = pPlaylist->GetPlayFile(szPlayFile);
      if (szPlayFile.empty()) {
         LOGE("PlayFile not found for playlist %s: %s", pPlaylist->GetFolder().c_str(), parts[6].c_str());
         return nullptr;
      }
   }

   PUPTrigger::Action playAction;
   if (StrCompareNoCase(triggerPlayAction, "Loop"s))
      playAction = PUPTrigger::Action::Loop;
   else if (StrCompareNoCase(triggerPlayAction, "SplashReset"s))
      playAction = PUPTrigger::Action::SplashReset;
   else if (StrCompareNoCase(triggerPlayAction, "SplashReturn"s))
      playAction = PUPTrigger::Action::SplashReturn;
   else if (StrCompareNoCase(triggerPlayAction, "StopPlayer"s))
      playAction = PUPTrigger::Action::StopPlayer;
   else if (StrCompareNoCase(triggerPlayAction, "StopFile"s))
      playAction = PUPTrigger::Action::StopFile;
   else if (StrCompareNoCase(triggerPlayAction, "SetBG"s))
      playAction = PUPTrigger::Action::SetBG;
   else if (StrCompareNoCase(triggerPlayAction, "PlaySSF"s))
      playAction = PUPTrigger::Action::PlaySSF;
   else if (StrCompareNoCase(triggerPlayAction, "SkipSamePrty"s))
      playAction = PUPTrigger::Action::SkipSamePriority;
   else if (StrCompareNoCase(triggerPlayAction, "CustomFunc"s))
      playAction = PUPTrigger::Action::CustomFunction;
   else
      playAction = PUPTrigger::Action::Normal;

   std::istringstream stream(parts[3]);
   string triggerString;
   while (std::getline(stream, triggerString, ','))
   {
      if (triggerString.empty())
      {
         LOGE("Empty token found in trigger string: %s", parts[3].c_str());
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

   return (SDL_GetTicks() - m_lastTriggered) < ((uint64_t)m_restSeconds * 1000);
}

std::function<void()> PUPTrigger::Trigger() {
   if (IsResting()) {
      LOGD("skipping resting trigger: trigger={%s}", ToString().c_str());
      return [](){};
   }
   if (m_pScreen->GetMode() == PUPScreen::Mode::Off) {
      LOGD("skipping trigger on Off screen: trigger={%s}", ToString().c_str());
      return [](){};
   }
   m_lastTriggered = SDL_GetTicks();
   LOGD("processing trigger: trigger={%s}", ToString().c_str());
   return m_action;
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
      ", playAction=" + ToString(m_playAction);
}

const string& PUPTrigger::ToString(Action value)
{
   static const string actionStrings[] = { "PUPTrigger::Normal"s, "PUPTrigger::Loop"s, "PUPTrigger::SplashReset"s, "PUPTrigger::SplashReturn"s, "PUPTrigger::StopPlayer"s,
      "PUPTrigger::StopFile"s, "PUPTrigger::SetBG"s, "PUPTrigger::PlaySSF"s, "PUPTrigger::SkipSamePriority"s, "PUPTrigger::CustomFunction"s };
   static const string error = "Unknown"s;
   if ((int)value < 0 || (size_t)value >= std::size(actionStrings))
      return error;
   return actionStrings[(int)value];
}


}
