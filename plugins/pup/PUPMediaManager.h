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
   float volume;
   int priority;
};

class PUPMediaManager final
{
private:
   PUPMediaManagerPlayer m_player1;
   PUPMediaManagerPlayer m_player2;

   PUPMediaManagerPlayer* m_pMainPlayer;
   PUPMediaManagerPlayer* m_pBackgroundPlayer;

   PUPScreen* m_pScreen;
   SDL_Rect m_bounds;
   bool m_pop;

public:
   PUPMediaManager(PUPScreen* pScreen);
   ~PUPMediaManager() {}

   void Play(PUPPlaylist* pPlaylist, const std::string& szPlayFile, float volume, int priority, bool skipSamePriority, int length);
   void SetBG(bool isBackground);
   void SetLoop(bool isLoop);
   void Stop();
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const string& szPlayFile);
   void Render(VPXRenderContext2D* const ctx);

   void SetBounds(const SDL_Rect& rect);
};

}
