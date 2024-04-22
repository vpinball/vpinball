/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 */

#include "core/stdafx.h"

#include "PUPWindow.h"

PUPWindow::PUPWindow(const string& szTitle, int x, int y, int w, int h, int z, int rotation)
    : VP::Window(szTitle, x, y, w, h, z, rotation)
{
   m_visible = true;

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   m_pFormatContext = NULL;
   m_pAudioContext = NULL;
   m_pVideoContext = NULL;
   m_pPacket = NULL;
   m_pFrame = NULL;
   m_audioStream = -1;
   m_videoStream = -1;
   m_pVideoConversionContext = NULL;
   m_videoFormat = (SDL_PixelFormatEnum)SDL_PIXELFORMAT_UNKNOWN;
   m_videoWidth = 0;
   m_videoHeight = 0;
   m_videoStart = 0;
   m_pAudioConversionContext = NULL;
   m_audioFormat = (AVSampleFormat)AV_SAMPLE_FMT_NONE;
   m_done = false;
   m_flushing = false;
   m_decoded = false;
   m_firstPTS = -1.0;
#endif

   m_pAudioPlayer = new AudioPlayer();
   m_pAudioPlayer->StreamInit(44100, 2, 1.0f);
   m_pVideoTexture = NULL;
   m_pOverlayTexture = NULL;
   m_pOverlay = NULL;
   m_terminate = false;

   m_pBackgroundVideo = nullptr;
   m_pCurrentVideo = nullptr;
}

PUPWindow::~PUPWindow()
{
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_terminate = true;
   }

   m_cv.notify_one();

   if (m_pVideoTexture)
      SDL_DestroyTexture(m_pVideoTexture);

   if (m_pOverlayTexture)
      SDL_DestroyTexture(m_pOverlayTexture);

   delete m_pAudioPlayer;
}

bool PUPWindow::Init()
{
   if (!VP::Window::Init())
      return false;

   std::thread([this]() {
      PlayVideo();
   }).detach();

   return true;
}

void PUPWindow::Play(const string& szFilename, int volume, PUP_TRIGGER_PLAY_ACTION action, int priority)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   m_pCurrentVideo->isPaused = true;
   m_playlist.push_back(new PUPVideo(szFilename, volume, action, priority));
   m_cv.notify_one();  
}
  
void PUPWindow::SetBG(const string& szFilename, int volume, int priority)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_pBackgroundVideo = new PUPVideo(szFilename, volume, PUP_TRIGGER_PLAY_ACTION_LOOP, priority);
   m_cv.notify_one();
}

void PUPWindow::SetOverlay(const string& szFilename)
{
   m_pOverlay = IMG_Load(szFilename.c_str());
}

void PUPWindow::PlayVideo()
{
   std::unique_lock<std::mutex> lock(m_mutex);  
   while (!m_terminate) {
      if (!m_playlist.empty()) {
         m_pCurrentVideo = m_playlist.front();
         m_playlist.pop_front();
      }
      else if (m_pBackgroundVideo) {
         m_pCurrentVideo = m_pBackgroundVideo;
         m_pCurrentVideo->isPaused = false;
      }
      else {
         m_cv.wait(lock, [this] { return !m_playlist.empty() || m_terminate; });
         continue;
      }

      lock.unlock();
      if (!m_pCurrentVideo->isPaused)
         ProcessVideo();
      lock.lock();

      m_pCurrentVideo = nullptr;
   }
}

