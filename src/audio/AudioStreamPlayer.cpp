// license:GPLv3+

#include "core/stdafx.h"
#include "AudioStreamPlayer.h"
#include "utils/denormals.h"

#include "miniaudio/miniaudio.h"

namespace VPX
{

// A custom ma_data_source that drains a lock-free PCM ring buffer. The plugin pushes PCM
// through Enqueue (producer); miniaudio pulls through onRead on the audio thread (consumer).
// On underflow it outputs silence rather than stalling the graph.
struct vpx_stream_source
{
   ma_data_source_base base;
   ma_pcm_rb* rb;
   ma_format format;
   ma_uint32 channels;
   ma_uint32 sampleRate;
};

static ma_result vpx_stream_source_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
{
   auto* src = reinterpret_cast<vpx_stream_source*>(pDataSource);
   const ma_uint32 bpf = ma_get_bytes_per_frame(src->format, src->channels);
   ma_uint64 totalRead = 0;
   auto* out = static_cast<ma_uint8*>(pFramesOut);

   while (totalRead < frameCount)
   {
      ma_uint32 avail = (ma_uint32)(frameCount - totalRead);
      void* pRead = nullptr;
      if (ma_pcm_rb_acquire_read(src->rb, &avail, &pRead) != MA_SUCCESS || avail == 0)
         break;
      memcpy(out + totalRead * bpf, pRead, (size_t)avail * bpf);
      ma_pcm_rb_commit_read(src->rb, avail);
      totalRead += avail;
   }

   // Underflow: pad the rest with silence so the graph keeps running cleanly.
   if (totalRead < frameCount)
      memset(out + totalRead * bpf, 0, (size_t)(frameCount - totalRead) * bpf);

   if (pFramesRead)
      *pFramesRead = frameCount; // always report full (silence-padded) so the sound never ends
   return MA_SUCCESS;
}

static ma_result vpx_stream_source_get_format(
   ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
   auto* src = reinterpret_cast<vpx_stream_source*>(pDataSource);
   if (pFormat)
      *pFormat = src->format;
   if (pChannels)
      *pChannels = src->channels;
   if (pSampleRate)
      *pSampleRate = src->sampleRate;
   if (pChannelMap)
      ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, src->channels);
   return MA_SUCCESS;
}

static ma_data_source_vtable g_vpx_stream_source_vtable = {
   vpx_stream_source_read,
   nullptr, // onSeek: not seekable
   vpx_stream_source_get_format,
   nullptr, // onGetCursor
   nullptr, // onGetLength
   nullptr, // onSetLooping
   0
};

struct AudioStreamPlayer::Impl
{
   ma_engine* engine = nullptr;
   ma_pcm_rb rb {};
   bool rbInit = false;
   vpx_stream_source source {};
   bool sourceInit = false;
   ma_sound sound {};
   bool soundInit = false;
   bool started = false; // playback deferred until the pre-roll threshold is buffered
   ma_uint32 prerollFrames = 0; // start playing once this many frames are buffered
   bool resync = false; // set when the backlog grows too large: next Enqueue drops it and jumps live
};

std::unique_ptr<AudioStreamPlayer> AudioStreamPlayer::Create(ma_engine* engine, void* attachTo, int frequency, int channels, bool isFloat)
{
   auto player = std::make_unique<AudioStreamPlayer>(engine, attachTo, frequency, channels, isFloat);
   if (!player->IsValid())
      return nullptr;
   return player;
}

AudioStreamPlayer::AudioStreamPlayer(ma_engine* engine, void* attachTo, int frequency, int channels, bool isFloat)
   : m_frequency(frequency)
   , m_channels(channels)
   , m_isFloat(isFloat)
{
   m_impl = std::make_unique<Impl>();
   m_impl->engine = engine;
   if (engine == nullptr)
      return;

   const ma_format format = isFloat ? ma_format_f32 : ma_format_s16;

   // Ring buffer: ~1s of audio at the source rate, to absorb the plugin's bursty pushes.
   const ma_uint32 rbFrames = (ma_uint32)max(frequency, 4096);
   // Pre-roll: hold playback until ~150ms is buffered, so the pull side (which runs
   // continuously at the device rate) does not drain an empty buffer and underrun.
   m_impl->prerollFrames = (ma_uint32)max(frequency / 7, 1024);
   if (ma_pcm_rb_init(format, channels, rbFrames, nullptr, nullptr, &m_impl->rb) != MA_SUCCESS)
   {
      PLOGE << "AudioStreamPlayer: failed to init ring buffer";
      return;
   }
   m_impl->rbInit = true;
   ma_pcm_rb_set_sample_rate(&m_impl->rb, (ma_uint32)frequency);

   // Custom data source over the ring buffer.
   ma_data_source_config dsConfig = ma_data_source_config_init();
   dsConfig.vtable = &g_vpx_stream_source_vtable;
   if (ma_data_source_init(&dsConfig, &m_impl->source.base) != MA_SUCCESS)
   {
      PLOGE << "AudioStreamPlayer: failed to init data source";
      return;
   }
   m_impl->source.rb = &m_impl->rb;
   m_impl->source.format = format;
   m_impl->source.channels = (ma_uint32)channels;
   m_impl->source.sampleRate = (ma_uint32)frequency;
   m_impl->sourceInit = true;

   // Sound node from the data source, attached to the caller-provided limiter node (shared
   // by all plugin streams), or the engine endpoint if none was given.
   ma_sound_config soundConfig = ma_sound_config_init_2(engine);
   soundConfig.pDataSource = &m_impl->source;
   soundConfig.flags = MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_STREAM;
   soundConfig.pInitialAttachment = attachTo ? static_cast<ma_node*>(attachTo) : static_cast<ma_node*>(ma_engine_get_endpoint(engine));
   if (ma_sound_init_ex(engine, &soundConfig, &m_impl->sound) != MA_SUCCESS)
   {
      PLOGE << "AudioStreamPlayer: failed to init sound";
      return;
   }
   m_impl->soundInit = true;
   ApplyVolume();
   // Do not start yet: wait until Enqueue has buffered prerollFrames (see Enqueue).

   m_valid = true;
}

