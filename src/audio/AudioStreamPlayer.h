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
   static AudioStreamPlayer* Create(int sdlDevice, int frequency, int channels, bool isFloat)
   {
      SDL_AudioSpec audioSpec;
      audioSpec.freq = frequency;
      audioSpec.format = isFloat ? SDL_AUDIO_F32 : SDL_AUDIO_S16;
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
      , m_startTimestamp(SDL_GetTicks())
   {
      assert(stream != nullptr);
      SDL_GetAudioStreamFormat(m_stream, &m_audioSpec, nullptr);
      SDL_SetAudioStreamGetCallback(m_stream, &AudioStreamCallback, this);
   }

   ~AudioStreamPlayer()
   {
      SDL_DestroyAudioStream(m_stream);
   }

   void Enqueue(void* buffer, int length)
   {
      // If we are really late and decided to resync, do it when pushing new data (limit silence time, handle situation where no more data are coming)
      if (m_resync)
      {
         SDL_ClearAudioStream(m_stream);
         m_startTimestamp = SDL_GetTicks();
         m_streamedTotal = 0;
         m_resync = false;
         PLOGI << "Audio stream sync was lost and reseted";
      }
      // Just enqueue, syncing and compensation is done on a regular basis when data is requested for playing
      SDL_PutAudioStreamData(m_stream, buffer, length);
      m_streamedTotal += length;
   }

   void EndStream(std::function<void()> flushCallback)
   {
      assert(!m_flush);
      m_flush = true;
      m_flushCallback = flushCallback;
      SDL_FlushAudioStream(m_stream);
   }

   void SetStreamVolume(const float volume)
   {
      if (m_streamVolume != volume)
      {
         m_streamVolume = volume;
         SDL_SetAudioStreamGain(m_stream, m_streamVolume * m_mainVolume);
      }
   }

   void SetMainVolume(const float volume)
   {
      if (m_mainVolume != volume)
      {
         m_mainVolume = volume;
         SDL_SetAudioStreamGain(m_stream, m_streamVolume * m_mainVolume);
      }
   }

private:
   static void AudioStreamCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
   {
      AudioStreamPlayer* const me = static_cast<AudioStreamPlayer*>(userdata);
      const int nQueueSize = max(0, SDL_GetAudioStreamQueued(me->m_stream) - total_amount);
      const uint64_t nBytePerSec = me->m_audioSpec.freq * SDL_AUDIO_FRAMESIZE(me->m_audioSpec);
      const uint64_t sourceTS = (1000 * me->m_streamedTotal) / nBytePerSec; // Total amount of music streamed (ms)
      const uint64_t playedTS = (1000 * (me->m_streamedTotal - nQueueSize)) / nBytePerSec; // Playing position (ms)
      const uint64_t nowTS = SDL_GetTicks() - me->m_startTimestamp; // Where we should be in the music (ms)
      // Note that we do not handle situation where we would get ahead of the source since they are seldom and doing it preventively would increase sound lag
      float throttle = 1.f;
      if (nowTS > playedTS)
      {
         uint64_t deltaTS = nowTS - playedTS;
         if (nQueueSize > 1000 * nBytePerSec && deltaTS > 1000)
         {
            // We are really late, just resync on next stream update (don't change throttling to avoid adding some glitches to the already glitched stream)
            throttle = me->m_throttling;
            me->m_resync = true;
         }
         /* else if (nQueueSize > 200 * nBytePerSec && deltaTS > ((me->m_throttling == 1.f) ? 100 : 50))
         {
            // We are a bit late, try to catch up by slightly increasing the pitch
            throttle = 1.0f + static_cast<float>(min(nowTS - playedTS - 20, 500ull)) / 500.f;
         }*/
      }
      if (me->m_throttling != throttle)
      {
         me->m_throttling = throttle;
         SDL_SetAudioStreamFrequencyRatio(me->m_stream, throttle);
         PLOGI << "PlayedTS: " << playedTS << "ms / NowTS: " << nowTS << "ms / Delta: " << (nowTS - playedTS) << "ms / Buffer: " << (sourceTS - playedTS) << "ms / Frequency ratio : " << throttle;
      }
      // Also detect end of stream and run callback if requested
      if (nQueueSize == 0 && me->m_flush)
         MsgPluginManager::GetInstance().GetMsgAPI().RunOnMainThread(0, FlushCallback, me);
   }

   static void FlushCallback(void* userdata)
   {
      AudioStreamPlayer* const me = static_cast<AudioStreamPlayer*>(userdata);
      if (me->m_flush && me->m_flushCallback)
         me->m_flushCallback();
      me->m_flush = false;
   }

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
