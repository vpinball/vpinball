// license:GPLv3+

#include "PUPMediaManager.h"
#include "PUPScreen.h"

#include <SDL3_image/SDL_image.h>
#include <algorithm>

namespace PUP {

PUPMediaManager::PUPMediaManager(PUPScreen* pScreen)
   : m_player("PUP.#" + std::to_string(pScreen->GetScreenNum()))
   , m_pScreen(pScreen)
   , m_bounds()
{
}

PUPMediaManager::~PUPMediaManager() = default;

void PUPMediaManager::SetGameTime(double gameTime)
{
   m_player.SetGameTime(gameTime);
   if (!m_queue.empty() && !m_player.IsPlaying())
      OnPlayerEnd();
}

void PUPMediaManager::Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, PlayAction action, int length)
{
   const bool isBg = (action == PlayAction::SetBG);
   const bool mainPlaying = !m_queue.empty();
   const int currentPriority = mainPlaying ? m_queue.front().priority : 0;

   if (!isBg && priority == 999)
   {
      if (mainPlaying)
         Stop();
      return;
   }

   if (!isBg && mainPlaying && priority < currentPriority)
   {
      LOGD(std::format("Dropping lower priority: screen={}, current={}, new={}, file={}", m_pScreen->GetScreenNum(), currentPriority, priority, szPlayFile.string()));
      return;
   }

   if (action == PlayAction::SkipSamePriority && mainPlaying && priority > 0 && priority == currentPriority)
   {
      LOGW(std::format("Skipping same priority, screen={{{}}}, playlist={}, playFile={{{}}}, priority={}", m_pScreen->ToString(false), pPlaylist->ToString(), szPlayFile.string(), priority));
      return;
   }

   const bool preempting = (priority > 0) && (priority > currentPriority);
   if (action == PlayAction::Normal && !preempting && pPlaylist->IsResting())
   {
      LOGD(std::format("Resting playlist, skipping: screen={}, playlist={}, restSeconds={}", m_pScreen->GetScreenNum(), pPlaylist->GetFolder().string(), pPlaylist->GetRestSeconds()));
      return;
   }

   std::filesystem::path szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (szPath.empty()) {
      LOGE(std::format("PlayFile not found: screen={{{}}}, playlist={{{}}}, playFile={}", m_pScreen->ToString(false), pPlaylist->ToString(), szPlayFile.string()));
      return;
   }

   if (isBg)
   {
      LOGD(std::format("BG CONFIG: screen={}, file={}", m_pScreen->GetScreenNum(), szPath.filename().string()));
      m_bg.szPath = szPath;
      m_bg.volume = volume;
      m_bg.active = true;
      pPlaylist->MarkPlayed();
      if (m_queue.empty())
         PlayBackground();
      return;
   }

   if (mainPlaying && m_queue.front().szPath == szPath)
   {
      LOGD(std::format("MAIN SKIP (same file): screen={}, file={}", m_pScreen->GetScreenNum(), szPath.filename().string()));
      m_player.SetVolume(volume);
      pPlaylist->MarkPlayed();
      return;
   }
   if (!mainPlaying && szPath == m_bg.szPath && m_bg.active && m_player.IsPlaying())
   {
      LOGD(std::format("MAIN SKIP (bg has file): screen={}, file={}", m_pScreen->GetScreenNum(), szPath.filename().string()));
      pPlaylist->MarkPlayed();
      return;
   }

   const PlayItem item{szPath, volume, priority, length, action == PlayAction::Loop};

   // SplashReset/SplashReturn push at front without popping; the displaced item resumes when the splash ends.
   // SplashReturn would seek-resume the displaced item, but the player doesn't expose seek so for now
   // both restart the displaced item from the beginning.
   const bool splash = (action == PlayAction::SplashReset || action == PlayAction::SplashReturn);
   if (!splash && !m_queue.empty())
      m_queue.pop_front();
   m_queue.push_front(item);
   StartCurrent();

   pPlaylist->MarkPlayed();
}

