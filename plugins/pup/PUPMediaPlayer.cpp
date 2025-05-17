/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 */

//#include "audio/pinsound.h"
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
   //m_pPinSound = new PinSound(nullptr);
   //m_pPinSound->StreamInit(44100, 2, 0.0f);
}

PUPMediaPlayer::~PUPMediaPlayer()
{
   Stop();
   //delete m_pPinSound;
}

void PUPMediaPlayer::Play(const string& szFilename)
{
   LOGD("filename=%s", szFilename.c_str());

   Stop();

   m_szFilename = szFilename;
   m_volume = 0.0f;
   m_loop = false;
   m_startTimestamp = SDL_GetTicks();

   // Open file
   if (avformat_open_input(&m_pFormatContext, szFilename.c_str(), NULL, NULL) != 0) {
      LOGE("Unable to open: filename=%s", szFilename.c_str());
      return;
   }

   // Find video stream
   for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
   {
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
         LOGD("Video stream: %s %dx%d", avcodec_get_name(m_pVideoContext->codec_id), pCodecParameters->width, pCodecParameters->height);
      }
      else {
         LOGE("Unable to open video stream: filename=%s", szFilename.c_str());
      }
   }
   else {
      m_videoStream = -1;
   }

   // Find audio stream
   if (m_videoStream >= 0) {
      m_audioStream = av_find_best_stream(m_pFormatContext, AVMEDIA_TYPE_AUDIO, -1, m_videoStream, NULL, 0);
      if (m_audioStream == AVERROR_DECODER_NOT_FOUND) {
         LOGE("No audio stream found: filename=%s", szFilename.c_str());
      }
   }
   else {
      for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++) {
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
         LOGD("Audio stream: %s %d channels, %d Hz\n", avcodec_get_name(m_pAudioContext->codec_id), pCodecParameters->ch_layout.nb_channels, pCodecParameters->sample_rate);
      }
      else {
         LOGE("Unable to open audio stream: filename=%s", szFilename.c_str());
      }
   }

   if (!m_pVideoContext && !m_pAudioContext) {
      LOGE("No video or audio stream found: filename=%s", szFilename.c_str());
      Stop();
      return;
   }

   LOGD("Playing: filename=%s", m_szFilename.c_str());
   //m_pPinSound->StreamVolume(0);

   m_running = true;
   m_thread = std::thread(&PUPMediaPlayer::Run, this);
}

bool PUPMediaPlayer::IsPlaying()
{
   std::lock_guard<std::mutex> lock(m_mutex);
   return m_running;
}

void PUPMediaPlayer::Pause(bool pause)
{
   if (m_paused != pause)
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_paused = pause;
      if (m_paused)
         m_pauseTimestamp = static_cast<double>(SDL_GetTicks() - m_startTimestamp) / 1000.0; // Freeze at the current playing time
      else
         m_startTimestamp = SDL_GetTicks() - static_cast<Uint64>(1000.0 * m_pauseTimestamp); // Adjust start time to restart from freeze point
   }
}

void PUPMediaPlayer::Stop()
{
   if (IsPlaying())
   {
      LOGD("Stop: %s", m_szFilename.c_str());
   }

   // Stop decoder thread and flush queue
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_running = false;
   }
   if (m_thread.joinable())
      m_thread.join();

   if (m_pFormatContext)
      avformat_close_input(&m_pFormatContext);
   m_pFormatContext = nullptr;

   if (m_pVideoContext)
      avcodec_free_context(&m_pVideoContext);
   m_pVideoContext = nullptr;
   m_videoStream = -1;

   if (m_rgbFrames)
   {
      for (int i = 0; i < m_nRgbFrames; i++)
         if (m_rgbFrames[i])
            av_frame_free(&m_rgbFrames[i]);
      delete[] m_rgbFrames;
      m_rgbFrames = nullptr;
   }
   if (m_rgbFrameBuffers)
   {
      for (int i = 0; i < m_nRgbFrames; i++)
         if (m_rgbFrameBuffers[i])
            av_freep(&m_rgbFrameBuffers[i]);
      delete[] m_rgbFrameBuffers;
      m_rgbFrameBuffers = nullptr;
   }
   m_nRgbFrames = 0;
   m_activeRgbFrame = 0;

   if (m_swsContext)
      sws_freeContext(m_swsContext);
   m_swsContext = nullptr;

   if (m_videoTexture)
      DeleteTexture(m_videoTexture);
   m_videoTexture = nullptr;
   m_videoTextureId = 0xFFFFFF;

   if (m_pAudioContext)
      avcodec_free_context(&m_pAudioContext);
   m_pAudioContext = nullptr;
   m_audioFormat = AV_SAMPLE_FMT_NONE;
   m_audioStream = -1;

   if (m_pAudioConversionContext)
      swr_free(&m_pAudioConversionContext);
   m_pAudioConversionContext = nullptr;
}

