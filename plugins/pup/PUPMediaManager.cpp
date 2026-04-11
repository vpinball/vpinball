// license:GPLv3+

#include "PUPMediaManager.h"
#include "PUPScreen.h"

#include <SDL3_image/SDL_image.h>

namespace PUP {

PUPMediaManager::PUPMediaManager(PUPScreen* pScreen)
   : m_player("PUP.#" + std::to_string(pScreen->GetScreenNum()))
   , m_pScreen(pScreen)
   , m_bounds()
{
   m_player.SetOnEndCallback([this](PUPMediaPlayer* player) { OnPlayerEnd(player); });
}

PUPMediaManager::~PUPMediaManager()
{
   m_shuttingDown = true;
}

void PUPMediaManager::SetGameTime(double gameTime)
{
   m_player.SetGameTime(gameTime);
}

void PUPMediaManager::Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, bool skipSamePriority, int length, bool background)
{
   if (!background && skipSamePriority && IsMainPlaying() && priority <= m_mainPriority)
   {
      LOGE(std::format("Skipping same priority, screen={{{}}}, playlist={}, playFile={{{}}}, priority={}", m_pScreen->ToString(false), pPlaylist->ToString(), szPlayFile.string(), priority));
      return;
   }

   std::filesystem::path szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (szPath.empty()) {
      LOGE(std::format("PlayFile not found: screen={{{}}}, playlist={{{}}}, playFile={}", m_pScreen->ToString(false), pPlaylist->ToString(), szPlayFile.string()));
      return;
   }

   // Single player per screen — background config stores what to play when main ends.
   if (background)
   {
      LOGD(std::format("BG CONFIG: screen={}, file={}", m_pScreen->GetScreenNum(), szPath.filename().string()));
      m_bg.szPath = szPath;
      m_bg.volume = volume;
      m_bg.active = true;
      if (!m_playingMain)
         PlayBackground();
   }
   else
   {
      if (szPath == m_mainPath && m_player.IsPlaying() && m_playingMain)
      {
         LOGD(std::format("MAIN SKIP (same file): screen={}, file={}", m_pScreen->GetScreenNum(), szPath.filename().string()));
         m_player.SetVolume(volume);
      }
      else if (szPath == m_bg.szPath && m_bg.active && m_player.IsPlaying() && !m_playingMain)
      {
         LOGD(std::format("MAIN SKIP (bg has file): screen={}, file={}", m_pScreen->GetScreenNum(), szPath.filename().string()));
      }
      else if (m_playingMain && priority < m_mainPriority)
      {
         // Lower priority than current — queue it
         LOGD(std::format("QUEUED: screen={}, pri={}, file={}", m_pScreen->GetScreenNum(), priority, szPath.filename().string()));
         m_playQueue.push_back({ szPath, volume, priority, length, 0 });
      }
      else
      {
         PlayImmediate(szPath, volume, priority, length);
      }
   }
}

void PUPMediaManager::PlayImmediate(const std::filesystem::path& szPath, float volume, int priority, int length)
{
   LOGD(std::format("MAIN PLAY: screen={}, vol={:.0f}, pri={}, len={}, file={}", m_pScreen->GetScreenNum(), volume, priority, length, szPath.filename().string()));
   m_playingMain = true;
   m_player.Play(szPath, volume);
   m_player.SetLength(length);
   m_mainPath = szPath;
   m_mainVolume = volume;
   m_mainPriority = priority;
   m_playQueue.clear();
}

// When main video ends, background config is loaded into the same player with loop enabled.
void PUPMediaManager::PlayBackground()
{
   if (m_bg.active && !m_bg.szPath.empty())
   {
      LOGD(std::format("BG PLAY: screen={}, file={}", m_pScreen->GetScreenNum(), m_bg.szPath.filename().string()));
      m_player.Play(m_bg.szPath, m_bg.volume);
      m_player.SetLoop(true);
      m_playingMain = false;
      m_mainPath.clear();
   }
}

void PUPMediaManager::PlayNextFromQueue()
{
   uint64_t now = SDL_GetTicks();
   while (!m_playQueue.empty())
   {
      auto item = m_playQueue.front();
      m_playQueue.pop_front();
      if (item.expiry > 0 && now > item.expiry)
         continue;
      PlayImmediate(item.szPath, item.volume, item.priority, item.length);
      return;
   }
   PlayBackground();
}

void PUPMediaManager::Pause()
{
   m_player.Pause(true);
}

void PUPMediaManager::Resume()
{
   m_player.Pause(false);
}

