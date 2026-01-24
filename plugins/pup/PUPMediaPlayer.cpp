/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 */

#include "PUPMediaPlayer.h"

namespace PUP {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

PUPMediaPlayer::PUPMediaPlayer(const string& name)
   : m_name(name)
   , m_rgbFrames(3)
   , m_videoTextures(3)
   , m_scaledMask(nullptr, &SDL_DestroySurface)
   , m_libAv(LibAV::LibAV::GetInstance())
   , m_commandQueue(1)
{
   assert(m_libAv.isLoaded);
   assert(m_rgbFrames.size() == m_videoTextures.size());
   SetName(name);
}

PUPMediaPlayer::~PUPMediaPlayer()
{
   Stop();
}

bool PUPMediaPlayer::IsPlaying() const
{
   // Not worth the lock (which can face the async opening of a video file)
   //std::lock_guard<std::mutex> lock(m_mutex);
   return m_running;
}

void PUPMediaPlayer::SetName(const string& name)
{
   m_commandQueue.enqueue([this, name]()
   {
      string threadName(name);
      SetThreadName(threadName.append(".CmdQueue"));
      std::lock_guard lock(m_mutex);
      m_name = name;
   }); 
}

void PUPMediaPlayer::SetBounds(const SDL_Rect& rect)
{
   m_commandQueue.enqueue([this, rect]()
   {
      std::lock_guard lock(m_mutex);
      m_bounds = rect;
   });
}

void PUPMediaPlayer::SetMask(std::shared_ptr<SDL_Surface> mask)
{
   m_commandQueue.enqueue([this, mask]()
   {
      std::lock_guard lock(m_mutex);
      m_mask = mask;
      m_scaledMask.reset();
   });
}

void PUPMediaPlayer::SetGameTime(double gameTime) { m_gameTime = gameTime; }

double PUPMediaPlayer::GetPlayTime() const
{
   assert(m_gameTime >= 0.0 || !m_syncOnGameTime);
   return (m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0)) - m_startTimestamp;
}


void PUPMediaPlayer::Play(const std::filesystem::path& filename, float volume)
{
   m_commandQueue.enqueue(
      [this, filename, volume]()
   {
      LOGD("> Playing filename=%s", filename.string().c_str());

      //Should we do the callback when we are switching from a video to another ?
      //std::function<void(PUPMediaPlayer*)> onEndCallback = m_onEndCallback;
      //m_onEndCallback = [](PUPMediaPlayer*) { };
      StopBlocking();
      //m_onEndCallback = onEndCallback;

      std::lock_guard lock(m_mutex);

      m_filename = filename;
      m_volume = volume;
      m_loop = false;
      m_paused = false;
      m_syncOnGameTime = m_gameTime >= 0.0;
      m_startTimestamp = m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0);

      // Open file
      if (m_libAv._avformat_open_input(&m_pFormatContext, filename.string().c_str(), NULL, NULL) != 0)
      {
         LOGE("Unable to open: filename=%s", filename.c_str());
         return;
      }

      // Retrieve stream information (some formats do not have these available in the header, so this is needed to read a few frames and get the info)
      m_libAv._avformat_find_stream_info(m_pFormatContext, nullptr);

      // Find video stream
      for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
      {
         if (m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !(m_pFormatContext->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC))
         {
            m_videoStream = i;
            break;
         }
      }

      // Open video stream
      if (m_videoStream >= 0)
      {
         AVStream* pStream = m_pFormatContext->streams[m_videoStream];
         AVCodecParameters* pCodecParameters = pStream->codecpar;
         m_pVideoContext = OpenStream(m_pFormatContext, m_videoStream);
         if (m_pVideoContext)
         {
            LOGD("Video stream: %s %dx%d", m_libAv._avcodec_get_name(m_pVideoContext->codec_id), pCodecParameters->width, pCodecParameters->height);
         }
         else
         {
            LOGE("Unable to open video stream: filename=%s", filename.c_str());
         }
      }
      else
      {
         m_videoStream = -1;
      }

      // Find audio stream
      if (m_videoStream >= 0)
      {
         m_audioStream = m_libAv._av_find_best_stream(m_pFormatContext, AVMEDIA_TYPE_AUDIO, -1, m_videoStream, NULL, 0);
         if (m_audioStream == AVERROR_DECODER_NOT_FOUND)
         {
            LOGE("No audio stream found: filename=%s", filename.c_str());
         }
      }
      else
      {
         for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
         {
            if (m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
               m_audioStream = i;
               break;
            }
         }
      }

      // Open audio stream
      if (m_audioStream >= 0)
      {
         AVStream* pStream = m_pFormatContext->streams[m_audioStream];
         AVCodecParameters* pCodecParameters = pStream->codecpar;
         m_pAudioContext = OpenStream(m_pFormatContext, m_audioStream);
         if (m_pAudioContext)
         {
            LOGD("Audio stream: %s %d channels, %d Hz", m_libAv._avcodec_get_name(m_pAudioContext->codec_id), pCodecParameters->ch_layout.nb_channels, pCodecParameters->sample_rate);
         }
         else
         {
            LOGE("Unable to open audio stream: filename=%s", filename.c_str());
         }
      }

      if (!m_pVideoContext && !m_pAudioContext)
      {
         LOGE("No video or audio stream found: filename=%s", filename.c_str());
         StopBlocking();
         return;
      }

      m_running = true;
      m_thread = std::thread(&PUPMediaPlayer::Run, this);
   });
}

