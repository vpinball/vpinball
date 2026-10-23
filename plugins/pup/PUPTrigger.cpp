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
         expected = string_to_int(token.substr(equalPos + 1), 1);
      }
      else if (token.length() >= 2)
      {
         // Parse triggers without state (e.g., "S10")
         type = token[0];
         index = string_to_int(token.substr(1), -1);
         expected = 1;
      }
      else
      {
         continue;
      }
      triggers.emplace_back(type, index, expected);
   }
   return triggers;
}

PUPTrigger::PUPTrigger(const string& szDescript, const string& szTrigger, PUPScreen* pScreen, PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume,
   int priority, int length, int counter, int restSeconds, PlayAction playAction, bool useDefaults, const string& szCustomFunction)
   : m_szDescript(szDescript)
   , m_szTrigger(szTrigger)
   , m_szCustomFunction(szCustomFunction)
   , m_conditions(ParseConditions(szTrigger))
   , m_pScreen(pScreen)
   , m_pPlaylist(pPlaylist)
   , m_szPlayFile(szPlayFile)
   , m_volume(volume)
   , m_priority(priority)
   , m_length(length)
   , m_counter(counter)
   , m_restSeconds(restSeconds)
   , m_playAction(playAction)
   , m_useDefaults(useDefaults && playAction != PlayAction::PlaySSF)
{
}

void PUPTrigger::Run()
{
   if (m_useDefaults)
   {
      m_pScreen->Play(m_pPlaylist, std::filesystem::path(), static_cast<float>(m_pScreen->GetDefaultVolume()), 0, PlayAction::Normal, 0);
      return;
   }
   switch (m_playAction) {
   case PlayAction::Normal:
   case PlayAction::Loop:
   case PlayAction::SetBG:
   case PlayAction::SplashReset:
   case PlayAction::SplashReturn:
   case PlayAction::SkipSamePriority:
   case PlayAction::QueueSameP:
   case PlayAction::QueueAlways:
      m_pScreen->Play(m_pPlaylist, m_szPlayFile, m_volume, m_priority, m_playAction, m_length);
      break;

   case PlayAction::StopPlayer:
      m_pScreen->Stop(m_priority);
      break;

   case PlayAction::StopFile:
      m_pScreen->Stop(m_pPlaylist, m_szPlayFile);
      break;

   case PlayAction::PlaySSF:
   {
      int pos[3] = { 0, 0, 0 };
      std::istringstream stream(m_szCustomFunction);
      string field;
      for (int i = 0; i < 3 && std::getline(stream, field, ','); i++)
         pos[i] = string_to_int(trim_string(field), 0);
      m_pScreen->GetManager()->PlaySound(PluginStrings::PathToUTF8(m_szPlayFile), pos[0], pos[1], pos[2], static_cast<int>(m_volume), ""s);
      break;
   }

   case PlayAction::CustomFunction:
      m_pScreen->GetManager()->CustomFunction(m_pScreen->GetScreenNum(), m_szCustomFunction);
      if (!m_szPlayFile.empty())
         m_pScreen->GetManager()->SendMsg(PluginStrings::PathToUTF8(m_szPlayFile));
      break;

   default:
      LOGE("Invalid play action: " + PlayActionToString(m_playAction));
      break;
   }
}

