/*
 * Portions of this code was derived from SDL_gfx and BBCSDL:
 *
 * https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx
 * https://github.com/rtrussell/BBCSDL/blob/master/src/SDL2_gfxPrimitives.c
 */

#include "common.h"
#include "VPXGraphics.h"

#include <cmath>
#include <cstring>

#define MAX_GRAPHICS_POLYSIZE 16384

namespace B2SLegacy {

static int GraphicsCompareFloat(const void *a, const void *b)
{
   float diff = *((float*)a + 1) - *((float*)b + 1);
   if (diff != 0.f)
      return (diff > 0.f) - (diff < 0.f);
   diff = *(float*)a - *(float*)b;
   return (diff > 0.f) - (diff < 0.f);
}

VPXGraphics::VPXGraphics(VPXPluginAPI* vpxApi, int width, int height)
   : m_vpxApi(vpxApi),
     m_width(width),
     m_height(height),
     m_texture(nullptr),
     m_translateX(0),
     m_translateY(0),
     m_color(RGB(0, 0, 0)),
     m_alpha(255),
     m_needsTextureUpdate(true)
{
   m_bufferSize = width * height * 4;
   m_pixelBuffer = new uint8_t[m_bufferSize];
   memset(m_pixelBuffer, 0, m_bufferSize);

   m_pModelMatrix = new Matrix();
}

VPXGraphics::~VPXGraphics()
{
   if (m_texture)
      m_vpxApi->DeleteTexture(m_texture);
   delete[] m_pixelBuffer;
   delete m_pModelMatrix;
}

void VPXGraphics::Clear()
{
   memset(m_pixelBuffer, 0, m_bufferSize);
   m_needsTextureUpdate = true;
}

void VPXGraphics::SetColor(uint32_t color, uint8_t alpha)
{
   m_color = color;
   m_alpha = alpha;
}

void VPXGraphics::FillPath(GraphicsPath* pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath->GetPoints();
   if (pPoints->size() < 3)
      return;

   FillPolygon(*pPoints);
   m_needsTextureUpdate = true;
}

void VPXGraphics::DrawPath(GraphicsPath* pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath->GetPoints();
   if (pPoints->size() < 2)
      return;

   DrawPolygonOutline(*pPoints);
   m_needsTextureUpdate = true;
}

void VPXGraphics::FillPolygon(const std::vector<SDL_FPoint>& points)
{
   int n = (int)points.size();
   if (n < 3) return;

   double* const __restrict vx = new double[n];
   double* const __restrict vy = new double[n];

   // Transform points and store in arrays
   int i = 0;
   for (const auto &point : points) {
      float x = point.x;
      float y = point.y;
      if (m_pModelMatrix)
         m_pModelMatrix->TransformPoint(x, y);
      vx[i] = static_cast<double>(x) + m_translateX;
      vy[i++] = static_cast<double>(y) + m_translateY;
   }

   int xi, yi;
   double y0, y1;
   double minx = 99999.0;
   double maxx = -99999.0;
   double prec = 0.00001;
   uint8_t r = GetRValue(m_color);
   uint8_t g = GetGValue(m_color);
   uint8_t b = GetBValue(m_color);
   uint8_t a = m_alpha;

   for (i = 0; i < n; i++) {
      double x = vx[i];
      double y = std::abs(vy[i]);
      if (x < minx) minx = x;
      if (x > maxx) maxx = x;
      if (y > prec) prec = y;
   }
   minx = std::floor(minx);
   maxx = std::floor(maxx);
   prec = std::floor(std::pow(2,19) / prec);

   float* const __restrict list = new float[MAX_GRAPHICS_POLYSIZE];

   yi = 0;
   y0 = std::floor(vy[n - 1] * prec) / prec;
   y1 = std::floor(vy[0] * prec) / prec;
   for (i = 1; i <= n; i++) {
      if (yi > MAX_GRAPHICS_POLYSIZE - 4) {
         delete[] list;
         delete[] vx;
         delete[] vy;
         return;
      }
      double y2 = std::floor(vy[i % n] * prec) / prec;
      if (((y1 < y2) - (y1 > y2)) == ((y0 < y1) - (y0 > y1))) {
         list[yi++] = -100002.0f;
         list[yi++] = (float)y1;
         list[yi++] = -100002.0f;
         list[yi++] = (float)y1;
      }
      else {
         if (y0 != y1) {
            list[yi++] = (float)((y1 < y0) - (y1 > y0)) - 100002.0f;
            list[yi++] = (float)y1;
         }
         if (y1 != y2) {
            list[yi++] = (float)((y1 < y2) - (y1 > y2)) - 100002.0f;
            list[yi++] = (float)y1;
         }
      }
      y0 = y1;
      y1 = y2;
   }
   xi = yi;

   qsort (list, yi / 2, sizeof(float) * 2, GraphicsCompareFloat);

   for (i = 1; i <= n; i++) {
      double x, y;
      double d = 0.5 / prec;

      double x1 = vx[i - 1];
      y1 = floor(vy[i - 1] * prec) / prec;
      double x2 = vx[i % n];
      double y2 = std::floor(vy[i % n] * prec) / prec;

      if (y2 < y1) {
         double tmp1 = x1; x1 = x2; x2 = tmp1;
         double tmp2 = y1; y1 = y2; y2 = tmp2;
      }
      if (y2 != y1)
         y0 = (x2 - x1) / (y2 - y1);

      for (int j = 1; j < xi; j += 4) {
         y = list[j];
         if (((y + d) <= y1) || (y == list[j + 4]))
            continue;
         if ((y -= d) >= y2)
            break;
         if (yi > MAX_GRAPHICS_POLYSIZE - 4) {
            delete[] list;
            delete[] vx;
            delete[] vy;
            return;
         }
         if (y > y1) {
            list[yi++] = (float)(x1 + y0 * (y - y1));
            list[yi++] = (float)y;
         }
         y += d * 2.0;
         if (y < y2) {
            list[yi++] = (float)(x1 + y0 * (y - y1));
            list[yi++] = (float)y;
         }
      }

      y = std::floor(y1) + 1.0;
      while (y <= y2) {
         x = x1 + y0 * (y - y1);
         if (yi > MAX_GRAPHICS_POLYSIZE - 2) {
            delete[] list;
            delete[] vx;
            delete[] vy;
            return;
         }
         list[yi++] = (float)x;
         list[yi++] = (float)y;
         y += 1.0;
      }
   }

   qsort (list, yi / 2, sizeof(float) * 2, GraphicsCompareFloat);

   float* const __restrict strip = new float[(size_t)(maxx - minx) + 2];
   memset(strip, 0, ((size_t)(maxx - minx) + 2) * sizeof(float));
   n = yi;
   yi = (int)list[1];
   int j = 0;
   for (i = 0; i < n - 7; i += 4) {
      float x1 = list[i + 0];
      float y1 = list[i + 1];
      float x3 = list[i + 2];
      float x2 = list[i + j + 0];
      float y2 = list[i + j + 1];
      float x4 = list[i + j + 2];

      if (x1 + x3 == -200002.0f)
         j += 4;
      else if (x1 + x3 == -200006.0f)
         j -= 4;
      else if ((x1 >= minx) && (x2 >= minx)) {
         if (x1 > x2) { float tmp = x1; x1 = x2; x2 = tmp; }
         if (x3 > x4) { float tmp = x3; x3 = x4; x4 = tmp; }

         for ( xi = (int)(x1 - minx); xi <= (int)(x4 - minx); xi++ ) {
            float u, v;
            float x = (float)(minx + xi);
            if (x < x2)  u = (x - x1 + 1.f) / (x2 - x1 + 1.f); else u = 1.0f;
            if (x >= x3 - 1.f) v = (x4 - x) / (x4 - x3 + 1.f); else v = 1.0f;
            if ((u > 0.0f) && (v > 0.0f))
               strip[xi] += (y2 - y1) * (u + v - 1.0f);
         }
      }

      if ((yi == (int)(list[i + 5] - 1.0f)) || (i == n - 8)) {
         for (xi = 0; xi <= maxx - minx; xi++) {
            if (strip[xi] != 0.0f) {
               if (strip[xi] >= 0.996f) {
                  // Fill solid pixels
                  int x0 = xi;
                  while (strip[++xi] >= 0.996f) ;
                  xi--;
                  for (int x = x0; x <= xi; x++) {
                     SetPixel((int)minx + x, yi, r, g, b, a);
                  }
               }
               else {
                  // Anti-aliased pixel
                  uint8_t blendedAlpha = (uint8_t)((float)a * strip[xi]);
                  SetPixelBlended((int)minx + xi, yi, r, g, b, blendedAlpha);
               }
            }
         }
         memset(strip, 0, ((size_t)(maxx - minx) + 2) * sizeof(float));
         yi++;
      }
   }
   delete[] list;
   delete[] strip;
   delete[] vx;
   delete[] vy;
}

void VPXGraphics::DrawPolygonOutline(const std::vector<SDL_FPoint>& points)
{
   if (points.size() < 2)
      return;

   uint8_t r = GetRValue(m_color);
   uint8_t g = GetGValue(m_color);
   uint8_t b = GetBValue(m_color);
   uint8_t a = m_alpha;

   for (size_t i = 0; i < points.size(); i++) {
      size_t nextIndex = (i + 1) % points.size();

      float x1 = points[i].x;
      float y1 = points[i].y;
      float x2 = points[nextIndex].x;
      float y2 = points[nextIndex].y;

      if (m_pModelMatrix) {
         m_pModelMatrix->TransformPoint(x1, y1);
         m_pModelMatrix->TransformPoint(x2, y2);
      }

      x1 += (float)m_translateX;
      y1 += (float)m_translateY;
      x2 += (float)m_translateX;
      y2 += (float)m_translateY;

      DrawLine((int)x1, (int)y1, (int)x2, (int)y2, r, g, b, a);
   }
}

void VPXGraphics::DrawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   int dx = abs(x2 - x1);
   int dy = abs(y2 - y1);
   int sx = (x1 < x2) ? 1 : -1;
   int sy = (y1 < y2) ? 1 : -1;
   int err = dx - dy;

