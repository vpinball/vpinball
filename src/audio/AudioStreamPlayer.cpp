// license:GPLv3+

#include "core/stdafx.h"
#include "AudioStreamPlayer.h"
#include "AudioPlayer.h"
#include "MixNode.h"
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

std::unique_ptr<AudioStreamPlayer> AudioStreamPlayer::Create(const AudioPlayer* audioPlayer, int frequency, int channels, bool isFloat)
{
   ma_engine* engine = audioPlayer->GetEngine(SoundOutTypes::SNDOUT_BACKGLASS);
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

   // Attach to the backglass spatial source when spatial audio is enabled, positioned on the backglass speakers
   SpatialAudioMixer* mixer = audioPlayer->GetSpatialMixer(SoundOutTypes::SNDOUT_BACKGLASS);
   if (mixer != nullptr && mixer->GetMixBus() != nullptr)
   {
      player->m_spatialSource = std::make_unique<SpatialAudioSource>(engine, mixer, static_cast<unsigned int>(channels));
      if (player->m_spatialSource->GetNode() == nullptr)
         player->m_spatialSource = nullptr;
      else
         audioPlayer->PositionBackglassSource(*player->m_spatialSource);
   }

   // Without spatial audio, a multichannel device needs the mixing node too: miniaudio's channel conversion
   // (simple mode, default channel maps) also maps stereo FL/FR onto SL/SR, leaking backglass audio into the
   // side (playfield) channels of 6ch/8ch setups. The Backglass mix mode only drives the front pair.
   const bool frontLeft = audioPlayer->GetBackglassMode() == SNDCFG_BGFRONTLEFT;
   if (player->m_spatialSource == nullptr && (frontLeft || ma_engine_get_channels(engine) > 2))
   {
      // Route through the channel mixing node to downmix to the front left channel, or to keep the
      // stream on the front pair of a multichannel device (on mono and stereo devices, the 2 front
      // channel mode goes straight to the endpoint, which handles the channel mapping)
      player->m_vpxMixNode = std::make_unique<vpx_node>();
      const ma_result result = vpx_node_init(ma_engine_get_node_graph(engine), static_cast<unsigned int>(channels), ma_engine_get_channels(engine), nullptr, player->m_vpxMixNode.get());
      if (result != MA_SUCCESS)
      {
         PLOGE << "Failed to initialize custom mixer.";
         player->m_vpxMixNode = nullptr;
         return nullptr;
      }
      player->m_vpxMixNode->mode = frontLeft ? SoundMixMode::BackglassFrontLeft : SoundMixMode::Backglass;
      player->m_vpxMixNode->volume = 1.f; // Volume is applied on the sound, not on the mixing node
      ma_node_attach_output_bus(player->m_vpxMixNode.get(), 0, ma_engine_get_endpoint(engine), 0);
   }

   // TODO a stream not at the engine rate goes through the sound's pitch resampler, which miniaudio runs without low-pass filter
   // (lpfOrder = 0, for biquad stability): more aliasing than the former SDL3 converter (e.g. 44.1 kHz PUP videos on a 48 kHz
   // device, while PinMAME/AltSound render at the engine rate). Revisit with the next big miniaudio update, or resample to the
   // engine rate in the data source (custom resampler or SDL_AudioStream)
   player->m_sound = std::make_unique<ma_sound>();
   ma_sound_config config = ma_sound_config_init_2(engine);
   config.pDataSource = &player->m_dataSource->base;
   config.flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
   // Mono expands to the front stereo pair only: the default duplicate mode would leak the
   // stream on all the device channels (rears, LFE) when routing straight to the endpoint
   config.monoExpansionMode = ma_mono_expansion_mode_stereo_only;
   // Node attachments require matching channel counts: the endpoint expects the engine channel
   // count (0 = default) while the spatial source and mixing nodes declare the stream channel count
   config.channelsOut = (player->m_spatialSource || player->m_vpxMixNode) ? static_cast<ma_uint32>(channels) : 0;
   config.pInitialAttachment = player->m_spatialSource ? static_cast<ma_node*>(player->m_spatialSource->GetNode())
      : player->m_vpxMixNode                           ? static_cast<ma_node*>(player->m_vpxMixNode.get())
                                                       : ma_engine_get_endpoint(engine);
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
   DetachFromEngine();
   if (m_dataSource)
      ma_data_source_uninit(&m_dataSource->base);
}

void AudioStreamPlayer::DetachFromEngine()
{
   if (m_sound)
   {
      ma_sound_uninit(m_sound.get());
      m_sound = nullptr;
   }
   m_spatialSource = nullptr;
   if (m_vpxMixNode)
   {
      vpx_node_uninit(m_vpxMixNode.get(), nullptr);
      m_vpxMixNode = nullptr;
   }
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
