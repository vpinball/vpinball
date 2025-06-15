// license:GPLv3+

#pragma once

#include <SDL3_mixer/SDL_mixer.h>

namespace VPX
{

// Simple class wrapper around SDL mixer music feature (stream from a file to SDL mixer)
// This class expects SDL_audio & SDL_mixer library to be fully initialized before use
class MusicPlayer
{
public:
   static MusicPlayer* Create(const string& filename)
   {
      Mix_Music* music = Mix_LoadMUS(filename.c_str());
      return music ? new MusicPlayer(music) : nullptr;
   }

   ~MusicPlayer()
   {
      Mix_HookMusicFinished(nullptr);
      Mix_HaltMusic();
      Mix_FreeMusic(m_music);
   }

   bool IsPlaying() const
   {
      return Mix_PlayingMusic();
   }

   void Pause()
   {
      Mix_PauseMusic();
   }

   void Unpause()
   {
      Mix_ResumeMusic();
   }

   double GetPosition() const
   {
      return Mix_GetMusicPosition(m_music);
   }

   void SetPosition(double seconds)
   {
      Mix_SetMusicPosition(seconds);
   }

   void SetMusicVolume(const float volume)
   {
      m_musicVolume = volume;
      Mix_VolumeMusic(static_cast<int>(m_musicVolume * m_mainVolume * static_cast<float>(MIX_MAX_VOLUME)));
   }

   void SetMainVolume(const float volume)
   {
      m_mainVolume = volume;
      Mix_VolumeMusic(static_cast<int>(m_musicVolume * m_mainVolume * static_cast<float>(MIX_MAX_VOLUME)));
   }

private:
   MusicPlayer(Mix_Music* mixMusic)
      : m_music(mixMusic)
   {
      assert(mixMusic != nullptr);
      Mix_PlayMusic(m_music, 0);
   }

   Mix_Music* const m_music;
   float m_mainVolume = 1.f;
   float m_musicVolume = 1.f;
};

}