void PUPMediaPlayer::Pause(bool pause)
{
   m_commandQueue.enqueue([this, pause]()
   {
      if (m_paused != pause)
      {
         std::lock_guard lock(m_mutex);
         m_paused = pause;
         if (m_paused)
            m_pauseTimestamp = static_cast<double>(SDL_GetTicks()) / 1000.0 - m_startTimestamp; // Freeze at the current playing time
         else
            m_startTimestamp = static_cast<double>(SDL_GetTicks()) / 1000.0 - m_pauseTimestamp; // Adjust start time to restart from freeze point
      }
   });
}

void PUPMediaPlayer::Stop()
{
   m_commandQueue.enqueue([this]() { StopBlocking(); });
}

void PUPMediaPlayer::StopBlocking()
{
   if (IsPlaying())
   {
      LOGD("Stop: %s", m_filename.string().c_str());
   }

   // Stop decoder thread and flush queue
   std::unique_lock lock(m_mutex);
   m_running = false;

   lock.unlock();
   if (m_thread.joinable())
      m_thread.join();
   lock.lock();

   if (m_pFormatContext)
      m_libAv._avformat_close_input(&m_pFormatContext);
   m_pFormatContext = nullptr;

   if (m_pVideoContext)
      m_libAv._avcodec_free_context(&m_pVideoContext);
   m_pVideoContext = nullptr;
   m_videoStream = -1;

   for (size_t i = 0; i < m_videoTextures.size(); i++)
   {
      if (m_rgbFrames[i])
         m_libAv._av_frame_free(&m_rgbFrames[i]);
      if (m_videoTextures[i])
      {
         DeleteTexture(m_videoTextures[i]);
         m_videoTextures[i] = nullptr;
      }
   }
   m_activeRgbFrame = 0;

   if (m_swsContext)
      m_libAv._sws_freeContext(m_swsContext);
   m_swsContext = nullptr;

   m_videoTexture = nullptr;
   m_videoTextureId = 0xFFFFFF;

   if (m_pAudioContext)
      m_libAv._avcodec_free_context(&m_pAudioContext);
   m_pAudioContext = nullptr;
   m_audioFormat = AV_SAMPLE_FMT_NONE;
   m_audioStream = -1;

   if (m_pAudioConversionContext)
      m_libAv._swr_free(&m_pAudioConversionContext);
   m_pAudioConversionContext = nullptr;

   StopAudioStream(m_audioResId);
   m_audioResId.id = 0;
}

void PUPMediaPlayer::SetLoop(bool loop)
{
   m_commandQueue.enqueue([this, loop]()
   {
      std::lock_guard lock(m_mutex);
      if (m_loop != loop)
      {
         LOGD("setting loop: loop=%d", loop);
         m_loop = loop;
      }
   });
}

void PUPMediaPlayer::SetVolume(float volume)
{
   m_commandQueue.enqueue([this, volume]()
   {
      std::lock_guard lock(m_mutex);
      if (m_volume != volume)
      {
         LOGD("setting volume: volume=%.1f%%", volume);
         m_volume = volume;
      }
   });
}

void PUPMediaPlayer::SetLength(int length)
{
   m_commandQueue.enqueue([this, length]()
   {
      std::lock_guard lock(m_mutex);
      if (m_length != length)
      {
         LOGD("setting length: length=%d", length);
         m_length = length;
      }
   });
}

