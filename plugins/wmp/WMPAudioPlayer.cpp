// license:GPLv3+

#include "WMPAudioPlayer.h"
#include "WMPCore.h"
#include <algorithm>
#include <format>

namespace WMP {

struct AudioCallbackData {
   MsgPluginAPI* msgApi;
   uint32_t endpointId;
   unsigned int onAudioUpdateId;
   AudioUpdateMsg* msg;
};

static uint32_t nextAudioResId = 1;

WMPAudioPlayer::WMPAudioPlayer(MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int onAudioUpdateId) 
   : m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_onAudioUpdateId(onAudioUpdateId)
   , m_isLoaded(false)
   , m_isPlaying(false) 
   , m_isPaused(false)
   , m_volume(0.5f)
   , m_sampleRate(44100)
   , m_channels(2)
{
   m_audioResId.endpointId = m_endpointId;
   m_audioResId.resId = nextAudioResId++;
}

WMPAudioPlayer::~WMPAudioPlayer()
{
   UnloadFile();
}

bool WMPAudioPlayer::LoadFile(const string& filepath)
{
   UnloadFile();

   LOGI("Loading audio file: " + filepath);

   const ma_decoder_config config = wmp_ma_decoder_config_init(ma_format_f32, 0, 0);
   ma_result result = wmp_ma_decoder_init_file(filepath.c_str(), &config, &m_decoder);
   if (result != MA_SUCCESS) {
      LOGE("Failed to initialize decoder for file: " + filepath + " (error: " + std::to_string(result) + ')');
      return false;
   }

   ma_format format;
   ma_uint32 channels;
   ma_uint32 sampleRate;
   result = wmp_ma_decoder_get_data_format(&m_decoder, &format, &channels, &sampleRate, nullptr, 0);
   if (result != MA_SUCCESS) {
      LOGE("Failed to get decoder format info"s);
      wmp_ma_decoder_uninit(&m_decoder);
      return false;
   }

   m_sampleRate = sampleRate;
   m_channels = channels;

   result = wmp_ma_engine_init_null_device(m_channels, m_sampleRate, BUFFER_SIZE_FRAMES, EngineProcess, this, &m_context, &m_engine);
   if (result != MA_SUCCESS) {
      LOGE("Failed to initialize miniAudio engine (error: " + std::to_string(result) + ')');
      wmp_ma_decoder_uninit(&m_decoder);
      return false;
   }
   m_engineInited = true;

   result = wmp_ma_sound_init_from_decoder(&m_engine, &m_decoder, MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH, &m_sound);
   if (result != MA_SUCCESS) {
      LOGE("Failed to initialize miniAudio sound (error: " + std::to_string(result) + ')');
      wmp_ma_engine_uninit(&m_engine);
      wmp_ma_context_uninit(&m_context);
      m_engineInited = false;
      wmp_ma_decoder_uninit(&m_decoder);
      return false;
   }
   m_soundInited = true;
   wmp_ma_sound_set_volume(&m_sound, m_volume.load());
   wmp_ma_sound_set_end_callback(&m_sound, SoundEndCallback, this);

   m_loadedFile = filepath;
   m_isLoaded = true;

   // miniAudio owns the audio thread (a realtime-paced null device); it calls
   // EngineProcess with the mixed buffer, which we forward to the host. The
   // engine runs until UnloadFile(); playback is gated by m_isPlaying.
   wmp_ma_engine_start(&m_engine);

   LOGD("Audio loaded: " + std::to_string(sampleRate) + " Hz, " + std::to_string(channels) + " channels, format: f32");
   return true;
}

void WMPAudioPlayer::UnloadFile()
{
   Stop();

   if (m_isLoaded) {
      // Stop miniAudio's audio thread before tearing anything down so no
      // EngineProcess callback runs during teardown.
      if (m_engineInited)
         wmp_ma_engine_stop(&m_engine);
      if (m_soundInited) {
         wmp_ma_sound_uninit(&m_sound);
         m_soundInited = false;
      }
      if (m_engineInited) {
         wmp_ma_engine_uninit(&m_engine);
         wmp_ma_context_uninit(&m_context);
         m_engineInited = false;
      }
      wmp_ma_decoder_uninit(&m_decoder);
      m_isLoaded = false;
      m_loadedFile.clear();
      LOGI("Audio file unloaded"s);
   }
}

void WMPAudioPlayer::Play()
{
   if (!m_isLoaded)
      return;

   if (m_isPlaying && !m_isPaused)
      return;

   LOGI("Starting playback"s);

   m_endSignaled = false;
   m_isPaused = false;
   m_isPlaying = true;

   if (m_soundInited) {
      // Rewind if a previous playback ran to completion so this is a fresh play.
      if (wmp_ma_sound_at_end(&m_sound))
         wmp_ma_sound_seek_to_pcm_frame(&m_sound, 0);
      wmp_ma_sound_start(&m_sound);
   }
}

void WMPAudioPlayer::Pause()
{
   if (!m_isPlaying)
      return;

   LOGI("Pausing playback"s);

   m_isPaused = true;

   if (m_soundInited)
      wmp_ma_sound_stop(&m_sound);

   SendClear();
}

void WMPAudioPlayer::Stop()
{
   if (!m_isPlaying)
      return;

   LOGI("Stopping playback"s);

   m_isPlaying = false;
   m_isPaused = false;
   m_endSignaled = false;

   if (m_soundInited) {
      wmp_ma_sound_stop(&m_sound);
      wmp_ma_sound_seek_to_pcm_frame(&m_sound, 0);
   }

   SendClear();
}

double WMPAudioPlayer::GetPosition()
{
   if (!m_isLoaded || m_sampleRate == 0)
      return 0.0;

   ma_uint64 currentFrame = 0;
   const ma_result result = wmp_ma_decoder_get_cursor_in_pcm_frames(&m_decoder, &currentFrame);

   if (result != MA_SUCCESS) {
      LOGE("Failed to get decoder cursor position"s);
      return 0.0;
   }

   return (double)currentFrame / (double)m_sampleRate;
}

void WMPAudioPlayer::SetPosition(double positionInSeconds)
{
   if (!m_isLoaded)
      return;

   const ma_uint64 targetFrame = (ma_uint64)(positionInSeconds * m_sampleRate);
   const ma_result result = m_soundInited
      ? wmp_ma_sound_seek_to_pcm_frame(&m_sound, targetFrame)
      : wmp_ma_decoder_seek_to_pcm_frame(&m_decoder, targetFrame);

   if (result == MA_SUCCESS) {
      LOGI(std::format("Seek to position: {:.2f} seconds (frame {})", positionInSeconds, targetFrame));
   }
   else {
      LOGE(std::format("Failed to seek to position: {:.2f} seconds", positionInSeconds));
   }
}

void WMPAudioPlayer::SetVolume(float volume)
{
   m_volume = volume;
   if (m_soundInited)
      wmp_ma_sound_set_volume(&m_sound, m_volume.load());
   LOGI(std::format("Volume set to: {:.2f}", m_volume.load()));
}

void WMPAudioPlayer::UpdateVolume(int volume, bool mute)
{
   int clampedVolume = std::max(0, std::min(volume, 100));

   float audioVolume = static_cast<float>(clampedVolume) / 100.0f;
   if (mute)
      audioVolume = 0.0f;

   m_volume = audioVolume;
   if (m_soundInited)
      wmp_ma_sound_set_volume(&m_sound, m_volume.load());
   LOGI(std::format("Volume updated: {}{} -> {:.2f}", clampedVolume, mute ? " (muted)" : "", m_volume.load()));
}

void WMPAudioPlayer::EngineProcess(void* pUserData, float* pFramesOut, ma_uint64 frameCount)
{
   static_cast<WMPAudioPlayer*>(pUserData)->OnEngineProcess(pFramesOut, frameCount);
}

void WMPAudioPlayer::SoundEndCallback(void* pUserData, ma_sound* pSound)
{
   // Fired by miniAudio (audio thread) the moment the sound reaches its end. Per
   // miniAudio's contract we only set a flag here; OnEngineProcess acts on it.
   static_cast<WMPAudioPlayer*>(pUserData)->m_endSignaled = true;
}

void WMPAudioPlayer::OnEngineProcess(float* pFramesOut, ma_uint64 frameCount)
{
   // Called on miniAudio's audio thread with the mixed buffer. Forward it to the
   // host only while actively playing; the engine keeps running when idle.
   if (!m_isPlaying || m_isPaused)
      return;

   SendAudioChunk(pFramesOut, (size_t)frameCount);

   // This buffer carried the tail of the sound; finish playback after sending it.
   if (m_endSignaled.exchange(false)) {
      m_isPlaying = false;
      SendClear();
      LOGI("Playback completed"s);
   }
}

void WMPAudioPlayer::SendClear()
{
   AudioUpdateMsg* pAudioUpdateMsg = new AudioUpdateMsg();
   pAudioUpdateMsg->id = m_audioResId;
   pAudioUpdateMsg->buffer = nullptr;
   pAudioUpdateMsg->bufferSize = 0;

   AudioCallbackData* cbData = new AudioCallbackData{m_msgApi, m_endpointId, m_onAudioUpdateId, pAudioUpdateMsg};

   m_msgApi->RunOnMainThread(m_endpointId, 0, [](void* userData) {
      AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
      data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
      delete data->msg;
      delete data;
   }, cbData);
}

void WMPAudioPlayer::SendAudioChunk(const float* samples, size_t frameCount)
{
   if (!m_msgApi || m_onAudioUpdateId == 0)
      return;

   const size_t bufferSizeBytes = frameCount * m_channels * sizeof(float);

   AudioUpdateMsg* pAudioUpdateMsg = new AudioUpdateMsg();
   pAudioUpdateMsg->id = m_audioResId;
   pAudioUpdateMsg->type = (m_channels == 1) ? CTLPI_AUDIO_SRC_BACKGLASS_MONO : CTLPI_AUDIO_SRC_BACKGLASS_STEREO;
   pAudioUpdateMsg->format = CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT;
   pAudioUpdateMsg->sampleRate = m_sampleRate;
   pAudioUpdateMsg->volume = m_volume.load();
   pAudioUpdateMsg->bufferSize = static_cast<unsigned int>(bufferSizeBytes);
   pAudioUpdateMsg->buffer = new uint8_t[bufferSizeBytes];

   memcpy(pAudioUpdateMsg->buffer, samples, bufferSizeBytes);

   AudioCallbackData* cbData = new AudioCallbackData{m_msgApi, m_endpointId, m_onAudioUpdateId, pAudioUpdateMsg};

   m_msgApi->RunOnMainThread(m_endpointId, 0, [](void* userData) {
      AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
      data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
      delete[] data->msg->buffer;
      delete data->msg;
      delete data;
   }, cbData);
}

}
