#include "Bitmap.h"
#include <SDL3_image/SDL_image.h>

Bitmap::Bitmap(void* pData, AssetType assetType)
{
   m_pData = pData;
   m_assetType = assetType;
}

Bitmap::Bitmap(Bitmap* pCachedBitmap)
{
   m_pData = (void*)SDL_ConvertSurface(pCachedBitmap->GetSurface(), SDL_PIXELFORMAT_RGBA32);
   m_assetType = pCachedBitmap->m_assetType;
}

Bitmap::~Bitmap()
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         SDL_DestroySurface((SDL_Surface*)m_pData);
      else if (m_assetType == AssetType_GIF)
         IMG_FreeAnimation((IMG_Animation*)m_pData);
   }
}

void Bitmap::SetData(void* pData)
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         SDL_DestroySurface((SDL_Surface*)m_pData);
      else if (m_assetType == AssetType_GIF)
        IMG_FreeAnimation((IMG_Animation*)m_pData);
   }

   m_pData = pData;
}

SDL_Surface* Bitmap::GetSurface() const
{
   return (m_pData && (m_assetType == AssetType_Image)) ? (SDL_Surface*)m_pData : nullptr;
}

SDL_Surface* Bitmap::GetFrameSurface(int pos) const
{
   return (m_pData && (m_assetType == AssetType_GIF)) ? ((IMG_Animation*)m_pData)->frames[pos] : nullptr;
}

int Bitmap::GetWidth() const
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         return ((SDL_Surface*)m_pData)->w;
      else if (m_assetType == AssetType_GIF)
         return ((IMG_Animation*)m_pData)->w;
   }

   return 0;
}

int Bitmap::GetHeight() const
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         return ((SDL_Surface*)m_pData)->h;
      else if (m_assetType == AssetType_GIF)
         return ((IMG_Animation*)m_pData)->h;
   }

   return 0;
}

float Bitmap::GetLength() const
{
   int length = 0;
   if (m_pData && m_assetType == AssetType_GIF)
   {
      IMG_Animation* pAnimation = (IMG_Animation*)m_pData;
      for (int index = 0; index < pAnimation->count; index++)
         length += pAnimation->delays[index];
   }
   return (float)length;
}

float Bitmap::GetFrameDelay(int pos) const
{
   if (m_pData && m_assetType == AssetType_GIF)
      return static_cast<float>(((IMG_Animation*)m_pData)->delays[pos]);

   return 0.0f;
}

int Bitmap::GetFrameCount() const
{
   if (m_pData && m_assetType == AssetType_GIF)
      return ((IMG_Animation*)m_pData)->count;

   return 0;
}
