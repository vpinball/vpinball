// license:GPLv3+

#include "core/stdafx.h"
#include "MediaPlayer.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace VPX
{

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
#define MEDIA_PLAYER_SWS_DST_PADDING 64

MediaPlayer* MediaPlayer::TryOpen(const char* path)
{
   // The FFmpeg runtime is lazily loaded on Windows: report its absence as a
   // simple open failure
   if (!LibAV::LibAV::GetInstance().isLoaded)
   {
      PLOGE << "MediaPlayer: FFmpeg runtime is not available";
      return nullptr;
   }
   return new MediaPlayer(path);
}

MediaPlayer::MediaPlayer(const char* path)
{
   m_libAV._av_log_set_level(AV_LOG_ERROR);

   if (m_libAV._avformat_open_input(&m_formatContext, path, nullptr, nullptr) != 0)
   {
      PLOGE << "MediaPlayer: unable to open: " << path;
      return;
   }

   // Retrieve stream information (some formats do not have these available in the header, so this is needed to read a few frames and get the info)
   m_libAV._avformat_find_stream_info(m_formatContext, nullptr);

   for (unsigned int i = 0; i < m_formatContext->nb_streams; i++)
   {
      if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !(m_formatContext->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC))
      {
         m_videoStream = i;
         break;
      }
   }

   if (m_videoStream >= 0)
   {
      m_videoContext = OpenStream(m_videoStream);
      if (m_videoContext == nullptr)
         PLOGE << "MediaPlayer: unable to open video stream: " << path;
   }

   if (m_videoStream >= 0)
   {
      m_audioStream = m_libAV._av_find_best_stream(m_formatContext, AVMEDIA_TYPE_AUDIO, -1, m_videoStream, nullptr, 0);
      if (m_audioStream < 0)
      {
         if (m_audioStream == AVERROR_DECODER_NOT_FOUND)
            PLOGE << "MediaPlayer: no audio stream found: " << path;
         m_audioStream = -1;
      }
   }
   else
   {
      for (unsigned int i = 0; i < m_formatContext->nb_streams; i++)
      {
         if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
         {
            m_audioStream = i;
            break;
         }
      }
   }

   if (m_audioStream >= 0)
   {
      m_audioContext = OpenStream(m_audioStream);
      if (m_audioContext == nullptr)
         PLOGE << "MediaPlayer: unable to open audio stream: " << path;
   }

   if (!m_videoContext && !m_audioContext)
   {
      PLOGE << "MediaPlayer: no video or audio stream found: " << path;
      m_libAV._avformat_close_input(&m_formatContext);
      return;
   }

   m_packet = m_libAV._av_packet_alloc();
   if (!m_packet)
   {
      PLOGE << "MediaPlayer: unable to allocate packet";
      m_libAV._avcodec_free_context(&m_videoContext);
      m_libAV._avcodec_free_context(&m_audioContext);
      m_libAV._avformat_close_input(&m_formatContext);
      return;
   }

   if (m_videoContext)
   {
      const AVRational videoTimeBase = m_formatContext->streams[m_videoStream]->time_base;
      m_videoTimeBaseSec = static_cast<double>(videoTimeBase.num) / videoTimeBase.den;
      const double frameRate = av_q2d(m_formatContext->streams[m_videoStream]->avg_frame_rate);
      if (frameRate > 0.)
         m_frameDuration = 1. / frameRate;
   }
   if (m_audioContext)
   {
      const AVRational audioTimeBase = m_formatContext->streams[m_audioStream]->time_base;
      m_audioTimeBaseSec = static_cast<double>(audioTimeBase.num) / audioTimeBase.den;
   }
}

MediaPlayer::~MediaPlayer()
{
   assert(m_liveFrames == 0); // The API contract requires all frames to be released before closing the session
   m_outputs.clear();
   if (m_swsContext)
      m_libAV._sws_freeContext(m_swsContext);
   if (m_audioConversionContext)
      m_libAV._swr_free(&m_audioConversionContext);
   if (m_videoContext)
      m_libAV._avcodec_free_context(&m_videoContext);
   if (m_audioContext)
      m_libAV._avcodec_free_context(&m_audioContext);
   if (m_formatContext)
      m_libAV._avformat_close_input(&m_formatContext);
   if (m_packet)
      m_libAV._av_packet_free(&m_packet);
   for (const PooledPixels& pooled : m_pixelsPool)
      m_libAV._av_free(pooled.data);
}