void PUPMediaManager::StartCurrent()
{
   if (m_queue.empty())
      return;
   const PlayItem& head = m_queue.front();
   LOGD(std::format("MAIN PLAY: screen={}, vol={:.0f}, pri={}, len={}, file={}", m_pScreen->GetScreenNum(), head.volume, head.priority, head.length, head.szPath.filename().string()));
   m_player.Play(head.szPath, head.volume);
   m_player.SetLength(head.length);
   m_player.SetLoop(head.loop);
   m_currentAlpha = (m_fadeStep < 255) ? m_fadeStep : 255;
}

void PUPMediaManager::PlayBackground()
{
   if (m_bg.active && !m_bg.szPath.empty())
   {
      LOGD(std::format("BG PLAY: screen={}, file={}", m_pScreen->GetScreenNum(), m_bg.szPath.filename().string()));
      m_player.Play(m_bg.szPath, m_bg.volume);
      m_player.SetLoop(true);
   }
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
      if (!m_queue.empty())
      {
         const PlayItem& head = m_queue.front();
         m_bg.szPath = head.szPath;
         m_bg.volume = head.volume;
         m_bg.active = true;
         m_bg.setViaSetBackGround = true;
         m_player.SetLoop(true);
         m_queue.clear();
      }
   }
   else {
      if (m_bg.setViaSetBackGround)
      {
         m_bg.szPath.clear();
         m_bg.active = false;
         m_bg.setViaSetBackGround = false;
         if (m_queue.empty())
            m_player.Stop();
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
   LOGD(std::format("STOP: screen={}, queueDepth={}, bgActive={}", m_pScreen->GetScreenNum(), m_queue.size(), m_bg.active));
   m_queue.clear();
   if (m_bg.active && !m_bg.szPath.empty())
      PlayBackground();
   else
      m_player.Stop();
}

void PUPMediaManager::StopBackground()
{
   m_bg.szPath.clear();
   m_bg.active = false;
   if (m_queue.empty())
      m_player.Stop();
}

void PUPMediaManager::Stop(int priority)
{
   const int currentPriority = m_queue.empty() ? 0 : m_queue.front().priority;
   if (priority == 0 || priority > currentPriority) {
      LOGD(std::format("Stopping playback: screen={{{}}}, priority={}, current={}", m_pScreen->ToString(false), priority, currentPriority));
      Stop();
   }
   else {
      LOGD(std::format("Priority too low to stop: screen={{{}}}, priority={}, current={}", m_pScreen->ToString(false), priority, currentPriority));
   }
}

void PUPMediaManager::Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile)
{
   std::filesystem::path szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (!szPath.empty() && !m_queue.empty() && m_queue.front().szPath == szPath) {
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

// Called from SetGameTime (API thread) when the player has stopped while the queue still has a head.
// Pop the head; if more pending, play next; otherwise fall back to bg.
void PUPMediaManager::OnPlayerEnd()
{
   LOGD(std::format("ON END: screen={}, queueDepth={}, bgActive={}", m_pScreen->GetScreenNum(), m_queue.size(), m_bg.active));
   m_queue.pop_front();
   if (!m_queue.empty())
   {
      StartCurrent();
      return;
   }
   m_pScreen->OnMainMediaEnd();
   if (m_onMainEndCallback)
      m_onMainEndCallback();
   PlayBackground();
}

bool PUPMediaManager::IsMainPlaying() { return !m_queue.empty() && m_player.IsPlaying(); }

bool PUPMediaManager::IsBackgroundPlaying() { return m_bg.active && !m_bg.szPath.empty() && m_queue.empty() && m_player.IsPlaying(); }

void PUPMediaManager::Render(VPXRenderContext2D* const ctx, float alpha)
{
   if (m_player.IsPlaying()) {
      if (m_currentAlpha < 255) {
         m_currentAlpha = std::min(255, m_currentAlpha + m_fadeStep);
      }
      m_player.Render(ctx, m_bounds, alpha * (static_cast<float>(m_currentAlpha) / 255.f));
   }
}

void PUPMediaManager::SetFadeStep(int step)
{
   m_fadeStep = (step < 0) ? 255 : std::min(step, 255);
   if (m_fadeStep >= 255)
      m_currentAlpha = 255;
}

}
