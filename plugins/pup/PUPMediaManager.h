// license:GPLv3+

#pragma once

#include "PUPMediaPlayer.h"
#include <deque>

namespace PUP {

class PUPScreen;

class PUPMediaManager final
{
public:
   explicit PUPMediaManager(PUPScreen* pScreen);
   ~PUPMediaManager();

   void SetGameTime(double gameTime);

   void Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, bool skipSamePriority, int length, bool background);
   void Pause();
   void Resume();
   void SetAsBackGround(bool isBackground);
   void SetLoop(bool isLoop);
   void SetMaxLength(int length);
   void SetVolume(float volume);
   void Stop();
   void StopBackground();
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile);
   void Render(VPXRenderContext2D* const ctx, float alpha = 1.f);
   bool IsMainPlaying();
   bool IsBackgroundPlaying();

   void SetBounds(const SDL_Rect& rect);
   void SetMask(const std::filesystem::path& path);

   void SetOnMainEndCallback(std::function<void()> callback) { m_onMainEndCallback = std::move(callback); }

private:
   void OnPlayerEnd(PUPMediaPlayer* player);
   void PlayBackground();
   void PlayNextFromQueue();
   void PlayImmediate(const std::filesystem::path& szPath, float volume, int priority, int length);

   PUPMediaPlayer m_player; // Single player per screen, switches between main and background content

   // Background config — stores what to play when main ends, not a running player
   struct BackgroundConfig
   {
      std::filesystem::path szPath;
      float volume = 1.0f;
      bool active = false;
      bool setViaSetBackGround = false;
   };
   BackgroundConfig m_bg;

   // Play queue — pending items played in order when current ends
   struct PlayItem
   {
      std::filesystem::path szPath;
      float volume;
      int priority;
      int length;
      uint64_t expiry; // SDL_GetTicks() deadline, 0 = no expiry
   };
   std::deque<PlayItem> m_playQueue;

   std::filesystem::path m_mainPath;
   float m_mainVolume = 1.0f;
   int m_mainPriority = 0;
   bool m_playingMain = false;
   bool m_shuttingDown = false;

   std::function<void()> m_onMainEndCallback;

   std::shared_ptr<SDL_Surface> m_mask = nullptr;

   PUPScreen* const m_pScreen;
   SDL_Rect m_bounds;
};

}
