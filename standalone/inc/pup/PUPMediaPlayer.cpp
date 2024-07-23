/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 */

#include "stdafx.h"

#include "PUPMediaPlayer.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

PUPMediaPlayer::PUPMediaPlayer()
{
   m_loop = false;
   m_volume = 100.0f;
   m_pRenderer = NULL;
   m_pTexture = NULL;
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   m_pFormatContext = NULL;
   m_videoStream = -1;
   m_pVideoContext = NULL;
   m_pVideoConversionContext = NULL;
   m_videoFormat = (SDL_PixelFormatEnum)SDL_PIXELFORMAT_UNKNOWN;
   m_videoWidth = 0;
   m_videoHeight = 0;
   m_audioStream = -1;
   m_pAudioContext = NULL;
   m_pAudioConversionContext = NULL;
   m_audioFormat = AV_SAMPLE_FMT_NONE;
#endif
   m_pAudioPlayer = new AudioPlayer();
   m_pAudioPlayer->StreamInit(44100, 2, 0.0f);
   m_running = false;
   m_paused = false;
}

PUPMediaPlayer::~PUPMediaPlayer()
{
   Stop();

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   if (m_pFormatContext)
      avformat_close_input(&m_pFormatContext);

   if (m_pVideoContext)
      avcodec_free_context(&m_pVideoContext);

   if (m_pVideoConversionContext)
      sws_freeContext(m_pVideoConversionContext);

   if (m_pAudioContext)
      avcodec_free_context(&m_pAudioContext);

   if (m_pAudioConversionContext)
      swr_free(&m_pAudioConversionContext);
#endif

   delete m_pAudioPlayer;
}

void PUPMediaPlayer::SetRenderer(SDL_Renderer* pRenderer)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_pRenderer = pRenderer;
}

void PUPMediaPlayer::Play(const string& szFilename)
{
   PLOGW.printf("filename=%s", szFilename.c_str());

   Stop();

   m_szFilename = szFilename;
   m_volume = 0.0f;
   m_loop = false;

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   if (m_pFormatContext)
      avformat_close_input(&m_pFormatContext);

   m_videoStream = -1;

   if (m_pVideoContext)
      avcodec_free_context(&m_pVideoContext);

   m_videoFormat = (SDL_PixelFormatEnum)SDL_PIXELFORMAT_UNKNOWN;
   m_videoWidth = 0;
   m_videoHeight = 0;

   m_audioStream = -1;

   if (m_pAudioContext)
      avcodec_free_context(&m_pAudioContext);

   m_audioFormat = AV_SAMPLE_FMT_NONE;

   // Open file

   if (avformat_open_input(&m_pFormatContext, szFilename.c_str(), NULL, NULL) != 0) {
      PLOGE.printf("Unable to open: filename=%s", szFilename.c_str());
      return;
   }

   // Find video stream

   for (int i = 0; i < m_pFormatContext->nb_streams; i++) {
      if (m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
         !(m_pFormatContext->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
         m_videoStream = i;
         break;
      }
   }

   // Open video stream

   if (m_videoStream >= 0) {
      AVStream* pStream = m_pFormatContext->streams[m_videoStream];
      AVCodecParameters* pCodecParameters = pStream->codecpar;
      m_pVideoContext = OpenStream(m_pFormatContext, m_videoStream);
      if (m_pVideoContext) {
         PLOGD.printf("Video stream: %s %dx%d", avcodec_get_name(m_pVideoContext->codec_id), pCodecParameters->width, pCodecParameters->height);
      }
      else {
         PLOGE.printf("Unable to open video stream: filename=%s", szFilename.c_str());
      }
   }
   else {
      m_videoStream = -1;
   }

   // Find audio stream

   if (m_videoStream >= 0) {
      m_audioStream = av_find_best_stream(m_pFormatContext, AVMEDIA_TYPE_AUDIO, -1, m_videoStream, NULL, 0);
      if (m_audioStream == AVERROR_DECODER_NOT_FOUND) {
         PLOGW.printf("No audio stream found: filename=%s", szFilename.c_str());
      }
   }
   else {
      for (int i = 0; i < m_pFormatContext->nb_streams; i++) {
         if (m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStream = i;
            break;
         }
      }
   }

   // Open audio stream

   if (m_audioStream >= 0) {
      AVStream* pStream = m_pFormatContext->streams[m_audioStream];
      AVCodecParameters* pCodecParameters = pStream->codecpar;
      m_pAudioContext = OpenStream(m_pFormatContext, m_audioStream);
      if (m_pAudioContext) {
         PLOGD.printf("Audio stream: %s %d channels, %d Hz\n", avcodec_get_name(m_pAudioContext->codec_id), pCodecParameters->ch_layout.nb_channels, pCodecParameters->sample_rate);
      }
      else {
         PLOGE.printf("Unable to open audio stream: filename=%s", szFilename.c_str());
      }
   }

   if (!m_pVideoContext && !m_pAudioContext) {
      PLOGE.printf("No video or audio stream found: filename=%s", szFilename.c_str());
      return;
   }

   PLOGW.printf("Playing: filename=%s", m_szFilename.c_str());
   m_pAudioPlayer->StreamVolume(0);

   m_running = true;
   m_thread = std::thread(&PUPMediaPlayer::Run, this);
#endif
}

bool PUPMediaPlayer::IsPlaying()
{
   std::lock_guard<std::mutex> lock(m_mutex);
   return m_running;
}

void PUPMediaPlayer::Pause(bool pause)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_paused = pause;
}

