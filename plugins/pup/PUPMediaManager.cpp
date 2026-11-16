// license:GPLv3+

#include "PUPMediaManager.h"
#include "PUPScreen.h"

#include <SDL3_image/SDL_image.h>
#include <algorithm>

namespace PUP {

static double StillDuration(const PUPScreen* screen, const std::filesystem::path& path)
{
   if (screen->GetManager()->GetEngine() == PUPManager::Engine::PinUP20)
      return 0.0;
   const string ext = extension_from_path(PluginStrings::PathToUTF8(path));
   return (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp") ? 10.0 : 0.0;
}

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
   CheckEndTime();
   if (!m_queue.empty() && !m_player.IsPlaying())
      OnPlayerEnd();
   else if (const unsigned int loopCount = m_player.GetLoopCount(); loopCount != m_loopCount)
   {
      m_loopCount = loopCount;
      if (m_jukebox && JukeboxSkip(2, true))
         return;
      if (m_player.IsPlaying())
      {
         m_pScreen->OnMediaEnd();
         m_player.SetVolume(m_pScreen->StartMediaVolume(m_queue.empty() ? m_bg.volume : m_queue.front().volume));
         ApplyDuckPrefix(m_queue.empty() ? m_bg.szPath : m_queue.front().szPath);
         m_pScreen->OnMediaStart(m_queue.empty() ? m_bg.szPath : m_queue.front().szPath);
      }
   }
}

void PUPMediaManager::CheckEndTime()
{
   const uint64_t now = SDL_GetTicks();
   if (!m_queue.empty())
   {
      PlayItem& head = m_queue.front();
      if (head.endTicks && now >= head.endTicks)
      {
         head.endTicks = 0;
         head.loop = false;
         m_player.Stop();
      }
   }
   else if (m_bgEndTicks && now >= m_bgEndTicks)
   {
      m_bgEndTicks = 0;
      PlayBackground();
   }
}

bool PUPMediaManager::Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, int volume, int priority, PlayAction action, int length)
{
   const bool isBg = (action == PlayAction::SetBG);
   const bool mainPlaying = !m_queue.empty();
   const int currentPriority = mainPlaying ? m_queue.front().priority : 0;

   if (priority == 999)
   {
      if (mainPlaying)
         Stop();
      return false;
   }

   // A SetBG of a lower priority than the playing media is dropped like any other play, the background is not changed
   if (mainPlaying && priority < currentPriority && action != PlayAction::QueueAlways)
   {
      LOGD(std::format("Dropping lower priority: screen={}, current={}, new={}, file={}", m_pScreen->GetScreenNum(), currentPriority, priority, PluginStrings::PathToUTF8(szPlayFile)));
      return false;
   }

   if (action == PlayAction::SkipSamePriority && mainPlaying && priority > 0 && priority == currentPriority)
   {
      LOGW(std::format("Skipping same priority, screen={{{}}}, playlist={}, playFile={{{}}}, priority={}", m_pScreen->ToString(false), pPlaylist->ToString(), PluginStrings::PathToUTF8(szPlayFile), priority));
      return false;
   }

   const bool preempting = (priority > 0) && (priority > currentPriority);
   if (action == PlayAction::Normal && !preempting && pPlaylist->IsResting())
   {
      LOGD(std::format("Resting playlist, skipping: screen={}, playlist={}, restSeconds={}", m_pScreen->GetScreenNum(), PluginStrings::PathToUTF8(pPlaylist->GetFolder()), pPlaylist->GetRestSeconds()));
      return false;
   }

   std::filesystem::path szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (szPath.empty()) {
      LOGE(std::format("PlayFile not found: screen={{{}}}, playlist={{{}}}, playFile={}", m_pScreen->ToString(false), pPlaylist->ToString(), PluginStrings::PathToUTF8(szPlayFile)));
      return false;
   }

   if (isBg)
   {
      const bool sameFile = mainPlaying ? m_queue.front().szPath == szPath : (m_bg.active && m_bg.szPath == szPath && m_player.IsPlaying());
      LOGD(std::format("BG CONFIG: screen={}, pri={}, current={}, file={}, alreadyPlaying={}", m_pScreen->GetScreenNum(), priority, currentPriority, PluginStrings::PathToUTF8(szPath.filename()), sameFile));
      m_bg.pPlaylist = pPlaylist;
      m_bg.szPath = szPath;
      m_bg.volume = volume;
      m_bg.active = true;
      pPlaylist->MarkPlayed();
      if (sameFile)
      {
         if (mainPlaying)
            m_queue.front().volume = volume;
         m_pScreen->SetMediaVolume(volume);
         return true;
      }
      m_queue.clear();
      PlayBackground();
      return true;
   }

   if (mainPlaying && m_queue.front().szPath == szPath)
   {
      LOGD(std::format("MAIN SKIP (same file): screen={}, file={}", m_pScreen->GetScreenNum(), PluginStrings::PathToUTF8(szPath.filename())));
      m_queue.front().volume = volume;
      m_pScreen->SetMediaVolume(volume);
      pPlaylist->MarkPlayed();
      return true;
   }
   if (!mainPlaying && szPath == m_bg.szPath && m_bg.active && m_player.IsPlaying())
   {
      LOGD(std::format("MAIN SKIP (bg has file): screen={}, file={}", m_pScreen->GetScreenNum(), PluginStrings::PathToUTF8(szPath.filename())));
      pPlaylist->MarkPlayed();
      return false;
   }

   const PlayItem item { pPlaylist, szPath, volume, priority, length > 0 ? SDL_GetTicks() + static_cast<uint64_t>(length) * 1000 : 0, action == PlayAction::Loop };

   if ((action == PlayAction::QueueSameP || action == PlayAction::QueueAlways) && mainPlaying)
   {
      m_queue.insert(m_queue.begin() + 1, item);
      pPlaylist->MarkPlayed();
      return true;
   }

   const bool splash = (action == PlayAction::SplashReset || action == PlayAction::SplashReturn);
   if (splash && !m_queue.empty())
      m_queue.front().resumeAt = (action == PlayAction::SplashReturn) ? m_player.GetPosition() : 0.0;
   else if (splash && IsBackgroundPlaying())
      m_bgResumeAt = (action == PlayAction::SplashReturn) ? m_player.GetPosition() : 0.0;
   if (!splash && !m_queue.empty())
      m_queue.pop_front();
   m_queue.push_front(item);
   StartCurrent();

   pPlaylist->MarkPlayed();
   return true;
}

