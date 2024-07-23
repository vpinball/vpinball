#pragma once

#include "PUPMediaPlayer.h"

class PUPScreen;

struct PUPMediaManagerPlayer
{
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
   bool m_pop;
   SDL_Renderer* m_pRenderer;

public:
   PUPMediaManager(PUPScreen* pScreen);
   ~PUPMediaManager() {};

   void SetRenderer(SDL_Renderer* pRenderer);
   void Play(PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, bool skipSamePriority, int length);
   void SetBG(bool isBackground);
   void SetLoop(bool isLoop);
   void Stop();
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const string& szPlayFile);
   void Render(const SDL_Rect& destRect);
};
