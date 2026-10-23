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

   bool Play(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, PlayAction action, int length);
   void Pause();
   void Resume();
   void SetAsBackGround(bool isBackground);
   void SetLoop(bool isLoop);
   void SetLengthFromNow(int seconds);
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

private:
   void OnPlayerEnd();
   void PlayBackground();
   void StartCurrent();
   void ApplyDuckPrefix(const std::filesystem::path& path);
   void CheckEndTime();

   PUPMediaPlayer m_player;

   struct PlayItem
   {
      std::filesystem::path szPath;
      float volume = 1.f;
      int priority = 0;
      uint64_t endTicks = 0;
      bool loop = false;
      double resumeAt = 0.0;
   };

   // Playback queue. front() is currently playing; the rest are pending.
   // Splash actions push at front without popping; the displaced item resumes when the splash ends.
   std::deque<PlayItem> m_queue;

   struct BackgroundConfig
   {
      std::filesystem::path szPath;
      float volume = 1.0f;
      bool active = false;
   };
   BackgroundConfig m_bg;
   uint64_t m_bgEndTicks = 0;
   double m_bgResumeAt = 0.0;

   std::shared_ptr<SDL_Surface> m_mask = nullptr;

   PUPScreen* const m_pScreen;
   SDL_Rect m_bounds;

   unsigned int m_loopCount = 0;
   int m_fadeStep = 255;
   int m_currentAlpha = 255;
};

}
