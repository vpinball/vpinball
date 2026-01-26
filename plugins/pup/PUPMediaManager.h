// license:GPLv3+

#pragma once

#include "PUPMediaPlayer.h"

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
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile);
   void Render(VPXRenderContext2D* const ctx);
   bool IsMainPlaying() const;
   bool IsBackgroundPlaying() const;

   void SetBounds(const SDL_Rect& rect);
   void SetMask(const std::filesystem::path& path);

private:
   void OnPlayerEnd(PUPMediaPlayer* player);

   class PUPMediaManagerPlayer final
   {
   public:
      explicit PUPMediaManagerPlayer(const string& name)
         : player(name)
      {
      }
      ~PUPMediaManagerPlayer() = default;

      PUPMediaPlayer player;
      std::filesystem::path szPath;
      float volume = 1.0f;
      int priority = 0;
   };

   std::shared_ptr<SDL_Surface> m_mask = nullptr;

   std::unique_ptr<PUPMediaManagerPlayer> m_pBackgroundPlayer;
   std::unique_ptr<PUPMediaManagerPlayer> m_pMainPlayer;

   PUPScreen* const m_pScreen;

   SDL_Rect m_bounds;
};

}