void PUPWindow::ProcessVideo()
{
   PLOGI.printf("Playing %s", m_pCurrentVideo->filename.c_str());

   m_pAudioPlayer->StreamVolume(m_pCurrentVideo->volume / 100.0f);

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   if (avformat_open_input(&m_pFormatContext, m_pCurrentVideo->filename.c_str(), NULL, NULL) != 0) {
      PLOGE.printf("Unable to open: filename=%s", m_pCurrentVideo->filename.c_str());
      Cleanup();
      return;
   }

   m_videoStream = av_find_best_stream(m_pFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
   if (m_videoStream >= 0) {
      m_pVideoContext = OpenVideoStream(m_pFormatContext, m_videoStream);
      if (!m_pVideoContext) {
         PLOGE.printf("Unable to open video stream: filename=%s", m_pCurrentVideo->filename.c_str());
         Cleanup();
         return;
      }
   }

   m_audioStream = av_find_best_stream(m_pFormatContext, AVMEDIA_TYPE_AUDIO, -1, m_videoStream, NULL, 0);
   if (m_audioStream >= 0) {
      m_pAudioContext = OpenAudioStream(m_pFormatContext, m_audioStream);
      if (!m_pAudioContext) {
         PLOGE.printf("Unable to open audio stream: filename=%s", m_pCurrentVideo->filename.c_str());
         Cleanup();
         return;
      }
   }

   m_pPacket = av_packet_alloc();
   if (!m_pPacket) {
      PLOGE.printf("Unable to allocate packet: filename=%s", m_pCurrentVideo->filename.c_str());
      Cleanup();
      return;
   }

   m_pFrame = av_frame_alloc();
   if (!m_pFrame) {
      PLOGE.printf("Unable to allocate frame: filename=%s", m_pCurrentVideo->filename.c_str());
      Cleanup();
      return;
   }

   while (!m_done && !m_pCurrentVideo->isPaused) {
      ProcessFrame();
   }

   Cleanup();
#endif
}

void PUPWindow::RenderWithOverlay()
{
   SDL_SetRenderDrawColor(m_pRenderer, 0xA0, 0xA0, 0xA0, 0xFF);
   SDL_RenderClear(m_pRenderer);

   if (m_pVideoTexture)
      SDL_RenderCopyEx(m_pRenderer, m_pVideoTexture, NULL, NULL, 0.0, NULL, SDL_FLIP_NONE);

   if (m_pOverlay) {
      if (!m_pOverlayTexture)
         m_pOverlayTexture = SDL_CreateTextureFromSurface(m_pRenderer, m_pOverlay);

      if (m_pOverlayTexture)
         SDL_RenderCopy(m_pRenderer, m_pOverlayTexture, NULL, NULL);
   }

   SDL_RenderPresent(m_pRenderer);
}

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
AVCodecContext* PUPWindow::OpenStream(AVFormatContext* pInputFormatContext, int stream)
{
   AVCodecContext* pContext;
   const AVCodec* pCodec;

   pContext = avcodec_alloc_context3(NULL);
   if (!pContext)
      return NULL;

   if (avcodec_parameters_to_context(pContext, pInputFormatContext->streams[stream]->codecpar) < 0) {
      avcodec_free_context(&pContext);
      return NULL;
   }

   pContext->pkt_timebase = pInputFormatContext->streams[stream]->time_base;

   pCodec = avcodec_find_decoder(pContext->codec_id);
   if (!pCodec) {
      PLOGE.printf("Couldn't find codec %s", avcodec_get_name(pContext->codec_id));
      avcodec_free_context(&pContext);
      return NULL;
   }

   pContext->codec_id = pCodec->id;
   if (avcodec_open2(pContext, pCodec, NULL) != 0) {
      PLOGE.printf("Couldn't open codec %s", avcodec_get_name(pContext->codec_id));
      avcodec_free_context(&pContext);
      return NULL;
   }

   return pContext;
}

AVCodecContext* PUPWindow::OpenVideoStream(AVFormatContext* pInputFormatContext, int stream)
{
   AVStream* pStream = pInputFormatContext->streams[stream];
   AVCodecParameters* pCodecParameters = pStream->codecpar;
   AVCodecContext* pContext = OpenStream(pInputFormatContext, stream);

   if (pContext) {
      PLOGD.printf("Video stream: %s %dx%d", avcodec_get_name(pContext->codec_id), pCodecParameters->width, pCodecParameters->height);
   }

   return pContext;
}

AVCodecContext* PUPWindow::OpenAudioStream(AVFormatContext* pInputFormatContext, int stream)
{
   AVStream* pStream = pInputFormatContext->streams[stream];
   AVCodecParameters* pCodecParameters = pStream->codecpar;
   AVCodecContext* pContext = OpenStream(pInputFormatContext, stream);

   if (pContext) {
      int channels = av_get_channel_layout_nb_channels(pCodecParameters->channel_layout);
      PLOGD.printf("Audio stream: %s %d channels, %d Hz", avcodec_get_name(pContext->codec_id), channels, pCodecParameters->sample_rate);
   }

   return pContext;
}

void PUPWindow::ProcessFrame()
{
   if (m_pCurrentVideo->isPaused) {
      Cleanup();
      return;
   }

   if (!m_flushing) {
      if (av_read_frame(m_pFormatContext, m_pPacket) < 0) {
         PLOGD.printf("End of stream, finishing decode");
         if (m_pAudioContext)
            avcodec_flush_buffers(m_pAudioContext);

         if (m_pVideoContext)
            avcodec_flush_buffers(m_pVideoContext);

         m_flushing = true;
      }
      else {
         if (m_pPacket->stream_index == m_audioStream) {
            if (avcodec_send_packet(m_pAudioContext, m_pPacket) != 0) {
               PLOGE.printf("Unable to send audio packet");
            }
         }
         else if (m_pPacket->stream_index == m_videoStream) {
            if (avcodec_send_packet(m_pVideoContext, m_pPacket) != 0) {
               PLOGE.printf("Unable to send video packet");
            }
         }
         av_packet_unref(m_pPacket);
      }
   }

   m_decoded = false;

   if (m_pAudioContext) {
      while (avcodec_receive_frame(m_pAudioContext, m_pFrame) >= 0) {
         HandleAudioFrame(m_pFrame);
         m_decoded = true;
      }
      if (m_flushing) {
        SDL_Delay(100);
      }
   }
   if (m_pVideoContext) {
      while (avcodec_receive_frame(m_pVideoContext, m_pFrame) >= 0) {
         double pts = ((double)m_pFrame->pts * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
         if (m_firstPTS < 0.0) {
            m_firstPTS = pts;
         }
         pts -= m_firstPTS;
         HandleVideoFrame(m_pFrame, pts);
         m_decoded = true;

         RenderWithOverlay();
      }
   }

   if (m_flushing && !m_decoded) {
      SDL_Delay(100);
      m_done = true;
   }
}

void PUPWindow::HandleVideoFrame(AVFrame* pFrame, double pts)
{
   SDL_RendererFlip flip = SDL_FLIP_NONE;

   SDL_PixelFormatEnum format = GetVideoFormat((enum AVPixelFormat)pFrame->format);
   if (!m_pVideoTexture || format != m_videoFormat || pFrame->width != m_videoWidth || pFrame->height != m_videoHeight) {
      if (m_pVideoTexture)
         SDL_DestroyTexture(m_pVideoTexture);

      if (format == SDL_PIXELFORMAT_UNKNOWN)
         m_pVideoTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, pFrame->width, pFrame->height);
      else
         m_pVideoTexture = SDL_CreateTexture(m_pRenderer, format, SDL_TEXTUREACCESS_STREAMING, pFrame->width, pFrame->height);

      m_videoFormat = format;
      m_videoWidth = pFrame->width;
      m_videoHeight = pFrame->height;
   }

   switch (format) {
      case SDL_PIXELFORMAT_UNKNOWN:
         m_pVideoConversionContext = sws_getCachedContext(m_pVideoConversionContext,
            pFrame->width, pFrame->height, (enum AVPixelFormat)pFrame->format, pFrame->width, pFrame->height,
            AV_PIX_FMT_BGRA, SWS_POINT, NULL, NULL, NULL);
         if (m_pVideoConversionContext != NULL) {
            uint8_t* pPixels[4];
            int pitch[4];
            if (SDL_LockTexture(m_pVideoTexture, NULL, (void **)pPixels, pitch) == 0) {
               sws_scale(m_pVideoConversionContext, (const uint8_t * const *)pFrame->data, pFrame->linesize, 0, pFrame->height, pPixels, pitch);
               SDL_UnlockTexture(m_pVideoTexture);
            }
         }
         break;
      case SDL_PIXELFORMAT_IYUV:
         if (pFrame->linesize[0] > 0 && pFrame->linesize[1] > 0 && pFrame->linesize[2] > 0) {
            SDL_UpdateYUVTexture(m_pVideoTexture, NULL, pFrame->data[0], pFrame->linesize[0],
               pFrame->data[1], pFrame->linesize[1], pFrame->data[2], pFrame->linesize[2]);
         }
         else if (pFrame->linesize[0] < 0 && pFrame->linesize[1] < 0 && pFrame->linesize[2] < 0) {
            SDL_UpdateYUVTexture(m_pVideoTexture, NULL, pFrame->data[0] + pFrame->linesize[0] * (pFrame->height - 1), -pFrame->linesize[0],
               pFrame->data[1] + pFrame->linesize[1] * (AV_CEIL_RSHIFT(pFrame->height, 1) - 1), -pFrame->linesize[1],
               pFrame->data[2] + pFrame->linesize[2] * (AV_CEIL_RSHIFT(pFrame->height, 1) - 1), -pFrame->linesize[2]);
            flip = SDL_FLIP_VERTICAL;
         }
         SetYUVConversionMode(pFrame);
         break;
      default:
         if (pFrame->linesize[0] < 0) {
            SDL_UpdateTexture(m_pVideoTexture, NULL, pFrame->data[0] + pFrame->linesize[0] * (pFrame->height - 1), -pFrame->linesize[0]);
            flip = SDL_FLIP_VERTICAL;
         }
         else
            SDL_UpdateTexture(m_pVideoTexture, NULL, pFrame->data[0], pFrame->linesize[0]);
         break;
   }

   if (!m_videoStart)
      m_videoStart = SDL_GetTicks();

   double now = (double)(SDL_GetTicks() - m_videoStart) / 1000.0;
   while (now < pts - 0.001) {
      SDL_Delay(1);
      now = (double)(SDL_GetTicks() - m_videoStart) / 1000.0;
   }
}

void PUPWindow::HandleAudioFrame(AVFrame* pFrame)
{
   static int destNbChannels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
   static enum AVSampleFormat destFmt = AV_SAMPLE_FMT_S16;
   static int destFreq = 44100;

   AVSampleFormat format = (AVSampleFormat)pFrame->format;
   if (!m_pAudioConversionContext || m_audioFormat != format) {
      swr_free(&m_pAudioConversionContext);
      m_pAudioConversionContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO,
         destFmt, destFreq, av_get_default_channel_layout(pFrame->channels),
         (AVSampleFormat)pFrame->format, pFrame->sample_rate, 0, NULL);

      if (!m_pAudioConversionContext || swr_init(m_pAudioConversionContext) < 0) {
         PLOGE.printf("Failed to initialize the resampling context");
         swr_free(&m_pAudioConversionContext);
         m_pAudioConversionContext = NULL;
         return;
      }

      m_audioFormat = format;
   }

   int wantedNbSamples = pFrame->nb_samples;
   uint8_t* pBuffer = NULL;
   unsigned int bufSize = 0;
   uint8_t** ppOut = &pBuffer;
   const uint8_t** ppIn = (const uint8_t**)pFrame->extended_data;
   int outCount = (int64_t)wantedNbSamples * destFreq / pFrame->sample_rate + 256;
   int outSize = av_samples_get_buffer_size(NULL, destNbChannels, outCount, destFmt, 0);

   if (outSize < 0) {
      PLOGE.printf("av_samples_get_buffer_size() failed");
      return;
   }
   if (wantedNbSamples != pFrame->nb_samples) {
      if (swr_set_compensation(m_pAudioConversionContext, (wantedNbSamples - pFrame->nb_samples) * destFreq / pFrame->sample_rate,
         wantedNbSamples * destFreq / pFrame->sample_rate) < 0) {
         PLOGE.printf("swr_set_compensation() failed");
         return;
      }
   }
   av_fast_malloc(&pBuffer, &bufSize, outSize);
   if (!pBuffer)
      return;
   int len2 = swr_convert(m_pAudioConversionContext, ppOut, outCount, ppIn, pFrame->nb_samples);
   if (len2 < 0) {
      PLOGE.printf("swr_convert() failed");
      av_free(pBuffer);
      return;
   }
   if (len2 == outCount) {
      PLOGE.printf("audio buffer is probably too small");
      if (swr_init(m_pAudioConversionContext) < 0) {
         swr_free(&m_pAudioConversionContext);
         m_pAudioConversionContext = NULL;
         av_free(pBuffer);
         return;
      }
   }
   int resampledDataSize = len2 * destNbChannels * av_get_bytes_per_sample(destFmt);

   m_pAudioPlayer->StreamUpdate(pBuffer, resampledDataSize);

   av_free(pBuffer);
}

