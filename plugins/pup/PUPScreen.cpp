#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "PUPTrigger.h"
#include "PUPManager.h"
#include "PUPPlaylist.h"
#include "PUPLabel.h"
#include "PUPMediaManager.h"

#include <SDL3_image/SDL_image.h>

namespace PUP {

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

PUPScreen::PUPScreen(PUPManager* manager, PUPScreen::Mode mode, int screenNum, const string& szScreenDes, bool transparent, float volume, std::unique_ptr<PUPCustomPos> pCustomPos, const std::vector<PUPPlaylist*>& playlists)
   : m_pManager(manager)
   , m_screenNum(screenNum)
   , m_mode(mode)
   , m_screenDes(szScreenDes)
   , m_transparent(transparent)
   , m_volume(volume)
   , m_pCustomPos(std::move(pCustomPos))
   , m_commandQueue(1)
{
   memset(&m_background, 0, sizeof(m_background));
   memset(&m_overlay, 0, sizeof(m_overlay));
   m_pMediaPlayerManager = std::make_unique<PUPMediaManager>(this);
   m_commandQueue.enqueue([this]() { SetThreadName("PUPScreen#"s.append(std::to_string(m_screenNum)).append(".CommandQueue")); }); 

   for (const PUPPlaylist* pPlaylist : playlists) {
      // make a copy of the playlist
      PUPPlaylist *pPlaylistCopy = new PUPPlaylist(*pPlaylist);
      AddPlaylist(pPlaylistCopy);
   }

   LoadTriggers();
}

PUPScreen::~PUPScreen()
{
   m_commandQueue.wait_until_empty();
   m_commandQueue.wait_until_nothing_in_flight();
   
   if (m_pageTimer)
      SDL_RemoveTimer(m_pageTimer);

   for (auto& [key, pPlaylist] : m_playlistMap)
      delete pPlaylist;

   for (auto& [key, pTriggers] : m_triggerMap) {
      for (PUPTrigger* pTrigger : pTriggers)
         delete pTrigger;
   }

   for (PUPLabel* pLabel : m_labels)
      delete pLabel;

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren })
      pChildren->clear();
}

std::unique_ptr<PUPScreen> PUPScreen::CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 8) {
      LOGE("Failed to parse screen line, expected 8 columns but got %d: %s", parts.size(), line.c_str());
      return nullptr;
   }

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
   else if (StrCompareNoCase(parts[5], "MusicOnly"s))
      mode = PUPScreen::Mode::MusicOnly;
   else if (StrCompareNoCase(parts[5], "Off"s))
      mode = PUPScreen::Mode::Off;
   else {
      LOGE("Invalid screen mode: %s", parts[5].c_str());
      mode = PUPScreen::Mode::Off;
   }

   auto screen = std::make_unique<PUPScreen>(
      manager,
      mode,
      string_to_int(parts[0], 0), // screenNum
      parts[1], // screenDes
      parts[4] == "1", // transparent
      string_to_float(parts[6], 100.0f), // volume
      PUPCustomPos::CreateFromCSV(parts[7]), playlists);

   // Optional initial background playlist
   if (!parts[2].empty())
   {
      screen->QueuePlay(parts[2], parts[3], screen->GetVolume(), -1);
      screen->QueueBG(true);
   }

   return screen;
}

std::unique_ptr<PUPScreen> PUPScreen::CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists)
{
   if (manager->HasScreen(screenNum)) {
      LOGE("Screen already exists: screenNum=%d", screenNum);
      return nullptr;
   }
   switch(screenNum) {
   case PUP_SCREEN_TOPPER: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_TOPPER, "Topper"s, false, 100.0f, nullptr, playlists);
   case PUP_SCREEN_DMD: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_DMD, "DMD"s, false, 100.0f, nullptr, playlists);
   case PUP_SCREEN_BACKGLASS: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_BACKGLASS, "Backglass"s, false, 100.0f, nullptr, playlists);
   case PUP_SCREEN_PLAYFIELD: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_PLAYFIELD, "Playfield"s, false, 100.0f, nullptr, playlists);
   default: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, screenNum, "Unknown"s, false, 100.0f, nullptr, playlists);
   }
}

