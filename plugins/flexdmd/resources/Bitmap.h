#pragma once

#include "VPXPlugin.h"
#include "AssetSrc.h"
#include <SDL3/SDL_surface.h>

class Bitmap final
{
public:
   Bitmap(void* pData, AssetType assetType);
   Bitmap(Bitmap* pCachedBitmap);
   ~Bitmap();
   
   PSC_IMPLEMENT_REFCOUNT()

   int GetWidth() const;
   int GetHeight() const;
   float GetLength() const;
   float GetFrameDelay(int pos) const;
   int GetFrameCount() const;
   SDL_Surface* GetSurface() const;
   SDL_Surface* GetFrameSurface(int pos) const;
   void SetData(void* pData);

private:
   AssetType m_assetType;
   void* m_pData;
};
