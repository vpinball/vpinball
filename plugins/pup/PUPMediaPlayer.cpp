/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 */

#include "PUPMediaPlayer.h"

#include <cfloat>

namespace PUP {

/*
 * swscale SIMD paths may read/write past the end of image planes, so destination buffers
 * must be over-allocated. 64 matches FFmpeg's internal STRIDE_ALIGN (AVX-512 store width)
 * and Chromium's kFFmpegBufferAddressAlignment on x86.
 *
 * https://ffmpeg.org/doxygen/trunk/structAVFrame.html ("some filters and swscale can read
 * up to 16 bytes beyond the planes")
 * https://trac.ffmpeg.org/ticket/9254 (sws_scale writes out of buffer, ssse3 YUV->RGB)
 * https://trac.ffmpeg.org/ticket/10852 (sws_scale overflows buffer for some resolutions)
 * https://source.chromium.org/chromium/chromium/src/+/main:media/base/limits.h
 */
#define PUP_SWS_DST_PADDING 64

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

PUPMediaPlayer::PUPMediaPlayer(const string& name)
   : m_name(name)
   , m_frames(3)
   , m_scaledMask(nullptr, &SDL_DestroySurface)
   , m_libAv(LibAV::LibAV::GetInstance())
   , m_commandQueue(1)
{
   assert(m_libAv.isLoaded);
   SetName(name);
}

PUPMediaPlayer::~PUPMediaPlayer()
{
   Stop();
}

bool PUPMediaPlayer::IsPlaying()
{
   return m_running.load(std::memory_order_relaxed) || (m_pendingPlay.load(std::memory_order_relaxed) > m_pendingStop.load(std::memory_order_relaxed));
}

void PUPMediaPlayer::SetName(const string& name)
{
   m_commandQueue.enqueue([this, name]()
   {
      SetThreadName(name + ".CmdQueue");
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
   m_pendingPlay++;
   m_commandQueue.enqueue(
      [this, filename, volume]()
   {
      LOGD("> Playing filename=" + filename.string());

      // Suppress end callback during play-to-play transition — the previous video
      // is being replaced, not ending naturally.
      auto savedCallback = m_onEndCallback;
      m_onEndCallback = [](){};
      StopBlocking();
      m_onEndCallback = savedCallback;

      AVFormatContext* pFormatContext = nullptr;
      if (m_libAv._avformat_open_input(&pFormatContext, filename.string().c_str(), nullptr, nullptr) != 0)
      {
         LOGE("Unable to open: filename=" + filename.string());
         m_pendingPlay--;
         return;
      }

      // Retrieve stream information (some formats do not have these available in the header, so this is needed to read a few frames and get the info)
      m_libAv._avformat_find_stream_info(pFormatContext, nullptr);

      int videoStream = -1;
      int audioStream = -1;
      AVCodecContext* pVideoContext = nullptr;
      AVCodecContext* pAudioContext = nullptr;

      for (unsigned int i = 0; i < pFormatContext->nb_streams; i++)
      {
         if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !(pFormatContext->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC))
         {
            videoStream = i;
            break;
         }
      }

      if (videoStream >= 0)
      {
         AVCodecParameters* pCodecParameters = pFormatContext->streams[videoStream]->codecpar;
         pVideoContext = OpenStream(pFormatContext, videoStream);
         if (pVideoContext)
         {
            LOGD(std::format("Video stream: {} {}x{} file={}", m_libAv._avcodec_get_name(pVideoContext->codec_id), pCodecParameters->width, pCodecParameters->height, filename.string()));
         }
         else
         {
            LOGE("Unable to open video stream: filename=" + filename.string());
         }
      }

      if (videoStream >= 0)
      {
         audioStream = m_libAv._av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1, videoStream, NULL, 0);
         if (audioStream < 0)
         {
            if (audioStream == AVERROR_DECODER_NOT_FOUND)
               LOGE("No audio stream found: filename=" + filename.string());
            audioStream = -1;
         }
      }
      else
      {
         for (unsigned int i = 0; i < pFormatContext->nb_streams; i++)
         {
            if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
               audioStream = i;
               break;
            }
         }
      }

      if (audioStream >= 0)
      {
         AVCodecParameters* pCodecParameters = pFormatContext->streams[audioStream]->codecpar;
         pAudioContext = OpenStream(pFormatContext, audioStream);
         if (pAudioContext)
         {
            LOGD(std::format("Audio stream: {} {} channels, {} Hz", m_libAv._avcodec_get_name(pAudioContext->codec_id), pCodecParameters->ch_layout.nb_channels, pCodecParameters->sample_rate));
         }
         else
         {
            LOGE("Unable to open audio stream: filename=" + filename.string());
         }
      }

      if (!pVideoContext && !pAudioContext)
      {
         LOGE("No video or audio stream found: filename=" + filename.string());
         m_libAv._avformat_close_input(&pFormatContext);
         m_pendingPlay--;
         return;
      }

      {
         std::lock_guard lock(m_mutex);
         m_filename = filename;
         m_volume = volume;
         m_loop = false;
         m_paused = false;
         m_syncOnGameTime = m_gameTime >= 0.0;
         m_startTimestamp = m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0);
         m_pFormatContext = pFormatContext;
         m_videoStream = videoStream;
         m_pVideoContext = pVideoContext;
         m_videoPtsBaseValid = false;
         m_videoPtsBase = 0.0;
         m_audioStream = audioStream;
         m_pAudioContext = pAudioContext;
         m_firstRenderPending = pVideoContext != nullptr;
         m_lastDecodedPts = 0.0;
         m_lastRenderTicks = 0;
         m_running = true;
         m_thread = std::thread(&PUPMediaPlayer::Run, this);
      }
      m_pendingPlay--;
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
            m_pauseTimestamp = GetPlayTime(); // Freeze at the current playing time
         else
            m_startTimestamp = m_startTimestamp + GetPlayTime() - m_pauseTimestamp; // Adjust start time to restart from freeze point
      }
   });
}

