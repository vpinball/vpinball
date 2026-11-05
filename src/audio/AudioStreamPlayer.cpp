// license:GPLv3+

#include "core/stdafx.h"
#include "AudioStreamPlayer.h"
#include "SpatialAudio.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#include "miniaudio/miniaudio.h"

namespace VPX
{

// Custom data source exposing the stream's queued frames to a miniaudio sound.
// Underrun outputs silence: a live audio stream has no end.
struct vpx_stream_data_source
{
   ma_data_source_base base;
   AudioStreamPlayer* player;

   static ma_result Read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
   {
      auto* ds = static_cast<vpx_stream_data_source*>(pDataSource);
      *pFramesRead = ds->player->ReadFrames(static_cast<float*>(pFramesOut), frameCount);
      return MA_SUCCESS;
   }

   static ma_result Seek(ma_data_source* pDataSource, ma_uint64 frameIndex)
   {
      return MA_NOT_IMPLEMENTED;
   }

   static ma_result GetDataFormat(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap,
      size_t channelMapCap)
   {
      auto* ds = static_cast<vpx_stream_data_source*>(pDataSource);
      *pFormat = ma_format_f32;
      *pChannels = ds->player->m_channels;
      *pSampleRate = ds->player->m_frequency;
      return MA_SUCCESS;
   }

   static ma_result GetCursor(ma_data_source* pDataSource, ma_uint64* pCursor)
   {
      auto* ds = static_cast<vpx_stream_data_source*>(pDataSource);
      std::lock_guard lock(ds->player->m_mutex);
      *pCursor = ds->player->m_readIdx;
      return MA_SUCCESS;
   }

   static ma_result GetLength(ma_data_source* pDataSource, ma_uint64* pLength)
   {
      return MA_NOT_IMPLEMENTED;
   }

