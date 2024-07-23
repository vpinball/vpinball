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

   void SetRenderer(SDL_Renderer* pRenderer);
   void Play(const string& szFilename);
   bool IsPlaying();
   void Pause(bool pause);
   const string& GetFilename() const { return m_szFilename; }
   void Stop();
   void SetVolume(float volume);
   void SetLoop(bool loop);
   void Render(const SDL_Rect& destRect);

private:
   void Run();
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   AVCodecContext* OpenStream(AVFormatContext* pInputFormatContext, int stream);
   SDL_PixelFormatEnum GetVideoFormat(enum AVPixelFormat format);
   void SetYUVConversionMode(AVFrame* pFrame);
   void HandleAudioFrame(AVFrame* pFrame);
#endif

   string m_szFilename;
   bool m_loop;
   float m_volume;
   SDL_Renderer* m_pRenderer;
   SDL_Texture* m_pTexture;
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   AVFormatContext* m_pFormatContext;
   int m_videoStream;
   AVCodecContext* m_pVideoContext;
   struct SwsContext* m_pVideoConversionContext;
   SDL_PixelFormatEnum m_videoFormat;
   int m_videoWidth;
   int m_videoHeight;
   int m_audioStream;
   AVCodecContext* m_pAudioContext;
   struct SwrContext* m_pAudioConversionContext;
   AVSampleFormat m_audioFormat;
#endif
   AudioPlayer* m_pAudioPlayer;
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   std::queue<AVFrame*> m_queue;
#endif
   std::mutex m_mutex;
   std::thread m_thread;
   bool m_running;
   bool m_paused;
};