void PUPMediaPlayer::Stop()
{
   if (IsPlaying()) {
      PLOGW.printf("Stop: %s", m_szFilename.c_str());
   }

   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_running = false;
   }

   if (m_thread.joinable())
      m_thread.join();

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      while (!m_queue.empty()) {
         AVFrame* pFrame = m_queue.front();
         av_frame_free(&pFrame);
         m_queue.pop();
      }
   }
#endif
}

void PUPMediaPlayer::Run()
{
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   AVPacket* pPacket = av_packet_alloc();
   if (!pPacket) {
      PLOGE.printf("Unable to allocate packet");
      return;
   }

   AVFrame* pFrame = av_frame_alloc();
   if (!pFrame) {
      PLOGE.printf("Unable to allocate frame");
      av_packet_free(&pPacket);
      return;
   }

   bool flushing = false;

   Uint64 videoStart = 0;
   double videoFirstPTS = -1.0;
   int count = 0;
   SDL_Renderer* pRenderer = nullptr;

   while (true) {
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         if (!m_running)
            break;

         if (!pRenderer)
            pRenderer = m_pRenderer;

         m_pAudioPlayer->StreamVolume(m_volume / 100.0f);
      }

      {
         std::unique_lock<std::mutex> lock(m_mutex);
         if (m_paused) {
            lock.unlock();
            SDL_Delay(100);
            continue;
         }
      }

      if (!flushing) {
         if (av_read_frame(m_pFormatContext, pPacket) < 0) {
            if (count == 0)  {
               PLOGW.printf("End of stream, finishing decode: %s", m_szFilename.c_str());
            }

            if (m_pAudioContext)
               avcodec_flush_buffers(m_pAudioContext);

            if (m_pVideoContext)
               avcodec_flush_buffers(m_pVideoContext);

            flushing = true;
         }
         else {
            if (pPacket->stream_index == m_audioStream) {
               if (avcodec_send_packet(m_pAudioContext, pPacket) != 0) {
                  PLOGE.printf("Unable to send audio packet");
               }
            }
            else if (pPacket->stream_index == m_videoStream) {
               if (avcodec_send_packet(m_pVideoContext, pPacket) != 0) {
                  PLOGE.printf("Unable to send video packet");
               }
            }
            av_packet_unref(pPacket);
         }
      }

      bool decoded = false;

      if (m_pAudioContext) {
         while (avcodec_receive_frame(m_pAudioContext, pFrame) >= 0) {
            HandleAudioFrame(pFrame);
            decoded = true;
         }
         if (flushing)
            SDL_Delay(100);
      }

      if (m_pVideoContext) {
         while (avcodec_receive_frame(m_pVideoContext, pFrame) >= 0) {
            double pts = ((double)pFrame->pts * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
            if (videoFirstPTS < 0.0)
               videoFirstPTS = pts;
            pts -= videoFirstPTS;

            if (pRenderer) {
               AVFrame* pClonedFrame = av_frame_clone(pFrame);
               if (pClonedFrame) {
                  std::lock_guard<std::mutex> lock(m_mutex);
                  m_queue.push(pClonedFrame);
               }
            }

            if (!videoStart)
               videoStart = SDL_GetTicks();

            double now = (double)(SDL_GetTicks() - videoStart) / 1000.0;
            while (now < pts - 0.001) {
               SDL_Delay(8);
               now = (double)(SDL_GetTicks() - videoStart) / 1000.0;
            }
            decoded = true;
         }
      }

      if (flushing && !decoded) {
         SDL_Delay(100);

         {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_loop)
               break;
         }

         if (m_pVideoContext) {
            if (av_seek_frame(m_pFormatContext, m_videoStream, 0, 0) < 0) {
               PLOGE.printf("Unable to seek video stream");
            }
            videoFirstPTS = -1.0;
            videoStart = 0;
         }

         if (m_pAudioContext) {
            if (av_seek_frame(m_pFormatContext, m_audioStream, 0, 0) < 0) {
               PLOGE.printf("Unable to seek audio stream");
            }
         }

         count++;
         flushing = false;
      }
   }

   av_frame_free(&pFrame);
   av_packet_free(&pPacket);

   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_running = false;
   }