   static ma_result SetLooping(ma_data_source* pDataSource, ma_bool32 isLooping)
   {
      return MA_NOT_IMPLEMENTED;
   }
};

static ma_data_source_vtable vpx_stream_ds_vtable = { vpx_stream_data_source::Read, vpx_stream_data_source::Seek, vpx_stream_data_source::GetDataFormat,
   vpx_stream_data_source::GetCursor, vpx_stream_data_source::GetLength, vpx_stream_data_source::SetLooping, 0 };

std::unique_ptr<AudioStreamPlayer> AudioStreamPlayer::Create(ma_engine* engine, SpatialAudioMixer* mixer, int frequency, int channels, bool isFloat)
{
   if (engine == nullptr || frequency <= 0 || (channels != 1 && channels != 2))
      return nullptr;

   std::unique_ptr<AudioStreamPlayer> player(new AudioStreamPlayer());
   player->m_frequency = static_cast<uint32_t>(frequency);
   player->m_channels = static_cast<uint32_t>(channels);
   player->m_isFloat = isFloat;
   player->m_capFrames = static_cast<size_t>(frequency); // Never lag by more than one second of audio
   player->m_buffer.assign(player->m_capFrames * player->m_channels, 0.f);

   player->m_dataSource = std::make_unique<vpx_stream_data_source>();
   player->m_dataSource->player = player.get();
   ma_data_source_config dsConfig = ma_data_source_config_init();
   dsConfig.vtable = &vpx_stream_ds_vtable;
   if (ma_data_source_init(&dsConfig, &player->m_dataSource->base) != MA_SUCCESS)
      return nullptr;

   // Attach to the backglass spatial source when spatial audio is enabled, like the other backglass sounds
   if (mixer != nullptr && mixer->GetMixBus() != nullptr)
   {
      player->m_spatialSource = std::make_unique<SpatialAudioSource>(engine, mixer, static_cast<unsigned int>(channels));
      if (player->m_spatialSource->GetNode() == nullptr)
         player->m_spatialSource = nullptr;
      else
      {
         player->m_spatialSource->SetPositionPolar(0.f, 15.f, 1.f);
         player->m_spatialSource->SetReferenceDistance(1.f);
      }
   }

   // TODO a stream not at the engine rate goes through the sound's pitch resampler, which miniaudio runs without low-pass filter
   // (lpfOrder = 0, for biquad stability): more aliasing than the former SDL3 converter (e.g. 44.1 kHz PUP videos on a 48 kHz
   // device, while PinMAME/AltSound render at the engine rate). Revisit with the next big miniaudio update, or resample to the
   // engine rate in the data source (custom resampler or SDL_AudioStream)
   player->m_sound = std::make_unique<ma_sound>();
   ma_sound_config config = ma_sound_config_init_2(engine);
   config.pDataSource = &player->m_dataSource->base;
   config.flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
   config.pInitialAttachment
      = player->m_spatialSource ? static_cast<ma_node*>(player->m_spatialSource->GetNode()) : ma_engine_get_endpoint(engine);
   if (ma_sound_init_ex(engine, &config, player->m_sound.get()) != MA_SUCCESS)
   {
      player->m_sound = nullptr;
      return nullptr;
   }

   ma_sound_start(player->m_sound.get());
   return player;
}

AudioStreamPlayer::~AudioStreamPlayer()
{
   if (m_sound)
      ma_sound_uninit(m_sound.get());
   m_spatialSource = nullptr;
   if (m_dataSource)
      ma_data_source_uninit(&m_dataSource->base);
}

void AudioStreamPlayer::Enqueue(const uint8_t* buffer, int length)
{
   const int bytesPerSample = m_isFloat ? (int)sizeof(float) : (int)sizeof(int16_t);
   const size_t nFrames = static_cast<size_t>(length) / (m_channels * bytesPerSample);
   std::lock_guard lock(m_mutex);
   size_t first = 0;
   if (m_writeIdx - m_readIdx + nFrames > m_capFrames)
   {
      // Over 1 sec queued (the source runs ahead of playback, e.g. after an audio thread stall): resync by dropping all queued
      // data, as dropping only the oldest frames would keep the latency at its maximum from then on
      PLOGI << "Audio stream " << m_name << " lost sync and was reset";
      m_readIdx = m_writeIdx;
      if (nFrames > m_capFrames)
         first = nFrames - m_capFrames;
   }
   for (size_t i = first; i < nFrames; ++i)
   {
      float* dst = &m_buffer[(m_writeIdx % m_capFrames) * m_channels];
      if (m_isFloat)
         memcpy(dst, buffer + i * m_channels * sizeof(float), m_channels * sizeof(float));
      else
         for (uint32_t ch = 0; ch < m_channels; ++ch)
            dst[ch] = reinterpret_cast<const int16_t*>(buffer)[i * m_channels + ch] / 32768.f;
      ++m_writeIdx;
   }
}

uint64_t AudioStreamPlayer::ReadFrames(float* out, uint64_t frameCount)
{
   std::lock_guard lock(m_mutex);
   const uint64_t avail = m_writeIdx - m_readIdx;
   uint64_t n = avail < frameCount ? avail : frameCount;
   if (out == nullptr)
   {
      // A read without output buffer is a forward seek (miniaudio data source contract)
      m_readIdx += n;
      return n;
   }
   for (uint64_t i = 0; i < n; ++i)
      memcpy(&out[i * m_channels], &m_buffer[((m_readIdx + i) % m_capFrames) * m_channels], m_channels * sizeof(float));
   m_readIdx += n;
   if (n < frameCount && !m_eof.load(std::memory_order_relaxed))
   {
      // Underrun on a live stream: play silence and keep the sound alive
      memset(&out[n * m_channels], 0, static_cast<size_t>(frameCount - n) * m_channels * sizeof(float));
      n = frameCount;
   }
   return n;
}

void AudioStreamPlayer::FlushStream()
{
   m_eof = true;
}

bool AudioStreamPlayer::IsFinished() const
{
   return m_sound == nullptr || ma_sound_at_end(m_sound.get());
}

int AudioStreamPlayer::GetQueuedSize() const
{
   std::lock_guard lock(m_mutex);
   return static_cast<int>((m_writeIdx - m_readIdx) * m_channels * sizeof(float));
}

void AudioStreamPlayer::ApplyVolume()
{
   const float gain = sqrtf(m_streamVolume * m_mainVolume);
   if (m_spatialSource)
      m_spatialSource->SetVolume(gain);
   else if (m_sound)
      ma_sound_set_volume(m_sound.get(), gain);
}

void AudioStreamPlayer::SetStreamVolume(const float volume)
{
   if (m_streamVolume != volume)
   {
      m_streamVolume = volume;
      // sqrt loudness curve, matching SoundPlayer::ApplyVolume so the shared
      // backglass volume setting produces the same perceived level
      ApplyVolume();
   }
}

void AudioStreamPlayer::SetMainVolume(const float volume)
{
   if (m_mainVolume != volume)
   {
      m_mainVolume = volume;
      ApplyVolume();
   }
}

}
