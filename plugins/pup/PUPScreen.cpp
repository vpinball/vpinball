// license:GPLv3+

#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "PUPTrigger.h"
#include "PUPManager.h"
#include "PUPPlaylist.h"
#include "PUPLabel.h"
#include "PUPMediaManager.h"

#include <algorithm>
#include <cmath>

namespace PUP {

static constexpr int PUP_VOLUME_MAX = 200;
static constexpr uint64_t PUP_VOLUME_FADE_INTERVAL_MS = 50;

/*
   screens.pup: ScreenNum,ScreenDes,PlayList,PlayFile,Loopit,Active,Priority,CustomPos
   PuP Pack Editor: Mode,ScreenNum,ScreenDes,Background Playlist,Background Filename,Transparent,CustomPos,Volume %

   mappings:

     ScreenNum = ScreenNum
     ScreenDes = ScreenDes
     PlayList = Background Playlist
     PlayFile = Background Filename
     Loopit = Transparent
     Active = Mode
     Priority = Volume %
     CustomPos = CustomPos
*/

PUPScreen::PUPScreen(PUPManager* manager, PUPScreen::Mode mode, int screenNum, const string& szScreenDes, bool transparent, int volumeMultiplier, std::unique_ptr<PUPCustomPos> pCustomPos, const std::vector<PUPPlaylist*>& playlists)
   : m_pManager(manager)
   , m_screenNum(screenNum)
   , m_mode(mode)
   , m_topmost(mode == Mode::ForceOn || mode == Mode::ForcePop)
   , m_screenDes(szScreenDes)
   , m_transparent(transparent)
   , m_volumeMultiplier(volumeMultiplier)
   , m_pCustomPos(std::move(pCustomPos))
   , m_apiThread(std::this_thread::get_id())
{
   m_pMediaPlayerManager = std::make_unique<PUPMediaManager>(this);

   for (const PUPPlaylist* pPlaylist : playlists) {
      // make a copy of the playlist
      PUPPlaylist *pPlaylistCopy = new PUPPlaylist(*pPlaylist);
      AddPlaylist(pPlaylistCopy);
   }

   LoadTriggers();
}

PUPScreen::~PUPScreen()
{
   for (auto& [key, pPlaylist] : m_playlistMap)
      delete pPlaylist;

   for (auto& [key, pTriggers] : m_triggerMap) {
      for (PUPTrigger* pTrigger : pTriggers)
         delete pTrigger;
   }

   for (PUPLabel* pLabel : m_labels)
      delete pLabel;
}

std::unique_ptr<PUPScreen> PUPScreen::CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists)
{
   if (trim_string(line).empty())
      return nullptr;
   vector<string> parts = parse_csv_line(line);
   if (parts.size() < 6) {
      LOGE(std::format("Failed to parse screen line, expected at least 6 columns but got {}: {}", parts.size(), line));
      return nullptr;
   }
   parts.resize(std::max<size_t>(parts.size(), 8));

   PUPScreen::Mode mode;
   if (StrCompareNoCase(parts[5], "Show"s))
      mode = PUPScreen::Mode::Show;
   else if (StrCompareNoCase(parts[5], "ForceON"s))
      mode = PUPScreen::Mode::ForceOn;
   else if (StrCompareNoCase(parts[5], "ForcePoP"s))
      mode = PUPScreen::Mode::ForcePop;
   else if (StrCompareNoCase(parts[5], "ForceBack"s))
      mode = PUPScreen::Mode::ForceBack;
   else if (StrCompareNoCase(parts[5], "ForcePopBack"s))
      mode = PUPScreen::Mode::ForcePopBack;
   else if (StrCompareNoCase(parts[5], "MusicOnly"s) || StrCompareNoCase(parts[5], "ForceHide"s))
      mode = PUPScreen::Mode::MusicOnly;
   else if (StrCompareNoCase(parts[5], "Off"s) || StrCompareNoCase(parts[5], "Disable"s) || StrCompareNoCase(parts[5], "JavaScript"s))
      mode = PUPScreen::Mode::Off;
   else {
      LOGE("Invalid screen mode: " + parts[5]);
      mode = PUPScreen::Mode::Off;
   }

   const int volumeColumn = string_to_int(parts[6], 100);

   auto screen = std::make_unique<PUPScreen>(
      manager,
      mode,
      string_to_int(parts[0], 0), // screenNum
      parts[1], // screenDes
      parts[4] == "1", // transparent
      volumeColumn > 0 ? volumeColumn : 100,
      PUPCustomPos::CreateFromCSV(parts[7]), playlists);

   // Optional initial background playlist
   if (PUPPlaylist* const backgroundPlaylist = parts[2].empty() ? nullptr : screen->GetPlaylist(parts[2]); backgroundPlaylist)
      screen->Play(backgroundPlaylist, PluginStrings::PathFromUTF8(parts[3]), 0.f, -1, PlayAction::SetBG, 0);

   return screen;
}

