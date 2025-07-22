#pragma once

#include "PUPMediaPlayer.h"

namespace PUP {

class PUPScreen;

class PUPMediaManagerPlayer final
{
public:
   PUPMediaManagerPlayer(const string& name) : player(name) {}
   ~PUPMediaManagerPlayer() {}

   PUPMediaPlayer player;
   string szPath;
   float volume = 1.0f;
   int priority = 0;
   bool isBackground = false;
};

class PUPMediaManager final
{
public:
   PUPMediaManager(PUPScreen* pScreen);
   ~PUPMediaManager() {}

   void Play(PUPPlaylist* pPlaylist, const std::string& szPlayFile, float volume, int priority, bool skipSamePriority, int length);
   void Pause();
   void Resume();
   void SetBG(bool isBackground);
   void SetLoop(bool isLoop);
   void SetVolume(float volume);
   void Stop();
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const string& szPlayFile);
   void Render(VPXRenderContext2D* const ctx);

   void SetBounds(const SDL_Rect& rect);
   void SetMask(std::shared_ptr<SDL_Surface> mask);

private:
   void OnMainPlayerEnd();

   std::unique_ptr<PUPMediaManagerPlayer> m_pBackgroundPlayer;
   std::unique_ptr<PUPMediaManagerPlayer> m_pMainPlayer;

   PUPScreen* const m_pScreen;

   SDL_Rect m_bounds;
};

}