AVCodecContext* MediaPlayer::OpenStream(int streamIndex)
{
   AVCodecContext* context = m_libAV._avcodec_alloc_context3(nullptr);
   if (!context)
      return nullptr;

   if (m_libAV._avcodec_parameters_to_context(context, m_formatContext->streams[streamIndex]->codecpar) < 0)
   {
      m_libAV._avcodec_free_context(&context);
      return nullptr;
   }

   context->pkt_timebase = m_formatContext->streams[streamIndex]->time_base;

   const AVCodec* codec = m_libAV._avcodec_find_decoder(context->codec_id);
   if (!codec)
   {
      PLOGE << "MediaPlayer: couldn't find codec " << m_libAV._avcodec_get_name(context->codec_id);
      m_libAV._avcodec_free_context(&context);
      return nullptr;
   }

   context->codec_id = codec->id;
   if (m_libAV._avcodec_open2(context, codec, nullptr) != 0)
   {
      PLOGE << "MediaPlayer: couldn't open codec " << m_libAV._avcodec_get_name(context->codec_id);
      m_libAV._avcodec_free_context(&context);
      return nullptr;
   }

   return context;
}

void MediaPlayer::GetInfo(MediaInfo* info) const
{
   memset(info, 0, sizeof(MediaInfo));
   if (!m_formatContext)
      return;

   info->duration = (m_formatContext->duration != AV_NOPTS_VALUE) ? static_cast<double>(m_formatContext->duration) / AV_TIME_BASE : 0.;

   const int clockStream = (m_videoStream >= 0) ? m_videoStream : m_audioStream;
   if (clockStream >= 0)
   {
      const AVStream* stream = m_formatContext->streams[clockStream];
      info->startTime = (stream->start_time != AV_NOPTS_VALUE) ? static_cast<double>(stream->start_time) * stream->time_base.num / stream->time_base.den : 0.;
   }

   if (m_videoContext)
   {
      const AVStream* stream = m_formatContext->streams[m_videoStream];
      info->videoWidth = stream->codecpar->width;
      info->videoHeight = stream->codecpar->height;
      info->videoDuration = (stream->duration > 0) ? static_cast<double>(stream->duration) * stream->time_base.num / stream->time_base.den : 0.;
      info->frameRate = av_q2d(stream->avg_frame_rate);
   }

   if (m_audioContext)
   {
      info->audioSampleRate = m_audioContext->sample_rate;
      info->audioChannels = m_audioContext->ch_layout.nb_channels;
   }
}

int MediaPlayer::Read(MediaOutput* output)
{
   while (m_outputs.empty())
   {
      if (m_inputEof)
      {
         output->type = MEDIA_OUT_EOF;
         return 0;
      }

      const int rfRet = m_libAV._av_read_frame(m_formatContext, m_packet);
      if (rfRet == AVERROR_EOF)
      {
         m_inputEof = true;
         // Flush both decoders so the last buffered frames are not dropped
         if (m_videoContext)
            m_libAV._avcodec_send_packet(m_videoContext, nullptr);
         if (m_audioContext)
            m_libAV._avcodec_send_packet(m_audioContext, nullptr);
         ReceiveFrames(m_audioContext);
         ReceiveFrames(m_videoContext);
      }
      else if (rfRet < 0)
      {
         PLOGE << "MediaPlayer: error reading frame, ret=" << rfRet;
         output->type = MEDIA_OUT_ERROR;
         return 0;
      }
      else
      {
         AVCodecContext* codecContext = nullptr;
         if (m_packet->stream_index == m_videoStream)
            codecContext = m_videoContext;
         else if (m_packet->stream_index == m_audioStream)
            codecContext = m_audioContext;
         if (codecContext && m_libAV._avcodec_send_packet(codecContext, m_packet) != 0)
            PLOGE << "MediaPlayer: unable to send packet";
         m_libAV._av_packet_unref(m_packet);
         ReceiveFrames(m_audioContext);
         ReceiveFrames(m_videoContext);
      }
   }

   m_lastOutput = std::move(m_outputs.front());
   m_outputs.pop_front();
   *output = m_lastOutput.output;
   return 0;
}