void PUPMediaPlayer::Stop()
{
   m_pendingStop++;
   m_commandQueue.enqueue(
      [this]()
      {
         StopBlocking();
         m_pendingStop--;
      });
}

void PUPMediaPlayer::StopBlocking()
{
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

   for (auto& frame : m_frames)
   {
      if (frame.frame)
         m_libAv._av_frame_free(&frame.frame);
      if (frame.buffer)
         m_libAv._av_free(frame.buffer);
      if (frame.texture)
         DeleteTexture(frame.texture);
      frame.valid = false;
      frame.frame = nullptr;
      frame.buffer = nullptr;
      frame.texture = nullptr;
      frame.uploaded = false;
      frame.pts = -1.0;
   }

   if (m_swsContext)
      m_libAv._sws_freeContext(m_swsContext);
   m_swsContext = nullptr;

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
         LOGD("Setting loop: loop=" + std::to_string(loop));
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
         LOGD(std::format("Setting volume: volume={:.1f}%%", volume));
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
         LOGD("Setting length: length=" + std::to_string(length));
         m_length = length;
      }
   });
}

void PUPMediaPlayer::Render(VPXRenderContext2D* const ctx, const SDL_Rect& destRect, float alpha)
{
   if (!m_running)
      return;

   std::lock_guard lock(m_mutex);

   const uint64_t nowTicks = SDL_GetTicks();
   const uint64_t renderGap = m_lastRenderTicks != 0 ? (nowTicks - m_lastRenderTicks) : 0;
   m_lastRenderTicks = nowTicks;

   // The playback clock only advances while the screen is actually being rendered: start the
   // timeline at the first render, and shift it across render stalls (table load, ...) so the
   // video holds and resumes instead of skipping ahead
   if (!m_paused)
   {
      if (m_firstRenderPending)
      {
         m_firstRenderPending = false;
         const double late = GetPlayTime();
         if (late > 0.0)
            m_startTimestamp += late;
      }
      else if (renderGap > 250)
      {
         m_startTimestamp += static_cast<double>(renderGap) / 1000.0;
      }
   }

   const double playPts = m_paused ? m_pauseTimestamp : GetPlayTime();

   // Search for the best frame to display (the one nearest to our play time), also ageing all slots to let the decoder select the eldest used slot
   int selectedFrameSlot = -1;
   double bestDelta = DBL_MAX;
   for (int i = 0; i < static_cast<int>(m_frames.size()); i++)
   {
      m_frames[i].age++;
      if (const double delta = abs(playPts - m_frames[i].pts); m_frames[i].valid && (delta < bestDelta))
      {
         bestDelta = delta;
         selectedFrameSlot = i;
      }
   }
   if (selectedFrameSlot == -1)
      return;
   FrameInfo& selectedFrame = m_frames[selectedFrameSlot];

   if (!selectedFrame.uploaded)
   {
      // Hand a fresh texture over for each new frame: the previous texture may still be referenced
      // by a pending GPU upload, so it is released (kept alive by the pending reference until
      // consumed) instead of being updated in place
      selectedFrame.uploaded = true;
      if (selectedFrame.texture != nullptr)
      {
         DeleteTexture(selectedFrame.texture);
         selectedFrame.texture = nullptr;
      }
      UpdateTexture(&selectedFrame.texture, selectedFrame.frame->width, selectedFrame.frame->height, VPXTextureFormat::VPXTEXFMT_sRGBA8, selectedFrame.frame->data[0]);
   }
   if (selectedFrame.texture == nullptr)
      return;

   const VPXTextureInfo* texInfo = GetTextureInfo(selectedFrame.texture);
   ctx->DrawImage(ctx, selectedFrame.texture, 1.f, 1.f, 1.f, alpha, 0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), 0.f, 0.f, 0.f,
      static_cast<float>(destRect.x), static_cast<float>(destRect.y), static_cast<float>(destRect.w), static_cast<float>(destRect.h));

   selectedFrame.age = 0;
}

