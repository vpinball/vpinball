// license:GPLv3+

#include "core/stdafx.h"
#include "Texture.h"

#ifndef __STANDALONE__
#include "FreeImage.h"
#else
#include "standalone/FreeImage.h"
#endif

#include "math/math.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG // only use the SSE2-JPG path from stbi, as all others are not faster than FreeImage //!! can remove stbi again if at some point FreeImage incorporates libjpeg-turbo or something similar
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#if defined(ENABLE_BGFX)
#include <bimg/bimg.h>
#include <bimg/decode.h>
#include <bimg/encode.h>
#include <bx/file.h>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <bx/math.h>
#endif

#ifdef __STANDALONE__
#include <fstream>
#include <iostream>
#endif

static inline int GetPixelSize(const BaseTexture::Format format)
{
   switch (format)
   {
   case BaseTexture::BW: return 1;
   case BaseTexture::RGB: return 3;
   case BaseTexture::RGBA: return 4;
   case BaseTexture::SRGB: return 3;
   case BaseTexture::SRGBA: return 4;
   case BaseTexture::SRGB565: return 2;
   case BaseTexture::RGB_FP16: return 2 * 3;
   case BaseTexture::RGBA_FP16: return 2 * 4;
   case BaseTexture::RGB_FP32: return 4 * 3;
   case BaseTexture::RGBA_FP32: return 4 * 4;
   default: assert(false); return 0;
   }
}

BaseTexture::BaseTexture(const unsigned int w, const unsigned int h, const Format format)
   : m_realWidth(w)
   , m_realHeight(h)
   , m_format(format)
   , m_width(w)
   , m_height(h)
   , m_liveHash(((unsigned long long)this) ^ usec() ^ ((unsigned long long)w << 16) ^ ((unsigned long long)h << 32) ^ format)
   , m_data(new BYTE[w * h * GetPixelSize(format)])
{
}

BaseTexture::~BaseTexture()
{
   delete[] m_data;
}

unsigned int BaseTexture::pitch() const
{
   return m_width * GetPixelSize(m_format);
}

BaseTexture* BaseTexture::Create(const unsigned int w, const unsigned int h, const Format format) noexcept
{
   BaseTexture* tex = nullptr;
   try
   {
      tex = new BaseTexture(w, h, format);
   }
   // failed to get mem?
   catch (...)
   {
      delete tex;
      return nullptr;
   }
   return tex;
}

BaseTexture* BaseTexture::CreateFromFile(const string& filename, unsigned int maxTexDimension, bool resizeOnLowMem) noexcept
{
   if (filename.empty())
      return nullptr;
   PinBinary ppb;
   ppb.ReadFromFile(filename);
   return CreateFromData(ppb.m_pdata, ppb.m_cdata, maxTexDimension, resizeOnLowMem);
}

BaseTexture* BaseTexture::CreateFromData(const void* data, const size_t size, unsigned int maxTexDimension, bool resizeOnLowMem) noexcept
{
   BaseTexture* tex = nullptr;
   
   // Try to load using fast JPG path via stbi if no texture resize must be triggered
   if (maxTexDimension == 0 && !resizeOnLowMem)
   {
      int x, y, channels_in_file = 0;
      const int ok = stbi_info_from_memory(static_cast<stbi_uc const *>(data), static_cast<int>(size), &x, &y, &channels_in_file); // Request stbi to convert image to BW, SRGB or SRGBA
      assert(channels_in_file != 2);
      assert(channels_in_file <= 4); // 2 or >4 should never happen for JPEGs (4 also not, but we handle it anyway)
      unsigned char * const __restrict stbi_data = (ok && channels_in_file != 2 && channels_in_file <= 4) ?
          stbi_load_from_memory(static_cast<stbi_uc const *>(data), static_cast<int>(size), &x, &y, &channels_in_file, channels_in_file) :
          nullptr;
      if (stbi_data) // will only enter this path for JPG files
      {
         tex = BaseTexture::Create(x, y, channels_in_file == 4 ? BaseTexture::SRGBA : ((channels_in_file == 1) ? BaseTexture::BW : BaseTexture::SRGB));
         if (tex)
         {
            BYTE* const __restrict pdst = tex->data();
            const BYTE* const __restrict psrc = (BYTE*)stbi_data;
            memcpy(pdst, psrc, x * y * channels_in_file);
            stbi_image_free(stbi_data);
            return tex;
         }
         stbi_image_free(stbi_data);
      }
   }

   if (tex == nullptr)
   {
      FIMEMORY * const dataHandle = FreeImage_OpenMemory((BYTE*)data, (DWORD)size);
      if (!dataHandle)
         return nullptr;
      // Check the file signature and deduce its format then check that the plugin has reading capabilities
      const FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(dataHandle, (int)size);
      if ((fif == FIF_UNKNOWN) || !FreeImage_FIFSupportsReading(fif))
      {
         FreeImage_CloseMemory(dataHandle);
         return nullptr;
      }
      // Load
      FIBITMAP * const dib = FreeImage_LoadFromMemory(fif, dataHandle, 0);
      FreeImage_CloseMemory(dataHandle);
      tex = dib ? BaseTexture::CreateFromFreeImage(dib, maxTexDimension, resizeOnLowMem) : nullptr;
   }
   
   #ifdef __OPENGLES__
   if (tex && (tex->m_format == SRGB || tex->m_format == RGB_FP16 || tex->m_format == RGB_FP32))
   {
      BaseTexture* newTex = tex->NewWithAlpha();
      delete tex;
      tex = newTex;
   }
   #endif
   
   return tex;
}