#endif
}

void PUPMediaPlayer::SetLoop(bool loop)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if (m_loop != loop) {
      PLOGW.printf("setting loop: loop=%d", loop);
      m_loop = loop;
   }
}

void PUPMediaPlayer::SetVolume(float volume)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if (m_volume != volume) {
       PLOGW.printf("setting volume: volume=%.1f%%", volume);
       m_volume = volume;
   }
}

void PUPMediaPlayer::Render(const SDL_Rect& destRect)
{
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   AVFrame* pFrame = NULL;

   {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (!m_queue.empty()) {
         pFrame = m_queue.front();
         m_queue.pop();
      }
   }

   if (pFrame) {
      SDL_PixelFormatEnum format = GetVideoFormat((enum AVPixelFormat)pFrame->format);
      if (!m_pTexture || format != m_videoFormat || pFrame->width != m_videoWidth || pFrame->height != m_videoHeight) {
         if (m_pTexture)
             SDL_DestroyTexture(m_pTexture);

         if (format == SDL_PIXELFORMAT_UNKNOWN)
            m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, pFrame->width,pFrame->height);
         else
            m_pTexture = SDL_CreateTexture(m_pRenderer, format, SDL_TEXTUREACCESS_STREAMING, pFrame->width, pFrame->height);

         m_videoFormat = format;
         m_videoWidth = pFrame->width;
         m_videoHeight = pFrame->height;
      }

      SDL_RendererFlip flip = SDL_FLIP_NONE;

      switch (format) {
         case SDL_PIXELFORMAT_UNKNOWN:
            m_pVideoConversionContext = sws_getCachedContext(m_pVideoConversionContext,
               pFrame->width, pFrame->height, (enum AVPixelFormat)pFrame->format, pFrame->width, pFrame->height,
               AV_PIX_FMT_BGRA, SWS_POINT, NULL, NULL, NULL);
            if (m_pVideoConversionContext != NULL) {
               uint8_t* pPixels[4];
               int pitch[4];
               if (SDL_LockTexture(m_pTexture, NULL, (void **)pPixels, pitch) == 0) {
                  sws_scale(m_pVideoConversionContext, (const uint8_t * const *)pFrame->data, pFrame->linesize, 0, pFrame->height, pPixels, pitch);
                  SDL_UnlockTexture(m_pTexture);
               }
            }
            break;
         case SDL_PIXELFORMAT_IYUV:
            if (pFrame->linesize[0] > 0 && pFrame->linesize[1] > 0 && pFrame->linesize[2] > 0) {
               SDL_UpdateYUVTexture(m_pTexture, NULL, pFrame->data[0], pFrame->linesize[0],
                  pFrame->data[1], pFrame->linesize[1], pFrame->data[2], pFrame->linesize[2]);
            }
            else if (pFrame->linesize[0] < 0 && pFrame->linesize[1] < 0 && pFrame->linesize[2] < 0) {
               SDL_UpdateYUVTexture(m_pTexture, NULL, pFrame->data[0] + pFrame->linesize[0] * (pFrame->height - 1), -pFrame->linesize[0],
                  pFrame->data[1] + pFrame->linesize[1] * (AV_CEIL_RSHIFT(pFrame->height, 1) - 1), -pFrame->linesize[1],
                  pFrame->data[2] + pFrame->linesize[2] * (AV_CEIL_RSHIFT(pFrame->height, 1) - 1), -pFrame->linesize[2]);
               flip = SDL_FLIP_VERTICAL;
            }
            SetYUVConversionMode(pFrame);
            break;
         default:
            if (pFrame->linesize[0] < 0) {
               SDL_UpdateTexture(m_pTexture, NULL, pFrame->data[0] + pFrame->linesize[0] * (pFrame->height - 1), -pFrame->linesize[0]);
               flip = SDL_FLIP_VERTICAL;
            }
            else
               SDL_UpdateTexture(m_pTexture, NULL, pFrame->data[0], pFrame->linesize[0]);
      }
      av_frame_free(&pFrame);
   }

   if (m_pTexture)
      SDL_RenderCopy(m_pRenderer, m_pTexture, NULL, &destRect);
