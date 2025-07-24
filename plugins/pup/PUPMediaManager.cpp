#include "PUPMediaManager.h"
#include "PUPScreen.h"

#include <SDL3_image/SDL_image.h>

namespace PUP {

static string GetPlayerName(PUPScreen* pScreen, bool isMain)
{
   return "PUP.#"s.append(std::to_string(pScreen->GetScreenNum())).append(isMain ? ".Main" : ".Back");
}

PUPMediaManager::PUPMediaManager(PUPScreen* pScreen)
   : m_pScreen(pScreen)
   , m_pMainPlayer(std::make_unique<PUPMediaManagerPlayer>(GetPlayerName(pScreen, true)))
   , m_pBackgroundPlayer(nullptr)
   , m_bounds()
{
   m_pMainPlayer->player.SetOnEndCallback([this](PUPMediaPlayer* player) { OnPlayerEnd(player); });
}

PUPMediaManager::~PUPMediaManager()
{
   AsyncCallback::InvalidateAllPending(m_pendingEndCallbackList, m_pendingEndCallbackListMutex);
}

void PUPMediaManager::Play(PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, bool skipSamePriority, int length)
{
   if (skipSamePriority && IsPlaying() && !m_pMainPlayer->isBackground && priority <= m_pMainPlayer->priority) {
      LOGE("Skipping same priority, screen={%s}, playlist={%s}, playFile=%s, priority=%d", m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str(), priority);
      return;
   }

   string szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (szPath.empty()) {
      LOGE("PlayFile not found: screen={%s}, playlist={%s}, playFile=%s", m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str());
      return;
   }

   LOGD("> Play screen={%s}, playlist={%s}, playFile=%s, path=%s, volume=%.1f, priority=%d, length=%d", m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str(), szPath.c_str(), volume, priority, length);

   if (m_pMainPlayer->isBackground)
   {
      LOGD(". Background video {%s} paused while playing {%s}", m_pMainPlayer->szPath.c_str(), szPath.c_str());
      std::swap(m_pBackgroundPlayer, m_pMainPlayer);
      m_pBackgroundPlayer->player.SetName(GetPlayerName(m_pScreen, false));
      m_pBackgroundPlayer->player.Pause(true);
      if (m_pMainPlayer == nullptr)
      {
         m_pMainPlayer = std::make_unique<PUPMediaManagerPlayer>(GetPlayerName(m_pScreen, true));
         m_pMainPlayer->player.SetOnEndCallback([this](PUPMediaPlayer* player) { OnPlayerEnd(player); });
      }
      else
         m_pMainPlayer->player.SetName(GetPlayerName(m_pScreen, true));
   }
   
   m_pMainPlayer->player.Play(szPath);
   m_pMainPlayer->player.SetVolume(volume);
   m_pMainPlayer->player.SetLength(length);
   m_pMainPlayer->szPath = szPath;
   m_pMainPlayer->volume = volume;
   m_pMainPlayer->priority = priority;
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
      if (m_pBackgroundPlayer) {
         LOGD("Replacing background player, screen={%s}", m_pScreen->ToString(false).c_str());
         m_pBackgroundPlayer = nullptr;
      }
      m_pMainPlayer->isBackground = true;
      m_pMainPlayer->player.SetLoop(true);
   }
   else if (m_pBackgroundPlayer) {
      LOGD("Removing looping from background player, screen={%s}", m_pScreen->ToString(false).c_str());
      m_pMainPlayer->isBackground = true;
      m_pBackgroundPlayer->player.SetLoop(false);
   }
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
   if (m_pBackgroundPlayer)
      m_pBackgroundPlayer->player.SetVolume(volume);
   m_pMainPlayer->player.SetVolume(volume);
}

void PUPMediaManager::Stop()
{
   m_pMainPlayer->player.Stop();
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

void PUPMediaManager::Stop(PUPPlaylist* pPlaylist, const string& szPlayFile)
{
   string szPath = pPlaylist->GetPlayFilePath(szPlayFile);
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
   if (m_pBackgroundPlayer)
      m_pBackgroundPlayer->player.SetBounds(rect);
   m_pMainPlayer->player.SetBounds(rect);
}

void PUPMediaManager::SetMask(const string& path)
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
   }
   if (m_pBackgroundPlayer)
      m_pBackgroundPlayer->player.SetMask(m_mask);
   m_pMainPlayer->player.SetMask(m_mask);
}

void PUPMediaManager::OnPlayerEnd(PUPMediaPlayer* player)
{
   AsyncCallback::DispatchOnMainThread(m_pScreen->GetManager()->GetMsgAPI(), m_pendingEndCallbackList, m_pendingEndCallbackListMutex,
      [this, player]()
      {
         if (player == &m_pMainPlayer->player && m_pBackgroundPlayer != nullptr)
         {
            LOGD(". Background video {%s} unpaused ({%s} is finished)", m_pBackgroundPlayer->szPath.c_str(), m_pMainPlayer->szPath.c_str());
            std::swap(m_pBackgroundPlayer, m_pMainPlayer);
            m_pBackgroundPlayer->player.SetName(GetPlayerName(m_pScreen, false));
            m_pMainPlayer->player.SetName(GetPlayerName(m_pScreen, true));
            m_pMainPlayer->player.Pause(false);
         }
      }
   );
}

bool PUPMediaManager::IsPlaying() const {
   return m_pMainPlayer->player.IsPlaying();
}

void PUPMediaManager::Render(VPXRenderContext2D* const ctx) {
   if (!m_pScreen->IsPop() || m_pMainPlayer->player.IsPlaying())
      m_pMainPlayer->player.Render(ctx, m_bounds);
}

}