void PUPScreen::LoadTriggers()
{
   string szPlaylistsPath = find_case_insensitive_file_path(m_pManager->GetPath() + "triggers.pup");
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
         AddTrigger(PUPTrigger::CreateFromCSV(this, line));
      }
   }
}

void PUPScreen::SetVolume(float volume)
{
   m_volume = volume;
   m_pMediaPlayerManager->SetVolume(volume);
}

void PUPScreen::SetVolumeCurrent(float volume)
{
   m_pMediaPlayerManager->SetVolume(volume);
}

void PUPScreen::AddChild(std::shared_ptr<PUPScreen> pScreen)
{
   switch (pScreen->GetMode()) {
      case PUPScreen::Mode::ForceOn:
      case PUPScreen::Mode::ForcePop:
         m_topChildren.push_back(pScreen);
         break;
      case PUPScreen::Mode::ForceBack:
      case PUPScreen::Mode::ForcePopBack:
         m_backChildren.push_back(pScreen);
         break;
      default:
          m_defaultChildren.push_back(pScreen);
   }
   pScreen->m_pParent = this;
}

void PUPScreen::SendToFront()
{
   if (m_pParent) {
      if (m_mode == PUPScreen::Mode::ForceOn || m_mode == PUPScreen::Mode::ForcePop) {
         auto it = std::ranges::find_if(m_pParent->m_topChildren, [this](std::shared_ptr<PUPScreen> a) { return a.get() == this; });
         if (it != m_pParent->m_topChildren.end())
            std::rotate(it, it + 1, m_pParent->m_topChildren.end());
      }
      else if (m_mode == PUPScreen::Mode::ForceBack || m_mode == PUPScreen::Mode::ForcePopBack) {
         auto it = std::ranges::find_if(m_pParent->m_backChildren, [this](std::shared_ptr<PUPScreen> a) { return a.get() == this; });
         if (it != m_pParent->m_backChildren.end())
            std::rotate(it, it + 1, m_pParent->m_backChildren.end());
      }
   }
}

void PUPScreen::AddPlaylist(PUPPlaylist* pPlaylist)
{
   if (!pPlaylist)
      return;

   m_playlistMap[lowerCase(pPlaylist->GetFolder())] = pPlaylist;
}

PUPPlaylist* PUPScreen::GetPlaylist(const string& szFolder)
{
   ankerl::unordered_dense::map<string, PUPPlaylist*>::const_iterator it = m_playlistMap.find(lowerCase(szFolder));
   return it != m_playlistMap.end() ? it->second : nullptr;
}

void PUPScreen::AddTrigger(PUPTrigger* pTrigger)
{
   if (!pTrigger)
      return;

   m_triggerMap[pTrigger->GetTrigger()].push_back(pTrigger);
}

vector<PUPTrigger*>* PUPScreen::GetTriggers(const string& szTrigger)
{
   ankerl::unordered_dense::map<string, vector<PUPTrigger*>>::iterator it = m_triggerMap.find(szTrigger);
   return it != m_triggerMap.end() ? &it->second : nullptr;
}

void PUPScreen::AddLabel(PUPLabel* pLabel)
{
   if (GetLabel(pLabel->GetName())) {
      LOGE("Duplicate label: screen={%s}, label=%s", ToString(false).c_str(), pLabel->ToString().c_str());
      delete pLabel;
      return;
   }

   pLabel->SetScreen(this);
   m_labelMap[lowerCase(pLabel->GetName())] = pLabel;
   m_labels.push_back(pLabel);
}

PUPLabel* PUPScreen::GetLabel(const string& szLabelName)
{
   auto it = m_labelMap.find(lowerCase(szLabelName));
   return it != m_labelMap.end() ? it->second : nullptr;
}

void PUPScreen::SendLabelToBack(PUPLabel* pLabel)
{
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(m_labels.begin(), it, it + 1);
}

void PUPScreen::SendLabelToFront(PUPLabel* pLabel)
{
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(it, it + 1, m_labels.end());
}

