#include "core/stdafx.h"
#include "BitmapFilter.h"

BitmapFilter::BitmapFilter()
{
}

UINT32 BitmapFilter::GetPixel(SDL_Surface* surface, int x, int y)
{
   const SDL_PixelFormatDetails* format = SDL_GetPixelFormatDetails(surface->format);
   UINT8* pixel = (UINT8*)surface->pixels + y * surface->pitch + x * format->bytes_per_pixel;

   switch (format->bytes_per_pixel) {
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