std::unique_ptr<PUPScreen> PUPScreen::CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists)
{
   const char* screenDes;
   switch (screenNum) {
   case PUP_SCREEN_TOPPER: screenDes = "Topper"; break;
   case PUP_SCREEN_DMD: screenDes = "DMD"; break;
   case PUP_SCREEN_BACKGLASS: screenDes = "Backglass"; break;
   case PUP_SCREEN_PLAYFIELD: screenDes = "Playfield"; break;
   default: screenDes = "Unknown"; break;
   }
   return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, screenNum, string(screenDes), false, 100, nullptr, playlists);
}

void PUPScreen::LoadTriggers()
{
   assert(std::this_thread::get_id() == m_apiThread);
   std::filesystem::path szPlaylistsPath = find_case_insensitive_file_path(m_pManager->GetPath() / "triggers.pup"sv);
   std::ifstream triggersFile;
   triggersFile.open(szPlaylistsPath, std::ifstream::in);
   if (triggersFile.is_open())
   {
      string line;
      int i = 0;
      while (std::getline(triggersFile, line))
      {
         if (++i == 1)
            continue;
         AddTrigger(PUPTrigger::CreateFromCSV(this, PluginStrings::TextFromUTF8OrCP1252(line)));
      }
   }
}

void PUPScreen::SetMode(Mode mode)
{
   if (mode == m_mode)
      return;
   bool wasVisible = (m_mode != Mode::Off && m_mode != Mode::MusicOnly);
   m_mode = mode;
   bool isVisible = (m_mode != Mode::Off && m_mode != Mode::MusicOnly);
   if (wasVisible && !isVisible)
      m_pMediaPlayerManager->Stop();
}

int PUPScreen::ResolveVolume(float volume, const PUPPlaylist* pPlaylist) const
{
   int vol = static_cast<int>(std::lround(volume));
   if (vol == -1 && pPlaylist)
      vol = static_cast<int>(std::lround(pPlaylist->GetVolume()));
   return std::clamp(vol, 1, PUP_VOLUME_MAX);
}

float PUPScreen::EffectiveVolume(int volume) const
{
   const int scaled = std::clamp(static_cast<int>(std::lround(volume * m_volumeMultiplier / 100.0)), 1, 100);
   return m_mainVolume * static_cast<float>(scaled);
}

void PUPScreen::ApplyVolume(int volume)
{
   m_currentVolume = volume;
   m_pMediaPlayerManager->SetVolume(EffectiveVolume(volume));
}

void PUPScreen::SetMainVolume(float volume)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_mainVolume = volume;
   m_pMediaPlayerManager->SetVolume(EffectiveVolume(m_currentVolume));
}

void PUPScreen::SetVolumeMultiplier(int multiplier)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_volumeMultiplier = std::max(multiplier, 0);
   m_pMediaPlayerManager->SetVolume(EffectiveVolume(m_currentVolume));
}

void PUPScreen::SetDefaultVolume(int volume)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_defaultVolume = std::clamp(volume, 1, PUP_VOLUME_MAX);
}

void PUPScreen::SetVolumeCurrent(int volume)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_fadeVolume = -1;
   ApplyVolume(std::clamp(volume, 1, PUP_VOLUME_MAX));
}

void PUPScreen::StartVolumeFade(int from, int to, int step)
{
   m_fadeVolume = from;
   m_fadeTarget = to;
   m_volumeFadeStep = step;
   m_fadeNextTick = SDL_GetTicks();
}

void PUPScreen::UpdateVolumeFade()
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (m_fadeVolume < 0)
      return;
   const uint64_t now = SDL_GetTicks();
   if (now < m_fadeNextTick)
      return;
   m_fadeNextTick = now + PUP_VOLUME_FADE_INTERVAL_MS;
   if (m_fadeVolume > m_fadeTarget)
      m_fadeVolume = std::max(m_fadeTarget, m_fadeVolume - m_volumeFadeStep);
   else
      m_fadeVolume = std::min(m_fadeTarget, m_fadeVolume + m_volumeFadeStep);
   ApplyVolume(m_fadeVolume);
   if (m_fadeVolume == m_fadeTarget)
      m_fadeVolume = -1;
}

