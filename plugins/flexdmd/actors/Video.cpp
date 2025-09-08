#include "Video.h"

namespace Flex {

Video::Video(FlexDMD* pFlexDMD, const string& name)
    : AnimatedActor(pFlexDMD, name) {}

Video::~Video()
{
   for (auto* pSurface : m_frames)
      SDL_DestroySurface(pSurface);

   if (m_pVideoConversionContext)
      sws_freeContext(m_pVideoConversionContext);

   if (m_pCodecContext)
      avcodec_free_context(&m_pCodecContext);

   if (m_pFormatContext)
      avformat_close_input(&m_pFormatContext);
}

Video* Video::Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name, bool loop)
{
   AssetSrc* pSrc = pAssetManager->ResolveSrc(path, NULL);
   Video* pVideo = new Video(pFlexDMD, name);
   pVideo->SetLoop(loop);

   if (avformat_open_input(&pVideo->m_pFormatContext, pSrc->GetPath().c_str(), NULL, NULL) != 0)
      return nullptr;

   avformat_find_stream_info(pVideo->m_pFormatContext, NULL);
   for (unsigned i = 0; i < pVideo->m_pFormatContext->nb_streams; ++i) {
      if (pVideo->m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
         pVideo->m_videoStreamIndex = i;
         break;
      }
   }

   if (pVideo->m_videoStreamIndex < 0)
      return nullptr;

   AVStream* pStream = pVideo->m_pFormatContext->streams[pVideo->m_videoStreamIndex];
   pVideo->m_frameDuration = 1.0f / av_q2d(pStream->avg_frame_rate);
   const AVCodec* codec = avcodec_find_decoder(pStream->codecpar->codec_id);
   pVideo->m_pCodecContext = avcodec_alloc_context3(codec);
   avcodec_parameters_to_context(pVideo->m_pCodecContext, pStream->codecpar);
   avcodec_open2(pVideo->m_pCodecContext, codec, NULL);

   int w = pVideo->m_pCodecContext->width;
   int h = pVideo->m_pCodecContext->height;
   AVPacket* pPacket = av_packet_alloc();
   AVFrame* pFrame = av_frame_alloc();
   while (av_read_frame(pVideo->m_pFormatContext, pPacket) >= 0) {
      if (pPacket->stream_index == pVideo->m_videoStreamIndex) {
         avcodec_send_packet(pVideo->m_pCodecContext, pPacket);
         while (avcodec_receive_frame(pVideo->m_pCodecContext, pFrame) >= 0) {
         if (!pVideo->m_pVideoConversionContext)
            pVideo->m_pVideoConversionContext = sws_getContext(w, h, pVideo->m_pCodecContext->pix_fmt,
                                              w, h, AV_PIX_FMT_RGBA,
                                              SWS_FAST_BILINEAR, NULL, NULL, NULL);

            int bufSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, w, h, 1);
            uint8_t* pBuffer = (uint8_t*)av_malloc(bufSize);
            uint8_t* dst[4] = { pBuffer, NULL, NULL, NULL };
            int dstLines[4] = { 4 * w, 0, 0, 0 };
            sws_scale(pVideo->m_pVideoConversionContext, pFrame->data, pFrame->linesize, 0, h, dst, dstLines);

            SDL_Surface* pSurface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
            SDL_LockSurface(pSurface);
            memcpy(pSurface->pixels, pBuffer, bufSize);
            SDL_UnlockSurface(pSurface);
            av_free(pBuffer);
            pVideo->m_frames.push_back(pSurface);
         }
      }
      av_packet_unref(pPacket);
    }

    av_frame_free(&pFrame);
    av_packet_free(&pPacket);

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