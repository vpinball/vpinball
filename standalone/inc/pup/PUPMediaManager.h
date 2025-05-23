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

   PUPMediaManagerPlayer* m_pMainPlayer = nullptr;
   PUPMediaManagerPlayer* m_pBackgroundPlayer = nullptr;

   PUPScreen* m_pScreen = nullptr;
   bool m_pop = false;
   VPXPluginAPI* m_pVpxApi = nullptr;

public:
   PUPMediaManager(PUPScreen* pScreen);
   ~PUPMediaManager() {};

   void SetVpxApi(VPXPluginAPI* pVpxApi);
   void Play(PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, bool skipSamePriority, int length);
   void SetBG(bool isBackground);
   void SetLoop(bool isLoop);
   void Stop();
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const string& szPlayFile);
   void Render(VPXRenderContext2D* ctx, const SDL_Rect& destRect);
};