#endif
}

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
AVCodecContext* PUPMediaPlayer::OpenStream(AVFormatContext* pInputFormatContext, int stream)
{
   AVCodecContext* pContext = avcodec_alloc_context3(NULL);
   if (!pContext)
      return NULL;

   if (avcodec_parameters_to_context(pContext, pInputFormatContext->streams[stream]->codecpar) < 0) {
      avcodec_free_context(&pContext);
      return NULL;
   }

   pContext->pkt_timebase = pInputFormatContext->streams[stream]->time_base;

   const AVCodec* pCodec = avcodec_find_decoder(pContext->codec_id);
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

void PUPMediaPlayer::HandleAudioFrame(AVFrame* pFrame)
{
   static AVChannelLayout destChLayout = AV_CHANNEL_LAYOUT_STEREO;
   static enum AVSampleFormat destFmt = AV_SAMPLE_FMT_S16;
   static int destFreq = 44100;

   AVSampleFormat format = (AVSampleFormat)pFrame->format;
   if (!m_pAudioConversionContext || m_audioFormat != format) {
      swr_free(&m_pAudioConversionContext);
      swr_alloc_set_opts2(&m_pAudioConversionContext, &destChLayout, destFmt, destFreq, &pFrame->ch_layout,
         (enum AVSampleFormat)pFrame->format, pFrame->sample_rate, 0, NULL);

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
   int outSize = av_samples_get_buffer_size(NULL, destChLayout.nb_channels, outCount, destFmt, 0);

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
   int resampledDataSize = len2 * destChLayout.nb_channels * av_get_bytes_per_sample(destFmt);
   m_pAudioPlayer->StreamUpdate(pBuffer, resampledDataSize);

   av_free(pBuffer);
}

SDL_PixelFormatEnum PUPMediaPlayer::GetVideoFormat(enum AVPixelFormat format)
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

void PUPMediaPlayer::SetYUVConversionMode(AVFrame *frame)
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
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
