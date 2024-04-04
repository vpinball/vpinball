#pragma once

#include "../common/Window.h"

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
#define VIDEO_WINDOW_HAS_LIBS 1
#endif

#ifdef VIDEO_WINDOW_HAS_LIBS
extern "C" {
   #include "libavformat/avformat.h"
   #include "libavdevice/avdevice.h"
   #include "libswscale/swscale.h"
   #include "libswresample/swresample.h"
   #include "libavcodec/avcodec.h"
}
#endif

#include <SDL2/SDL_image.h>

namespace VP {

class VideoWindow : public VP::Window
{
public:
   VideoWindow(const string& szTitle, int x, int y, int w, int h, int rotation, int z);
   ~VideoWindow();

   bool Init() override;
   void Play(const string& szFilename, int volume);
   void SetOverlay(const string& szFilename);

private:
   void RenderWithOverlay();

#ifdef VIDEO_WINDOW_HAS_LIBS
   void Cleanup();
   AVCodecContext* OpenVideoStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenAudioStream(AVFormatContext* pInputFormatContext, int stream);
   void ProcessFrame();
   void HandleVideoFrame(AVFrame* pFrame, double pts);
   void HandleAudioFrame(AVFrame* pFrame);
   SDL_PixelFormatEnum GetVideoFormat(enum AVPixelFormat format);
   void SetYUVConversionMode(AVFrame* pFrame);

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
#endif
   SDL_Texture* m_pVideoTexture;
   SDL_Texture* m_pOverlayTexture;

   AudioPlayer* m_pAudioPlayer;
   SDL_Surface* m_pOverlay;
   bool m_running;
   std::thread* m_pThread;
};

}