PUPTrigger* PUPTrigger::CreateFromCSV(PUPScreen* pScreen, const string& line)
{
   if (trim_string(line).empty())
      return nullptr;
   vector<string> parts = parse_csv_line(line);
   if (parts.size() < 13) {
      if (parts.size() > 4 && string_to_int(parts[4], -1) != pScreen->GetScreenNum())
         return nullptr;
      LOGE(std::format("Failed to parse trigger line, expected at least 13 columns but got {}: {}", parts.size(), line));
      return nullptr;
   }
   parts.resize(std::max<size_t>(parts.size(), 14));

   const string& triggerPlaylist = parts[5];
   const string& triggerPlayAction = parts[12];

   int screen = string_to_int(parts[4], -1);
   if (screen != pScreen->GetScreenNum())
      return nullptr;

   if (string_to_int(parts[1], 0) != 1) {
      LOGD("Inactive trigger: " + line);
      return nullptr;
   }

   if (StrCompareNoCase(triggerPlayAction, "PlaySSF"s)) {
      // The file column is the sound name and the counter column its "x,y,z" position;
      // PinUP plays nothing when fewer than three position fields are given.
      if (std::count(parts[10].begin(), parts[10].end(), ',') < 2)
      {
         LOGE("PlaySSF trigger without a x,y,z position: " + line);
         return nullptr;
      }
      return new PUPTrigger(parts[2], parts[3], pScreen, nullptr, PluginStrings::PathFromUTF8(parts[6]), parts[7].empty() ? 100.f : string_to_float(parts[7], 100), 0, 0, 0,
         string_to_int(parts[11], 0), PlayAction::PlaySSF, false, parts[10]);
   }

   if (StrCompareNoCase(triggerPlayAction, "CustomFunc"s)) {
      // The playlist column holds the custom function JSON, applied to this screen. The
      // file column, when set, is posted as a regular PuP message (PinUP SendSelfPuPMSG).
      return new PUPTrigger(parts[2], parts[3], pScreen, nullptr, PluginStrings::PathFromUTF8(parts[6]), 100.f, 0, 0, string_to_int(parts[10], 0),
         string_to_int(parts[11], 0), PlayAction::CustomFunction, false, triggerPlaylist);
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

   std::filesystem::path szPlayFile = PluginStrings::PathFromUTF8(parts[6]);
   if (!szPlayFile.empty()) {
      szPlayFile = pPlaylist->GetPlayFile(szPlayFile);
      if (szPlayFile.empty()) {
         LOGE("PlayFile not found for playlist " + PluginStrings::PathToUTF8(pPlaylist->GetFolder()) + ": " + parts[6]);
         return nullptr;
      }
   }

   PlayAction playAction;
   if (StrCompareNoCase(triggerPlayAction, "Loop"s))
      playAction = PlayAction::Loop;
   else if (StrCompareNoCase(triggerPlayAction, "SplashReset"s))
      playAction = PlayAction::SplashReset;
   else if (StrCompareNoCase(triggerPlayAction, "SplashResume"s) || StrCompareNoCase(triggerPlayAction, "SplashReturn"s))
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
   else if (StrCompareNoCase(triggerPlayAction, "QueueSameP"s))
      playAction = PlayAction::QueueSameP;
   else if (StrCompareNoCase(triggerPlayAction, "QueueAlways"s))
      playAction = PlayAction::QueueAlways;
   else
      playAction = PlayAction::Normal;

   return new PUPTrigger(
      parts[2], // descript
      parts[3], // trigger
      pScreen,
      pPlaylist,
      szPlayFile,
      parts[7].empty() ? pPlaylist->GetVolume() : string_to_float(parts[7], 100), // volume
      string_to_int(parts[8], 0), // priority
      string_to_int(parts[9], 0), // length
      string_to_int(parts[10], 0), // counter
      string_to_int(parts[11], 0), // rest seconds
      playAction,
      string_to_int(parts[13], 0) == 1 // defaults
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

void PUPTrigger::Invoke() {
   if (m_pScreen->GetMode() == PUPScreen::Mode::Off) {
      LOGD(std::format("Skipping trigger on Off screen: trigger={{{}}}", ToString()));
      return;
   }
   if (m_restSeconds < 0) {
      if (!m_delayPending) {
         m_delayPending = true;
         const double delay = (m_restSeconds > -99) ? -m_restSeconds : -m_restSeconds / 1000.0;
         m_pScreen->GetManager()->RunLater(delay, [this]() { m_delayPending = false; Fire(false); });
      }
      return;
   }
   Fire(true);
}

void PUPTrigger::Fire(bool checkRest) {
   // Fires on hits 1, N+1, 2N+1... like PinUP Player, except for counter 1: it never fires in PinUP Player, here it fires on every hit
   if (m_counter > 1) {
      m_hitCount++;
      if (m_hitCount >= m_counter) {
         m_hitCount = 0;
         return;
      }
      if (m_hitCount > 1)
         return;
   }
   if (checkRest && IsResting()) {
      LOGD(std::format("Skipping resting trigger: trigger={{{}}}", ToString()));
      return;
   }
   m_lastTriggered = SDL_GetTicks();
   LOGD(std::format("Processing trigger: trigger={{{}}}", ToString()));
   Run();
}

string PUPTrigger::ToString() const {
   return "descript=" + m_szDescript +
      ", trigger=" + m_szTrigger +
      ", screen={" + m_pScreen->ToString() + '}' +
      ", playlist={" + (m_pPlaylist ? m_pPlaylist->ToString() : m_szCustomFunction) + '}' +
      ", playFile=" + PluginStrings::PathToUTF8(m_szPlayFile) +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", length=" + std::to_string(m_length) +
      ", count=" + std::to_string(m_counter) +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", playAction=" + PlayActionToString(m_playAction);
}

}