BaseTexture* BaseTexture::CreateFromFreeImage(FIBITMAP* dib, unsigned int maxTexDim, bool resizeOnLowMem) noexcept
{
   // check if Textures exceed the maximum texture dimension
   if (maxTexDim <= 0)
      maxTexDim = 65536;

   const unsigned int pictureWidth  = FreeImage_GetWidth(dib);
   const unsigned int pictureHeight = FreeImage_GetHeight(dib);

   FIBITMAP* dibResized = dib;
   FIBITMAP* dibConv = dib;
   BaseTexture* tex = nullptr;

   // do loading in a loop, in case memory runs out and we need to scale the texture down due to this
   bool success = false;
   bool needsSignedHalf2Float = false;
   while (!success)
   {
      // the mem is so low that the texture won't even be able to be rescaled -> return
      if (maxTexDim <= 0)
      {
         FreeImage_Unload(dib);
         return nullptr;
      }

      if ((pictureHeight > maxTexDim) || (pictureWidth > maxTexDim))
      {
         unsigned int newWidth  = max(min(pictureWidth,  maxTexDim), MIN_TEXTURE_SIZE);
         unsigned int newHeight = max(min(pictureHeight, maxTexDim), MIN_TEXTURE_SIZE);
         /*
          * The following code tries to maintain the aspect ratio while resizing.
          */
         if (pictureWidth - newWidth > pictureHeight - newHeight)
             newHeight = min(pictureHeight * newWidth / pictureWidth,  maxTexDim);
         else
             newWidth  = min(pictureWidth * newHeight / pictureHeight, maxTexDim);
         dibResized = FreeImage_Rescale(dib, newWidth, newHeight, FILTER_BILINEAR); //!! use a better filter in case scale ratio is pretty high?
      }
      else if (pictureWidth < MIN_TEXTURE_SIZE || pictureHeight < MIN_TEXTURE_SIZE)
      {
         // some drivers seem to choke on small (1x1) textures, so be safe by scaling them up
         const unsigned int newWidth  = max(pictureWidth,  MIN_TEXTURE_SIZE);
         const unsigned int newHeight = max(pictureHeight, MIN_TEXTURE_SIZE);
         dibResized = FreeImage_Rescale(dib, newWidth, newHeight, FILTER_BOX);
      }

      // failed to get mem?
      if (dibResized == nullptr)
      {
         if (!resizeOnLowMem)
         {
            FreeImage_Unload(dib);
            return nullptr;
         }

         maxTexDim /= 2;
         while ((maxTexDim > pictureHeight) && (maxTexDim > pictureWidth))
            maxTexDim /= 2;

         continue;
      }

      const FREE_IMAGE_TYPE img_type = FreeImage_GetImageType(dibResized);
      const bool rgbf16 = (img_type == FIT_RGB16F) || (img_type == FIT_RGBA16F);
      const bool rgbf = (img_type == FIT_FLOAT) || (img_type == FIT_DOUBLE) || (img_type == FIT_RGBF) || (img_type == FIT_RGBAF); //(FreeImage_GetBPP(dibResized) > 32); //!! do handle/convert RGBAF better?
      const bool has_alpha = !rgbf && FreeImage_IsTransparent(dibResized);
      // already in correct format (24bits RGB, 32bits RGBA, 96bits RGBF) ?
      // Note that 8bits BW image are converted to 24bits RGB since they are in sRGB color space and there is no sGREY8 GPU format
      if(((img_type == FIT_BITMAP) && (FreeImage_GetBPP(dibResized) == (has_alpha ? 32 : 24))) || (img_type == FIT_RGBF) || rgbf16)
         dibConv = dibResized;
      else
      {
         dibConv = rgbf ? FreeImage_ConvertToRGBF(dibResized) : has_alpha ? FreeImage_ConvertTo32Bits(dibResized) : FreeImage_ConvertTo24Bits(dibResized);
         if (dibResized != dib) // did we allocate a rescaled copy?
            FreeImage_Unload(dibResized);

         // failed to get mem?
         if (dibConv == nullptr)
         {
            if (!resizeOnLowMem)
            {
               FreeImage_Unload(dib);
               return nullptr;
            }

            maxTexDim /= 2;
            while ((maxTexDim > pictureHeight) && (maxTexDim > pictureWidth))
               maxTexDim /= 2;

            continue;
         }
      }

      Format format;
      const unsigned int tex_w = FreeImage_GetWidth(dibConv);
      const unsigned int tex_h = FreeImage_GetHeight(dibConv);
      if (rgbf16)
      {
         format = (img_type == FIT_RGB16F) ? RGB_FP16 : RGBA_FP16;
         needsSignedHalf2Float = true; // As we did not evaluate file content
      }
      else if (rgbf)
      {
         float minval = FLT_MAX;
         float maxval = -FLT_MAX;
         const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
         const unsigned int pitch = FreeImage_GetPitch(dibConv);
         for (unsigned int y = 0; y < tex_h; ++y)
         {
            const Vertex2D minmax = min_max((const float*)bits, tex_w * 3);
            minval = min(minval, minmax.x);
            maxval = max(maxval, minmax.y);

            bits += pitch;
         }
         format = (maxval <= 65504.f && minval >= -65504.f) ? RGB_FP16 : RGB_FP32;
         needsSignedHalf2Float = minval < 0.f;
      }
      else
      {
         format = has_alpha ? SRGBA : SRGB;
      }

      try
      {
         tex = new BaseTexture(tex_w, tex_h, format);
         success = true;
      }
      // failed to get mem?
      catch(...)
      {
         delete tex;
         tex = nullptr;

         if (dibConv != dibResized) // did we allocate a copy from conversion?
            FreeImage_Unload(dibConv);
         else if (dibResized != dib) // did we allocate a rescaled copy?
            FreeImage_Unload(dibResized);

         if (!resizeOnLowMem)
         {
            FreeImage_Unload(dib);
            return nullptr;
         }

         maxTexDim /= 2;
         while ((maxTexDim > pictureHeight) && (maxTexDim > pictureWidth))
            maxTexDim /= 2;
      }
   }

   tex->m_realWidth = pictureWidth;
   tex->m_realHeight = pictureHeight;

   // Copy, applying channel and data format conversion, as well as flipping upside down
   // Note that free image uses RGB for float image, and the FI_RGBA_xxx for others
   if (tex->m_format == RGB_FP16 || tex->m_format == RGBA_FP16)
   {
      const FREE_IMAGE_TYPE img_type = FreeImage_GetImageType(dibConv);
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      const unsigned components = tex->m_format == RGB_FP16 ? 3 : 4;
      unsigned short* const __restrict pdst = (unsigned short*)tex->data();
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const size_t offs = (size_t)(tex->m_height - y - 1) * (tex->m_width*components);
         if (img_type == FIT_RGB16F || img_type == FIT_RGBA16F)
            memcpy(pdst+offs, bits, tex->m_width*components*sizeof(unsigned short));
         // we already did a range check above, so use faster float2half code variants
         else if (needsSignedHalf2Float)
            float2half_noF16MaxInfNaN(pdst+offs, (const float*)bits, tex->m_width*components);
         else
            float2half_pos_noF16MaxInfNaN(pdst+offs, (const float*)bits, tex->m_width*components);
         bits += pitch;
      }
      tex->SetIsOpaque(true);
   }
   else if (tex->m_format == RGB_FP32)
   {
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      float* const __restrict pdst = (float*)tex->data();
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const size_t offs = (size_t)(tex->m_height - y - 1) * (tex->m_width*3);
         memcpy(pdst+offs, bits, tex->m_width*3*sizeof(float));
         bits += pitch;
      }
      tex->SetIsOpaque(true);
   }
   else if (tex->m_format == BW)
   {
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      BYTE* const __restrict pdst = tex->data();
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const size_t offs = (size_t)(tex->m_height - y - 1) * tex->m_width;
         memcpy(pdst+offs, bits, tex->m_width);
         bits += pitch;
      }
      tex->SetIsOpaque(true);
   }
   else
   {
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      BYTE* const __restrict pdst = tex->data();
      const bool has_alpha = tex->HasAlpha();
      const unsigned int stride = has_alpha ? 4 : 3;
      #if (!((FI_RGBA_RED == 2) && (FI_RGBA_GREEN == 1) && (FI_RGBA_BLUE == 0) && (FI_RGBA_ALPHA == 3)))
         bool opaque = true;
      #endif
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const BYTE* __restrict pixel = (BYTE*)bits;
         const size_t offs = (size_t)(tex->m_height - y - 1) * (tex->width()*stride);
         #if (FI_RGBA_RED == 2) && (FI_RGBA_GREEN == 1) && (FI_RGBA_BLUE == 0) && (FI_RGBA_ALPHA == 3)
            if (has_alpha)
               copy_bgra_rgba<false>((unsigned int*)(pdst+offs),(const unsigned int*)pixel,tex->width());
            else
               copy_bgr_rgb(pdst+offs,pixel,tex->width());
         #else
            if (!has_alpha)
               memcpy(pdst+offs, pixel, tex->width()*3);
            else
            for (size_t o = offs; o < tex->width()*4+offs; o+=4,pixel+=4)
            {
               const unsigned int p = *(unsigned int*)pixel;
               *(unsigned int*)(pdst+o) = p;
               if ((p&0xFF000000u) != 0xFF000000u)
                  opaque = false;
            }
         #endif
         bits += pitch;
      }
      #if (!((FI_RGBA_RED == 2) && (FI_RGBA_GREEN == 1) && (FI_RGBA_BLUE == 0) && (FI_RGBA_ALPHA == 3)))
         tex->SetIsOpaque(opaque);
      #endif
   }

   if (dibConv != dibResized) // did we allocate a copy from conversion?
      FreeImage_Unload(dibConv);
   else if (dibResized != dib) // did we allocate a rescaled copy?
      FreeImage_Unload(dibResized);
   FreeImage_Unload(dib);

   return tex;
}

