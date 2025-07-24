// license:GPLv3+

#include "core/stdafx.h"
#include "AudioStreamPlayer.h"

namespace VPX
{

std::unique_ptr<AudioStreamPlayer> AudioStreamPlayer::Create(SDL_AudioDeviceID sdlDevice, int frequency, int channels, bool isFloat)
{
   SDL_AudioSpec streamSpec;
   streamSpec.freq = frequency;
   streamSpec.format = isFloat ? SDL_AUDIO_F32 : SDL_AUDIO_S16;
   streamSpec.channels = channels;
   SDL_AudioSpec deviceSpec;
   SDL_GetAudioDeviceFormat(sdlDevice, &deviceSpec, nullptr);
   SDL_AudioStream* stream = SDL_CreateAudioStream(&streamSpec, &deviceSpec);
   if (stream)
   {
      SDL_BindAudioStream(sdlDevice, stream);
      SDL_ResumeAudioStreamDevice(stream);
      return std::make_unique<AudioStreamPlayer>(stream);
   }
   else
   {
      PLOGE << "Failed to create stream: " << SDL_GetError();
      return nullptr;
   }
}

AudioStreamPlayer::AudioStreamPlayer(SDL_AudioStream* stream)
   : m_stream(stream)
   #ifdef ENABLE_DX9
   , m_startTimestamp(msec())
   #else
   , m_startTimestamp(SDL_GetTicks())
   #endif
{
   assert(stream != nullptr);
   SDL_GetAudioStreamFormat(m_stream, &m_audioSpec, nullptr);
   SDL_SetAudioStreamGetCallback(m_stream, &AudioStreamCallback, this);
}

AudioStreamPlayer::~AudioStreamPlayer()
{
   SDL_DestroyAudioStream(m_stream);
}

void AudioStreamPlayer::Enqueue(const uint8_t* buffer, int length)
{
   // If we are really late and decided to resync, do it when pushing new data (limit silence time, handle situation where no more data are coming)
   if (m_resync)
   {
      SDL_ClearAudioStream(m_stream);
      #ifdef ENABLE_DX9
      m_startTimestamp = msec();
      #else
      m_startTimestamp = SDL_GetTicks();
      #endif
      m_streamedTotal = 0;
      m_resync = false;
      PLOGI << "Audio stream sync was lost and reseted";
   }
   // Just enqueue, syncing and compensation is done on a regular basis when data is requested for playing
   SDL_PutAudioStreamData(m_stream, buffer, length);
   m_streamedTotal += length;
}

void AudioStreamPlayer::FlushStream()
{
   SDL_FlushAudioStream(m_stream);
}

int AudioStreamPlayer::GetQueuedSize() const
{
   return SDL_GetAudioStreamQueued(m_stream);
}

void AudioStreamPlayer::SetStreamVolume(const float volume)
{
   if (m_streamVolume != volume)
   {
      m_streamVolume = volume;
      SDL_SetAudioStreamGain(m_stream, m_streamVolume * m_mainVolume);
   }
}

void AudioStreamPlayer::SetMainVolume(const float volume)
{
   if (m_mainVolume != volume)
   {
      m_mainVolume = volume;
      SDL_SetAudioStreamGain(m_stream, m_streamVolume * m_mainVolume);
   }
}

void AudioStreamPlayer::AudioStreamCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
   auto const me = static_cast<AudioStreamPlayer*>(userdata);
   const int nQueueSize = max(0, SDL_GetAudioStreamQueued(stream) - total_amount);
   const uint64_t nBytePerSec = me->m_audioSpec.freq * SDL_AUDIO_FRAMESIZE(me->m_audioSpec);
   const uint64_t sourceTS = (1000 * me->m_streamedTotal) / nBytePerSec; // Total amount of music streamed (ms)
   const uint64_t playedTS = (1000 * (me->m_streamedTotal - nQueueSize)) / nBytePerSec; // Playing position (ms)
   #ifdef ENABLE_DX9
   const uint64_t nowTS = msec() - me->m_startTimestamp; // Where we should be in the music (ms)
   #else
   const uint64_t nowTS = SDL_GetTicks() - me->m_startTimestamp; // Where we should be in the music (ms)
   #endif
   float throttle = 1.f;
   //PLOGI << "Get stream data for " << me->m_name.c_str() << " enqueued: " << ((float)SDL_GetAudioStreamQueued(stream) / SDL_AUDIO_FRAMESIZE(me->m_audioSpec)) << " samples enqueued";
   if (playedTS > nowTS)
   {
      // We have played ahead of the source: either the source is paused or it is having issues => just resync silently
      me->m_startTimestamp += playedTS - nowTS;
   }
   else if (nowTS > playedTS)
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
         throttle = 1.0f + static_cast<float>(min(deltaTS - 20, 500ull)) / 500.f;
      }*/
   }
   if (me->m_throttling != throttle)
   {
      me->m_throttling = throttle;
      SDL_SetAudioStreamFrequencyRatio(me->m_stream, throttle);
      PLOGI << "PlayedTS: " << playedTS << "ms / NowTS: " << nowTS << "ms / Delta: " << (nowTS - playedTS) << "ms / Buffer: " << (sourceTS - playedTS) << "ms / Frequency ratio : " << throttle;
   }
}

}
