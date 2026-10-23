// license:GPLv3+

#include "PUPImage.h"

#include <SDL3_image/SDL_image.h>

namespace PUP {

PUPImage::PUPImage()
   : m_pSurface(nullptr, SDL_DestroySurface)
{
}

PUPImage::~PUPImage()
{
   if (m_pTexture)
      DeleteTexture(m_pTexture);
}

void PUPImage::AbandonPendingLoad()
{
   std::erase_if(m_abandonedLoads, [](const std::future<SurfacePtr>& load) { return load.wait_for(std::chrono::seconds(0)) == std::future_status::ready; });
   if (m_pendingSurface.valid())
      m_abandonedLoads.push_back(std::move(m_pendingSurface));
}

void PUPImage::Clear()
{
   m_file.clear();
   if (m_pTexture) {
      DeleteTexture(m_pTexture);
      m_pTexture = nullptr;
   }
   m_pSurface.reset();
   AbandonPendingLoad();
}

void PUPImage::Load(const std::filesystem::path& szFile)
{
   if (szFile == m_file)
      return;
   m_file = szFile;
   m_pSurface.reset();

   AbandonPendingLoad();
   m_pendingSurface = std::async(std::launch::async, [szFile]() {
      SurfacePtr surface(IMG_Load(PluginStrings::PathToUTF8(szFile).c_str()), SDL_DestroySurface);
      if (surface && surface->format != SDL_PIXELFORMAT_RGBA32)
         surface = SurfacePtr(SDL_ConvertSurface(surface.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
      return surface;
   });
}

void PUPImage::Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect, float alpha)
{
   if (m_pendingSurface.valid() && m_pendingSurface.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      m_pSurface = m_pendingSurface.get();
      if (m_pTexture) {
         DeleteTexture(m_pTexture);
         m_pTexture = nullptr;
      }
   }

   if (m_pTexture == nullptr && m_pSurface) {
      m_pTexture = CreateTexture(m_pSurface.get());
      m_pSurface = nullptr;
   }

   if (m_pTexture && alpha > 0.f)
   {
      VPXTextureInfo* texInfo = GetTextureInfo(m_pTexture);
      ctx->DrawImage(ctx, m_pTexture, 1.f, 1.f, 1.f, alpha,
         0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height),
         0.f, 0.f, 0.f,
         static_cast<float>(rect.x), static_cast<float>(rect.y), static_cast<float>(rect.w), static_cast<float>(rect.h));
   }
}

}