void PUPMediaPlayer::SetLoop(bool loop)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if (m_loop != loop) {
      LOGD("setting loop: loop=%d", loop);
      m_loop = loop;
   }
}

void PUPMediaPlayer::SetVolume(float volume)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if (m_volume != volume) {
       LOGD("setting volume: volume=%.1f%%", volume);
       m_volume = volume;
   }
}

void PUPMediaPlayer::SetLength(int length)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if (m_length != length)
   {
      LOGD("setting length: length=%d", length);
      m_length = length;
   }
}

void PUPMediaPlayer::Run()
{
   SetThreadName("PUPMediaPlayer.Run("s.append(m_szFilename).append(")"));

   AVPacket* pPacket = av_packet_alloc();
   if (!pPacket) {
      LOGE("Unable to allocate packet");
      return;
   }

   AVFrame* pFrame = av_frame_alloc();
   if (!pFrame) {
      LOGE("Unable to allocate frame");
      av_packet_free(&pPacket);
      return;
   }

   // Main loop which loops over read/decode/convert and handle video seeking/looping
   while (true)
   {
      // Interact with main thread
      bool loop;
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         if (!m_running)
            break;
         if (m_paused)
         {
            SDL_Delay(100);
            continue;
         }
         if (m_length != 0 && (static_cast<double>(SDL_GetTicks() - m_startTimestamp) / 1000.0) >= m_length)
            break;
         loop = m_loop;
         // m_pPinSound->StreamVolume(m_volume / 100.0f);
      }

      // Read next frame from source
      const int rfRet = av_read_frame(m_pFormatContext, pPacket);
      if (rfRet == AVERROR_EOF)
      {
         // End of stream, loop or stop
         if (!loop)
            break;
         if (m_pVideoContext)
         {
            if (av_seek_frame(m_pFormatContext, m_videoStream, 0, 0) < 0)
            {
               LOGE("Unable to seek video stream. Aborting loop");
               break;
            }
            avcodec_flush_buffers(m_pVideoContext);
         }
         if (m_pAudioContext)
         {
            if (av_seek_frame(m_pFormatContext, m_audioStream, 0, 0) < 0)
            {
               LOGE("Unable to seek audio stream. Aborting loop");
            }
            avcodec_flush_buffers(m_pAudioContext);
         }
         m_playIndex++;
         m_startTimestamp = SDL_GetTicks();
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
         if (avcodec_send_packet(m_pAudioContext, pPacket) != 0)
         {
            LOGE("Unable to send audio packet");
         }
      }
      else if (pPacket->stream_index == m_videoStream)
      {
         if (avcodec_send_packet(m_pVideoContext, pPacket) != 0)
         {
            LOGE("Unable to send video packet");
         }
      }
      av_packet_unref(pPacket);

      // Process decoded frames
      // TODO This should be done on anciliary threads to improve synchronization and better balance the load between CPU cores
      // Here, the synchronization entirely rely on the fact that HandleVideoFrame will block, waiting for a free slot in the circular frame buffer
      // This can cause audio overflow if there are no video (the stream is directly fully decoded) or audio glitches if the muxing is not great
      // (video waiting for frame while audio buffer is exhausted, this one being unlikely).
      if (m_pAudioContext)
      {
         while (avcodec_receive_frame(m_pAudioContext, pFrame) >= 0)
            HandleAudioFrame(pFrame);
      }
      if (m_pVideoContext)
      {
         while (avcodec_receive_frame(m_pVideoContext, pFrame) >= 0)
         {
            pFrame->opaque = reinterpret_cast<void*>(static_cast<uintptr_t>(m_playIndex));
            HandleVideoFrame(pFrame);
         }
      }
   }

   av_frame_free(&pFrame);
   av_packet_free(&pPacket);

   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_running = false;
   }
}

