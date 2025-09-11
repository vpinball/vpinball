#include "Video.h"

namespace Flex {

Video::Video(FlexDMD* pFlexDMD, const string& name)
   : AnimatedActor(pFlexDMD, name)
   , m_libAv(LibAV::LibAV::GetInstance())
{
   assert(m_libAv.isLoaded);
}

Video::~Video()
{
   for (auto* pSurface : m_frames)
      SDL_DestroySurface(pSurface);
}

Video* Video::Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name, bool loop)
{
   AssetSrc* pSrc = pAssetManager->ResolveSrc(path, NULL);
   Video* pVideo = new Video(pFlexDMD, name);
   pVideo->SetLoop(loop);

   const LibAV::LibAV& m_libAv = LibAV::LibAV::GetInstance();
   assert(m_libAv.isLoaded);

   SwsContext* m_pVideoConversionContext = nullptr;
   AVFormatContext* m_pFormatContext = nullptr;
   AVCodecContext* m_pCodecContext = nullptr;
   int m_videoStreamIndex = -1;

   if (m_libAv._avformat_open_input(&m_pFormatContext, pSrc->GetPath().c_str(), nullptr, nullptr) != 0)
      return nullptr;

   m_libAv._avformat_find_stream_info(m_pFormatContext, nullptr);
   for (unsigned i = 0; i < m_pFormatContext->nb_streams; ++i) {
      if (m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
         m_videoStreamIndex = i;
         break;
      }
   }

   if (m_videoStreamIndex < 0)
      return nullptr;

   AVStream* pStream = m_pFormatContext->streams[m_videoStreamIndex];
   pVideo->m_frameDuration = static_cast<float>(1.0 / av_q2d(pStream->avg_frame_rate));
   const AVCodec* codec = m_libAv._avcodec_find_decoder(pStream->codecpar->codec_id);
   m_pCodecContext = m_libAv._avcodec_alloc_context3(codec);
   m_libAv._avcodec_parameters_to_context(m_pCodecContext, pStream->codecpar);
   m_libAv._avcodec_open2(m_pCodecContext, codec, nullptr);

   int w = m_pCodecContext->width;
   int h = m_pCodecContext->height;
   AVPacket* pPacket = m_libAv._av_packet_alloc();
   AVFrame* pFrame = m_libAv._av_frame_alloc();
   while (m_libAv._av_read_frame(m_pFormatContext, pPacket) >= 0)
   {
      if (pPacket->stream_index == m_videoStreamIndex) {
         m_libAv._avcodec_send_packet(m_pCodecContext, pPacket);
         while (m_libAv._avcodec_receive_frame(m_pCodecContext, pFrame) >= 0)
         {
            if (!m_pVideoConversionContext)
               m_pVideoConversionContext = m_libAv._sws_getCachedContext(nullptr, w, h, m_pCodecContext->pix_fmt, w, h, AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

            int bufSize = m_libAv._av_image_get_buffer_size(AV_PIX_FMT_RGBA, w, h, 1);
            uint8_t* pBuffer = (uint8_t*)m_libAv._av_malloc(bufSize);
            uint8_t* dst[4] = { pBuffer, nullptr, nullptr, nullptr };
            int dstLines[4] = { 4 * w, 0, 0, 0 };
            m_libAv._sws_scale(m_pVideoConversionContext, pFrame->data, pFrame->linesize, 0, h, dst, dstLines);

            SDL_Surface* pSurface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
            SDL_LockSurface(pSurface);
            memcpy(pSurface->pixels, pBuffer, bufSize);
            SDL_UnlockSurface(pSurface);
            m_libAv._av_free(pBuffer);
            pVideo->m_frames.push_back(pSurface);
         }
      }
      m_libAv._av_packet_unref(pPacket);
    }

    m_libAv._av_frame_free(&pFrame);
    m_libAv._av_packet_free(&pPacket);

    if (m_pVideoConversionContext)
       m_libAv._sws_freeContext(m_pVideoConversionContext);

    if (m_pCodecContext)
       m_libAv._avcodec_free_context(&m_pCodecContext);

    if (m_pFormatContext)
       m_libAv._avformat_close_input(&m_pFormatContext);

    pVideo->SetPrefWidth((float)w);
    pVideo->SetPrefHeight((float)h);
    pVideo->m_length = pVideo->m_frames.size() * pVideo->m_frameDuration;
    pVideo->Rewind();
    pVideo->Pack();

    return pVideo;
}

void Video::OnStageStateChanged()
{
}

void Video::Rewind()
{
   AnimatedActor::Rewind();
   m_pos = 0;
   if (!m_frames.empty()) {
      SetFrameDuration(m_frameDuration);
      m_pActiveFrameSurface = m_frames[0];
   }
}

void Video::ReadNextFrame()
{
   if (m_frames.empty())
      return;

   if (m_pos >= (int)m_frames.size() - 1)
      SetEndOfAnimation(true);
   else {
      m_pos++;
      SetFrameTime(GetFrameTime() + m_frameDuration);
      SetFrameDuration(m_frameDuration);
      m_pActiveFrameSurface = m_frames[m_pos];
   }
}

void Video::Draw(Flex::SurfaceGraphics* pGraphics)
{
   if (GetVisible() && m_pActiveFrameSurface) {
      float w = 0;
      float h = 0;
      Layout::Scale(GetScaling(), GetPrefWidth(), GetPrefHeight(), GetWidth(), GetHeight(), w, h);
      float x = 0;
      float y = 0;
      Layout::Align(GetAlignment(), w, h, GetWidth(), GetHeight(), x, y);
      SDL_Rect rect = { (int)(GetX() + x), (int)(GetY() + y), (int)w, (int)h };
      pGraphics->DrawImage(m_pActiveFrameSurface, NULL, &rect);
   }
}

}