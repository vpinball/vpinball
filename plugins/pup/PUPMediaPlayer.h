// license:GPLv3+

#pragma once

#include "PUPManager.h"
#include "LibAv.h"
#include "ThreadPool.h"

namespace PUP {

class PUPMediaPlayer final
{
public:
   explicit PUPMediaPlayer(const string& name);
   ~PUPMediaPlayer();

   void SetGameTime(double gameTime);

   void Play(const std::filesystem::path& filename, float volume);
   bool IsPlaying() const;
   void Pause(bool pause);
   const std::filesystem::path& GetFilename() const { return m_filename; }
   int GetPriority() const { return m_priority; }
   void Stop();
   void SetVolume(float volume);
   void SetLoop(bool loop);
   void SetLength(int length);
   void Render(VPXRenderContext2D* const ctx, const SDL_Rect& destRect);

   void SetName(const string& name);
   void SetOnEndCallback(const std::function<void(PUPMediaPlayer*)>& onEndCallback) { std::lock_guard lock(m_mutex); m_onEndCallback = onEndCallback; }
   void SetBounds(const SDL_Rect& rect);
   void SetMask(std::shared_ptr<SDL_Surface> mask);

private:
   void StopBlocking();
   void Run();
   AVCodecContext* OpenStream(AVFormatContext* pInputFormatContext, int stream);
   void HandleAudioFrame(AVFrame* pFrame, bool sync);
   void HandleVideoFrame(AVFrame* pFrame, bool sync);

   string m_name;
   SDL_Rect m_bounds;

   std::function<void(PUPMediaPlayer*)> m_onEndCallback = [](PUPMediaPlayer*) { };

   double GetPlayTime() const;
   bool m_syncOnGameTime = false;
   double m_gameTime = -1.0;
   double m_startTimestamp = 0.0; // timestamp in seconds when the play command was called

   std::filesystem::path m_filename;
   bool m_loop = false;
   int m_playIndex = 0;
   float m_volume = 100.f;
   int m_length = 0;
   int m_priority = -1;

   bool m_paused = false;
   double m_pauseTimestamp = 0.0;

   AVFormatContext* m_pFormatContext = nullptr;

   int m_videoStream = -1;
   AVCodecContext* m_pVideoContext = nullptr;

   // Circular buffer of m_nRgbFrames frames, ready to be rendered if framePTS >= playPTS
   int m_activeRgbFrame = 0;
   vector<AVFrame*> m_rgbFrames;
   vector<VPXTexture> m_videoTextures;
   SwsContext* m_swsContext = nullptr;

   std::shared_ptr<SDL_Surface> m_mask = nullptr;
   std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> m_scaledMask;

   VPXTexture m_videoTexture = nullptr;
   unsigned int m_videoTextureId = 0xFFFFFF;

   int m_audioStream = -1;
   AVCodecContext* m_pAudioContext = nullptr;
   struct SwrContext* m_pAudioConversionContext = nullptr;
   AVSampleFormat m_audioFormat = AV_SAMPLE_FMT_NONE;
   void* m_pAudioLoop = nullptr;
   int m_audioFreq = 0;
   CtlResId m_audioResId {};

   std::mutex m_mutex;
   std::thread m_thread;
   bool m_running = false;

   const LibAV::LibAV& m_libAv;

   ThreadPool m_commandQueue;
};

}