void PUPMediaPlayer::HandleVideoFrame(AVFrame* frame)
{
   // Unknown frame format, don't process frame (would crash in getContext)
   if (frame->format < 0)
      return;

   // Create video frame conversion context and frame queue
   if (m_nRgbFrames == 0)
   {
      const AVPixelFormat targetFormat = AV_PIX_FMT_RGBA;
      const int targetWidth = m_pVideoContext->width; // TODO we could also apply downscaling to the expected render size
      const int targetHeight = m_pVideoContext->height;
      m_nRgbFrames = 3; // TODO shouldn't the queue size be adapted to the video characteristics ?
      m_rgbFrames = new AVFrame*[m_nRgbFrames];
      memset(m_rgbFrames, 0, sizeof(AVFrame*) * m_nRgbFrames);
      m_rgbFrameBuffers = new uint8_t*[m_nRgbFrames];
      memset(m_rgbFrameBuffers, 0, sizeof(uint8_t*) * m_nRgbFrames);
      int rgbFrameSize = av_image_get_buffer_size(targetFormat, targetWidth, targetHeight, 1);
      for (int i = 0; i < m_nRgbFrames; i++)
      {
         m_rgbFrames[i] = av_frame_alloc();
         if (m_rgbFrames[i] == nullptr)
         {
            LOGE("Failed to create RGB buffer frame");
            m_running = false;
            return;
         }
         m_rgbFrameBuffers[i] = static_cast<uint8_t*>(av_malloc(rgbFrameSize * sizeof(uint8_t)));
         if (m_rgbFrameBuffers[i] == nullptr)
         {
            LOGE("Failed to allocate RGB buffer");
            m_running = false;
            return;
         }
         m_rgbFrames[i]->width = targetWidth;
         m_rgbFrames[i]->height = targetHeight;
         m_rgbFrames[i]->format = targetFormat;
         av_image_fill_arrays(m_rgbFrames[i]->data, m_rgbFrames[i]->linesize, m_rgbFrameBuffers[i], targetFormat, targetWidth, targetHeight, 1);
      }
   }

   // m_activeRgbFrame points to the last frame (the one with the highest presentation timestamp)
   int nextFrame = (m_activeRgbFrame + 1) % m_nRgbFrames;
   AVFrame* rgbFrame = m_rgbFrames[nextFrame];

   // Create/Update conversion context when source format is known (so after decoding at least one frame)
   m_swsContext = sws_getCachedContext(m_swsContext, 
      frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
      rgbFrame->width, rgbFrame->height, static_cast<AVPixelFormat>(rgbFrame->format),
      SWS_BILINEAR, NULL, NULL, NULL
   );
   if (m_swsContext == nullptr)
      return;

   // Wait for the buffer slot to be outdated (do not overwrite a frame that is waiting to be displayed), but only in the same play sequence (stored in the opaque field of the frame. if we looped or seek, skip)
   if (rgbFrame->opaque == frame->opaque)
   {
      const double oldPts = (static_cast<double>(rgbFrame->pts) * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
      while (m_running && (static_cast<double>(SDL_GetTicks() - m_startTimestamp) / 1000.0) < oldPts)
         SDL_Delay(8);
   }

   // Convert to a renderable format (we do not lock as the consumer thread is not supposed to be accessing an outdated frame, and this operation can be a bit lengthy)
   const bool resized = sws_scale(m_swsContext, frame->data, frame->linesize, 0, m_pVideoContext->height, rgbFrame->data, rgbFrame->linesize) == m_pVideoContext->height;

   // Update frame PTS and pointer to latest frame under a lock as this modification impacts the consumer thread frame selection
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      av_frame_copy_props(rgbFrame, frame);
      m_activeRgbFrame++;
   }
}

