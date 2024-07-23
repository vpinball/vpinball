#include "stdafx.h"
#include "BitmapFilter.h"

BitmapFilter::BitmapFilter()
{
}

UINT32 BitmapFilter::GetPixel(SDL_Surface* surface, int x, int y)
{
   UINT8* pixel = (UINT8*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;

   switch (surface->format->BytesPerPixel) {
      case 1:
         return *pixel;
      case 2:
         return *(Uint16*)pixel;
      case 3:
         if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
            return pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
         else
            return (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
      case 4:
         return *(UINT32*)pixel;
      default:
         return 0;
   }
}