   while (true) {
      SetPixel(x1, y1, r, g, b, a);

      if (x1 == x2 && y1 == y2)
         break;

      int e2 = 2 * err;
      if (e2 > -dy) {
         err -= dy;
         x1 += sx;
      }
      if (e2 < dx) {
         err += dx;
         y1 += sy;
      }
   }
}

void VPXGraphics::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   if (/*x < 0 ||*/ (unsigned int)x >= (unsigned int)m_width /*|| y < 0*/ || (unsigned int)y >= (unsigned int)m_height) // unsigned int test incl. <0
      return;

   int offset = (y * m_width + x) * 4;
   m_pixelBuffer[offset + 0] = r;
   m_pixelBuffer[offset + 1] = g;
   m_pixelBuffer[offset + 2] = b;
   m_pixelBuffer[offset + 3] = a;
}

void VPXGraphics::SetPixelBlended(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   if (/*x < 0 ||*/ (unsigned int)x >= (unsigned int)m_width /*|| y < 0*/ || (unsigned int)y >= (unsigned int)m_height) // unsigned int test incl. <0
      return;

   int offset = (y * m_width + x) * 4;

   uint8_t existingR = m_pixelBuffer[offset + 0];
   uint8_t existingG = m_pixelBuffer[offset + 1];
   uint8_t existingB = m_pixelBuffer[offset + 2];
   uint8_t existingA = m_pixelBuffer[offset + 3];

   float alpha = (float)a / 255.0f;
   float invAlpha = 1.0f - alpha;

   m_pixelBuffer[offset + 0] = (uint8_t)(r * alpha + (float)existingR * invAlpha);
   m_pixelBuffer[offset + 1] = (uint8_t)(g * alpha + (float)existingG * invAlpha);
   m_pixelBuffer[offset + 2] = (uint8_t)(b * alpha + (float)existingB * invAlpha);
   m_pixelBuffer[offset + 3] = (uint8_t)(a         + (float)existingA * invAlpha);
}

