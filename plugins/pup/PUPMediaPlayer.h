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
   const std::filesystem::path& GetFilename() const { return m_filename; }
   bool IsPlaying();
   void Pause(bool pause);
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
   void HandleVideoFrame(AVFrame* pFrame);

   string m_name;
   SDL_Rect m_bounds;

   std::atomic<int> m_pendingPlay = 0;
   std::atomic<int> m_pendingStop = 0;

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

   bool m_paused = false;
   double m_pauseTimestamp = 0.0;

   AVFormatContext* m_pFormatContext = nullptr;

   int m_videoStream = -1;
   AVCodecContext* m_pVideoContext = nullptr;

   // Slots of unordered decoded frames
   struct FrameInfo
   {
      bool valid = false;
      AVFrame* frame = nullptr;
      VPXTexture texture = nullptr;
      double pts = -1.0;
      bool uploaded = false;
      int age = 0;
   };
   vector<FrameInfo> m_frames;
   SwsContext* m_swsContext = nullptr;

   std::shared_ptr<SDL_Surface> m_mask = nullptr;
   std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> m_scaledMask;

   int m_audioStream = -1;
   AVCodecContext* m_pAudioContext = nullptr;
   struct SwrContext* m_pAudioConversionContext = nullptr;
   AVSampleFormat m_audioFormat = AV_SAMPLE_FMT_NONE;
   void* m_pAudioLoop = nullptr;
   int m_audioFreq = 0;
   CtlResId m_audioResId {};

   std::mutex m_mutex;
   std::thread m_thread;
   std::atomic<bool> m_running = false;

   const LibAV::LibAV& m_libAv;

   ThreadPool m_commandQueue;
};

}