SDL_PixelFormatEnum PUPWindow::GetVideoFormat(enum AVPixelFormat format)
{
   switch (format) {
      case AV_PIX_FMT_RGB8: return SDL_PIXELFORMAT_RGB332;
      case AV_PIX_FMT_RGB444: return SDL_PIXELFORMAT_RGB444;
      case AV_PIX_FMT_RGB555: return SDL_PIXELFORMAT_RGB555;
      case AV_PIX_FMT_BGR555: return SDL_PIXELFORMAT_BGR555;
      case AV_PIX_FMT_RGB565: return SDL_PIXELFORMAT_RGB565;
      case AV_PIX_FMT_BGR565: return SDL_PIXELFORMAT_BGR565;
      case AV_PIX_FMT_RGB24: return SDL_PIXELFORMAT_RGB24;
      case AV_PIX_FMT_BGR24: return SDL_PIXELFORMAT_BGR24;
      case AV_PIX_FMT_0RGB32: return SDL_PIXELFORMAT_XRGB8888;
      case AV_PIX_FMT_0BGR32: return SDL_PIXELFORMAT_XBGR8888;
      case AV_PIX_FMT_NE(RGB0, 0BGR): return SDL_PIXELFORMAT_RGBX8888;
      case AV_PIX_FMT_NE(BGR0, 0RGB): return SDL_PIXELFORMAT_BGRX8888;
      case AV_PIX_FMT_RGB32: return SDL_PIXELFORMAT_ARGB8888;
      case AV_PIX_FMT_RGB32_1: return SDL_PIXELFORMAT_RGBA8888;
      case AV_PIX_FMT_BGR32: return SDL_PIXELFORMAT_ABGR8888;
      case AV_PIX_FMT_BGR32_1: return SDL_PIXELFORMAT_BGRA8888;
      case AV_PIX_FMT_YUV420P: return SDL_PIXELFORMAT_IYUV;
      case AV_PIX_FMT_YUYV422: return SDL_PIXELFORMAT_YUY2;
      case AV_PIX_FMT_UYVY422: return SDL_PIXELFORMAT_UYVY;
      default: return SDL_PIXELFORMAT_UNKNOWN;
   }
}

