#include "stdafx.h"

#include "PUPMediaManager.h"
#include "PUPScreen.h"

PUPMediaManager::PUPMediaManager(PUPScreen* pScreen)
{
   m_pMainPlayer = &m_player1;
   m_pBackgroundPlayer = nullptr;
   m_pScreen = pScreen;
   m_pop = (pScreen->GetMode() == PUP_SCREEN_MODE_FORCE_POP_BACK || pScreen->GetMode() == PUP_SCREEN_MODE_FORCE_POP);
   m_pRenderer = nullptr;
}

void PUPMediaManager::SetRenderer(SDL_Renderer* pRenderer)
{
   m_pRenderer = pRenderer;
}

void PUPMediaManager::Play(PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, bool skipSamePriority, int length)
{
   if (skipSamePriority && priority == m_pMainPlayer->priority) {
      PLOGW.printf("skipping same priority, screen={%s}, playlist={%s}, playFile=%s, priority=%d, length=%d",
         m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str(), priority, length);
      return;
   }

   string szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (szPath.empty()) {
      PLOGE.printf("PlayFile not found: screen={%s}, playlist={%s}, playFile=%s",
         m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str());
      return;
   }

   PLOGD.printf("screen={%s}, playlist={%s}, playFile=%s, path=%s, volume=%.1f, priority=%d, length=%d",
      m_pScreen->ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str(), szPath.c_str(), volume, priority, length);

   m_pMainPlayer->player.Play(szPath);
   m_pMainPlayer->player.SetVolume(volume);
   m_pMainPlayer->player.SetLength(length);
   m_pMainPlayer->szPath = szPath;
   m_pMainPlayer->volume = volume;
   m_pMainPlayer->priority = priority;
}

void PUPMediaManager::SetBG(bool isBackground)
{
   if (isBackground) {
      if (m_pBackgroundPlayer) {
         PLOGD.printf("Stopping background player, screen={%s}", m_pScreen->ToString(false).c_str());
         m_pBackgroundPlayer->player.Stop();
      }
      PLOGD.printf("Transferring main player to background, screen={%s}", m_pScreen->ToString(false).c_str());
      m_pBackgroundPlayer = m_pMainPlayer;
      m_pBackgroundPlayer->player.SetLoop(true);
      m_pMainPlayer = (m_pMainPlayer == &m_player1) ? &m_player2 : &m_player1;
   }
   else {
      if (m_pBackgroundPlayer) {
         PLOGD.printf("Removing looping from background player, screen={%s}", m_pScreen->ToString(false).c_str());
         m_pBackgroundPlayer->player.SetLoop(false);
      }
   }
}

void PUPMediaManager::SetLoop(bool isLoop)
{
   m_pMainPlayer->player.SetLoop(isLoop);
}

void PUPMediaManager::Stop()
{
   m_pMainPlayer->player.Stop();
}

void PUPMediaManager::Stop(int priority)
{
   if (priority > m_pMainPlayer->priority) {
      PLOGD.printf("Priority > main player priority: screen={%s}, priority=%d", m_pScreen->ToString(false).c_str(), priority);
      m_pMainPlayer->player.Stop();
   }
   else {
      PLOGD.printf("Priority <= main player priority: screen={%s}, priority=%d", m_pScreen->ToString(false).c_str(), priority);
   }
}

void PUPMediaManager::Stop(PUPPlaylist* pPlaylist, const string& szPlayFile)
{
   string szPath = pPlaylist->GetPlayFilePath(szPlayFile);
   if (!szPath.empty() && szPath == m_pMainPlayer->szPath) {
      PLOGD.printf("Main player stopping playback: screen={%s}, path=%s", m_pScreen->ToString(false).c_str(), szPath.c_str());
      m_pMainPlayer->player.Stop();
   }
   else {
      PLOGD.printf("Main player playback stop requested but currently not playing: screen={%s}, path=%s", m_pScreen->ToString(false).c_str(), szPath.c_str());
   }
}

void PUPMediaManager::Render(const SDL_Rect& destRect)
{
   bool mainPlayerPlaying = m_pMainPlayer->player.IsPlaying();
   bool backgroundPlaying = false;

   if (m_pBackgroundPlayer) {
      backgroundPlaying = m_pBackgroundPlayer->player.IsPlaying();
      if (backgroundPlaying || (!m_pop && !mainPlayerPlaying))
          m_pBackgroundPlayer->player.Render(m_pRenderer, destRect);
   }

   if (mainPlayerPlaying || (!m_pop && !backgroundPlaying)) {
      m_pMainPlayer->player.Render(m_pRenderer, destRect);
   }

   if (m_pBackgroundPlayer)
      m_pBackgroundPlayer->player.SetVolume(mainPlayerPlaying ? 0.0f : m_pBackgroundPlayer->volume);
}