BaseTexture* BaseTexture::CreateFromHBitmap(const HBITMAP hbmp, unsigned int maxTexDim, bool with_alpha) noexcept
{
   #ifdef __STANDALONE__
      return nullptr;
   #else
      // from the FreeImage FAQ page
      BITMAP bm;
      GetObject(hbmp, sizeof(BITMAP), &bm);
      FIBITMAP* dib = FreeImage_Allocate(bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);
      if (!dib)
         return nullptr;
      // The GetDIBits function clears the biClrUsed and biClrImportant BITMAPINFO members (don't know why)
      // So we save these infos below. This is needed for palettized images only.
      const int nColors = FreeImage_GetColorsUsed(dib);
      const HDC dc = GetDC(nullptr);
      /*const int Success =*/ GetDIBits(dc, hbmp, 0, FreeImage_GetHeight(dib),
         FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
      ReleaseDC(nullptr, dc);
      // restore BITMAPINFO members
      FreeImage_GetInfoHeader(dib)->biClrUsed = nColors;
      FreeImage_GetInfoHeader(dib)->biClrImportant = nColors;

      if (!dib)
         return nullptr;
      if (with_alpha && FreeImage_GetBPP(dib) == 24)
      {
         FIBITMAP* dibConv = FreeImage_ConvertTo32Bits(dib);
         FreeImage_Unload(dib);
         dib = dibConv;
         if (!dib)
            return nullptr;
      }
      return BaseTexture::CreateFromFreeImage(dib, true, maxTexDim);
   #endif
}

void BaseTexture::Update(BaseTexture** texture, const unsigned int width, const unsigned int height, const Format texFormat, const uint8_t* image)
{
   const int pixelSize = GetPixelSize(texFormat);
   if (*texture != nullptr)
   {
      BaseTexture* tex = *texture;
      if ((tex->m_width == width) && (tex->m_height == height) && (tex->m_format == texFormat))
      {
         assert(tex->pitch() * tex->height() == width * height * pixelSize);
         memcpy(tex->data(), image, width * height * pixelSize);
         if (g_pplayer)
            g_pplayer->m_renderer->m_renderDevice->m_texMan.SetDirty(tex);
         return;
      }
      else if (g_pplayer)
      {
         // Delay texture deletion since it may be used by the render frame which is processed asynchronously. If so, deleting would cause a deadlock & invalid access
         g_pplayer->m_renderer->m_renderDevice->AddEndOfFrameCmd([tex] {
               g_pplayer->m_renderer->m_renderDevice->m_texMan.UnloadTexture(tex);
               delete tex;
            });
      }
      else
      {
         delete tex;
      }
   }
   BaseTexture* baseTex = BaseTexture::Create(width, height, texFormat);
   if (baseTex)
      memcpy(baseTex->data(), image, width * height * pixelSize);
   *texture = baseTex;
}

BaseTexture* BaseTexture::NewWithAlpha() const
{
   BaseTexture* tex = nullptr;
   switch (m_format)
   {
   case RGB:
      tex = BaseTexture::Create(m_width, m_height, RGBA);
      if (tex == nullptr)
         return nullptr;
      copy_rgb_rgba<false>((unsigned int*)tex->data(), datac(), (size_t)width() * height());
      break;
   case SRGB:
      tex = BaseTexture::Create(m_width, m_height, SRGBA);
      if (tex == nullptr)
         return nullptr;
      copy_rgb_rgba<false>((unsigned int*)tex->data(), datac(), (size_t)width() * height());
      break;
   case RGB_FP16:
      {
         tex = BaseTexture::Create(m_width, m_height, RGBA_FP16);
         if (tex == nullptr)
            return nullptr;
         unsigned short* const __restrict dest_data16 = (unsigned short*)tex->data();
         const unsigned short* const __restrict src_data16 = (unsigned short*)datac();
         size_t o = 0;
         for (unsigned int j = 0; j < height(); ++j)
            for (unsigned int i = 0; i < width(); ++i, ++o)
            {
               dest_data16[o * 4 + 0] = src_data16[o * 3 + 0];
               dest_data16[o * 4 + 1] = src_data16[o * 3 + 1];
               dest_data16[o * 4 + 2] = src_data16[o * 3 + 2];
               dest_data16[o * 4 + 3] = 0x3C00; //=1.f
            }
      }
      break;
   case RGB_FP32:
      {
         tex = BaseTexture::Create(m_width, m_height, RGBA_FP32);
         if (tex == nullptr)
            return nullptr;
         UINT32* const __restrict dest_data32 = (UINT32*)tex->data();
         const UINT32* const __restrict src_data32 = (UINT32*)datac();
         size_t o = 0;
         for (unsigned int j = 0; j < height(); ++j)
            for (unsigned int i = 0; i < width(); ++i, ++o)
            {
               dest_data32[o * 4 + 0] = src_data32[o * 3 + 0];
               dest_data32[o * 4 + 1] = src_data32[o * 3 + 1];
               dest_data32[o * 4 + 2] = src_data32[o * 3 + 2];
               dest_data32[o * 4 + 3] = 0x3f800000; //=1.f
            }
      }
      break;
   }
   if (tex)
      tex->SetIsOpaque(true);
   return tex;
}

BaseTexture* BaseTexture::ToBGRA() const
{
   BaseTexture* tex = BaseTexture::Create(m_width, m_height, RGBA);
   if (tex == nullptr)
      return nullptr;

   tex->m_realWidth = m_realWidth;
   tex->m_realHeight = m_realHeight;
   if (IsOpaqueComputed())
      tex->SetIsOpaque(IsOpaque());
   BYTE* const __restrict tmp = tex->data();

   if (m_format == BaseTexture::RGB_FP32) // Tonemap for 8bpc-Display
   {
      const float* const __restrict src = (const float*)datac();
      size_t o = 0;
      for (unsigned int j = 0; j < height(); ++j)
         for (unsigned int i = 0; i < width(); ++i, ++o)
         {
            const float r = src[o * 3 + 0];
            const float g = src[o * 3 + 1];
            const float b = src[o * 3 + 2];
            const float l = r * 0.176204f + g * 0.812985f + b * 0.0108109f;
            const float n = (l * (float)(255. * 0.25) + 255.0f) / (l + 1.0f); // simple tonemap and scale by 255, overflow is handled by clamp below
            tmp[o * 4 + 0] = (int)clamp(b * n, 0.f, 255.f);
            tmp[o * 4 + 1] = (int)clamp(g * n, 0.f, 255.f);
            tmp[o * 4 + 2] = (int)clamp(r * n, 0.f, 255.f);
            tmp[o * 4 + 3] = 255;
         }
   }
   else if (m_format == BaseTexture::RGB_FP16) // Tonemap for 8bpc-Display
   {
      const unsigned short* const __restrict src = (const unsigned short*)datac();
      size_t o = 0;
      for (unsigned int j = 0; j < height(); ++j)
         for (unsigned int i = 0; i < width(); ++i, ++o)
         {
            const float r = half2float(src[o * 3 + 0]);
            const float g = half2float(src[o * 3 + 1]);
            const float b = half2float(src[o * 3 + 2]);
            const float l = r * 0.176204f + g * 0.812985f + b * 0.0108109f;
            const float n = (l * (float)(255. * 0.25) + 255.0f) / (l + 1.0f); // simple tonemap and scale by 255, overflow is handled by clamp below
            tmp[o * 4 + 0] = (int)clamp(b * n, 0.f, 255.f);
            tmp[o * 4 + 1] = (int)clamp(g * n, 0.f, 255.f);
            tmp[o * 4 + 2] = (int)clamp(r * n, 0.f, 255.f);
            tmp[o * 4 + 3] = 255;
         }
   }
   else if (m_format == BaseTexture::RGBA_FP16) // Tonemap for 8bpc-Display
   {
      const unsigned short* const __restrict src = (const unsigned short*)datac();
      size_t o = 0;
      for (unsigned int j = 0; j < height(); ++j)
         for (unsigned int i = 0; i < width(); ++i, ++o)
         {
            const float rf = half2float(src[o * 4 + 0]);
            const float gf = half2float(src[o * 4 + 1]);
            const float bf = half2float(src[o * 4 + 2]);
            const int alpha = (int)clamp(half2float(src[o * 4 + 3]) * 255.f, 0.f, 255.f);
            const float l = rf * 0.176204f + gf * 0.812985f + bf * 0.0108109f;
            const float n = (l * (float)(255. * 0.25) + 255.0f) / (l + 1.0f); // simple tonemap and scale by 255, overflow is handled by clamp below
            int r = (int)clamp(bf * n, 0.f, 255.f);
            int g = (int)clamp(gf * n, 0.f, 255.f);
            int b = (int)clamp(rf * n, 0.f, 255.f);
            // use the alpha in the bitmap, thus RGB needs to be premultiplied with alpha, due to how AlphaBlend() works
            if (alpha == 0) // adds a checkerboard where completely transparent (for the image manager display)
            {
               r = g = b = ((((i >> 4) ^ (j >> 4)) & 1) << 7) + 127;
            }
            else if (alpha != 255) // premultiply alpha for win32 AlphaBlend()
            {
               r = r * alpha >> 8;
               g = g * alpha >> 8;
               b = b * alpha >> 8;
            }
            tmp[o * 4 + 0] = r;
            tmp[o * 4 + 1] = g;
            tmp[o * 4 + 2] = b;
            tmp[o * 4 + 3] = alpha;
         }
   }
   else if (m_format == BaseTexture::BW)
   {
      const BYTE* const __restrict src = datac();
      size_t o = 0;
      for (unsigned int j = 0; j < height(); ++j)
         for (unsigned int i = 0; i < width(); ++i, ++o)
         {
            tmp[o * 4 + 0] =
            tmp[o * 4 + 1] =
            tmp[o * 4 + 2] = src[o];
            tmp[o * 4 + 3] = 255; // A
         }
   }
   else if (m_format == BaseTexture::RGB || m_format == BaseTexture::SRGB)
   {
      copy_rgb_rgba<true>((unsigned int*)tmp, datac(), (size_t)width() * height());
   }
   else if (m_format == BaseTexture::RGBA || m_format == BaseTexture::SRGBA)
   {
      const BYTE* const __restrict psrc = datac();
      size_t o = 0;
      for (unsigned int j = 0; j < height(); ++j)
      {
         for (unsigned int i = 0; i < width(); ++i, ++o)
         {
            int r = psrc[o * 4 + 0];
            int g = psrc[o * 4 + 1];
            int b = psrc[o * 4 + 2];
            const int alpha = psrc[o * 4 + 3];
            // use the alpha in the bitmap, thus RGB needs to be premultiplied with alpha, due to how AlphaBlend() works
            if (alpha == 0) // adds a checkerboard where completely transparent (for the image manager display)
            {
               r = g = b = ((((i >> 4) ^ (j >> 4)) & 1) << 7) + 127;
            }
            else if (alpha != 255) // premultiply alpha for win32 AlphaBlend()
            {
               r = r * alpha >> 8;
               g = g * alpha >> 8;
               b = b * alpha >> 8;
            }
            tmp[o * 4 + 0] = b;
            tmp[o * 4 + 1] = g;
            tmp[o * 4 + 2] = r;
            tmp[o * 4 + 3] = alpha;
         }
      }
   }
   else
      assert(!"unknown format");

   return tex;
}

void BaseTexture::UpdateMD5() const
{
   if (!m_isMD5Dirty)
      return;
   m_isMD5Dirty = false;
   generateMD5((uint8_t*)m_data, pitch() * height(), m_md5Hash);
}

void BaseTexture::UpdateOpaque() const
{
   if (!m_isOpaqueDirty)
      return;
   m_isOpaqueDirty = false;
   m_isOpaque = true;
   if (m_format == RGBA || m_format == SRGBA)
   {
      // RGBA_FP16/RGBA_FP32 could be transparent but for the time being, the alpha channel is always opaque, only added for driver's texture format support
      BYTE* const __restrict pdst = m_data;
      constexpr unsigned int stride = 4;
      for (unsigned int y = 0; y < m_height && m_isOpaque; ++y)
      {
         const size_t offs = (size_t)(m_height - y - 1) * (m_width * stride);
         for (size_t o = offs; o < m_width * stride + offs; o += stride)
         {
            if (pdst[o + 3] != 255)
            {
               m_isOpaque = false;
               break;
            }
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::Texture(const string& name, PinBinary* ppb, unsigned int width, unsigned int height)
   : m_name(name)
   , m_ppb(ppb)
   , m_width(width)
   , m_height(height)
{
   assert(m_ppb != nullptr);
   assert(m_width > 0);
   assert(m_height > 0);
}

Texture* Texture::CreateFromStream(IStream *pstream, int version, PinTable *pt, bool resizeOnLowMem, unsigned int maxTexDimension)
{
   string name;
   string path;
   unsigned int width = 0;
   unsigned int height = 0;
   float alphaTestValue = static_cast<float>(-1.0 / 255.0);
   PinBinary* ppb = nullptr;
   bool isMD5Dirty = true;
   uint8_t md5Hash[16] = { 0 };
   bool isOpaqueDirty = true;
   bool isOpaque = true;
   BiffReader br(pstream, nullptr, pt, version, 0, 0);
   br.Load([&](const int id, BiffReader* const pbr)
   {
      switch(id)
      {
      case FID(NAME): pbr->GetString(name); break;
      case FID(PATH): pbr->GetString(path); break;
      case FID(WDTH): pbr->GetInt(width); break;
      case FID(HGHT): pbr->GetInt(height); break;
      case FID(ALTV): pbr->GetFloat(alphaTestValue); alphaTestValue *= (float)(1.0 / 255.0); break;
      case FID(MD5H): pbr->GetStruct(md5Hash, 16); isMD5Dirty = false; break;
      case FID(OPAQ): pbr->GetBool(isOpaque); isOpaqueDirty = false; break;
      case FID(BITS):
      {
         // Old files, used to store some bitmaps as a 32-bit SBGRA picture, we now (10.8.1+) always use a compressed file format, so convert here to simplify the code
         const size_t size = height * width;
         assert(ppb == nullptr && size != 0);

         BYTE* const __restrict tmp = new BYTE[size * 4];
         LZWReader lzwreader(pbr->m_pistream, reinterpret_cast<int*>(tmp), width * 4, height, width * 4);
         lzwreader.Decoder();

         // Find out if all alpha values are 0x00 or 0xFF
         #ifdef __OPENGLES__
            bool has_alpha = true;
         #else
            bool has_alpha = false;
            for (size_t o = 3; o < size * 4; o += 4)
               if (tmp[o] != 0 && tmp[o] != 255)
               {
                  has_alpha = true;
                  break;
               }
         #endif

         // Create a FreeImage from LZW data, converting from BGR to RGB, eventually dropping the alpha channel
         FIBITMAP* dib = FreeImage_Allocate(width, height, has_alpha ? 32 : 24);
         BYTE* const pdst = FreeImage_GetBits(dib);
         const unsigned int ch = has_alpha ? 4 : 3;
         const unsigned int pitch = width * 4;
         const unsigned int pitch_dst = FreeImage_GetPitch(dib);
         const BYTE* spch = tmp + (height * pitch);
         for (unsigned int i = 0; i < height; i++)
         {
            const BYTE* __restrict src = (spch -= pitch); // start on previous previous line
            BYTE* __restrict dst = pdst + i * pitch_dst;
            for (unsigned int x = 0; x < width; x++, src += 4, dst += ch) // copy and swap red & blue
            {
               dst[0] = src[2];
               dst[1] = src[1];
               dst[2] = src[0];
               if (has_alpha)
                  dst[3] = src[3];
            }
         }

         // Convert to a lossless webp
         auto memStream = FreeImage_OpenMemory();
         FreeImage_SaveToMemory(FREE_IMAGE_FORMAT::FIF_WEBP, dib, memStream, WEBP_LOSSLESS);
         ppb = new PinBinary();
         ppb->m_cdata = FreeImage_TellMemory(memStream);
         ppb->m_pdata = new uint8_t[ppb->m_cdata];
         ppb->m_name = name;
         string ext = extension_from_path(path);
         if (!ext.empty())
         {
            path.erase(path.length() - ext.length());
            path += "webp";
         }
         ppb->m_path = path;
         FreeImage_SeekMemory(memStream, 0, SEEK_SET);
         FreeImage_ReadMemory(ppb->m_pdata, 1, ppb->m_cdata, memStream);
         FreeImage_CloseMemory(memStream);
         FreeImage_Unload(dib);
         break;
      }
      case FID(JPEG): // JPEG may be misleading as this chunk contains original binary image data (in whatever format JPEG, PNG, EXR,...)
      {
         assert(ppb == nullptr);
         ppb = new PinBinary();
         if (ppb->LoadFromStream(pbr->m_pistream, pbr->m_version) != S_OK)
         {
            assert(!"Invalid binary image file");
            return false;
         }
         break;
      }
      case FID(LINK):
      {
         int linkid;
         pbr->GetInt(linkid);
         PinTable * const pt = (PinTable *)pbr->m_pdata;
         ppb = pt->GetImageLinkBinary(linkid);
         if (!ppb)
         {
            assert(!"Invalid PinBinary");
            return false;
         }
         break;
      }
      }
      return true;
   });

   if (ppb == nullptr)
      return nullptr;

   Texture* tex = new Texture(name, ppb, width, height);
   tex->m_alphaTestValue = alphaTestValue;
   if (!isOpaqueDirty)
      tex->SetIsOpaque(isOpaque);
   if (!isMD5Dirty)
      tex->SetMD5Hash(md5Hash);

   // For the time being, we always perform decoding after loading
   tex->m_imageBuffer = BaseTexture::CreateFromData(ppb->m_pdata, ppb->m_cdata, maxTexDimension, resizeOnLowMem);

   if (tex->m_imageBuffer)
      return tex;
   
   delete tex;
   return nullptr;
}

Texture* Texture::CreateFromFile(const string& filename)
{
   PinBinary* const ppb = new PinBinary();
   ppb->ReadFromFile(filename);

   BaseTexture* const imageBuffer = BaseTexture::CreateFromData(ppb->m_pdata, ppb->m_cdata, 0, false);
   if (imageBuffer == nullptr)
   {
      delete ppb;
      return nullptr;
   }
   
   int begin, end;
   const int len = (int)filename.length();
   for (begin = len; begin >= 0; begin--)
   {
      if (filename[begin] == PATH_SEPARATOR_CHAR)
      {
         begin++;
         break;
      }
   }
   for (end = len; end >= 0; end--)
      if (filename[end] == '.')
         break;
   if (end == 0)
      end = len - 1;
   const string name = filename.substr(begin, end - begin);

   Texture* tex = new Texture(name, ppb, imageBuffer->m_realWidth, imageBuffer->m_realHeight);
   tex->m_imageBuffer = imageBuffer;
   tex->UpdateMD5();
   tex->UpdateOpaque();
   return tex;
}

Texture::~Texture()
{
   delete m_ppb;
   delete m_imageBuffer;
   #ifndef __STANDALONE__
      if (m_hbmGDIVersion)
      {
         if(m_hbmGDIVersion != g_pvp->m_hbmInPlayMode)
             DeleteObject(m_hbmGDIVersion);
      }
   #endif
}

HRESULT Texture::SaveToStream(IStream *pstream, const PinTable *pt)
{
   BiffWriter bw(pstream, 0);
   bw.WriteString(FID(NAME), m_name);
   bw.WriteString(FID(PATH), m_ppb->m_path);
   bw.WriteInt(FID(WDTH), m_width);
   bw.WriteInt(FID(HGHT), m_height);
   if (pt->GetImageLink(this))
      bw.WriteInt(FID(LINK), 1);
   else
   {
      bw.WriteTag(FID(JPEG));
      m_ppb->SaveToStream(pstream);
   }
   bw.WriteFloat(FID(ALTV), m_alphaTestValue * 255.0f);
   bw.WriteStruct(FID(MD5H), GetMD5Hash(), 16);
   bw.WriteBool(FID(OPAQ), IsOpaque());
   bw.WriteTag(FID(ENDB));
   return S_OK;
}

HBITMAP Texture::GetGDIBitmap() const
{
#ifndef __STANDALONE__
   if (m_hbmGDIVersion)
      return m_hbmGDIVersion;

   if (g_pvp->m_table_played_via_command_line || g_pvp->m_table_played_via_SelectTableOnStart) // only do anything in here (and waste memory/time on it) if UI needed (i.e. if not just -Play via command line is triggered or selected on VPX start with the file popup!)
   {
      m_hbmGDIVersion = g_pvp->m_hbmInPlayMode;
      return m_hbmGDIVersion;
   }

   if (GetRawBitmap() == nullptr)
   {
      m_hbmGDIVersion = g_pvp->m_hbmInPlayMode; // We should return an error bitmap
      return m_hbmGDIVersion;
   }

   const HDC hdcScreen = GetDC(nullptr);
   m_hbmGDIVersion = CreateCompatibleBitmap(hdcScreen, m_width, m_height);
   const HDC hdcNew = CreateCompatibleDC(hdcScreen);
   const HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, m_hbmGDIVersion);

   BITMAPINFO bmi = {};
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = m_width;
   bmi.bmiHeader.biHeight = -(LONG)m_height;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = 0;

   BaseTexture* bgr32bits = m_imageBuffer->ToBGRA();
   SetStretchBltMode(hdcNew, COLORONCOLOR);
   StretchDIBits(hdcNew,
      0, 0, m_width, m_height,
      0, 0, m_width, m_height,
      bgr32bits->data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
   delete bgr32bits;

   SelectObject(hdcNew, hbmOld);
   DeleteDC(hdcNew);
   ReleaseDC(nullptr, hdcScreen);
   return m_hbmGDIVersion;
#else
   return nullptr;
#endif
}

void Texture::UpdateMD5() const
{
   if (!m_isMD5Dirty)
      return;
   m_isMD5Dirty = false;
   generateMD5(reinterpret_cast<uint8_t*>(m_ppb->m_pdata), m_ppb->m_cdata, m_md5Hash);
   if (m_imageBuffer)
      m_imageBuffer->SetMD5Hash(m_md5Hash);
}

void Texture::UpdateOpaque() const
{
   if (!m_isOpaqueDirty)
      return;
   m_isOpaqueDirty = false;
   m_isOpaque = GetRawBitmap()->IsOpaque();
}