void PUPScreen::Duck(int percent)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (!m_pMediaPlayerManager->IsMainPlaying() && !m_pMediaPlayerManager->IsBackgroundPlaying())
      return;
   if (m_duckOrigVolume < 0)
      m_duckOrigVolume = m_currentVolume;
   const int target = std::clamp(static_cast<int>(std::lround(percent / 100.0 * m_duckOrigVolume)), 1, PUP_VOLUME_MAX);
   StartVolumeFade(m_currentVolume, target, 5);
}

void PUPScreen::Unduck()
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (m_duckOrigVolume < 0)
      return;
   const int orig = m_duckOrigVolume;
   m_duckOrigVolume = -1;
   if (m_pMediaPlayerManager->IsMainPlaying() || m_pMediaPlayerManager->IsBackgroundPlaying())
      StartVolumeFade(m_currentVolume, orig, 6);
   else
      ApplyVolume(orig);
}

void PUPScreen::OnMediaEnd()
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (m_hideDuringPlay)
   {
      m_hideDuringPlay = false;
      m_hudVisible = true;
   }
   // Resolve whatever LabelShowPage queued when a splash page started.
   const HudReturn action = m_hudReturn;
   m_hudReturn = HudReturn::None;
   if (action == HudReturn::RestoreHud)
   {
      SetPage(m_defaultPagenum, 0);
      if (PUPLabel* pBackground = GetLabel("pBackground"s); pBackground && !pBackground->IsAnimating())
         pBackground->SetVisible(true);
   }
   else if (action == HudReturn::RestorePage)
      SetPage(m_defaultPagenum, 0);

   m_fadeVolume = -1;
   m_duckOrigVolume = -1;
   if (m_duckMaster)
   {
      m_duckMaster = false;
      m_pManager->Unduck();
   }
}

void PUPScreen::AddChild(std::shared_ptr<PUPScreen> pScreen)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_children.push_back(pScreen);
   pScreen->m_pParent = this;
}

void PUPScreen::ReplaceChild(std::shared_ptr<PUPScreen> pChild, std::shared_ptr<PUPScreen> pScreen)
{
   assert(std::this_thread::get_id() == m_apiThread);
   for (size_t i = 0; i < m_children.size(); i++)
      if (m_children[i] == pChild)
         m_children[i] = pScreen;
   pChild->m_pParent = nullptr;
   pScreen->m_pParent = this;
}
   
void PUPScreen::AddPlaylist(PUPPlaylist* pPlaylist)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (!pPlaylist)
      return;

   m_playlistMap[lowerCase(PluginStrings::PathToUTF8(pPlaylist->GetFolder()))] = pPlaylist;
}

PUPPlaylist* PUPScreen::GetPlaylist(const string& szFolder)
{
   assert(std::this_thread::get_id() == m_apiThread);
   ankerl::unordered_dense::map<string, PUPPlaylist*>::const_iterator it = m_playlistMap.find(lowerCase(szFolder));
   return it != m_playlistMap.end() ? it->second : nullptr;
}

void PUPScreen::AddTrigger(PUPTrigger* pTrigger)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (!pTrigger)
      return;

   m_triggerMap[pTrigger->GetTrigger()].push_back(pTrigger);
}

void PUPScreen::LabelInit()
{
   assert(std::this_thread::get_id() == m_apiThread);
   // PinUP's label canvas always starts with a pBACKGROUND label drawn below the others.
   if (!GetLabel("pBackground"s))
   {
      PUPLabel* const pBackground = new PUPLabel(m_pManager, "pBackground"s, "Liberation Sans"s, 50.f, 0xFF, 0.f,
         PUP_LABEL_XALIGN_CENTER, PUP_LABEL_YALIGN_CENTER, 0.f, 0.f, -1, true);
      AddLabel(pBackground);
      SendLabelToBack(pBackground);
   }
}

void PUPScreen::AddLabel(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (GetLabel(pLabel->GetName())) {
      LOGE(std::format("Duplicate label: screen={{{}}}, label={}", ToString(false), pLabel->ToString()));
      delete pLabel;
      return;
   }

   pLabel->SetScreen(this);
   m_labelMap[lowerCase(pLabel->GetName())] = pLabel;
   m_labels.push_back(pLabel);
}