void PUPMediaPlayer::Render(VPXRenderContext2D* const ctx, const SDL_Rect& destRect)
{
   std::lock_guard lock(m_mutex);

   if (!m_running)
      return;

   const double playPts = m_paused ? m_pauseTimestamp : GetPlayTime();
   if ((m_length) != 0 && (playPts >= m_length))
      return;

   // Search for the best frame to display and update the video texture accordingly (if needed)
   unsigned int m_renderFrameId = m_videoTextureId;
   for (int i = 0; i < (int)m_rgbFrames.size(); i++)
   {
      if (m_activeRgbFrame >= i)
      {
         const AVFrame* rgbFrame = m_rgbFrames[(m_activeRgbFrame + (int)m_rgbFrames.size() - i) % (int)m_rgbFrames.size()];
         if (rgbFrame)
         {
            const double framePts = (static_cast<double>(rgbFrame->pts) * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
            if (playPts <= framePts) // We select the first frame after (or at) the current play timestamp
               m_renderFrameId = m_activeRgbFrame - i;
         }
      }
   }

   if (m_videoTextureId != m_renderFrameId)
   {
      const unsigned int index = m_renderFrameId % (unsigned int)m_videoTextures.size();
      m_videoTexture = m_videoTextures[index];
      if (m_videoTexture)
      {
         m_videoTextureId = m_renderFrameId;
         VPXTextureInfo* texInfo = GetTextureInfo(m_videoTexture);
         UpdateTexture(&m_videoTexture, texInfo->width, texInfo->height, texInfo->format, texInfo->data);
         // TODO to optimize a bit more we should update & upload a texture on a frame, then use it on the following render, this would remove the barrier between
         // the GPU upload/mipmap generation and the GPU render use, allowing more parallellism. Note that for the time being upload is only done on use
         //const double framePts = (static_cast<double>(rgbFrame->pts) * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
         //LOGD("Video tex update: play time: %8.3fs / frame pts: %8.3fs / delta: %8.3fs  [%s]", playPts, framePts, framePts - playPts, m_filename.c_str());
      }
   }

   // Render image
   if (m_videoTexture)
   {
      VPXTextureInfo* texInfo = GetTextureInfo(m_videoTexture);
      ctx->DrawImage(ctx, m_videoTexture, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), 0.f, 0.f, 0.f,
         static_cast<float>(destRect.x), static_cast<float>(destRect.y), static_cast<float>(destRect.w), static_cast<float>(destRect.h));
   }
}

