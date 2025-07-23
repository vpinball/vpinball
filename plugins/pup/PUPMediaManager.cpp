#include "PUPMediaManager.h"
#include "PUPScreen.h"

namespace PUP {

PUPMediaManager::PUPMediaManager(PUPScreen* pScreen)
   : m_pScreen(pScreen)
   , m_pMainPlayer(std::make_unique<PUPMediaManagerPlayer>("PUP.Screen #"s.append(std::to_string(pScreen->GetScreenNum())).append(".Main")))
   , m_pBackgroundPlayer(nullptr)
   , m_bounds()
{
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
      m_pBackgroundPlayer = std::move(m_pMainPlayer);
      m_pBackgroundPlayer->player.Pause(true);
      m_pBackgroundPlayer->player.SetName("PUP.Screen #"s.append(std::to_string(m_pScreen->GetScreenNum())).append(".Back"));
      m_pMainPlayer = std::make_unique<PUPMediaManagerPlayer>("PUP.Screen #"s.append(std::to_string(m_pScreen->GetScreenNum())).append(".Main"));
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

void PUPMediaManager::SetBG(bool isBackground)
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
   OnMainPlayerEnd();
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

void PUPMediaManager::SetMask(std::shared_ptr<SDL_Surface> mask)
{
   if (m_pBackgroundPlayer)
      m_pBackgroundPlayer->player.SetMask(mask);
   m_pMainPlayer->player.SetMask(mask);
}

void PUPMediaManager::OnMainPlayerEnd() {
   if (m_pBackgroundPlayer)
   {
      m_pMainPlayer = std::move(m_pBackgroundPlayer);
      m_pMainPlayer->player.Pause(false);
      m_pMainPlayer->player.SetName("PUP.Screen #"s.append(std::to_string(m_pScreen->GetScreenNum())).append(".Main"));
   }
}

bool PUPMediaManager::IsPlaying() const {
   return m_pMainPlayer->player.IsPlaying();
}


void PUPMediaManager::Render(VPXRenderContext2D* const ctx) {
   bool playing = m_pMainPlayer->player.IsPlaying();
   if (!playing && m_pBackgroundPlayer) {
      OnMainPlayerEnd();
      playing = m_pMainPlayer->player.IsPlaying();
   }
   if (!m_pScreen->IsPop() || m_pMainPlayer->player.IsPlaying())
      m_pMainPlayer->player.Render(ctx, m_bounds);
}

}
