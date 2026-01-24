// license:GPLv3+

#include "PUPMediaManager.h"
#include "PUPScreen.h"

#include <SDL3_image/SDL_image.h>

namespace PUP {

static string GetPlayerName(PUPScreen* pScreen, bool isMain)
{
   return "PUP.#"s.append(std::to_string(pScreen->GetScreenNum())).append(isMain ? ".Main" : ".Back");
}

PUPMediaManager::PUPMediaManager(PUPScreen* pScreen)
   : m_pBackgroundPlayer(std::make_unique<PUPMediaManagerPlayer>(GetPlayerName(pScreen, false)))
   , m_pMainPlayer(std::make_unique<PUPMediaManagerPlayer>(GetPlayerName(pScreen, true)))
   , m_pScreen(pScreen)
   , m_bounds()
{
   m_pBackgroundPlayer->player.SetOnEndCallback([this](PUPMediaPlayer* player) { OnPlayerEnd(player); });
   m_pMainPlayer->player.SetOnEndCallback([this](PUPMediaPlayer* player) { OnPlayerEnd(player); });
}

PUPMediaManager::~PUPMediaManager()
{
}

void PUPMediaManager::SetGameTime(double gameTime)
{
   m_pBackgroundPlayer->player.SetGameTime(gameTime);
   m_pMainPlayer->player.SetGameTime(gameTime);
}

void PUPMediaManager::Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, bool skipSamePriority, int length, bool background)
{
   if (!background && skipSamePriority && IsMainPlaying() && priority <= m_pMainPlayer->priority)
   {
      LOGE("Skipping same priority, screen={%s}, playlist={%s}, playFile=%s, priority=%d", m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str(), priority);
      return;
   }

   std::filesystem::path szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (szPath.empty()) {
      LOGE("PlayFile not found: screen={%s}, playlist={%s}, playFile=%s", m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str());
      return;
   }

   LOGD("> Play screen={%s}, playlist={%s}, playFile=%s, path=%s, volume=%.1f, priority=%d, length=%d, background=%d", m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str(), szPath.string().c_str(), volume, priority, length, background);
   if (background)
   {
      m_isBackgroundPlaying = true;
      m_pBackgroundPlayer->player.Play(szPath, volume);
      m_pBackgroundPlayer->player.SetLoop(true);
      m_pBackgroundPlayer->szPath = szPath;
      m_pBackgroundPlayer->volume = volume;
   }
   else
   {
      m_isFrontPlaying = true;
      m_pMainPlayer->player.Play(szPath, volume);
      m_pMainPlayer->player.SetLength(length);
      m_pMainPlayer->szPath = szPath;
      m_pMainPlayer->volume = volume;
      m_pMainPlayer->priority = priority;
   }
   if (IsMainPlaying())
      m_pBackgroundPlayer->player.Pause(true);
}

void PUPMediaManager::Pause()
{
   m_pMainPlayer->player.Pause(true);
}

void PUPMediaManager::Resume()
{
   m_pMainPlayer->player.Pause(false);
}

void PUPMediaManager::SetAsBackGround(bool isBackground)
{
   if (isBackground) {
      LOGD("Replacing background player, screen={%s}", m_pScreen->ToString(false).c_str());
      m_isBackgroundPlaying = m_isFrontPlaying;
      m_pBackgroundPlayer->player.Stop();
      std::swap(m_pMainPlayer, m_pBackgroundPlayer);
      m_pBackgroundPlayer->player.SetLoop(true);
      m_isFrontPlaying = false;
      assert(m_isBackgroundPlaying);
   }
   else {
      LOGD("Making background player the main player (no looping), screen={%s}", m_pScreen->ToString(false).c_str());
      m_isFrontPlaying = m_isBackgroundPlaying;
      m_pBackgroundPlayer->player.SetLoop(false);
      std::swap(m_pMainPlayer, m_pBackgroundPlayer);
      m_pBackgroundPlayer->player.Stop();
      m_isBackgroundPlaying = false;
   }
   m_pBackgroundPlayer->player.SetName(GetPlayerName(m_pScreen, false));
   m_pMainPlayer->player.SetName(GetPlayerName(m_pScreen, true));
}

void PUPMediaManager::SetLoop(bool isLoop)
{
   m_pMainPlayer->player.SetLoop(isLoop);
}

void PUPMediaManager::SetMaxLength(int length)
{
   m_pMainPlayer->player.SetLength(length);
}

void PUPMediaManager::SetVolume(float volume)
{
   m_pBackgroundPlayer->player.SetVolume(volume);
   m_pMainPlayer->player.SetVolume(volume);
}

void PUPMediaManager::Stop()
{
   m_pMainPlayer->player.Stop();
   m_isFrontPlaying = false;
}

void PUPMediaManager::Stop(int priority)
{
   if (priority > m_pMainPlayer->priority) {
      LOGD("Priority > main player priority: screen={%s}, priority=%d", m_pScreen->ToString(false).c_str(), priority);
      Stop();
   }
   else {
      LOGD("Priority <= main player priority: screen={%s}, priority=%d", m_pScreen->ToString(false).c_str(), priority);
   }
}

void PUPMediaManager::Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile)
{
   std::filesystem::path szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (!szPath.empty() && szPath == m_pMainPlayer->szPath) {
      LOGD("Main player stopping playback: screen={%s}, path=%s", m_pScreen->ToString(false).c_str(), szPath.c_str());
      Stop();
   }
   else {
      LOGD("Main player playback stop requested but currently not playing: screen={%s}, path=%s", m_pScreen->ToString(false).c_str(), szPath.c_str());
   }
}

void PUPMediaManager::SetBounds(const SDL_Rect& rect)
{
   m_bounds = rect;
   m_pBackgroundPlayer->player.SetBounds(rect);
   m_pMainPlayer->player.SetBounds(rect);
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
      for (int i = 0; i < m_mask->h; i++, rgba += (m_mask->pitch - m_mask->w * sizeof(uint32_t)))
         for (int j = 0; j < m_mask->w; j++, rgba++)
            *rgba = (*rgba == maskValue) ? 0x00000000u : 0xFFFFFFFFu;
      SDL_UnlockSurface(m_mask.get());
   }
   m_pBackgroundPlayer->player.SetMask(m_mask);
   m_pMainPlayer->player.SetMask(m_mask);
}

void PUPMediaManager::OnPlayerEnd(PUPMediaPlayer* player)
{
   if (player == &m_pMainPlayer->player)
   {
      if (m_pBackgroundPlayer->player.GetFilename().empty())
      {
         LOGD(". Background video %s unpaused (finished: %s)", m_pBackgroundPlayer->szPath.string().c_str(), m_pMainPlayer->szPath.string().c_str());
      }
      m_pBackgroundPlayer->player.Pause(false);
      m_isFrontPlaying = false;
   }
}

bool PUPMediaManager::IsMainPlaying() const { return m_isFrontPlaying; } // Not m_pMainPlayer->player.IsPlaying(); as this change asynchronously

bool PUPMediaManager::IsBackgroundPlaying() const { return m_isBackgroundPlaying; } // Not m_pBackgroundPlayer->player.IsPlaying(); as this change asynchronously

void PUPMediaManager::Render(VPXRenderContext2D* const ctx)
{
   if (m_pMainPlayer->player.IsPlaying())
      m_pMainPlayer->player.Render(ctx, m_bounds);
   else if (m_pBackgroundPlayer->player.IsPlaying())
      m_pBackgroundPlayer->player.Render(ctx, m_bounds);
}

}