void PUPMediaManager::StartCurrent()
{
   if (m_queue.empty())
      return;
   const PlayItem& head = m_queue.front();
   LOGD(std::format("MAIN PLAY: screen={}, vol={}, pri={}, file={}", m_pScreen->GetScreenNum(), head.volume, head.priority, PluginStrings::PathToUTF8(head.szPath.filename())));
   m_player.Play(head.szPath, m_pScreen->StartMediaVolume(head.volume), head.resumeAt, StillDuration(m_pScreen, head.szPath));
   m_queue.front().resumeAt = 0.0;
   m_player.SetLoop(head.loop, m_pScreen->GetManager()->GetEngine() == PUPManager::Engine::PinUP20 ? 0.9 : 0.0);
   m_loopCount = m_player.GetLoopCount();
   m_currentAlpha = (m_fadeStep < 255) ? m_fadeStep : 255;
   ApplyDuckPrefix(head.szPath);
   m_pScreen->OnMediaStart(head.szPath);
}

void PUPMediaManager::ApplyDuckPrefix(const std::filesystem::path& path)
{
   const string name = PluginStrings::PathToUTF8(path.filename());
   if (name.size() < 3 || name[0] != '-')
      return;
   int percent = m_pScreen->GetDefaultDuck();
   if (name[1] >= '0' && name[1] <= '9' && name[2] >= '0' && name[2] <= '9')
   {
      percent = (name[1] - '0') * 10 + (name[2] - '0');
      if (percent == 0)
         percent = 1;
   }
   if (percent > 0)
      m_pScreen->GetManager()->DuckAllExcept(percent, m_pScreen->GetScreenNum(), false);
}