void PUPMediaPlayer::Render(VPXRenderContext2D* const ctx, const SDL_Rect& destRect)
{
   if (m_length != 0)
   {
      const int elapsed = static_cast<int>(SDL_GetTicks() - m_startTimestamp) / 1000;
      if (elapsed >= m_length)
         return;
   }
   else if (!m_running)
   {
      return;
   }

   // Search for the best frame to display and update the video texture accordingly (if needed)
   const double playPts = m_paused ? m_pauseTimestamp : static_cast<double>(SDL_GetTicks() - m_startTimestamp) / 1000.0;
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      unsigned int m_renderFrameId = m_videoTextureId;
      for (int i = 0; i < m_nRgbFrames; i++)
      {
         if (m_activeRgbFrame >= i)
         {
            const AVFrame* rgbFrame = m_rgbFrames[(m_activeRgbFrame + m_nRgbFrames - i) % m_nRgbFrames];
            const double framePts = (static_cast<double>(rgbFrame->pts) * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
            if (playPts <= framePts) // We select the first frame after (or at) the current play timestamp
               m_renderFrameId = m_activeRgbFrame - i;
         }
      }
      if (m_videoTextureId != m_renderFrameId)
      {
         m_videoTextureId = m_renderFrameId;
         AVFrame* rgbFrame = m_rgbFrames[m_renderFrameId % m_nRgbFrames];
         UpdateTexture(&m_videoTexture, rgbFrame->width, rgbFrame->height, VPXTextureFormat::VPXTEXFMT_sRGBA, rgbFrame->data[0]);
         //const double framePts = (static_cast<double>(rgbFrame->pts) * m_pVideoContext->pkt_timebase.num) / m_pVideoContext->pkt_timebase.den;
         //LOGD("Video tex update: play time: %8.3fs / frame pts: %8.3fs / delta: %8.3fs  [%s]", playPts, framePts, framePts - playPts, m_szFilename.c_str());
      }
   }

   // Render image
   if (m_videoTexture)
   {
      int texWidth, texHeight;
      GetTextureInfo(m_videoTexture, &texWidth, &texHeight);
      ctx->DrawImage(ctx, m_videoTexture, 1.f, 1.f, 1.f, 1.f,
         0.f, 0.f, static_cast<float>(texWidth), static_cast<float>(texHeight),
         static_cast<float>(destRect.x), static_cast<float>(destRect.y), static_cast<float>(destRect.w), static_cast<float>(destRect.h));
   }
}

AVCodecContext* PUPMediaPlayer::OpenStream(AVFormatContext* pInputFormatContext, int stream)
{
   AVCodecContext* pContext = avcodec_alloc_context3(NULL);
   if (!pContext)
      return NULL;

   // Request to decode frames on different threads, limiting to the platform core minus 3 (magic number corresponding of the average core used by VPX)
   // TODO Disabled as this delay the frame queue by one frame, breaking single frame videos (and still images used as video)
   //pContext->thread_count = std::max(1u, std::min(16u, std::thread::hardware_concurrency() - 3));
   //pContext->thread_type = FF_THREAD_FRAME;

   if (avcodec_parameters_to_context(pContext, pInputFormatContext->streams[stream]->codecpar) < 0)
   {
      avcodec_free_context(&pContext);
      return NULL;
   }

   pContext->pkt_timebase = pInputFormatContext->streams[stream]->time_base;

   const AVCodec* pCodec = avcodec_find_decoder(pContext->codec_id);
   if (!pCodec) {
      LOGE("Couldn't find codec %s", avcodec_get_name(pContext->codec_id));
      avcodec_free_context(&pContext);
      return NULL;
   }

   pContext->codec_id = pCodec->id;
   if (avcodec_open2(pContext, pCodec, NULL) != 0) {
      LOGE("Couldn't open codec %s", avcodec_get_name(pContext->codec_id));
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
         LOGE("Failed to initialize the resampling context");
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
      LOGE("av_samples_get_buffer_size() failed");
      return;
   }
   if (wantedNbSamples != pFrame->nb_samples) {
      if (swr_set_compensation(m_pAudioConversionContext, (wantedNbSamples - pFrame->nb_samples) * destFreq / pFrame->sample_rate,
         wantedNbSamples * destFreq / pFrame->sample_rate) < 0) {
         LOGE("swr_set_compensation() failed");
         return;
      }
   }
   av_fast_malloc(&pBuffer, &bufSize, outSize);
   if (!pBuffer)
      return;
   int len2 = swr_convert(m_pAudioConversionContext, ppOut, outCount, ppIn, pFrame->nb_samples);
   if (len2 < 0) {
      LOGE("swr_convert() failed");
      av_free(pBuffer);
      return;
   }
   if (len2 == outCount) {
      LOGE("audio buffer is probably too small");
      if (swr_init(m_pAudioConversionContext) < 0) {
         swr_free(&m_pAudioConversionContext);
         m_pAudioConversionContext = NULL;
         av_free(pBuffer);
         return;
      }
   }
   //int resampledDataSize = len2 * destChLayout.nb_channels * av_get_bytes_per_sample(destFmt);
   //m_pPinSound->StreamUpdate(pBuffer, resampledDataSize);

   av_free(pBuffer);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
