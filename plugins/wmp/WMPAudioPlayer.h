#pragma once

#include "common.h"
#include "miniaudio_private.h"
#include <thread>
#include <atomic>

namespace WMP {

class WMPCore;

class WMPAudioPlayer
{
public:
   WMPAudioPlayer(MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int onAudioUpdateId);
   ~WMPAudioPlayer();

   bool LoadFile(const string& filepath);
   void UnloadFile();
   void Play();
   void Pause();
   void Stop();
   double GetPosition();
   void SetPosition(double positionInSeconds);
   bool IsLoaded() const { return m_isLoaded; }
   bool IsPlaying() const { return m_isPlaying; }
   bool IsPaused() const { return m_isPaused; }
   float GetVolume() const { return m_volume; }
   void SetVolume(float volume);
   void UpdateVolume(long volume, bool mute);

private:
   void StartStreaming();
   void StopStreaming();
   void SendAudioChunk(const float* samples, size_t frameCount);

   MsgPluginAPI* const m_msgApi;
   const uint32_t m_endpointId;
   const unsigned int m_onAudioUpdateId;

   ma_decoder m_decoder;

   std::atomic<bool> m_isLoaded;
   std::atomic<bool> m_isPlaying;
   std::atomic<bool> m_isPaused;
   std::atomic<float> m_volume;

   std::thread m_thread;
   std::atomic<bool> m_shouldStopStreaming;

   ma_uint32 m_sampleRate;
   ma_uint32 m_channels;
   string m_loadedFile;

   CtlResId m_audioResId;
   static constexpr size_t BUFFER_SIZE_FRAMES = 1024;
};

}
