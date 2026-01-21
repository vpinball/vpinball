// license:GPLv3+

#include "WMPAudioPlayer.h"
#include "WMPCore.h"
#include <algorithm>

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
   , m_shouldStopStreaming(false)
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

   LOGI("Loading audio file: %s", filepath.c_str());

   const ma_decoder_config config = wmp_ma_decoder_config_init(ma_format_f32, 0, 0);
   ma_result result = wmp_ma_decoder_init_file(filepath.c_str(), &config, &m_decoder);
   if (result != MA_SUCCESS) {
      LOGE("Failed to initialize decoder for file: %s (error: %d)", filepath.c_str(), result);
      return false;
   }

   ma_format format;
   ma_uint32 channels;
   ma_uint32 sampleRate;
   result = wmp_ma_decoder_get_data_format(&m_decoder, &format, &channels, &sampleRate, nullptr, 0);
   if (result != MA_SUCCESS) {
      LOGE("Failed to get decoder format info");
      wmp_ma_decoder_uninit(&m_decoder);
      return false;
   }

   m_sampleRate = sampleRate;
   m_channels = channels;
   m_loadedFile = filepath;
   m_isLoaded = true;

   LOGD("Audio loaded: %d Hz, %d channels, format: f32", sampleRate, channels);
   return true;
}

void WMPAudioPlayer::UnloadFile()
{
   Stop();

   if (m_isLoaded) {
      wmp_ma_decoder_uninit(&m_decoder);
      m_isLoaded = false;
      m_loadedFile.clear();
      LOGI("Audio file unloaded");
   }
}

void WMPAudioPlayer::Play()
{
   if (!m_isLoaded)
      return;

   if (m_isPlaying && !m_isPaused)
      return;

   LOGI("Starting playback");

   m_isPaused = false;
   m_isPlaying = true;

   StartStreaming();
}

void WMPAudioPlayer::Pause()
{
   if (!m_isPlaying)
      return;

   LOGI("Pausing playback");

   m_isPaused = true;
   StopStreaming();
}

void WMPAudioPlayer::Stop()
{
   if (!m_isPlaying)
      return;

   LOGI("Stopping playback");

   m_isPlaying = false;
   m_isPaused = false;

   StopStreaming();
   if (m_isLoaded)
      wmp_ma_decoder_seek_to_pcm_frame(&m_decoder, 0);
}

double WMPAudioPlayer::GetPosition()
{
   if (!m_isLoaded || m_sampleRate == 0)
      return 0.0;

   ma_uint64 currentFrame = 0;
   const ma_result result = wmp_ma_decoder_get_cursor_in_pcm_frames(&m_decoder, &currentFrame);

   if (result != MA_SUCCESS) {
      LOGE("Failed to get decoder cursor position");
      return 0.0;
   }

   return (double)currentFrame / (double)m_sampleRate;
}

void WMPAudioPlayer::SetPosition(double positionInSeconds)
{
   if (!m_isLoaded)
      return;

   const ma_uint64 targetFrame = (ma_uint64)(positionInSeconds * m_sampleRate);
   const ma_result result = wmp_ma_decoder_seek_to_pcm_frame(&m_decoder, targetFrame);

   if (result == MA_SUCCESS) {
      LOGI("Seek to position: %.2f seconds (frame %llu)", positionInSeconds, targetFrame);
   }
   else {
      LOGE("Failed to seek to position: %.2f seconds", positionInSeconds);
   }
}

void WMPAudioPlayer::SetVolume(float volume)
{
   m_volume = volume;
   LOGI("Volume set to: %.2f", m_volume.load());
}

void WMPAudioPlayer::UpdateVolume(long volume, bool mute)
{
   long clampedVolume = std::max(0L, std::min<long>(volume, 100L));

   float audioVolume = static_cast<float>(clampedVolume) / 100.0f;
   if (mute)
      audioVolume = 0.0f;

   m_volume = audioVolume;
   LOGI("Volume updated: %ld%s -> %.2f", clampedVolume, mute ? " (muted)" : "", m_volume.load());
}

void WMPAudioPlayer::StartStreaming()
{
   if (m_thread.joinable()) {
      m_shouldStopStreaming = true;
      m_thread.join();
   }

   m_shouldStopStreaming = false;

   m_thread = std::thread([this]() {
      constexpr size_t bufferSizeFrames = BUFFER_SIZE_FRAMES;
      float* audioBuffer = new float[bufferSizeFrames * m_channels];
   
      while (!m_shouldStopStreaming && m_isPlaying && !m_isPaused) {
         ma_uint64 framesRead = 0;
         const ma_result result = wmp_ma_decoder_read_pcm_frames(&m_decoder, audioBuffer, bufferSizeFrames, &framesRead);

         if (result != MA_SUCCESS || framesRead == 0) {
            LOGI("End of audio stream reached");
            break;
         }

         float volume = m_volume.load();
         if (volume != 1.0f) {
            for (size_t i = 0; i < framesRead * m_channels; ++i)
               audioBuffer[i] *= volume;
         }

         SendAudioChunk(audioBuffer, (size_t)framesRead);

         double bufferDurationMs = (double)framesRead / (double)m_sampleRate * 1000.0;
         std::this_thread::sleep_for(std::chrono::microseconds((int)(bufferDurationMs * 800)));
      }

      if (!m_shouldStopStreaming) {
         AudioUpdateMsg* endMsg = new AudioUpdateMsg();
         endMsg->id = m_audioResId;
         endMsg->buffer = nullptr;
         endMsg->bufferSize = 0;

         AudioCallbackData* cbData = new AudioCallbackData{m_msgApi, m_endpointId, m_onAudioUpdateId, endMsg};

         m_msgApi->RunOnMainThread(m_endpointId, 0, [](void* userData) {
            AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
            data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
            delete data->msg;
            delete data;
         }, cbData);
      }

      delete[] audioBuffer;

      if (m_isPlaying && !m_shouldStopStreaming) {
         m_isPlaying = false;
         LOGI("Playback completed");
      }
   });
}

void WMPAudioPlayer::StopStreaming()
{
   m_shouldStopStreaming = true;

   if (m_thread.joinable())
      m_thread.join();

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
