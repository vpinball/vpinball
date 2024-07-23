#pragma once

#include "../flexdmd_i.h"
#include "../Bitmap.h"

class BitmapFilter
{
public:
   BitmapFilter();
   virtual ~BitmapFilter() { };

   virtual void Filter(Bitmap* pBitmap) = 0;

protected:
   static UINT32 GetPixel(SDL_Surface* surface, int x, int y);
};
