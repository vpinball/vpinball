#pragma once

#include "PUPManager.h"

#include "../common/Window.h"

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
#define VIDEO_WINDOW_HAS_FFMPEG_LIBS 1
#endif

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
extern "C" {
   #include "libavformat/avformat.h"
   #include "libavdevice/avdevice.h"
   #include "libswscale/swscale.h"
   #include "libswresample/swresample.h"
   #include "libavcodec/avcodec.h"
}
#endif

#include <SDL2/SDL_image.h>

struct PUPVideo {
   string filename;
   int volume;
   bool isPaused;
   double playbackPosition;
   PUP_TRIGGER_PLAY_ACTION action;
   int priority;
   string position;

   PUPVideo(const string& fname, int vol, PUP_TRIGGER_PLAY_ACTION act, int pri, string pos = "")
        : filename(fname), volume(vol), isPaused(false), playbackPosition(0.0),
        action(act), priority(pri), position(pos) {}
};

class PUPWindow : public VP::Window
{
public:
   PUPWindow(const string& szTitle, int x, int y, int w, int h, int rotation, int z);
   ~PUPWindow();

   bool Init() override;

   void Play(const string& szFilename, int volume, PUP_TRIGGER_PLAY_ACTION action, int priority);
   void SetBG(const string& szFilename, int volume, int priority);
   void SetOverlay(const string& szFilename);

private:
   void PlayVideo();
   void ProcessVideo();
   void RenderWithOverlay();

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   AVCodecContext* OpenVideoStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenAudioStream(AVFormatContext* pInputFormatContext, int stream);
   void ProcessFrame();
   void HandleVideoFrame(AVFrame* pFrame, double pts);
   void HandleAudioFrame(AVFrame* pFrame);
   SDL_PixelFormatEnum GetVideoFormat(enum AVPixelFormat format);
   double GetCurrentPlaybackPosition();
   void SetYUVConversionMode(AVFrame* pFrame);
   void Cleanup();

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
   PUPVideo* m_pBackgroundVideo;
   PUPVideo* m_pCurrentVideo;
   std::deque<PUPVideo*> m_playlist;
   std::mutex m_mutex;
   std::condition_variable m_cv;
   bool m_terminate;
};