void PUPScreen::SetPage(int pagenum, int seconds)
{
   std::lock_guard<std::mutex> lock(m_renderMutex);
   if (m_pageTimer)
      SDL_RemoveTimer(m_pageTimer);
   m_pageTimer = 0;
   m_pagenum = pagenum;

   if (seconds == 0)
      m_defaultPagenum = pagenum;
   else
      m_pageTimer = SDL_AddTimer(seconds * 1000, PageTimerElapsed, this);
}

uint32_t PUPScreen::PageTimerElapsed(void* param, SDL_TimerID timerID, uint32_t interval)
{
   PUPScreen* me = static_cast<PUPScreen*>(param);
   std::lock_guard<std::mutex> lock(me->m_renderMutex);
   SDL_RemoveTimer(me->m_pageTimer);
   me->m_pageTimer = 0;
   me->m_pagenum = me->m_defaultPagenum;
   return interval;
}

void PUPScreen::SetSize(int w, int h)
{
   if (m_pCustomPos)
      m_rect = m_pCustomPos->ScaledRect(w, h);
   else
      m_rect = { 0, 0, w, h };

   m_pMediaPlayerManager->SetBounds(m_rect);

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren }) {
      for (auto pScreen : *pChildren)
          pScreen->SetSize(w, h);
   }
}

void PUPScreen::SetCustomPos(const string& szCustomPos)
{
   std::lock_guard<std::mutex> lock(m_renderMutex);
   m_pCustomPos = PUPCustomPos::CreateFromCSV(szCustomPos);
}

void PUPScreen::Play(PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, bool skipSamePriority, int length)
{
   //StopMedia(); // Does it stop the played media on all request like overlays or alphas ? I don't think so but unsure
   switch (pPlaylist->GetFunction())
   {
   case PUPPlaylist::Function::Default:
   {
      std::lock_guard<std::mutex> lock(m_renderMutex);
      m_pMediaPlayerManager->Play(pPlaylist, szPlayFile, m_pParent ? (volume / 100.0f) * m_pParent->GetVolume() : volume, priority, skipSamePriority, length);
   }
   break;

   case PUPPlaylist::Function::Frames:
   {
      std::lock_guard<std::mutex> lock(m_renderMutex);
      m_background.Load(pPlaylist->GetPlayFilePath(szPlayFile));
   }
   break;

   case PUPPlaylist::Function::Overlays:
   case PUPPlaylist::Function::Alphas:
   {
      std::lock_guard<std::mutex> lock(m_renderMutex);
      m_overlay.Load(pPlaylist->GetPlayFilePath(szPlayFile));
   }
   break;

   case PUPPlaylist::Function::Shapes: SetMask(pPlaylist->GetPlayFilePath(szPlayFile)); break;

   default: LOGE("Playlist function not implemented: %s", PUPPlaylist::ToString(pPlaylist->GetFunction()).c_str()); break;
   }
}

void PUPScreen::StopMedia()
{
   std::lock_guard<std::mutex> lock(m_renderMutex);
   m_pMediaPlayerManager->Stop();
}

