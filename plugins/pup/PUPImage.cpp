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

void PUPImage::Load(const std::filesystem::path& szFile)
{
   m_file = szFile;

   if (m_pTexture) {
      DeleteTexture(m_pTexture);
      m_pTexture = nullptr;
   }

   m_pSurface = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(IMG_Load(szFile.string().c_str()), SDL_DestroySurface);
   if (m_pSurface && m_pSurface->format != SDL_PIXELFORMAT_RGBA32)
      m_pSurface = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(SDL_ConvertSurface(m_pSurface.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
}

void PUPImage::Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect)
{
   // Update texture
   if (m_pTexture == nullptr && m_pSurface) {
      m_pTexture = CreateTexture(m_pSurface.get());
      m_pSurface = nullptr;
   }

   // Render image
   if (m_pTexture)
   {
      VPXTextureInfo* texInfo = GetTextureInfo(m_pTexture);
      ctx->DrawImage(ctx, m_pTexture, 1.f, 1.f, 1.f, 1.f,
         0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height),
         0.f, 0.f, 0.f, 
         static_cast<float>(rect.x), static_cast<float>(rect.y), static_cast<float>(rect.w), static_cast<float>(rect.h));
   }
}

}