VPXTexture VPXGraphics::GetTexture()
{
   if (m_needsTextureUpdate)
      SyncTexture();

   return m_texture;
}

void VPXGraphics::SyncTexture()
{
   UpdateTexture(&m_texture, m_width, m_height, VPXTEXFMT_sRGBA8, m_pixelBuffer);
   m_needsTextureUpdate = false;
}

void VPXGraphics::DrawToContext(VPXRenderContext2D* ctx, int left, int top)
{
   VPXTexture texture = GetTexture();
   if (texture && ctx->DrawImage) {
      ctx->DrawImage(ctx, texture, 1.0f, 1.0f, 1.0f, 1.0f,
                     0, 0, (float)m_width, (float)m_height, 0, 0, 0,
                     (float)left, (float)top, (float)m_width, (float)m_height);
   }
}

void VPXGraphics::TranslateTransform(int x, int y)
{
   m_translateX += x;
   m_translateY += y;
}

void VPXGraphics::ResetTransform()
{
   m_pModelMatrix->Reset();
   m_translateX = 0;
   m_translateY = 0;
}

void VPXGraphics::SetTransform(Matrix* pModelMatrix)
{
   delete m_pModelMatrix;
   m_pModelMatrix = pModelMatrix->Clone();
}

void VPXGraphics::FillRectangle(const SDL_Rect& rect)
{
   uint8_t r = GetRValue(m_color);
   uint8_t g = GetGValue(m_color);
   uint8_t b = GetBValue(m_color);
   uint8_t a = m_alpha;

   int x1 = rect.x + m_translateX;
   int y1 = rect.y + m_translateY;
   int x2 = x1 + rect.w;
   int y2 = y1 + rect.h;

   x1 = std::max(0, x1);
   y1 = std::max(0, y1);
   x2 = std::min(m_width, x2);
   y2 = std::min(m_height, y2);

   for (int y = y1; y < y2; y++) {
      for (int x = x1; x < x2; x++)
         SetPixel(x, y, r, g, b, a);
   }

   m_needsTextureUpdate = true;
}

