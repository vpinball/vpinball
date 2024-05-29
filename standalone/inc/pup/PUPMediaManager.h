#pragma once

#include "PUPMediaPlayer.h"

class PUPScreen;

struct PUPMediaManagerPlayer
{
   PUPMediaPlayer player;
   float volume;
   int priority;
};

class PUPMediaManager
{
private:
   PUPMediaManagerPlayer m_player1;
   PUPMediaManagerPlayer m_player2;

   PUPMediaManagerPlayer* m_pMainPlayer;
   PUPMediaManagerPlayer* m_pBackgroundPlayer;

   PUPScreen* m_pScreen;
   bool m_pop;

public:
   PUPMediaManager(PUPScreen* pScreen);
   ~PUPMediaManager() {};

   void SetRenderer(SDL_Renderer* pRenderer);

   void Play(const std::string& szMedia, float volume, int priority);
   void SetBG(bool isBackground);
   void SetLoop(bool isLoop);
   void Stop();

   void Render(const SDL_Rect& destRect);
};