#pragma once

#include "VPXPlugin.h"
#include "AssetSrc.h"
#include <SDL3/SDL_surface.h>

class Bitmap
{
public:
   Bitmap(void* pData, AssetType assetType);
   Bitmap(Bitmap* pCachedBitmap);
   ~Bitmap();
   
   PSC_IMPLEMENT_REFCOUNT()

   int GetWidth();
   int GetHeight();
   float GetLength();
   float GetFrameDelay(int pos);
   int GetFrameCount();
   SDL_Surface* GetSurface();
   SDL_Surface* GetFrameSurface(int pos);
   void SetData(void* pData);

private:
   AssetType m_assetType;
   void* m_pData;
};