void VPXGraphics::UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const uint8_t* image)
{
   if (m_vpxApi)
      m_vpxApi->UpdateTexture(texture, width, height, format, image);
}

VPXTexture VPXGraphics::CreateTexture(VPXPluginAPI* vpxApi, SDL_Surface* surf)
{
   VPXTexture texture = nullptr;
   if (vpxApi && surf) {
      SDL_LockSurface(surf);
      vpxApi->UpdateTexture(&texture, surf->w, surf->h, VPXTextureFormat::VPXTEXFMT_sRGBA8, static_cast<uint8_t*>(surf->pixels));
      SDL_UnlockSurface(surf);
   }
   return texture;
}

VPXTexture VPXGraphics::DuplicateTexture(VPXPluginAPI* vpxApi, VPXTexture sourceTexture)
{
   if (!vpxApi || !sourceTexture)
      return nullptr;

   SDL_Surface* tempSurface = VPXTextureToSDLSurface(vpxApi, sourceTexture);
   if (!tempSurface)
      return nullptr;

   VPXTexture duplicateTexture = SDLSurfaceToVPXTexture(vpxApi, tempSurface);
   SDL_DestroySurface(tempSurface);

   return duplicateTexture;
}

SDL_Surface* VPXGraphics::VPXTextureToSDLSurface(VPXPluginAPI* vpxApi, VPXTexture texture)
{
   if (!vpxApi || !texture)
      return nullptr;

   VPXTextureInfo* texInfo = vpxApi->GetTextureInfo(texture);
   if (!texInfo || !texInfo->data)
      return nullptr;

   SDL_PixelFormat sdlFormat;
   switch (texInfo->format) {
      case VPXTEXFMT_sRGBA8:
         sdlFormat = SDL_PIXELFORMAT_RGBA32;
         break;
      case VPXTEXFMT_sRGB8:
         sdlFormat = SDL_PIXELFORMAT_RGB24;
         break;
      case VPXTEXFMT_sRGB565:
         sdlFormat = SDL_PIXELFORMAT_RGB565;
         break;
      case VPXTEXFMT_BW:
         sdlFormat = SDL_PIXELFORMAT_INDEX8;
         break;
      default:
         return nullptr;
   }

   int bytesPerPixel;
   switch (texInfo->format) {
      case VPXTEXFMT_sRGBA8: bytesPerPixel = 4; break;
      case VPXTEXFMT_sRGB8: bytesPerPixel = 3; break;
      case VPXTEXFMT_sRGB565: bytesPerPixel = 2; break;
      case VPXTEXFMT_BW: bytesPerPixel = 1; break;
      default: return nullptr;
   }

   int pitch = texInfo->width * bytesPerPixel;
   SDL_Surface* surface = SDL_CreateSurfaceFrom(texInfo->width, texInfo->height, sdlFormat,
                                                texInfo->data, pitch);

   if (surface) {
      SDL_Surface* copy = SDL_DuplicateSurface(surface);
      SDL_DestroySurface(surface);
      return copy;
   }

   return nullptr;
}