void PUPMediaPlayer::Run()
{
   AVPacket* pPacket = m_libAv._av_packet_alloc();
   if (!pPacket) {
      LOGE("Unable to allocate packet"s);
      return;
   }

   AVFrame* pFrame = m_libAv._av_frame_alloc();
   if (!pFrame) {
      LOGE("Unable to allocate frame"s);
      m_libAv._av_packet_free(&pPacket);
      return;
   }

   // Main loop which loops over read/decode/convert and handle video seeking/looping
   #ifdef _DEBUG
      string name;
      bool paused = false;
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
         // Drain both decoders so the last buffered frames are not dropped at the loop point or at the natural end
         if (m_pVideoContext)
         {
            m_libAv._avcodec_send_packet(m_pVideoContext, nullptr);
            while (m_libAv._avcodec_receive_frame(m_pVideoContext, pFrame) >= 0)
            {
               pFrame->opaque = reinterpret_cast<void*>(static_cast<uintptr_t>(m_playIndex));
               HandleVideoFrame(pFrame);
            }
         }
         if (m_pAudioContext)
         {
            m_libAv._avcodec_send_packet(m_pAudioContext, nullptr);
            while (m_libAv._avcodec_receive_frame(m_pAudioContext, pFrame) >= 0)
            {
               pFrame->opaque = reinterpret_cast<void*>(static_cast<uintptr_t>(m_playIndex));
               HandleAudioFrame(pFrame, m_pVideoContext == nullptr);
            }
         }
         if (!loop)
         {
            while (m_running && !m_paused && m_pVideoContext && GetPlayTime() < m_lastDecodedPts + 0.05)
               SDL_Delay(8);
            break;
         }
         if (m_pVideoContext)
         {
            if (m_libAv._av_seek_frame(m_pFormatContext, m_videoStream, 0, 0) < 0)
            {
               LOGE("Unable to seek video stream. Aborting loop: " + m_filename.filename().string());
               break;
            }
            m_libAv._avcodec_flush_buffers(m_pVideoContext);
         }
         if (m_pAudioContext)
         {
            if (m_libAv._av_seek_frame(m_pFormatContext, m_audioStream, 0, 0) < 0)
            {
               LOGE("Unable to seek audio stream: " + m_filename.filename().string());
            }
            m_libAv._avcodec_flush_buffers(m_pAudioContext);
         }
         m_playIndex++;
         double duration = 0.0;
         if (m_videoStream >= 0)
         {
            const AVStream* vs = m_pFormatContext->streams[m_videoStream];
            if (vs->duration > 0)
               duration = static_cast<double>(vs->duration) * vs->time_base.num / vs->time_base.den;
         }
         {
            std::lock_guard lock(m_mutex);
            if (duration > 0.0)
            {
               // Keep the timeline continuous across the loop: the tail frames stay valid at
               // negative pts and play out while the next iteration's frames (pts 0..) decode
               m_startTimestamp += duration;
               m_lastDecodedPts -= duration;
               for (auto& frame : m_frames)
                  frame.pts -= duration;
            }
            else
            {
               m_startTimestamp = m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0);
               for (auto& frame : m_frames)
                  frame.valid = false;
            }
         }
         continue;
      }
      else if (rfRet < 0)
      {
         LOGE(std::format("Error reading frame: name={}, file={}, ret={}", m_name, m_filename.filename().string(), rfRet));
         break;
      }

      // Send to decoder
      if (pPacket->stream_index == m_audioStream)
      {
         if (m_libAv._avcodec_send_packet(m_pAudioContext, pPacket) != 0)
         {
            LOGE("Unable to send audio packet"s);
         }
      }
      else if (pPacket->stream_index == m_videoStream)
      {
         if (m_libAv._avcodec_send_packet(m_pVideoContext, pPacket) != 0)
         {
            LOGE("Unable to send video packet"s);
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
            HandleVideoFrame(pFrame);
         }
      }
   }

   m_libAv._av_frame_free(&pFrame);
   m_libAv._av_packet_free(&pPacket);

   {
      std::lock_guard lock(m_mutex);
      LOGD(std::format("Player stopped: name={}, file={}", m_name, m_filename.filename().string()));
      m_running = false;
      StopAudioStream(m_audioResId);
      m_audioResId.id = 0;
      m_onEndCallback();
   }
}

