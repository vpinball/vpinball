#include "core/stdafx.h"

#include "PUPMediaManager.h"
#include "PUPScreen.h"

PUPMediaManager::PUPMediaManager(PUPScreen* pScreen)
{
   m_pMainPlayer = &m_player1;
   m_pBackgroundPlayer = nullptr;
   m_pScreen = pScreen;
   m_pop = (pScreen->GetMode() == PUP_SCREEN_MODE_FORCE_POP_BACK || pScreen->GetMode() == PUP_SCREEN_MODE_FORCE_POP);
}

void PUPMediaManager::Play(const std::string& szMedia, float volume, int priority)
{
   PLOGW.printf("Play: media=%s, volume=%.1f, priority=%d, screen={%s}", szMedia.c_str(), volume, priority, m_pScreen->ToString(false).c_str());

   m_pMainPlayer->player.Play(szMedia);
   m_pMainPlayer->player.SetVolume(volume);
   m_pMainPlayer->volume = volume;
   m_pMainPlayer->priority = priority;
}

void PUPMediaManager::SetRenderer(SDL_Renderer* pRenderer)
{
   m_player1.player.SetRenderer(pRenderer);
   m_player2.player.SetRenderer(pRenderer);
}

void PUPMediaManager::SetBG(bool isBackground)
{
   if (isBackground) {
      if (m_pBackgroundPlayer) {
         PLOGW.printf("Stopping background player, screen={%s}", m_pScreen->ToString(false).c_str());
         m_pBackgroundPlayer->player.Stop();
      }
      PLOGW.printf("Transfering main player to background, screen={%s}", m_pScreen->ToString(false).c_str());
      m_pBackgroundPlayer = m_pMainPlayer;
      m_pBackgroundPlayer->player.SetLoop(true);
      m_pMainPlayer = (m_pMainPlayer == &m_player1) ? &m_player2 : &m_player1;
   }
   else {
      if (m_pBackgroundPlayer) {
         PLOGW.printf("Removing looping from background player, screen={%s}", m_pScreen->ToString(false).c_str());
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

void PUPMediaManager::Render(const SDL_Rect& destRect)
{
   bool mainPlayerPlaying = m_pMainPlayer->player.IsPlaying();
   bool backgroundPlaying = false;

   if (m_pBackgroundPlayer) {
      backgroundPlaying = m_pBackgroundPlayer->player.IsPlaying();
      if (backgroundPlaying || (!m_pop && !mainPlayerPlaying))
          m_pBackgroundPlayer->player.Render(destRect);
   }
 
   if (mainPlayerPlaying || (!m_pop && !backgroundPlaying)) {
      m_pMainPlayer->player.Render(destRect);
   }

   if (m_pBackgroundPlayer)
      m_pBackgroundPlayer->player.SetVolume(mainPlayerPlaying ? 0.0f : m_pBackgroundPlayer->volume);
}