AudioStreamPlayer::~AudioStreamPlayer()
{
   if (m_impl)
   {
      if (m_impl->soundInit)
      {
         ma_sound_stop(&m_impl->sound);
         ma_sound_uninit(&m_impl->sound);
      }
      if (m_impl->sourceInit)
         ma_data_source_uninit(&m_impl->source.base);
      if (m_impl->rbInit)
         ma_pcm_rb_uninit(&m_impl->rb);
   }
}

void AudioStreamPlayer::Enqueue(const uint8_t* buffer, int length)
{
   if (!m_valid || buffer == nullptr || length <= 0)
      return;

   const ma_format format = m_isFloat ? ma_format_f32 : ma_format_s16;
   const ma_uint32 bpf = ma_get_bytes_per_frame(format, (ma_uint32)m_channels);

   // If the backlog grew too large and we flagged a resync, drop everything buffered so a
   // stall costs bounded silence and audio jumps back near live instead of lagging video.
   if (m_impl->resync)
   {
      ma_pcm_rb_reset(&m_impl->rb);
      m_impl->resync = false;
   }

   ma_uint32 framesRemaining = (ma_uint32)(length / (int)bpf);
   const ma_uint8* in = buffer;
   while (framesRemaining > 0)
   {
      ma_uint32 toWrite = framesRemaining;
      void* pWrite = nullptr;
      if (ma_pcm_rb_acquire_write(&m_impl->rb, &toWrite, &pWrite) != MA_SUCCESS || toWrite == 0)
         break; // ring buffer full; drop the overflow (producer is ahead)
      memcpy(pWrite, in, (size_t)toWrite * bpf);
      ma_pcm_rb_commit_write(&m_impl->rb, toWrite);
      in += (size_t)toWrite * bpf;
      framesRemaining -= toWrite;
   }

   // Pre-roll: start playback only once enough is buffered to ride out the plugin's
   // bursty push cadence, so the continuous pull side does not immediately underrun.
   if (!m_impl->started && m_impl->soundInit
      && ma_pcm_rb_available_read(&m_impl->rb) >= m_impl->prerollFrames)
   {
      ma_sound_start(&m_impl->sound);
      m_impl->started = true;
   }
   if (!m_impl->started)
      return;

   // Coarse drift safety net: if the backlog grows very large (the plugin pushed far more
   // than has been consumed, e.g. after a long stall), drop the stale backlog on the next
   // push so audio jumps back near live instead of lagging video indefinitely. Plugin audio
   // is source-paced, so the buffer otherwise stays level without per-call rate correction;
   // chasing wall-clock with pitch adjustments mistakes normal buffering latency for drift.
   const uint32_t queuedMs = (uint32_t)((1000ull * ma_pcm_rb_available_read(&m_impl->rb)) / (uint64_t)m_frequency);
   if (queuedMs > 1000)
      m_impl->resync = true;
}

void AudioStreamPlayer::FlushStream()
{
   // Nothing to flush eagerly: the ring buffer drains on its own as the engine pulls.
}

int AudioStreamPlayer::GetQueuedSize() const
{
   if (!m_valid)
      return 0;
   const ma_format format = m_isFloat ? ma_format_f32 : ma_format_s16;
   const ma_uint32 bpf = ma_get_bytes_per_frame(format, (ma_uint32)m_channels);
   return (int)(ma_pcm_rb_available_read(&m_impl->rb) * bpf);
}

void AudioStreamPlayer::ApplyVolume()
{
   if (m_impl && m_impl->soundInit)
      ma_sound_set_volume(&m_impl->sound, m_streamVolume * m_mainVolume);
}

void AudioStreamPlayer::SetStreamVolume(const float volume)
{
   if (m_streamVolume != volume)
   {
      m_streamVolume = volume;
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
