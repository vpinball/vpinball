#pragma once

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
#define VIDEO_WINDOW_HAS_FFMPEG_LIBS 1
#endif

#include "PUPManager.h"

#include "../common/Window.h"

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
extern "C" {
   #include "libavutil/imgutils.h"
   #include "libavformat/avformat.h"
   #include "libavdevice/avdevice.h"
   #include "libswscale/swscale.h"
   #include "libswresample/swresample.h"
   #include "libavcodec/avcodec.h"
}
#endif

class PUPMediaPlayer
{
public:
   PUPMediaPlayer();
   ~PUPMediaPlayer();

   void Play(const string& szFilename, int volume);
   bool IsPlaying();
   void Stop();
   void Render(SDL_Renderer* pRenderer, SDL_Rect* pRect);
   void SetLoop(bool loop) { m_loop = loop; }

private:
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   void ProcessMedia();
   void ProcessFrame();
   AVCodecContext* OpenVideoStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenAudioStream(AVFormatContext* pInputFormatContext, int stream);
   void HandleAudioFrame(AVFrame* pFrame);
   SDL_PixelFormatEnum GetVideoFormat(enum AVPixelFormat format);
   void SetYUVConversionMode(AVFrame* pFrame);
   void Cleanup();

   AudioPlayer* m_pAudioPlayer;
   AVFormatContext* m_pFormatContext;
   AVCodecContext* m_pAudioContext;
   AVCodecContext* m_pVideoContext;
   AVPacket* m_pPacket;
   AVFrame* m_pFrame;
   int m_audioStream;
   int m_videoStream;
   struct SwsContext* m_pVideoConversionContext;
   SDL_PixelFormatEnum m_videoFormat;
   int m_videoWidth;
   int m_videoHeight;
   Uint64 m_videoStart;
   struct SwrContext* m_pAudioConversionContext;
   AVSampleFormat m_audioFormat;
   bool m_done;
   bool m_flushing;
   bool m_decoded;
   double m_firstPTS;
   SDL_Texture* m_pTexture;
   std::queue<AVFrame*> m_frameQueue;
   std::mutex m_frameQueueMutex;
   std::thread* m_pThread;
   string m_szFilename;
   float m_volume;
#endif
   bool m_loop;
   bool m_running;
   bool m_paused;
};