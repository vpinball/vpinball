#include "PUPImage.h"

#include <SDL3_image/SDL_image.h>

namespace PUP {

PUPImage::~PUPImage()
{
   if (m_pSurface)
      SDL_DestroySurface(m_pSurface);
   if (m_pTexture)
      DeleteTexture(m_pTexture);
}

void PUPImage::Load(const string& szFile)
{
   if (m_pSurface) {
      SDL_DestroySurface(m_pSurface);
      m_pSurface = nullptr;
   }
   if (m_pTexture) {
      DeleteTexture(m_pTexture);
      m_pTexture = nullptr;
   }
   
   m_pSurface = IMG_Load(szFile.c_str());
   if (m_pSurface && m_pSurface->format != SDL_PIXELFORMAT_RGBA32) {
      SDL_Surface* rgbaSurface = SDL_ConvertSurface(m_pSurface, SDL_PIXELFORMAT_RGBA32);
      SDL_DestroySurface(m_pSurface);
      m_pSurface = rgbaSurface;
   }
}

void PUPImage::Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect)
{
   // Update texture
   if (m_pTexture == nullptr && m_pSurface) {
      m_pTexture = CreateTexture(m_pSurface);
      SDL_DestroySurface(m_pSurface);
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