double PUPWindow::GetCurrentPlaybackPosition()
{
   //if (!m_pFrame) return 0.0;
   //return av_q2d(m_pVideoContext->time_base) * m_pVideoContext->pts.val;
   return 0.0;
}

void PUPWindow::SetYUVConversionMode(AVFrame *frame)
{
   SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
   if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
      if (frame->color_range == AVCOL_RANGE_JPEG)
         mode = SDL_YUV_CONVERSION_JPEG;
      else if (frame->colorspace == AVCOL_SPC_BT709)
         mode = SDL_YUV_CONVERSION_BT709;
      else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M)
         mode = SDL_YUV_CONVERSION_BT601;
   }
   SDL_SetYUVConversionMode(mode);
}

void PUPWindow::Cleanup()
{
   if (m_pFrame) {
      av_frame_free(&m_pFrame);
      m_pFrame = NULL;
   }

   if (m_pPacket) {
      av_packet_free(&m_pPacket);
      m_pPacket = NULL;
   }

   if (m_pVideoContext) {
      avcodec_free_context(&m_pVideoContext);
      m_pVideoContext = NULL;
   }

   if (m_pAudioContext) {
      avcodec_free_context(&m_pAudioContext);
      m_pAudioContext = NULL;
   }

   if (m_pFormatContext) {
      avformat_close_input(&m_pFormatContext);
      m_pFormatContext = NULL;
   }

   m_audioStream = -1;
   m_videoStream = -1;

   if (m_pVideoConversionContext) {
      sws_freeContext(m_pVideoConversionContext);
      m_pVideoConversionContext = NULL;
   }

   m_videoFormat = (SDL_PixelFormatEnum)SDL_PIXELFORMAT_UNKNOWN;
   m_videoWidth = 0;
   m_videoHeight = 0;
   m_videoStart = 0;

   if (m_pAudioConversionContext) {
      swr_free(&m_pAudioConversionContext);
      m_pAudioConversionContext = NULL;
   }

   m_audioFormat = (AVSampleFormat)AV_SAMPLE_FMT_NONE;

   m_done = false;
   m_flushing = false;
   m_decoded = false;
   m_firstPTS = -1.0;
}
#endif