void PUPScreen::SetMask(const string& path)
{
   // Defines a transparency mask from the pixel at 0,0 that is applied to the rendering inside this screen
   m_mask.reset();
   m_mask = std::shared_ptr<SDL_Surface>(IMG_Load(path.c_str()), SDL_DestroySurface);
   if (m_mask && m_mask->format != SDL_PIXELFORMAT_RGBA32)
      m_mask = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(m_mask.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
   if (m_mask)
   {
      SDL_LockSurface(m_mask.get());
      uint32_t* __restrict rgba = static_cast<uint32_t*>(m_mask->pixels);
      const uint32_t maskValue = rgba[0];
      for (int i = 0; i < m_mask->h; i++, rgba += (m_mask->pitch - m_mask->w * sizeof(uint32_t)))
         for (int j = 0; j < m_mask->w; j++, rgba++)
            *rgba = (*rgba == maskValue) ? 0x00000000u : 0xFFFFFFFFu;
      SDL_UnlockSurface(m_mask.get());
      m_pMediaPlayerManager->SetMask(m_mask);
   }
}

void PUPScreen::QueuePlay(const string& szPlaylist, const string& szPlayFile, float volume, int priority)
{
   PUPPlaylist* const pPlaylist = GetPlaylist(szPlaylist);
   if (!pPlaylist) {
      LOGE("Playlist not found: screen={%s}, playlist=%s", ToString(false).c_str(), szPlaylist.c_str());
      return;
   }
   QueuePlay(pPlaylist, szPlayFile, volume, priority, false, 0);
}

void PUPScreen::QueuePlay(PUPPlaylist* playlist, const string& szPlayFile, float volume, int priority, bool skipSamePriority, int length)
{
   LOGD("queueing play, screen={%s}, playlist={%s}, playFile=%s, volume=%.f, priority=%d", ToString(false).c_str(), playlist->ToString().c_str(), szPlayFile.c_str(), volume, priority);
   m_commandQueue.enqueue([this, playlist, szPlayFile, volume, priority, skipSamePriority, length]() { Play(playlist, szPlayFile, volume, priority, skipSamePriority, length); });
}

void PUPScreen::QueueStop(int priority)
{
   m_commandQueue.enqueue(
      [this, priority]()
      {
         std::lock_guard<std::mutex> lock(m_renderMutex);
         m_pMediaPlayerManager->Stop(priority);
      });
}

void PUPScreen::QueueStop(PUPPlaylist* pPlaylist, const std::string& szPlayFile)
{
   m_commandQueue.enqueue(
      [this, pPlaylist, szPlayFile]()
      {
         std::lock_guard<std::mutex> lock(m_renderMutex);
         m_pMediaPlayerManager->Stop(pPlaylist, szPlayFile);
      });
}

void PUPScreen::QueuePause()
{
   m_commandQueue.enqueue([this]() { m_pMediaPlayerManager->Pause(); });
}

void PUPScreen::QueueResume() {
   m_commandQueue.enqueue([this]() { m_pMediaPlayerManager->Resume(); });
}

void PUPScreen::QueueStop() {
   m_commandQueue.enqueue([this]() { m_pMediaPlayerManager->Stop(); });
}

void PUPScreen::QueueLoop(int state) {
   m_commandQueue.enqueue([this, state]() { m_pMediaPlayerManager->SetLoop(state != 0); });
}

void PUPScreen::QueueBG(int mode) {
   m_commandQueue.enqueue([this, mode]() { m_pMediaPlayerManager->SetBG(mode != 0); });
}

void PUPScreen::Render(VPXRenderContext2D* const ctx)
{
   std::lock_guard<std::mutex> lock(m_renderMutex);

   #if 0
   LOGD("Render #%d %s   Mask:%d", m_screenNum, m_screenDes.c_str(), m_mask != nullptr);
   LOGD(". Underlay: %s", m_background.GetFile());
   LOGD(". Back video: %s", m_pMediaPlayerManager->GetBackgroundPlayer() ? m_pMediaPlayerManager->GetBackgroundPlayer()->szPath.c_str() : "");
   LOGD(". Main video: %s", m_pMediaPlayerManager->GetMainPlayer() ? m_pMediaPlayerManager->GetMainPlayer()->szPath.c_str() : "");
   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren })
      for (auto pScreen : *pChildren)
         LOGD(". Children: #%d %s", pScreen->GetScreenNum(), pScreen->GetScreenDes().c_str());
   LOGD(". Overlay: %s", m_overlay.GetFile());
   #endif

   m_background.Render(ctx, m_rect);

   m_pMediaPlayerManager->Render(ctx);

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren }) {
      for (auto pScreen : *pChildren)
         pScreen->Render(ctx);
   }

   m_overlay.Render(ctx, m_rect);

   // FIXME port SDL_SetRenderClipRect(m_pRenderer, &m_rect);

   for (PUPLabel* pLabel : m_labels)
      pLabel->Render(ctx, m_rect, m_pagenum);

   // FIXME port SDL_SetRenderClipRect(m_pRenderer, NULL);
}

string PUPScreen::ToString(bool full) const
{
   if (full) {
      return "mode=" + ToString(m_mode) +
         ", screenNum=" + std::to_string(m_screenNum) +
         ", screenDes=" + m_screenDes +
         ", transparent=" + (m_transparent ? "true" : "false") +
         ", volume=" + std::to_string(m_volume) +
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