PUPLabel* PUPScreen::GetLabel(const string& szLabelName)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = m_labelMap.find(lowerCase(szLabelName));
   return it != m_labelMap.end() ? it->second : nullptr;
}

void PUPScreen::SendLabelToBack(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(m_labels.begin(), it, it + 1);
}

void PUPScreen::SendLabelToFront(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(it, it + 1, m_labels.end());
}

void PUPScreen::SendLabelToBackground(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it == m_labels.end())
      return;
   // PUP's zback value other than -1 puts the label just in front of the first
   // (background) label rather than behind everything.
   const size_t target = std::min<size_t>(1, m_labels.size() - 1);
   if (static_cast<size_t>(it - m_labels.begin()) == target)
      return;
   m_labels.erase(it);
   m_labels.insert(m_labels.begin() + target, pLabel);
}

void PUPScreen::RemoveLabel(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it == m_labels.end())
      return;
   m_labelMap.erase(lowerCase(pLabel->GetName()));
   m_labels.erase(it);
   delete pLabel;
}

void PUPScreen::SetPage(int pagenum, int durationMs, const string& special)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (m_hideDuringPlay)
      return;

   // Mirrors PinUpDisplayX TtextForm.SetPageNum: hidehudplay/returnplay arm an action for
   // the end of the current media, forceshow/forcehide only touch visibility, a same-page
   // call with nothing armed just clears animations and extends the timer, otherwise the
   // page switches, animations end, and labels on the new page get 'clear' / their default
   // visibility (pages above 1 only, and only when dodefault is on).
   const string szSpecial = lowerCase(special);
   if (szSpecial == "hidehudplay"s)
   {
      m_hudReturn = HudReturn::RestoreHud;
      if (PUPLabel* pBackground = GetLabel("pBackground"s); pBackground && !pBackground->IsAnimating())
         pBackground->SetVisible(false);
   }
   else if (szSpecial == "returnplay"s)
      m_hudReturn = HudReturn::RestorePage;
   else
      m_hudReturn = HudReturn::None;

   if (special == "forceshow"s || special == "forcehide"s)
   {
      if (pagenum >= 0)
         for (PUPLabel* pLabel : m_labels)
            if (pLabel->GetPagenum() == pagenum)
               pLabel->SetVisible(special == "forceshow"s && pLabel->GetOnShowVisible());
      return;
   }

   if (pagenum == m_pagenum && m_hudReturn == HudReturn::None)
   {
      for (PUPLabel* pLabel : m_labels)
         pLabel->ClearAnimation();
      if (m_pageExpiry && durationMs > 0)
         m_pageExpiry += static_cast<uint64_t>(durationMs);
      return;
   }

   m_pagenum = pagenum;
   for (PUPLabel* pLabel : m_labels)
   {
      pLabel->ClearAnimation();
      if (pLabel->GetPagenum() != pagenum)
         continue;
      if (szSpecial == "clear"s)
         pLabel->SetCaption(""s);
      if (pagenum > 1 && pLabel->GetDoDefault())
         pLabel->SetVisible(pLabel->GetOnShowVisible());
   }

   if (durationMs > 0)
      m_pageExpiry = SDL_GetTicks() + static_cast<uint64_t>(durationMs);
   else
   {
      m_pageExpiry = 0;
      if (pagenum >= 0)
         m_defaultPagenum = pagenum;
   }
}

void PUPScreen::UpdateTimers()
{
   assert(std::this_thread::get_id() == m_apiThread);
   const uint64_t now = SDL_GetTicks();
   if (m_pageExpiry && now >= m_pageExpiry)
   {
      m_pageExpiry = 0;
      SetPage(m_defaultPagenum, 0);
   }
   if (m_imageExpiry && now >= m_imageExpiry)
   {
      m_imageExpiry = 0;
      m_staticImage.Clear();
   }
}

void PUPScreen::SetBounds(int x, int y, int w, int h)
{
   assert(std::this_thread::get_id() == m_apiThread);
   SDL_Rect rect = m_pCustomPos ? m_pCustomPos->ScaledRect(w, h) : SDL_Rect { 0, 0, w, h };
   rect.x += x;
   rect.y += y;
   if (rect.x != m_rect.x || rect.y != m_rect.y || rect.w != m_rect.w || rect.h != m_rect.h)
   {
      m_rect = rect;
      m_pMediaPlayerManager->SetBounds(m_rect);
   }

   for (auto pChildren : m_children)
      pChildren->SetBounds(x, y, w, h);
}