bool PUPMediaManager::PlayBackground()
{
   if (m_bg.active && !m_bg.szPath.empty() && m_pScreen->GetMode() != PUPScreen::Mode::Off)
   {
      LOGD(std::format("BG PLAY: screen={}, file={}", m_pScreen->GetScreenNum(), PluginStrings::PathToUTF8(m_bg.szPath.filename())));
      m_bgEndTicks = 0;
      m_player.Play(m_bg.szPath, m_pScreen->StartMediaVolume(m_bg.volume), m_bgResumeAt, StillDuration(m_pScreen, m_bg.szPath));
      m_bgResumeAt = 0.0;
      m_player.SetLoop(true);
      m_loopCount = m_player.GetLoopCount();
      ApplyDuckPrefix(m_bg.szPath);
      m_pScreen->OnMediaStart(m_bg.szPath);
      return true;
   }
   return false;
}

void PUPMediaManager::Pause()
{
   m_player.Pause(true);
}

void PUPMediaManager::Resume()
{
   m_player.Pause(false);
}

void PUPMediaManager::SetAsBackGround(bool isBackground)
{
   if (isBackground && !m_queue.empty())
   {
      m_bg.pPlaylist = m_queue.front().pPlaylist;
      m_bg.szPath = m_queue.front().szPath;
      m_bg.volume = m_queue.front().volume;
      m_bg.active = true;
   }
   else if (!isBackground)
   {
      m_bg.pPlaylist = nullptr;
      m_bg.szPath.clear();
      m_bg.active = false;
   }
   if (m_pScreen->IsPop() || m_pScreen->GetMode() == PUPScreen::Mode::MusicOnly)
   {
      m_bg.szPath.clear();
      m_bg.active = false;
   }
}

void PUPMediaManager::SetLoop(bool isLoop)
{
   if (m_queue.empty())
      return;
   m_queue.front().loop = isLoop;
   m_player.SetLoop(isLoop);
}

void PUPMediaManager::SetLengthFromNow(int seconds)
{
   const uint64_t endTicks = SDL_GetTicks() + static_cast<uint64_t>(std::max(seconds, 0)) * 1000;
   if (!m_queue.empty())
      m_queue.front().endTicks = endTicks;
   else if (IsBackgroundPlaying())
      m_bgEndTicks = endTicks;
}

void PUPMediaManager::SetVolume(float volume)
{
   m_player.SetVolume(volume);
}

void PUPMediaManager::SetCurrentItemVolume(int volume)
{
   if (!m_queue.empty())
      m_queue.front().volume = volume;
   else if (IsBackgroundPlaying())
      m_bg.volume = volume;
}

// See pDMDStopBackLoop - stops main, starts background if configured
void PUPMediaManager::Stop()
{
   LOGD(std::format("STOP: screen={}, queueDepth={}, bgActive={}", m_pScreen->GetScreenNum(), m_queue.size(), m_bg.active));
   const bool mainPlaying = !m_queue.empty();
   const bool anyPlaying = mainPlaying || m_player.IsPlaying();
   m_queue.clear();
   if (anyPlaying)
      m_pScreen->OnMediaEnd();
   if (m_bg.active && !m_bg.szPath.empty() && m_pScreen->GetMode() != PUPScreen::Mode::Off)
   {
      if (mainPlaying || !m_player.IsPlaying())
         PlayBackground();
   }
   else
   {
      m_player.Stop();
      if (anyPlaying)
         m_pScreen->OnIdle();
   }
}