// See pDMDBackLoopStart — mode=1 saves current file as background config and loops it.
// mode=0 clears only if set via SetBackGround (not trigger SetBG). Popup screens are excluded.
void PUPMediaManager::SetAsBackGround(bool isBackground)
{
   if (isBackground) {
      if (m_pScreen->IsPop())
         return;
      if (m_playingMain && !m_mainPath.empty())
      {
         m_bg.szPath = m_mainPath;
         m_bg.volume = m_mainVolume;
         m_bg.active = true;
         m_bg.setViaSetBackGround = true;
         m_player.SetLoop(true);
         m_playingMain = false;
         m_mainPath.clear();
      }
   }
   else {
      if (m_bg.setViaSetBackGround)
      {
         m_bg.szPath.clear();
         m_bg.active = false;
         m_bg.setViaSetBackGround = false;
         if (!m_playingMain)
         {
            m_player.Stop();
         }
      }
   }
}

void PUPMediaManager::SetLoop(bool isLoop)
{
   m_player.SetLoop(isLoop);
}

void PUPMediaManager::SetMaxLength(int length)
{
   m_player.SetLength(length);
}

void PUPMediaManager::SetVolume(float volume)
{
   m_player.SetVolume(volume);
}

// See pDMDStopBackLoop — stops main, starts background if configured
void PUPMediaManager::Stop()
{
   LOGD(std::format("STOP: screen={}, wasMain={}, bgActive={}", m_pScreen->GetScreenNum(), m_playingMain, m_bg.active));
   m_playingMain = false;
   m_mainPath.clear();
   m_playQueue.clear();
   if (m_bg.active && !m_bg.szPath.empty())
      PlayBackground();
   else
      m_player.Stop();
}

void PUPMediaManager::StopBackground()
{
   m_bg.szPath.clear();
   m_bg.active = false;
   if (!m_playingMain)
      m_player.Stop();
}

void PUPMediaManager::Stop(int priority)
{
   if (priority > m_mainPriority) {
      LOGD(std::format("Priority > main player priority: screen={{{}}}, priority={}", m_pScreen->ToString(false), priority));
      Stop();
   }
   else {
      LOGD(std::format("Priority <= main player priority: screen={{{}}}, priority={}", m_pScreen->ToString(false), priority));
   }
}

void PUPMediaManager::Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile)
{
   std::filesystem::path szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (!szPath.empty() && szPath == m_mainPath) {
      LOGD(std::format("Main player stopping playback: screen={{{}}}, path={}", m_pScreen->ToString(false), szPath.string()));
      Stop();
   }
   else {
      LOGD(std::format("Main player playback stop requested but currently not playing: screen={{{}}}, path={}", m_pScreen->ToString(false), szPath.string()));
   }
}

void PUPMediaManager::SetBounds(const SDL_Rect& rect)
{
   m_bounds = rect;
   m_player.SetBounds(rect);
}

void PUPMediaManager::SetMask(const std::filesystem::path& path)
{
   // Defines a transparency mask from the pixel at 0,0 that is applied to the rendering inside this screen
   m_mask.reset();
   m_mask = std::shared_ptr<SDL_Surface>(IMG_Load(path.string().c_str()), SDL_DestroySurface);
   if (m_mask && m_mask->format != SDL_PIXELFORMAT_RGBA32)
      m_mask = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(m_mask.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
   if (m_mask)
   {
      SDL_LockSurface(m_mask.get());
      uint32_t* __restrict rgba = static_cast<uint32_t*>(m_mask->pixels);
      const uint32_t maskValue = rgba[0];
      const int rowPadding = (m_mask->pitch / static_cast<int>(sizeof(uint32_t))) - m_mask->w;
      for (int i = 0; i < m_mask->h; i++, rgba += rowPadding)
         for (int j = 0; j < m_mask->w; j++, rgba++)
            *rgba = (*rgba == maskValue) ? 0x00000000u : 0xFFFFFFFFu;
      SDL_UnlockSurface(m_mask.get());
   }
   m_player.SetMask(m_mask);
}

// When main video ends, check queue then fall back to background.
void PUPMediaManager::OnPlayerEnd(PUPMediaPlayer* player)
{
   if (m_shuttingDown)
      return;
   bool wasMain = m_playingMain;
   LOGD(std::format("ON END: screen={}, wasMain={}, mainFile={}, bgActive={}", m_pScreen->GetScreenNum(), wasMain, m_mainPath.filename().string(), m_bg.active));
   if (m_playingMain)
   {
      m_playingMain = false;
      m_mainPath.clear();
   }
   if (wasMain)
   {
      if (m_onMainEndCallback)
         m_onMainEndCallback();
      PlayNextFromQueue();
   }
}

bool PUPMediaManager::IsMainPlaying() { return m_player.IsPlaying(); }

bool PUPMediaManager::IsBackgroundPlaying() { return m_bg.active && !m_bg.szPath.empty() && !m_playingMain && m_player.IsPlaying(); }

void PUPMediaManager::Render(VPXRenderContext2D* const ctx)
{
   if (m_player.IsPlaying())
      m_player.Render(ctx, m_bounds);
}

}
