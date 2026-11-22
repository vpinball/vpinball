#include "Video.h"

namespace Flex {

Video::Video(FlexDMD* pFlexDMD, const string& name)
   : AnimatedActor(pFlexDMD, name)
{
}

Video::~Video()
{
   for (auto* pSurface : m_frames)
      SDL_DestroySurface(pSurface);
}

Video* Video::Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name, bool loop)
{
   AssetSrc* pSrc = pAssetManager->ResolveSrc(path, NULL);
   if (pSrc == nullptr)
      return nullptr;

   const std::filesystem::path videoPath = find_case_insensitive_file_path(PluginStrings::PathFromUTF8(pSrc->GetPath()));
   if (videoPath.empty())
      return nullptr;

   // The actual media decoding is performed by the VPX core through the MediaAPI.
   // When decoding is not available (API not provided, or runtime libraries
   // failing to load), degrade gracefully to a full black video.
   if (mediaApi == nullptr)
   {
      LOGE("Media decoding is not available, video will be black: " + pSrc->GetPath());
      return CreateBlack(pFlexDMD, name);
   }

   Media media = mediaApi->Open(PluginStrings::PathToUTF8(videoPath).c_str(), MEDIA_OPEN_DEFAULT);
   if (media == nullptr)
   {
      LOGE("Media decoding failed, video will be black: " + PluginStrings::PathToUTF8(videoPath));
      return CreateBlack(pFlexDMD, name);
   }

   MediaInfo info {};
   mediaApi->GetInfo(media, &info);
   if (info.videoWidth == 0)
   {
      mediaApi->Close(media);
      return nullptr;
   }

   Video* pVideo = new Video(pFlexDMD, name);
   pVideo->SetLoop(loop);
   pVideo->m_frameDuration = (info.frameRate > 0.) ? static_cast<float>(1.0 / info.frameRate) : 0.f;

   MediaOutput output {};
   while (mediaApi->Read(media, &output) == 0 && output.type != MEDIA_OUT_EOF && output.type != MEDIA_OUT_ERROR)
   {
      if (output.type != MEDIA_OUT_VIDEO)
         continue;
      const uint8_t* pixels = nullptr;
      unsigned int w = 0;
      unsigned int h = 0;
      if (mediaApi->GetFramePixels(media, output.videoFrame, &pixels, &w, &h) == 0)
      {
         SDL_Surface* pSurface = SDL_CreateSurface(static_cast<int>(w), static_cast<int>(h), SDL_PIXELFORMAT_RGBA32);
         SDL_LockSurface(pSurface);
         memcpy(pSurface->pixels, pixels, static_cast<size_t>(w) * h * 4);
         SDL_UnlockSurface(pSurface);
         pVideo->m_frames.push_back(pSurface);
      }
      mediaApi->ReleaseFrame(media, output.videoFrame);
   }
   mediaApi->Close(media);

   pVideo->SetPrefWidth(static_cast<int>(info.videoWidth));
   pVideo->SetPrefHeight(static_cast<int>(info.videoHeight));
   pVideo->m_length = (float)pVideo->m_frames.size() * pVideo->m_frameDuration;
   pVideo->Rewind();
   pVideo->Pack();

   return pVideo;
}

Video* Video::CreateBlack(FlexDMD* pFlexDMD, const string& name)
{
   Video* pVideo = new Video(pFlexDMD, name);
   SDL_Surface* pSurface = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
   static_cast<uint32_t*>(pSurface->pixels)[0] = SDL_MapSurfaceRGBA(pSurface, 0, 0, 0, 255);
   pVideo->m_frames.push_back(pSurface);
   pVideo->SetPrefWidth(1);
   pVideo->SetPrefHeight(1);
   pVideo->m_length = 0.f;
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