void PUPMediaPlayer::Run()
{
   AVPacket* pPacket = m_libAv._av_packet_alloc();
   if (!pPacket) {
      LOGE("Unable to allocate packet");
      return;
   }

   AVFrame* pFrame = m_libAv._av_frame_alloc();
   if (!pFrame) {
      LOGE("Unable to allocate frame");
      m_libAv._av_packet_free(&pPacket);
      return;
   }

   // Main loop which loops over read/decode/convert and handle video seeking/looping
   #ifdef _DEBUG
      string name;
      bool paused;
   #endif
   while (true)
   {
      // Interact with main thread
      bool loop;
      {
         std::unique_lock<std::mutex> lock(m_mutex);
         #ifdef _DEBUG
            if ((name != m_name) || (paused != m_paused))
            {
               name = m_name;
               paused = m_paused;
               SetThreadName(m_name + (paused ? ".Paused" : ".Play"));
            }
         #endif
         if (!m_running)
            break;
         if (m_paused)
         {
            lock.unlock();
            SDL_Delay(100);
            continue;
         }
         if (m_length != 0 && GetPlayTime() >= m_length)
            break;
         loop = m_loop;
      }

      // Read next frame from source
      const int rfRet = m_libAv._av_read_frame(m_pFormatContext, pPacket);
      if (rfRet == AVERROR_EOF)
      {
         // End of stream, loop or stop
         if (!loop)
            break;
         if (m_pVideoContext)
         {
            if (m_libAv._av_seek_frame(m_pFormatContext, m_videoStream, 0, 0) < 0)
            {
               LOGE("Unable to seek video stream. Aborting loop");
               break;
            }
            m_libAv._avcodec_flush_buffers(m_pVideoContext);
         }
         if (m_pAudioContext)
         {
            if (m_libAv._av_seek_frame(m_pFormatContext, m_audioStream, 0, 0) < 0)
            {
               LOGE("Unable to seek audio stream. Aborting loop");
            }
            m_libAv._avcodec_flush_buffers(m_pAudioContext);
         }
         m_playIndex++;
         m_startTimestamp = m_syncOnGameTime ? m_gameTime : SDL_GetTicks();
         continue;
      }
      else if (rfRet < 0)
      {
         // Error reading file, stop playing
         LOGE("Error while reading video frame");
         break;
      }

      // Send to decoder
      if (pPacket->stream_index == m_audioStream)
      {
         if (m_libAv._avcodec_send_packet(m_pAudioContext, pPacket) != 0)
         {
            LOGE("Unable to send audio packet");
         }
      }
      else if (pPacket->stream_index == m_videoStream)
      {
         if (m_libAv._avcodec_send_packet(m_pVideoContext, pPacket) != 0)
         {
            LOGE("Unable to send video packet");
         }
      }
      m_libAv._av_packet_unref(pPacket);

      // Process decoded frames
      // TODO This should be done on ancillary threads to improve synchronization and better balance the load between CPU cores
      // Here, the synchronization entirely rely on the fact that HandleVideoFrame will block, waiting for a free slot in the circular frame buffer
      // This can cause audio overflow if there are no video (the stream is directly fully decoded) or audio glitches if the muxing is not great
      // (video waiting for frame while audio buffer is exhausted, this one being unlikely).
      if (m_pAudioContext)
      {
         while (m_libAv._avcodec_receive_frame(m_pAudioContext, pFrame) >= 0)
         {
            pFrame->opaque = reinterpret_cast<void*>(static_cast<uintptr_t>(m_playIndex));
            HandleAudioFrame(pFrame, m_pVideoContext == nullptr);
         }
      }
      if (m_pVideoContext)
      {
         while (m_libAv._avcodec_receive_frame(m_pVideoContext, pFrame) >= 0)
         {
            pFrame->opaque = reinterpret_cast<void*>(static_cast<uintptr_t>(m_playIndex));
            HandleVideoFrame(pFrame, true);
         }
      }
   }

   m_libAv._av_frame_free(&pFrame);
   m_libAv._av_packet_free(&pPacket);

   {
      std::lock_guard lock(m_mutex);
      m_running = false;
      StopAudioStream(m_audioResId);
      m_audioResId.id = 0;
      m_onEndCallback(this);
   }

   LOGD("Play done %s", m_filename.string().c_str());
}