void PUPScreen::SetCustomPos(const string& szCustomPos)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pCustomPos = PUPCustomPos::CreateFromCSV(szCustomPos);
}

void PUPScreen::SetGameTime(double gameTime) { m_pMediaPlayerManager->SetGameTime(gameTime); }

void PUPScreen::Update(double gameTime)
{
   assert(std::this_thread::get_id() == m_apiThread);
   UpdateTimers();
   UpdateVolumeFade();
   m_pMediaPlayerManager->SetGameTime(gameTime);
}

void PUPScreen::Play(const string& szPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, PlayAction action, int length)
{
   assert(std::this_thread::get_id() == m_apiThread);
   PUPPlaylist* const pPlaylist = GetPlaylist(szPlaylist);
   if (!pPlaylist)
   {
      LOGE(std::format("Playlist not found: screen={{{}}}, playlist={}", ToString(false), szPlaylist));
      return;
   }
   Play(pPlaylist, szPlayFile, volume, priority, action, length);
}

void PUPScreen::Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, PlayAction action, int length)
{
   assert(std::this_thread::get_id() == m_apiThread);
   //LOGD(std::format("play, screen={{{}}}, playlist={{{}}}, playFile={}, volume={:.0f}, priority={}", ToString(false), pPlaylist->ToString(), PluginStrings::PathToUTF8(szPlayFile), volume, priority));
   //StopMedia(); // Does it stop the played media on all request like overlays or alphas ? I don't think so but unsure
   const bool background = (action == PlayAction::SetBG);
   switch (pPlaylist->GetFunction())
   {
   case PUPPlaylist::Function::Default:
   {
      // PNGs/JPGs bypass FFmpeg — loaded as static image on the video layer.
      // Persists until replaced by a new image or video play.
      const string ext = extension_from_path(PluginStrings::PathToUTF8(szPlayFile.empty() ? pPlaylist->PeekNextPlayFile() : szPlayFile));
      if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
      {
         const std::filesystem::path imagePath = pPlaylist->GetPlayFilePath(szPlayFile);
         if (imagePath.empty())
         {
            LOGE(std::format("PlayFile not found: screen={{{}}}, playlist={{{}}}, playFile={}", ToString(false), pPlaylist->ToString(), PluginStrings::PathToUTF8(szPlayFile)));
            break;
         }
         if (background)
         {
            m_background.Load(imagePath);
            m_pMediaPlayerManager->StopBackground();
         }
         else
         {
            Stop();
            m_staticImage.Load(imagePath);
            m_imageExpiry = (length > 0) ? (SDL_GetTicks() + static_cast<uint64_t>(length) * 1000) : 0;
         }
         break;
      }
      switch (m_mode)
      {
      case Mode::Off:
         return;

      case Mode::ForceBack:
         // Don't send to back: this is only done on creation
         break;

      case Mode::ForcePopBack:
         m_pManager->SendScreenToBack(this);
         break;

      case Mode::ForceOn:
         // Don't send to front: this is only done on creation
         break;

      case Mode::ForcePop:
         m_pManager->SendScreenToFront(this);
         break;

      case Mode::MusicOnly:
         // No window ordering as we are only playing music
         break;

      case Mode::Show:
         break;
      }
      if (const int vol = ResolveVolume(volume, pPlaylist); m_pMediaPlayerManager->Play(pPlaylist, szPlayFile, EffectiveVolume(vol), priority, action, length))
      {
         m_staticImage.Clear();
         m_currentVolume = vol;
      }
      break;
   }

   case PUPPlaylist::Function::Frames:
      m_background.Load(pPlaylist->GetPlayFilePath(szPlayFile));
      break;

   case PUPPlaylist::Function::Overlays:
   case PUPPlaylist::Function::Alphas:
      // PinUP routes both through assignAlphaFramePNG: the file becomes the pBackground
      // label's image, stretched over the label canvas, so it follows pBackground's
      // visibility (pDMDSetHUD, hidehudplay) and sits below the other labels.
      LabelInit();
      if (PUPLabel* pBackground = GetLabel("pBackground"s))
      {
         pBackground->SetCaption(PluginStrings::PathToUTF8(pPlaylist->GetFolder() / szPlayFile));
         pBackground->SetSpecial("{\"mt\":2, \"width\":100, \"height\":100, \"xpos\":50, \"ypos\":50}"s);
      }
      break;

   case PUPPlaylist::Function::Shapes:
      SetMask(pPlaylist->GetPlayFilePath(szPlayFile));
      break;

   default:
      LOGE("Invalid playlist function: " + PUPPlaylist::ToString(pPlaylist->GetFunction()));
      break;
   }
}