void PUPMediaManager::Halt()
{
   m_queue.clear();
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
      LOGD(std::format("Main player stopping playback: screen={{{}}}, path={}", m_pScreen->ToString(false), PluginStrings::PathToUTF8(szPath)));
      Stop();
   }
   else {
      LOGD(std::format("Main player playback stop requested but currently not playing: screen={{{}}}, path={}", m_pScreen->ToString(false), PluginStrings::PathToUTF8(szPath)));
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
   m_mask = std::shared_ptr<SDL_Surface>(IMG_Load(PluginStrings::PathToUTF8(path).c_str()), SDL_DestroySurface);
   if (m_mask && m_mask->format != SDL_PIXELFORMAT_RGBA32)
      m_mask = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(m_mask.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
   if (m_mask)
   {
      SDL_LockSurface(m_mask.get());
      uint32_t* __restrict rgba = static_cast<uint32_t*>(m_mask->pixels);
      const uint32_t maskValue = rgba[0];
      const int rowPadding = (m_mask->pitch / static_cast<int>(sizeof(uint32_t))) - m_mask->w;
      for (int i = 0; i < m_mask->h; i++, rgba += rowPadding)
      {
         uint32_t* const end = rgba + m_mask->w;
         for (; rgba < end; ++rgba)
            *rgba = (*rgba == maskValue) ? 0x00000000u : 0xFFFFFFFFu;
      }
      SDL_UnlockSurface(m_mask.get());
   }
   m_player.SetMask(m_mask);
}

// Called from SetGameTime (API thread) when the player has stopped while the queue still has a head.
// Pop the head; if more pending, play next; otherwise fall back to bg.
void PUPMediaManager::OnPlayerEnd()
{
   LOGD(std::format("ON END: screen={}, queueDepth={}, bgActive={}", m_pScreen->GetScreenNum(), m_queue.size(), m_bg.active));
   if (m_jukebox && JukeboxSkip(2, true))
      return;
   m_queue.pop_front();
   const uint64_t now = SDL_GetTicks();
   std::erase_if(m_queue, [now](const PlayItem& item) { return item.endTicks && now >= item.endTicks; });
   m_pScreen->OnMediaEnd();
   if (!m_queue.empty())
   {
      StartCurrent();
      return;
   }
   if (!PlayBackground())
      m_pScreen->OnIdle();
}

bool PUPMediaManager::JukeboxSkip(int direction, bool ended)
{
   PUPPlaylist* const pPlaylist = !m_queue.empty() ? m_queue.front().pPlaylist : (m_bg.active && m_player.IsPlaying() ? m_bg.pPlaylist : nullptr);
   if (pPlaylist == nullptr || !pPlaylist->HasFiles())
      return false;
   pPlaylist->SetRestSeconds(-1);
   const std::filesystem::path file = (direction == 1) ? pPlaylist->GetPriorPlayFile() : pPlaylist->GetNextPlayFile();
   LOGD(std::format("JUKEBOX: screen={}, direction={}, file={}", m_pScreen->GetScreenNum(), direction, PluginStrings::PathToUTF8(file)));
   if (!ended)
   {
      m_pScreen->Play(pPlaylist, file, static_cast<float>(m_pScreen->GetDefaultVolume()), 1, PlayAction::Normal, 0);
      return true;
   }
   if (!m_queue.empty())
   {
      m_queue.front().szPath.clear();
      m_queue.front().priority = 0;
      m_queue.front().loop = false;
   }
   m_pScreen->OnMediaEnd();
   m_pScreen->Play(pPlaylist, file, static_cast<float>(m_pScreen->GetDefaultVolume()), 1, PlayAction::Normal, 0);
   if (!m_queue.empty() && m_queue.front().szPath.empty())
   {
      m_queue.pop_front();
      if (!m_queue.empty())
         StartCurrent();
      else if (!PlayBackground())
         m_pScreen->OnIdle();
   }
   else if (m_queue.empty() && m_player.IsPlaying())
      m_pScreen->OnMediaStart(m_bg.szPath);
   return true;
}

void PUPMediaManager::SetSafeLoop()
{
   if (!m_queue.empty() || IsBackgroundPlaying())
      m_player.SetSafeLoop();
}

void PUPMediaManager::Seek(int ms)
{
   if (m_player.IsPlaying() && ms < m_player.GetDuration() * 1000.0)
      m_player.Seek(ms / 1000.0);
}

void PUPMediaManager::Jump(int direction, int percent)
{
   if ((m_queue.empty() && !IsBackgroundPlaying()) || !m_player.IsPlaying())
      return;
   const double length = m_player.GetDuration();
   const double step = std::round(percent / 100.0 * length * 1000.0) / 1000.0;
   if (direction == 1)
      m_player.Seek(std::min(m_player.GetPosition() + step, length - 2.0));
   else if (direction == 2)
      m_player.Seek(std::max(m_player.GetPosition() - step, 2.0));
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
   m_fadeStep = (step <= 0) ? 255 : std::min(step, 255);
   if (m_fadeStep >= 255)
      m_currentAlpha = 255;
}

}