void PUPMediaPlayer::HandleVideoFrame(AVFrame* frame, bool sync)
{
   // Unknown frame format, don't process frame (would crash in getContext)
   if (frame->format < 0)
      return;

   std::unique_lock lock(m_mutex);
   const int nextFrame = (m_activeRgbFrame + 1) % m_rgbFrames.size(); // m_activeRgbFrame points to the last frame (the one with the highest presentation timestamp)

   // Lazily create video frame conversion context and frame queue, adjusted to the render size
   const int targetWidth = m_bounds.w > 0 ? m_bounds.w : m_pVideoContext->width;
   const int targetHeight = m_bounds.h > 0 ? m_bounds.h : m_pVideoContext->height;
   const AVPixelFormat targetFormat = AV_PIX_FMT_RGBA;
   AVFrame* rgbFrame = m_rgbFrames[nextFrame];
   if ((rgbFrame != nullptr) && ((rgbFrame->width != targetWidth) || (rgbFrame->height != targetHeight)))
   {
      m_libAv._av_frame_free(&rgbFrame);
      m_rgbFrames[nextFrame] = nullptr;
      rgbFrame = nullptr;
      if (m_videoTextures[nextFrame] != nullptr)
      {
         if (m_videoTexture == m_videoTextures[nextFrame])
         {
            m_videoTexture = nullptr;
            m_videoTextureId = 0xFFFFFF;
         }
         DeleteTexture(m_videoTextures[nextFrame]);
         m_videoTextures[nextFrame] = nullptr;
      }
   }
   if (rgbFrame == nullptr)
   {
      rgbFrame = m_libAv._av_frame_alloc();
      if (rgbFrame == nullptr)
      {
         LOGE("Failed to create RGB buffer frame");
         m_running = false;
         return;
      }
      // Precreate the texture and uses there backing buffer to avoid copying on each update
      assert(m_videoTextures[nextFrame] == nullptr);
      UpdateTexture(&m_videoTextures[nextFrame], targetWidth, targetHeight, VPXTextureFormat::VPXTEXFMT_sRGBA8, nullptr);
      uint8_t* frameBuffer = static_cast<uint8_t*>(GetTextureInfo(m_videoTextures[nextFrame])->data);
      if (frameBuffer == nullptr)
      {
         LOGE("Failed to allocate RGB buffer");
         m_running = false;
         return;
      }
      rgbFrame->width = targetWidth;
      rgbFrame->height = targetHeight;
      rgbFrame->format = targetFormat;
      m_libAv._av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, frameBuffer, targetFormat, targetWidth, targetHeight, 1);
      m_rgbFrames[nextFrame] = rgbFrame;
   }

   // Create/Update conversion context when source format is known (so after decoding at least one frame)
   m_swsContext = m_libAv._sws_getCachedContext(m_swsContext, 
      frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
      rgbFrame->width, rgbFrame->height, static_cast<AVPixelFormat>(rgbFrame->format),
      SWS_BILINEAR, NULL, NULL, NULL
   );
   if (m_swsContext == nullptr)
      return;

   lock.unlock();
   {
      // Wait for the buffer slot to be outdated (do not overwrite a frame that is waiting to be displayed), but only in the same play sequence (stored in the opaque field of the frame. if we looped or seek, skip)
      if (sync && rgbFrame->opaque == frame->opaque)
      {
         const double oldPts = (static_cast<double>(rgbFrame->pts) * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
         while (m_running && GetPlayTime() < oldPts)
            SDL_Delay(8);
      }

      // Convert to a renderable format (we do not lock as the consumer thread is not supposed to be accessing an outdated frame, and this operation can be a bit lengthy)
      m_libAv._sws_scale(m_swsContext, frame->data, frame->linesize, 0, m_pVideoContext->height, rgbFrame->data, rgbFrame->linesize);
      rgbFrame->opaque = frame->opaque;

      // Apply the transparency mask
      if (m_mask)
      {
         SDL_Surface* sdlMask = m_mask.get();
         if (rgbFrame->width != m_mask->w || rgbFrame->height != m_mask->h)
         {
            if (m_scaledMask == nullptr || rgbFrame->width != m_scaledMask->w || rgbFrame->height != m_scaledMask->h)
               m_scaledMask = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
                  SDL_ScaleSurface(m_mask.get(), rgbFrame->width, rgbFrame->height, SDL_ScaleMode::SDL_SCALEMODE_LINEAR), SDL_DestroySurface);
            sdlMask = m_scaledMask.get();
         }
         if (sdlMask)
         {
            SDL_LockSurface(sdlMask);
            const uint32_t* __restrict mask = static_cast<uint32_t*>(sdlMask->pixels);
            uint32_t* __restrict frame = reinterpret_cast<uint32_t*>(rgbFrame->data[0]);
            for (int i = 0; i < sdlMask->h; i++)
            {
               for (int j = 0; j < sdlMask->w; j++, mask++, frame++)
                  *frame = *mask ? *frame : 0x00000000u;
               mask += sdlMask->pitch - sdlMask->w * sizeof(uint32_t);
               frame += rgbFrame->linesize[0] - sdlMask->w * sizeof(uint32_t);
            }
            SDL_UnlockSurface(sdlMask);
         }
      }
   }
   lock.lock();

   // Update frame PTS and pointer to latest frame under a lock as this modification impacts the consumer thread frame selection
   m_libAv._av_frame_copy_props(rgbFrame, frame);
   m_activeRgbFrame++;
}

AVCodecContext* PUPMediaPlayer::OpenStream(AVFormatContext* pInputFormatContext, int stream)
{
   AVCodecContext* pContext = m_libAv._avcodec_alloc_context3(NULL);
   if (!pContext)
      return nullptr;

   // Request to decode frames on different threads, limiting to the platform core minus 3 (magic number corresponding to the average core count used by VPX and common plugins)
   // TODO Disabled as this delay the frame queue by one frame, breaking single frame videos (and still images used as video)
   //pContext->thread_count = std::max(1u, std::min(16u, std::thread::hardware_concurrency() - 3));
   //pContext->thread_type = FF_THREAD_FRAME;

   if (m_libAv._avcodec_parameters_to_context(pContext, pInputFormatContext->streams[stream]->codecpar) < 0)
   {
      m_libAv._avcodec_free_context(&pContext);
      return nullptr;
   }

   pContext->pkt_timebase = pInputFormatContext->streams[stream]->time_base;

   const AVCodec* pCodec = m_libAv._avcodec_find_decoder(pContext->codec_id);
   if (!pCodec) {
      LOGE("Couldn't find codec %s", m_libAv._avcodec_get_name(pContext->codec_id));
      m_libAv._avcodec_free_context(&pContext);
      return nullptr;
   }

   pContext->codec_id = pCodec->id;
   if (m_libAv._avcodec_open2(pContext, pCodec, NULL) != 0)
   {
      LOGE("Couldn't open codec %s", m_libAv._avcodec_get_name(pContext->codec_id));
      m_libAv._avcodec_free_context(&pContext);
      return nullptr;
   }

   return pContext;
}