void PUPMediaPlayer::HandleVideoFrame(AVFrame* frame)
{
   // Unknown frame format, don't process frame (would crash in getContext)
   if (frame->format < 0)
      return;

   // Select the best buffer slot, eventually waiting for a free slot
   int selectedFrameSlot = -1;
   while (m_running)
   {
      const double playPTS = GetPlayTime();
      int maxAge = INT_MIN;
      for (int i = 0; i < static_cast<int>(m_frames.size()); i++)
      {
         if (!m_frames[i].valid)
         { // Unused slot => directly select
            maxAge = 2;
            selectedFrameSlot = i;
            break;
         }
         else if (m_frames[i].pts + 0.5 < playPTS)
         { // Far behind play time => reclaim regardless of age (screens that are never rendered do not age their slots)
            maxAge = 2;
            selectedFrameSlot = i;
            break;
         }
         else if ((m_frames[i].age > maxAge) && (m_frames[i].pts < playPTS))
         { // Search for the oldest slot with a frame behind play time
            maxAge = m_frames[i].age;
            selectedFrameSlot = i;
         }
      }
      if (maxAge >= 2)
         break;
      // This could be change by a condition variable, waiting on changes of either m_frames or m_running
      SDL_Delay(8);
   }
   if (!m_running)
      return;

   // Take ownership of the frame
   FrameInfo& selectedFrame = m_frames[selectedFrameSlot];
   int targetWidth, targetHeight;
   {
      std::lock_guard lock(m_mutex);
      selectedFrame.valid = false;
      // Read m_bounds under the lock to avoid a data race with SetBounds()
      targetWidth = m_bounds.w > 0 ? m_bounds.w : m_pVideoContext->width;
      targetHeight = m_bounds.h > 0 ? m_bounds.h : m_pVideoContext->height;
   }
   constexpr AVPixelFormat targetFormat = AV_PIX_FMT_RGBA;
   if ((selectedFrame.frame != nullptr) && ((selectedFrame.frame->width != targetWidth) || (selectedFrame.frame->height != targetHeight)))
   {
      m_libAv._av_frame_free(&selectedFrame.frame);
      if (selectedFrame.buffer != nullptr)
         m_libAv._av_free(selectedFrame.buffer);
      if (selectedFrame.texture != nullptr)
         DeleteTexture(selectedFrame.texture);
      selectedFrame.frame = nullptr;
      selectedFrame.buffer = nullptr;
      selectedFrame.texture = nullptr;
   }
   if (selectedFrame.frame == nullptr)
   {
      selectedFrame.frame = m_libAv._av_frame_alloc();
      if (selectedFrame.frame == nullptr)
      {
         LOGE("Failed to create RGB buffer frame"s);
         m_running = false;
         return;
      }
      // Decode into a slot private buffer: the texture backing store must not be touched as it
      // may still be referenced by a pending GPU upload
      const int bufferSize = m_libAv._av_image_get_buffer_size(targetFormat, targetWidth, targetHeight, 1);
      selectedFrame.buffer = static_cast<uint8_t*>(m_libAv._av_malloc(bufferSize + PUP_SWS_DST_PADDING));
      if (selectedFrame.buffer == nullptr)
      {
         LOGE("Failed to allocate RGB buffer"s);
         m_running = false;
         return;
      }
      selectedFrame.frame->width = targetWidth;
      selectedFrame.frame->height = targetHeight;
      selectedFrame.frame->format = targetFormat;
      m_libAv._av_image_fill_arrays(selectedFrame.frame->data, selectedFrame.frame->linesize, selectedFrame.buffer, targetFormat, targetWidth, targetHeight, 1);
   }

   // Create/Update conversion context when source format is known (so after decoding at least one frame)
   m_swsContext = m_libAv._sws_getCachedContext(m_swsContext, frame->width, frame->height, static_cast<AVPixelFormat>(frame->format), selectedFrame.frame->width, selectedFrame.frame->height,
      static_cast<AVPixelFormat>(selectedFrame.frame->format), SWS_BILINEAR, NULL, NULL, NULL);
   if (m_swsContext == nullptr)
      return;

   // Convert to a renderable format (we do not lock as the consumer thread is not supposed to be accessing an outdated frame, and this operation can be a bit lengthy)
   m_libAv._sws_scale(m_swsContext, frame->data, frame->linesize, 0, m_pVideoContext->height, selectedFrame.frame->data, selectedFrame.frame->linesize);
   selectedFrame.frame->opaque = frame->opaque;

   // Apply the transparency mask
   if (m_mask)
   {
      SDL_Surface* sdlMask = m_mask.get();
      if (selectedFrame.frame->width != m_mask->w || selectedFrame.frame->height != m_mask->h)
      {
         if (m_scaledMask == nullptr || selectedFrame.frame->width != m_scaledMask->w || selectedFrame.frame->height != m_scaledMask->h)
            m_scaledMask = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
               SDL_ScaleSurface(m_mask.get(), selectedFrame.frame->width, selectedFrame.frame->height, SDL_ScaleMode::SDL_SCALEMODE_LINEAR), SDL_DestroySurface);
         sdlMask = m_scaledMask.get();
      }
      if (sdlMask)
      {
         SDL_LockSurface(sdlMask);
         const uint8_t* __restrict maskRow = static_cast<const uint8_t*>(sdlMask->pixels);
         uint8_t* __restrict frameRow = selectedFrame.frame->data[0];
         for (int i = 0; i < sdlMask->h; i++)
         {
            const uint32_t* __restrict mask = reinterpret_cast<const uint32_t*>(maskRow);
            uint32_t* __restrict frame2 = reinterpret_cast<uint32_t*>(frameRow);
            for (int j = 0; j < sdlMask->w; j++, mask++, frame2++)
               *frame2 = *mask ? *frame2 : 0x00000000u;
            maskRow += sdlMask->pitch;
            frameRow += selectedFrame.frame->linesize[0];
         }
         SDL_UnlockSurface(sdlMask);
      }
   }

   // Update frame information & mark it as valid for selection by the renderer thread
   {
      std::lock_guard lock(m_mutex);
      m_libAv._av_frame_copy_props(selectedFrame.frame, frame);
      selectedFrame.age = 0;
      int64_t framePts = frame->best_effort_timestamp != AV_NOPTS_VALUE ? frame->best_effort_timestamp : frame->pts;
      const double framePtsSeconds = (framePts == AV_NOPTS_VALUE)
         ? GetPlayTime()
         : (static_cast<double>(framePts) * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
      if (!m_videoPtsBaseValid)
      {
         m_videoPtsBase = framePtsSeconds;
         m_videoPtsBaseValid = true;
      }
      selectedFrame.pts = framePtsSeconds - m_videoPtsBase;
      selectedFrame.uploaded = false;
      selectedFrame.valid = true;
      m_lastDecodedPts = selectedFrame.pts;
      // If playback ran ahead of decoding, resync the clock to the freshest frame instead of
      // fast-forwarding through the deficit. Only when rendering is current (or never happened,
      // which throttles decoding for screens that are not displayed) - render stalls are
      // compensated at render resume so correcting them here too would double count
      const double deficit = GetPlayTime() - selectedFrame.pts;
      if (deficit > 0.25 && (m_lastRenderTicks == 0 || SDL_GetTicks() - m_lastRenderTicks < 250))
         m_startTimestamp += deficit;
   }
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
      LOGE("Couldn't find codec "s + m_libAv._avcodec_get_name(pContext->codec_id));
      m_libAv._avcodec_free_context(&pContext);
      return nullptr;
   }

   pContext->codec_id = pCodec->id;
   if (m_libAv._avcodec_open2(pContext, pCodec, NULL) != 0)
   {
      LOGE("Couldn't open codec "s + m_libAv._avcodec_get_name(pContext->codec_id));
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
      const double decodeTS = GetPlayTime() + 0.5; // Target + 500ms buffer
      if (framePTS > decodeTS)
         SDL_Delay(static_cast<uint32_t>(1000.0 * (framePTS - decodeTS)));
      if (!m_running)
         return;
   }
   m_pAudioLoop = pFrame->opaque;

   const AVSampleFormat frameFormat = static_cast<AVSampleFormat>(pFrame->format);
   constexpr AVChannelLayout destChLayout = AV_CHANNEL_LAYOUT_STEREO;
   const enum AVSampleFormat destFmt = (frameFormat == AV_SAMPLE_FMT_FLT) ? AV_SAMPLE_FMT_FLT : AV_SAMPLE_FMT_S16;
   const int destFreq = pFrame->sample_rate;

   if (!m_pAudioConversionContext || (m_audioFormat != frameFormat) || (m_audioFreq != pFrame->sample_rate))
   {
      m_libAv._swr_free(&m_pAudioConversionContext);
      m_libAv._swr_alloc_set_opts2(&m_pAudioConversionContext, &destChLayout, destFmt, destFreq, &pFrame->ch_layout, frameFormat, pFrame->sample_rate, 0, NULL);
      if (!m_pAudioConversionContext || m_libAv._swr_init(m_pAudioConversionContext) < 0)
      {
         LOGE("Failed to initialize the resampling context"s);
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
      LOGE("av_samples_get_buffer_size() failed"s);
      return;
   }

   uint8_t* pBuffer = nullptr;
   unsigned int bufSize = 0;
   m_libAv._av_fast_malloc(&pBuffer, &bufSize, outSize);
   if (!pBuffer)
      return;

   int nConverted = m_libAv._swr_convert(m_pAudioConversionContext, &pBuffer, pFrame->nb_samples, pFrame->data, pFrame->nb_samples);
   if (nConverted < 0)
   {
      LOGE("swr_convert() failed"s);
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