VPXTexture VPXGraphics::SDLSurfaceToVPXTexture(VPXPluginAPI* vpxApi, SDL_Surface* surface)
{
   if (!vpxApi || !surface)
      return nullptr;

   SDL_Surface* convertedSurface = nullptr;
   if (surface->format != SDL_PIXELFORMAT_RGBA32) {
      convertedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
      if (!convertedSurface)
         return nullptr;
   }
   else
      convertedSurface = surface;

   VPXTexture texture = nullptr;
   SDL_LockSurface(convertedSurface);
   vpxApi->UpdateTexture(&texture, convertedSurface->w, convertedSurface->h,
                        VPXTEXFMT_sRGBA8, static_cast<uint8_t*>(convertedSurface->pixels));
   SDL_UnlockSurface(convertedSurface);

   if (convertedSurface != surface)
      SDL_DestroySurface(convertedSurface);

   return texture;
}

void VPXGraphics::DrawImage(VPXPluginAPI* vpxApi, VPXRenderContext2D* ctx, VPXTexture texture, SDL_Rect* srcRect, SDL_Rect* destRect)
{
   if (!ctx || !texture || !ctx->DrawImage)
      return;

   VPXTextureInfo* texInfo = vpxApi->GetTextureInfo(texture);
   if (!texInfo)
      return;

   float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f, alpha = 1.0f;
   float pivotX = 0.0f, pivotY = 0.0f, rotation = 0.0f;

   float texX = srcRect ? (float)srcRect->x : 0.0f;
   float texY = srcRect ? (float)srcRect->y : 0.0f;
   float texW = srcRect ? (float)srcRect->w : (float)texInfo->width;
   float texH = srcRect ? (float)srcRect->h : (float)texInfo->height;

   float srcX = destRect ? (float)destRect->x : 0.0f;
   float srcY = destRect ? (float)destRect->y : 0.0f;
   float srcW = destRect ? (float)destRect->w : ctx->srcWidth;
   float srcH = destRect ? (float)destRect->h : ctx->srcHeight;

   ctx->DrawImage(ctx, texture, tintR, tintG, tintB, alpha,
                  texX, texY, texW, texH, pivotX, pivotY, rotation,
                  srcX, srcY, srcW, srcH);
}

}
