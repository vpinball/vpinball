// license:GPLv3+

#pragma once

#include <SDL3/SDL_audio.h>
#include "../plugins/MsgPluginManager.h"

namespace VPX
{

// Stream provided raw buffer audio to the given SDL audio device.
class AudioStreamPlayer
{
public:
   static std::unique_ptr<AudioStreamPlayer> Create(int sdlDevice, int frequency, int channels, bool isFloat);
   explicit AudioStreamPlayer(SDL_AudioStream* stream);
   ~AudioStreamPlayer();

   void Enqueue(void* buffer, int length);
   void EndStream(std::function<void()> flushCallback);
   void SetStreamVolume(const float volume);
   void SetMainVolume(const float volume);

private:
   static void AudioStreamCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
   static void FlushCallback(void* userdata);;

   SDL_AudioStream* const m_stream = nullptr;
   SDL_AudioSpec m_audioSpec;
   float m_mainVolume = 1.f;
   float m_streamVolume = 1.f;
   float m_throttling = 1.f;
   uint64_t m_streamedTotal = 0;
   uint64_t m_startTimestamp = 0;
   bool m_resync = false;
   bool m_flush = false;
   std::function<void()> m_flushCallback;
};

}
