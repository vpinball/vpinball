// license:GPLv3+

#include "PUPImage.h"

#include <SDL3_image/SDL_image.h>
#include <thread>

namespace PUP {

PUPImage::PUPImage()
   : m_pSurface(nullptr, SDL_DestroySurface)
   , m_pendingSurface(nullptr, SDL_DestroySurface)
{
}

PUPImage::~PUPImage()
{
   while (m_loading.load())
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   if (m_pTexture)
      DeleteTexture(m_pTexture);
}

void PUPImage::Clear()
{
   m_file.clear();
   if (m_pTexture) {
      DeleteTexture(m_pTexture);
      m_pTexture = nullptr;
   }
   m_pSurface.reset();
   std::lock_guard lock(m_loadMutex);
   m_pendingSurface.reset();
}

void PUPImage::Load(const std::filesystem::path& szFile)
{
   m_file = szFile;

   if (m_pTexture) {
      DeleteTexture(m_pTexture);
      m_pTexture = nullptr;
   }
   m_pSurface.reset();

   m_loading.store(true);
   std::thread([this, szFile]() {
      auto surface = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(IMG_Load(szFile.string().c_str()), SDL_DestroySurface);
      if (surface && surface->format != SDL_PIXELFORMAT_RGBA32)
         surface = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(SDL_ConvertSurface(surface.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
      {
         std::lock_guard lock(m_loadMutex);
         m_pendingSurface = std::move(surface);
      }
      m_loading.store(false);
   }).detach();
}

void PUPImage::Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect, float alpha)
{
   // Pick up async-loaded surface
   {
      std::lock_guard lock(m_loadMutex);
      if (m_pendingSurface) {
         m_pSurface = std::move(m_pendingSurface);
         if (m_pTexture) {
            DeleteTexture(m_pTexture);
            m_pTexture = nullptr;
         }
      }
   }

   // Update texture
   if (m_pTexture == nullptr && m_pSurface) {
      m_pTexture = CreateTexture(m_pSurface.get());
      m_pSurface = nullptr;
   }

   // Render image
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
