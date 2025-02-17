#include "BitmapFilter.h"

BitmapFilter::BitmapFilter()
{
}

ColorRGBA32 BitmapFilter::GetPixel(const SDL_Surface* const surface, int x, int y)
{
   const SDL_PixelFormatDetails* const format = SDL_GetPixelFormatDetails(surface->format);
   const uint8_t* const pixel = (uint8_t*)surface->pixels + y * surface->pitch + x * format->bytes_per_pixel;

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
         return *(ColorRGBA32*)pixel;
      default:
         return 0;
   }
}
