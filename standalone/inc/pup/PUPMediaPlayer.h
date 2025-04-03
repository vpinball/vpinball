#pragma once

#include "PUPManager.h"
#include "audio/pinsound.h"
#include "../common/Window.h"

extern "C" {
   #include "libavutil/imgutils.h"
   #include "libavformat/avformat.h"
   #include "libavdevice/avdevice.h"
   #include "libswscale/swscale.h"
   #include "libswresample/swresample.h"
   #include "libavcodec/avcodec.h"
}

class PUPMediaPlayer
{
public:
   PUPMediaPlayer();
   ~PUPMediaPlayer();

   void SetRenderer(SDL_Renderer* pRenderer);
   void Play(const string& szFilename);
   bool IsPlaying();
   void Pause(bool pause);
   const string& GetFilename() const { return m_szFilename; }
   int GetPriority() const { return m_priority; }
   void Stop();
   void SetVolume(float volume);
   void SetLoop(bool loop);
   void Render(const SDL_Rect& destRect);

private:
   void Run();
   AVCodecContext* OpenStream(AVFormatContext* pInputFormatContext, int stream);
   SDL_PixelFormat GetVideoFormat(enum AVPixelFormat format);
   void SetYUVConversionMode(AVFrame* pFrame);
   void HandleAudioFrame(AVFrame* pFrame);

   string m_szFilename;
   bool m_loop;
   float m_volume;
   int m_priority;
   SDL_Renderer* m_pRenderer;
   SDL_Texture* m_pTexture;
   AVFormatContext* m_pFormatContext;
   int m_videoStream;
   AVCodecContext* m_pVideoContext;
   struct SwsContext* m_pVideoConversionContext;
   SDL_PixelFormat m_videoFormat;
   int m_videoWidth;
   int m_videoHeight;
   int m_audioStream;
   AVCodecContext* m_pAudioContext;
   struct SwrContext* m_pAudioConversionContext;
   AVSampleFormat m_audioFormat;
   PinSound* m_pPinSound;
   std::queue<AVFrame*> m_queue;
   std::mutex m_mutex;
   std::thread m_thread;
   bool m_running;
   bool m_paused;
};