void PUPScreen::SetMask(const std::filesystem::path& path)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetMask(path);
}

void PUPScreen::Stop()
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Stop();
}

void PUPScreen::Stop(int priority)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Stop(priority);
}

void PUPScreen::Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Stop(pPlaylist, szPlayFile);
}

void PUPScreen::Pause()
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Pause();
}

void PUPScreen::Resume()
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Resume();
}

void PUPScreen::SetLoop(int state)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetLoop(state != 0);
}

void PUPScreen::SetLength(int seconds)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetLengthFromNow(seconds);
   if (seconds > 0 && !m_staticImage.GetFile().empty())
      m_imageExpiry = SDL_GetTicks() + static_cast<uint64_t>(seconds) * 1000;
}

void PUPScreen::SetAsBackGround(int mode)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetAsBackGround(mode != 0);
}

void PUPScreen::SetFadeStep(int step)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetFadeStep(step);
}

bool PUPScreen::IsMainPlaying() const {
   assert(std::this_thread::get_id() == m_apiThread);
   return m_pMediaPlayerManager->IsMainPlaying();
}

bool PUPScreen::IsBackgroundPlaying() const
{
   assert(std::this_thread::get_id() == m_apiThread);
   return m_pMediaPlayerManager->IsBackgroundPlaying();
}

void PUPScreen::Render(VPXRenderContext2D* const ctx, int pass) {
   assert(std::this_thread::get_id() == m_apiThread);

   // ForcePop / ForcePopBack only affect z-order (topmost), handled by the render-order tiers
   // in PUPManager::Render. The screen still renders its background, overlay, labels and static
   // image when no main video is playing, so packs that drive a screen purely via background
   // videos, overlays, or labels stay visible.
   if (m_mode == Mode::Off || m_mode == Mode::MusicOnly || m_hidden)
      return;

   switch (pass)
   {
   case 0: m_background.Render(ctx, m_rect, m_screenAlpha); break;

   case 1:
      m_pMediaPlayerManager->Render(ctx, m_screenAlpha);
      if (!m_pMediaPlayerManager->IsMainPlaying() && !m_pMediaPlayerManager->IsBackgroundPlaying())
         m_staticImage.Render(ctx, m_rect, m_screenAlpha);
      break;

   case 3:
      if (!m_hudVisible)
         break;
      // PUP draws the animation copies after all regular labels, so an animated
      // label overlays the rest of the page regardless of its creation order.
      for (PUPLabel* pLabel : m_labels)
         if (!pLabel->IsAnimating())
            pLabel->Render(ctx, m_rect, m_pagenum, m_screenAlpha * m_overlayAlpha);
      for (PUPLabel* pLabel : m_labels)
         if (pLabel->IsAnimating())
            pLabel->Render(ctx, m_rect, m_pagenum, m_screenAlpha * m_overlayAlpha);
      break;
   }
}

string PUPScreen::ToString(bool full) const
{
   if (full) {
      return "mode=" + ToString(m_mode) +
         ", screenNum=" + std::to_string(m_screenNum) +
         ", screenDes=" + m_screenDes +
         ", transparent=" + (m_transparent ? "true" : "false") +
         ", volumeMultiplier=" + std::to_string(m_volumeMultiplier) +
         ", defaultVolume=" + std::to_string(m_defaultVolume) +
         ", m_customPos={" + (m_pCustomPos ? m_pCustomPos->ToString() : ""s) + '}';
   }

   return "screenNum=" + std::to_string(m_screenNum) +
      ", screenDes=" + m_screenDes +
      ", mode=" + ToString(m_mode);
}

const string& PUPScreen::ToString(Mode value)
{
   static const string modeStrings[] = { "Off"s, "Show"s, "ForceOn"s, "ForcePop"s, "ForceBack"s, "ForcePopBack"s, "MusicOnly"s };
   static const string error = "Unknown"s;
   if ((int)value < 0 || (size_t)value >= std::size(modeStrings))
      return error;
   return modeStrings[(int)value];
}

}