int MediaPlayer::Seek(double position)
{
   if (!m_formatContext)
      return -1;
   m_outputs.clear();
   const int ret = m_libAV._av_seek_frame(m_formatContext, -1, static_cast<int64_t>(position * AV_TIME_BASE), AVSEEK_FLAG_BACKWARD);
   if (m_videoContext)
      m_libAV._avcodec_flush_buffers(m_videoContext);
   if (m_audioContext)
      m_libAV._avcodec_flush_buffers(m_audioContext);
   m_inputEof = false;
   m_lastVideoPts = 0.;
   m_lastAudioPts = 0.;
   return ret;
}

void MediaPlayer::ReceiveFrames(AVCodecContext* codecContext)
{
   if (!codecContext)
      return;
   AVFrame* frame = m_libAV._av_frame_alloc();
   if (!frame)
      return;
   while (m_libAV._avcodec_receive_frame(codecContext, frame) >= 0)
   {
      if (codecContext == m_videoContext)
      {
         DecodedFrame* mediaFrame = new DecodedFrame();
         mediaFrame->frame = m_libAV._av_frame_alloc();
         m_libAV._av_frame_move_ref(mediaFrame->frame, frame);
         mediaFrame->pts = FramePtsSeconds(mediaFrame->frame, m_videoStream);
         QueuedOutput queued;
         queued.output.type = MEDIA_OUT_VIDEO;
         queued.output.pts = mediaFrame->pts;
         queued.output.videoFrame = mediaFrame;
         m_outputs.push_back(std::move(queued));
         m_liveFrames++;
      }
      else
      {
         EmitAudioFrame(frame);
      }
   }
   m_libAV._av_frame_free(&frame);
}

void MediaPlayer::EmitAudioFrame(AVFrame* frame)
{
   const AVSampleFormat frameFormat = static_cast<AVSampleFormat>(frame->format);
   constexpr AVChannelLayout destChLayout = AV_CHANNEL_LAYOUT_STEREO;
   const AVSampleFormat destFmt = (frameFormat == AV_SAMPLE_FMT_FLT) ? AV_SAMPLE_FMT_FLT : AV_SAMPLE_FMT_S16;
   const int destFreq = frame->sample_rate;

   if (!m_audioConversionContext || (m_audioFormat != frameFormat) || (m_audioFreq != frame->sample_rate))
   {
      m_libAV._swr_free(&m_audioConversionContext);
      m_libAV._swr_alloc_set_opts2(&m_audioConversionContext, &destChLayout, destFmt, destFreq, &frame->ch_layout, frameFormat, frame->sample_rate, 0, nullptr);
      if (!m_audioConversionContext || m_libAV._swr_init(m_audioConversionContext) < 0)
      {
         PLOGE << "MediaPlayer: failed to initialize the resampling context";
         m_libAV._swr_free(&m_audioConversionContext);
         m_audioConversionContext = nullptr;
         return;
      }
      m_audioFormat = frameFormat;
      m_audioFreq = frame->sample_rate;
   }

   const int outSize = m_libAV._av_samples_get_buffer_size(nullptr, destChLayout.nb_channels, frame->nb_samples, destFmt, 0);
   if (outSize < 0)
      return;

   QueuedOutput queued;
   queued.ownedAudio = std::make_unique_for_overwrite<uint8_t[]>(outSize);
   uint8_t* out = queued.ownedAudio.get();
   const int nConverted = m_libAV._swr_convert(m_audioConversionContext, &out, frame->nb_samples, const_cast<const uint8_t**>(frame->data), frame->nb_samples);
   if (nConverted < 0)
   {
      PLOGE << "MediaPlayer: m_libAV._swr_convert() failed";
      return;
   }
   assert(nConverted == frame->nb_samples); // Since we are not resampling we expect the same number of samples or sync will be lost
   if (nConverted <= 0)
      return;

   queued.output.type = MEDIA_OUT_AUDIO;
   queued.output.pts = (frame->pts != AV_NOPTS_VALUE) ? static_cast<double>(frame->pts) * m_audioTimeBaseSec : -1.;
   queued.output.audioData = queued.ownedAudio.get();
   queued.output.audioSize = nConverted * destChLayout.nb_channels * m_libAV._av_get_bytes_per_sample(destFmt);
   queued.output.audioSampleFormat = (destFmt == AV_SAMPLE_FMT_FLT) ? 1 : 0;
   queued.output.audioChannels = destChLayout.nb_channels;
   queued.output.audioSampleRate = destFreq;
   m_outputs.push_back(std::move(queued));
}

