#pragma once

#include "PUPManager.h"

#include "../common/Window.h"

extern "C" {
   #include "libavutil/imgutils.h"
   #include "libavformat/avformat.h"
   #include "libavdevice/avdevice.h"
   #include "libswscale/swscale.h"
   #include "libswresample/swresample.h"
   #include "libavcodec/avcodec.h"
}

typedef struct {
   string szFilename;
   int volume;
   bool isPaused;
   PUP_TRIGGER_PLAY_ACTION action;
   int priority;
} PUPVideo;

class PUPMediaPlayer
{
public:
   PUPMediaPlayer(SDL_Renderer* pRenderer, SDL_Rect* pRect);
   ~PUPMediaPlayer();

   void Play(const string& szFilename, int volume, PUP_TRIGGER_PLAY_ACTION action, int priority);
   void SetBG(const string& szFilename, int volume, int priority);
   void Render();

private:
   void RunLoop();
   void ProcessMedia();
   void ProcessFrame();
   AVCodecContext* OpenVideoStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenStream(AVFormatContext* pInputFormatContext, int stream);
   AVCodecContext* OpenAudioStream(AVFormatContext* pInputFormatContext, int stream);
   void HandleAudioFrame(AVFrame* pFrame);
   SDL_PixelFormatEnum GetVideoFormat(enum AVPixelFormat format);
   void SetYUVConversionMode(AVFrame* pFrame);
   void Cleanup();
   
   SDL_Renderer* m_pRenderer;
   SDL_Rect* m_pRect;
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
   SDL_Texture* m_pVideoTexture;
   PUPVideo* m_pBackgroundVideo;
   PUPVideo* m_pCurrentVideo;
   std::deque<PUPVideo*> m_playlist;
   std::mutex m_playlistMutex;
   std::queue<AVFrame*> m_frameQueue;
   std::mutex m_frameQueueMutex;

   std::thread* m_pThread;
   bool m_running;
};