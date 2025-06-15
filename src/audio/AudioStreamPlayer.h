// license:GPLv3+

#pragma once

#include <SDL3/SDL_audio.h>

namespace VPX
{

// Stream provided raw buffer audio to the given SDL audio audio device.
class AudioStreamPlayer
{
public:
   static AudioStreamPlayer* Create(int sdlDevice, int frequency, int channels)
   {
      SDL_AudioSpec audioSpec;
      audioSpec.freq = frequency;
      audioSpec.format = SDL_AUDIO_S16LE;
      audioSpec.channels = channels;

      SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(sdlDevice, &audioSpec, nullptr, nullptr);
      if (stream)
      {
         SDL_ResumeAudioStreamDevice(stream);
         return new AudioStreamPlayer(stream);
      }
      else
      {
         PLOGE << "Failed to create stream: " << SDL_GetError();
         return nullptr;
      }
   }

   AudioStreamPlayer(SDL_AudioStream* stream)
      : m_stream(stream)
   {
      assert(stream != nullptr);
   }

   ~AudioStreamPlayer()
   {
      SDL_DestroyAudioStream(m_stream);
   }

   void Enqueue(void* buffer, int length)
   {
      SDL_PutAudioStreamData(m_stream, buffer, length);
   }

   int GetQueued() const
   {
      return SDL_GetAudioStreamQueued(m_stream);
   }

   void SetStreamVolume(const float volume)
   {
      m_streamVolume = volume;
      SDL_SetAudioStreamGain(m_stream, m_streamVolume * m_mainVolume);
   }

   void SetMainVolume(const float volume)
   {
      m_mainVolume = volume;
      SDL_SetAudioStreamGain(m_stream, m_streamVolume * m_mainVolume);
   }

private:
   SDL_AudioStream* const m_stream = nullptr;
   float m_mainVolume = 1.f;
   float m_streamVolume = 1.f;
};

}