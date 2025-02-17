#pragma once

#include "Bitmap.h"
#include "common.h"
#include <SDL3/SDL_surface.h>

class BitmapFilter
{
public:
   BitmapFilter();
   virtual ~BitmapFilter() { };


   virtual void Filter(Bitmap* pBitmap) = 0;

protected:
   static ColorRGBA32 GetPixel(const SDL_Surface* const surface, int x, int y);
};
