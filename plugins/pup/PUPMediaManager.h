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

   void Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, PlayAction action, int length);
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

   void SetFadeStep(int step);

   void SetBounds(const SDL_Rect& rect);
   void SetMask(const std::filesystem::path& path);

   void SetOnMainEndCallback(std::function<void()> callback) { m_onMainEndCallback = std::move(callback); }

private:
   void OnPlayerEnd();
   void PlayBackground();
   void StartCurrent();

   PUPMediaPlayer m_player;

   struct PlayItem
   {
      std::filesystem::path szPath;
      float volume = 1.f;
      int priority = 0;
      int length = 0;
      bool loop = false;
   };

   // Playback queue. front() is currently playing; the rest are pending.
   // Splash actions push at front without popping; the displaced item resumes when the splash ends.
   std::deque<PlayItem> m_queue;

   struct BackgroundConfig
   {
      std::filesystem::path szPath;
      float volume = 1.0f;
      bool active = false;
      bool setViaSetBackGround = false;
   };
   BackgroundConfig m_bg;

   bool m_shuttingDown = false;

   std::function<void()> m_onMainEndCallback;

   std::shared_ptr<SDL_Surface> m_mask = nullptr;

   PUPScreen* const m_pScreen;
   SDL_Rect m_bounds;

   int m_fadeStep = 255;
   int m_currentAlpha = 255;
};

}