double MediaPlayer::FramePtsSeconds(const AVFrame* frame, int streamIndex)
{
   const double timeBaseSec = (streamIndex == m_videoStream) ? m_videoTimeBaseSec : m_audioTimeBaseSec;
   int64_t pts = frame->best_effort_timestamp;
   if (pts == AV_NOPTS_VALUE)
      pts = frame->pts;
   double& lastPts = (streamIndex == m_videoStream) ? m_lastVideoPts : m_lastAudioPts;
   if (pts == AV_NOPTS_VALUE)
      return lastPts + ((streamIndex == m_videoStream) ? m_frameDuration : 0.);
   lastPts = static_cast<double>(pts) * timeBaseSec;
   return lastPts;
}

uint8_t* MediaPlayer::AcquirePixels(unsigned int width, unsigned int height)
{
   for (size_t i = 0; i < m_pixelsPool.size(); i++)
   {
      if (m_pixelsPool[i].width == width && m_pixelsPool[i].height == height)
      {
         uint8_t* data = m_pixelsPool[i].data;
         m_pixelsPool.erase(m_pixelsPool.begin() + i);
         return data;
      }
   }
   return static_cast<uint8_t*>(m_libAV._av_malloc(width * height * 4 + MEDIA_PLAYER_SWS_DST_PADDING));
}

bool MediaPlayer::Materialize(DecodedFrame* mediaFrame)
{
   // Materialization may be requested from the plugin decoding thread (GetFramePixels)
   // or from the API thread (UpdateTextureFromFrame), so serialize access to the
   // conversion context and the pixel pool
   std::lock_guard lock(m_pixelsMutex);
   if (mediaFrame->pixels)
      return true;
   if (mediaFrame->frame == nullptr || mediaFrame->frame->format < 0)
      return false;

   const unsigned int targetWidth = m_targetWidth.load();
   const unsigned int targetHeight = m_targetHeight.load();
   const unsigned int width = targetWidth ? targetWidth : mediaFrame->frame->width;
   const unsigned int height = targetHeight ? targetHeight : mediaFrame->frame->height;

   m_swsContext = m_libAV._sws_getCachedContext(m_swsContext, mediaFrame->frame->width, mediaFrame->frame->height, static_cast<AVPixelFormat>(mediaFrame->frame->format), width, height,
      AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
   if (!m_swsContext)
      return false;

   uint8_t* buffer = AcquirePixels(width, height);
   if (!buffer)
      return false;
   uint8_t* dst[4] = { buffer, nullptr, nullptr, nullptr };
   int dstLines[4] = { static_cast<int>(width) * 4, 0, 0, 0 };
   m_libAV._sws_scale(m_swsContext, mediaFrame->frame->data, mediaFrame->frame->linesize, 0, mediaFrame->frame->height, dst, dstLines);

   mediaFrame->pixels = buffer;
   mediaFrame->width = width;
   mediaFrame->height = height;
   m_libAV._av_frame_free(&mediaFrame->frame);
   return true;
}

int MediaPlayer::GetFramePixels(MediaFrame frame, const uint8_t** data, unsigned int* width, unsigned int* height)
{
   DecodedFrame* mediaFrame = static_cast<DecodedFrame*>(frame);
   if (!mediaFrame || !Materialize(mediaFrame))
      return -1;
   *data = mediaFrame->pixels;
   *width = mediaFrame->width;
   *height = mediaFrame->height;
   return 0;
}

void MediaPlayer::ReleaseFrame(MediaFrame frame)
{
   DecodedFrame* mediaFrame = static_cast<DecodedFrame*>(frame);
   if (!mediaFrame)
      return;
   m_liveFrames--;
   if (mediaFrame->frame)
      m_libAV._av_frame_free(&mediaFrame->frame);
   if (mediaFrame->pixels)
   {
      std::lock_guard lock(m_pixelsMutex);
      m_pixelsPool.push_back({ mediaFrame->pixels, mediaFrame->width, mediaFrame->height });
   }
   delete mediaFrame;
}

}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