void PUPMediaPlayer::HandleAudioFrame(AVFrame* pFrame, bool sync)
{
   // If we have decoded enough data and we did not loop, then wait (if requested). Our aim is to enqueue up to our playing position + a reasonnable buffer
   if (sync && m_pAudioLoop == pFrame->opaque && AV_NOPTS_VALUE != pFrame->pts)
   {
      const double framePTS = static_cast<double>(pFrame->pts * m_pAudioContext->pkt_timebase.num) / static_cast<double>(m_pAudioContext->pkt_timebase.den);
      const double decodeTS = GetPlayTime() + 0.5; // Now + 500ms buffer
      if (framePTS > decodeTS)
         SDL_Delay(static_cast<uint32_t>(1000.0 * (framePTS - decodeTS)));
      if (!m_running)
         return;
   }
   m_pAudioLoop = pFrame->opaque;

   const AVSampleFormat frameFormat = static_cast<AVSampleFormat>(pFrame->format);
   const AVChannelLayout destChLayout = AV_CHANNEL_LAYOUT_STEREO;
   const enum AVSampleFormat destFmt = (frameFormat == AV_SAMPLE_FMT_FLT) ? AV_SAMPLE_FMT_FLT : AV_SAMPLE_FMT_S16;
   const int destFreq = pFrame->sample_rate;

   if (!m_pAudioConversionContext || (m_audioFormat != frameFormat) || (m_audioFreq != pFrame->sample_rate))
   {
      m_libAv._swr_free(&m_pAudioConversionContext);
      m_libAv._swr_alloc_set_opts2(&m_pAudioConversionContext, &destChLayout, destFmt, destFreq, &pFrame->ch_layout, frameFormat, pFrame->sample_rate, 0, NULL);
      if (!m_pAudioConversionContext || m_libAv._swr_init(m_pAudioConversionContext) < 0)
      {
         LOGE("Failed to initialize the resampling context");
         m_libAv._swr_free(&m_pAudioConversionContext);
         m_pAudioConversionContext = nullptr;
         return;
      }
      m_audioFormat = frameFormat;
      m_audioFreq = pFrame->sample_rate;
   }

   int outSize = m_libAv._av_samples_get_buffer_size(NULL, destChLayout.nb_channels, pFrame->nb_samples, destFmt, 0);
   if (outSize < 0)
   {
      LOGE("av_samples_get_buffer_size() failed");
      return;
   }

   uint8_t* pBuffer = NULL;
   unsigned int bufSize = 0;
   m_libAv._av_fast_malloc(&pBuffer, &bufSize, outSize);
   if (!pBuffer)
      return;

   int nConverted = m_libAv._swr_convert(m_pAudioConversionContext, &pBuffer, pFrame->nb_samples, pFrame->data, pFrame->nb_samples);
   if (nConverted < 0)
   {
      LOGE("swr_convert() failed");
      m_libAv._av_free(pBuffer);
      return;
   }
   assert(nConverted == pFrame->nb_samples); // Since we are not resampling we expect the same number of samples or sync will be lost
   if (nConverted > 0 && m_volume > 0.0f)
   {
      AudioUpdateMsg* audioUpdate = new AudioUpdateMsg();
      audioUpdate->id.id = m_audioResId.id;
      audioUpdate->type = CTLPI_AUDIO_SRC_BACKGLASS_STEREO;
      audioUpdate->format = (destFmt == AV_SAMPLE_FMT_FLT) ? CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT : CTLPI_AUDIO_FORMAT_SAMPLE_INT16;
      audioUpdate->sampleRate = destFreq;
      audioUpdate->bufferSize = nConverted * destChLayout.nb_channels * m_libAv._av_get_bytes_per_sample(destFmt);
      audioUpdate->buffer = pBuffer;
      audioUpdate->volume = m_volume / 100.0f;
      m_audioResId = UpdateAudioStream(audioUpdate);
   }
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

}
