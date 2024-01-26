#include "stdafx.h"
#include "Bitmap.h"

Bitmap::Bitmap(void* pData, AssetType assetType)
{
   m_pData = pData;
   m_assetType = assetType;
}

Bitmap::Bitmap(Bitmap* pCachedBitmap)
{
   SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
   m_pData = (void*)SDL_ConvertSurface(pCachedBitmap->GetSurface(), format, 0);
   SDL_FreeFormat(format);
   m_assetType = pCachedBitmap->m_assetType;
}

Bitmap::~Bitmap()
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         SDL_FreeSurface((SDL_Surface*)m_pData);
      else if (m_assetType == AssetType_GIF)
         IMG_FreeAnimation((IMG_Animation*)m_pData);
   }
}

void Bitmap::SetData(void* pData)
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         SDL_FreeSurface((SDL_Surface*)m_pData);
      else if (m_assetType == AssetType_GIF)
         IMG_FreeAnimation((IMG_Animation*)m_pData);
   }

   m_pData = pData;
}

SDL_Surface* Bitmap::GetSurface()
{
   return (m_pData && (m_assetType == AssetType_Image)) ? (SDL_Surface*)m_pData : NULL;
}

SDL_Surface* Bitmap::GetFrameSurface(int pos)
{
   return (m_pData && (m_assetType == AssetType_GIF)) ? ((IMG_Animation*)m_pData)->frames[pos] : NULL;
}

int Bitmap::GetWidth()
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         return ((SDL_Surface*)m_pData)->w;
      else if (m_assetType == AssetType_GIF)
         return ((IMG_Animation*)m_pData)->w;
   }

   return 0;
}

int Bitmap::GetHeight()
{
   if (m_pData) {
      if (m_assetType == AssetType_Image)
         return ((SDL_Surface*)m_pData)->h;
      else if (m_assetType == AssetType_GIF)
         return ((IMG_Animation*)m_pData)->h;
   }

   return 0;
}

float Bitmap::GetLength()
{
   float length = 0;
   if (m_pData && m_assetType == AssetType_GIF) {
      IMG_Animation* pAnimation = (IMG_Animation*)m_pData;
      for (int index = 0; index < pAnimation->count; index++)
         length += pAnimation->delays[index];
   }
   return length;
}

float Bitmap::GetFrameDelay(int pos)
{
   if (m_pData && m_assetType == AssetType_GIF)
      return ((IMG_Animation*)m_pData)->delays[pos];

   return 0.0f;
}

int Bitmap::GetFrameCount()
{
   if (m_pData && m_assetType == AssetType_GIF)
      return ((IMG_Animation*)m_pData)->count;

   return 0;
}