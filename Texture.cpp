#include "stdafx.h"
#include "Texture.h"

#include "freeimage.h"

#include "math/math.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG // only use the SSE2-JPG path from stbi, as all others are not faster than FreeImage //!! can remove stbi again if at some point FreeImage incorporates libjpeg-turbo or something similar
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#ifdef __STANDALONE__
#include <fstream>
#include <iostream>
#endif

BaseTexture::BaseTexture(const unsigned int w, const unsigned int h, const Format format)
   : m_width(w)
   , m_height(h)
   , m_data((format == RGBA || format == SRGBA ? 4 : (format == BW ? 1 : 3)) * (format == RGB_FP32 ? 4 : format == RGB_FP16 ? 2 : 1) * w * h)
   , m_realWidth(w)
   , m_realHeight(h)
   , m_format(format)
{
}

BaseTexture* BaseTexture::CreateFromFreeImage(FIBITMAP* dib, bool resize_on_low_mem)
{
   // check if Textures exceed the maximum texture dimension
   int maxTexDim = LoadValueIntWithDefault(regKey[RegName::Player], "MaxTexDimension"s, 0); // default: Don't resize textures
   if (maxTexDim <= 0)
      maxTexDim = 65536;

   const unsigned int pictureWidth  = FreeImage_GetWidth(dib);
   const unsigned int pictureHeight = FreeImage_GetHeight(dib);

   FIBITMAP* dibResized = dib;
   FIBITMAP* dibConv = dib;
   BaseTexture* tex = nullptr;

   // do loading in a loop, in case memory runs out and we need to scale the texture down due to this
   bool success = false;
   while(!success)
   {
      // the mem is so low that the texture won't even be able to be rescaled -> return
      if (maxTexDim <= 0)
      {
         FreeImage_Unload(dib);
         return nullptr;
      }

      if ((pictureHeight > (unsigned int)maxTexDim) || (pictureWidth > (unsigned int)maxTexDim))
      {
         unsigned int newWidth  = max(min(pictureWidth,  (unsigned int)maxTexDim), MIN_TEXTURE_SIZE);
         unsigned int newHeight = max(min(pictureHeight, (unsigned int)maxTexDim), MIN_TEXTURE_SIZE);
         /*
          * The following code tries to maintain the aspect ratio while resizing.
          */
         if (pictureWidth - newWidth > pictureHeight - newHeight)
             newHeight = min(pictureHeight * newWidth / pictureWidth,  (unsigned int)maxTexDim);
         else
             newWidth  = min(pictureWidth * newHeight / pictureHeight, (unsigned int)maxTexDim);
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
      if (!dibResized)
      {
         if (!resize_on_low_mem)
         {
            FreeImage_Unload(dib);
            return nullptr;
         }

         maxTexDim /= 2;
         while (((unsigned int)maxTexDim > pictureHeight) && ((unsigned int)maxTexDim > pictureWidth))
             maxTexDim /= 2;

         continue;
      }

      const FREE_IMAGE_TYPE img_type = FreeImage_GetImageType(dibResized);
      const bool rgbf = (img_type == FIT_FLOAT) || (img_type == FIT_DOUBLE) || (img_type == FIT_RGBF) || (img_type == FIT_RGBAF); //(FreeImage_GetBPP(dibResized) > 32);
      const bool has_alpha = !rgbf && FreeImage_IsTransparent(dibResized);
      // already in correct format (24bits RGB, 32bits RGBA, 96bits RGBF) ?
      // Note that 8bits BW image are converted to 24bits RGB since they are in sRGB color space and there is no sGREY8 GPU format
      if(((img_type == FIT_BITMAP) && (FreeImage_GetBPP(dibResized) == (has_alpha ? 32 : 24))) || (img_type == FIT_RGBF))
         dibConv = dibResized;
      else
      {
         dibConv = rgbf ? FreeImage_ConvertToRGBF(dibResized) : has_alpha ? FreeImage_ConvertTo32Bits(dibResized) : FreeImage_ConvertTo24Bits(dibResized);
         if (dibResized != dib) // did we allocate a rescaled copy?
            FreeImage_Unload(dibResized);

         // failed to get mem?
         if (!dibConv)
         {
            if (!resize_on_low_mem)
            {
               FreeImage_Unload(dib);
               return nullptr;
            }

            maxTexDim /= 2;
            while (((unsigned int)maxTexDim > pictureHeight) && ((unsigned int)maxTexDim > pictureWidth))
               maxTexDim /= 2;

            continue;
         }
      }

      Format format;
      const unsigned int tex_w = FreeImage_GetWidth(dibConv);
      const unsigned int tex_h = FreeImage_GetHeight(dibConv);
      if (rgbf)
      {
         float maxval = 0.f;
         const BYTE* __restrict bits = (BYTE*)FreeImage_GetBits(dibConv);
         const int pitch = FreeImage_GetPitch(dibConv);
         for (unsigned int y = 0; y < tex_h; ++y)
         {
            const float* const __restrict pixel = (float*)bits;
            for (unsigned int x = 0; x < tex_w * 3; ++x)
            {
               maxval = max(maxval, pixel[x]);
            }
            bits += pitch;
         }
         format = (maxval <= 65504.f) ? RGB_FP16 : RGB_FP32;
      }
      else
          format = has_alpha ? SRGBA : SRGB;

      try
      {
         tex = new BaseTexture(tex_w, tex_h, format);
         success = true;
      }
      // failed to get mem?
      catch(...)
      {
         delete tex;

         if (dibConv != dibResized) // did we allocate a copy from conversion?
            FreeImage_Unload(dibConv);
         else if (dibResized != dib) // did we allocate a rescaled copy?
            FreeImage_Unload(dibResized);

         if (!resize_on_low_mem)
         {
            FreeImage_Unload(dib);
            return nullptr;
         }

         maxTexDim /= 2;
         while (((unsigned int)maxTexDim > pictureHeight) && ((unsigned int)maxTexDim > pictureWidth))
            maxTexDim /= 2;
      }
   }

   tex->m_realWidth = pictureWidth;
   tex->m_realHeight = pictureHeight;

   // Copy, applying channel and data format conversion as well as flipping upside down
   // Note that free image use RGB for float image, and the FI_RGBA_xxx for others
   if (tex->m_format == RGB_FP16)
   {
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      unsigned short* const __restrict pdst = (unsigned short*)tex->data();
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const float* __restrict pixel = (float*)bits;
         const size_t offs = (size_t)(tex->m_height - y - 1) * (tex->m_width*3);
         for (size_t o = offs; o < tex->m_width*3+offs; o+=3)
         {
            pdst[o + 0] = float2half(pixel[0]);
            pdst[o + 1] = float2half(pixel[1]);
            pdst[o + 2] = float2half(pixel[2]);
            pixel += 3;
         }
         bits += pitch;
      }
   }
   else if (tex->m_format == RGB_FP32)
   {
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      float* const __restrict pdst = (float*)tex->data();
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const float* __restrict pixel = (float*)bits;
         const size_t offs = (size_t)(tex->m_height - y - 1) * (tex->m_width*3);
         for (size_t o = offs; o < tex->m_width*3+offs; o+=3)
         {
            pdst[o + 0] = pixel[0];
            pdst[o + 1] = pixel[1];
            pdst[o + 2] = pixel[2];
            pixel += 3;
         }
         bits += pitch;
      }
   }
   else if (tex->m_format == BW)
   {
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      BYTE* const __restrict pdst = tex->data();
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const BYTE* __restrict pixel = (BYTE*)bits;
         size_t offs = (size_t)(tex->m_height - y - 1) * tex->m_width;
         memcpy(&(pdst[offs]), pixel, tex->m_width);
         bits += pitch;
      }
   }
   else
   {
      const BYTE* __restrict bits = FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      BYTE* const __restrict pdst = tex->data();
      const bool has_alpha = (tex->m_format == RGBA) || (tex->m_format == SRGBA);
      const unsigned int stride = has_alpha ? 4 : 3;
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const BYTE* __restrict pixel = (BYTE*)bits;
         const size_t offs = (size_t)(tex->m_height - y - 1) * (tex->m_width*stride);
#if (FI_RGBA_RED == 2) && (FI_RGBA_GREEN == 1) && (FI_RGBA_BLUE == 0) && (FI_RGBA_ALPHA == 3)
         if (has_alpha)
            copy_bgra_rgba<false>((unsigned int*)(pdst+offs),(const unsigned int*)pixel,tex->m_width);
         else
#endif
         for (size_t o = offs; o < tex->m_width*stride+offs; o+=stride)
         {
            pdst[o + 0] = pixel[FI_RGBA_RED];
            pdst[o + 1] = pixel[FI_RGBA_GREEN];
            pdst[o + 2] = pixel[FI_RGBA_BLUE];
            if(has_alpha) pdst[o + 3] = pixel[FI_RGBA_ALPHA];
            pixel += stride;
         }
         bits += pitch;
      }
   }

   if (dibConv != dibResized) // did we allocate a copy from conversion?
      FreeImage_Unload(dibConv);
   else if (dibResized != dib) // did we allocate a rescaled copy?
      FreeImage_Unload(dibResized);
   FreeImage_Unload(dib);

   return tex;
}

BaseTexture* BaseTexture::CreateFromFile(const string& szfile)
{
   if (szfile.empty())
      return nullptr;

   // check the file signature and deduce its format
   FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(szfile.c_str(), 0);
   if (fif == FIF_UNKNOWN) {
      // try to guess the file format from the file extension
      fif = FreeImage_GetFIFFromFilename(szfile.c_str());
   }

   // check that the plugin has reading capabilities ...
   if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
      // ok, let's load the file
      FIBITMAP * const dib = FreeImage_Load(fif, szfile.c_str(), 0);
      if (!dib)
         return nullptr;
      
      BaseTexture* const mySurface = CreateFromFreeImage(dib, true);

      //if (bitsPerPixel == 24)
      //   mySurface->SetOpaque();

      return mySurface;
   }
   else
      return nullptr;
}

BaseTexture* BaseTexture::CreateFromData(const void *data, const size_t size)
{
   // check the file signature and deduce its format
   FIMEMORY * const dataHandle = FreeImage_OpenMemory((BYTE*)data, (DWORD)size);
   if (!dataHandle)
      return nullptr;
   const FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(dataHandle, (int)size);

   // check that the plugin has reading capabilities ...
   if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
      // ok, let's load the file
      FIBITMAP * const dib = FreeImage_LoadFromMemory(fif, dataHandle, 0);
      FreeImage_CloseMemory(dataHandle);
      if (!dib)
         return nullptr;
      return BaseTexture::CreateFromFreeImage(dib, true);
   }
   else
   {
      FreeImage_CloseMemory(dataHandle);
      return nullptr;
   }
}

// from the FreeImage FAQ page
static FIBITMAP* HBitmapToFreeImage(HBITMAP hbmp)
{
#ifndef __STANDALONE__
   BITMAP bm;
   GetObject(hbmp, sizeof(BITMAP), &bm);
   FIBITMAP* dib = FreeImage_Allocate(bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);
   if (!dib)
      return nullptr;
   // The GetDIBits function clears the biClrUsed and biClrImportant BITMAPINFO members (dont't know why)
   // So we save these infos below. This is needed for palettized images only.
   const int nColors = FreeImage_GetColorsUsed(dib);
   const HDC dc = GetDC(nullptr);
   /*const int Success =*/ GetDIBits(dc, hbmp, 0, FreeImage_GetHeight(dib),
      FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
   ReleaseDC(nullptr, dc);
   // restore BITMAPINFO members
   FreeImage_GetInfoHeader(dib)->biClrUsed = nColors;
   FreeImage_GetInfoHeader(dib)->biClrImportant = nColors;
   return dib;
#else
   return nullptr;
#endif
}

BaseTexture* BaseTexture::CreateFromHBitmap(const HBITMAP hbm, bool with_alpha)
{
   FIBITMAP* dib = HBitmapToFreeImage(hbm);
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
   BaseTexture* const pdds = BaseTexture::CreateFromFreeImage(dib, true);
   return pdds;
}

BaseTexture* BaseTexture::ToBGRA()
{
   BaseTexture* tex = new BaseTexture(m_width, m_height, RGBA);
   tex->m_realWidth = m_realWidth;
   tex->m_realHeight = m_realHeight;
   BYTE* const __restrict tmp = tex->data();

   if (m_format == BaseTexture::RGB_FP32) // Tonemap for 8bpc-Display
   {
      const float* const __restrict src = (float*)data();
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
      const unsigned short* const __restrict src = (unsigned short*)data();
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
   else if (m_format == BaseTexture::BW)
   {
      const BYTE* const __restrict src = data();
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
      const BYTE* const __restrict src = data();
      size_t o = 0;
      for (unsigned int j = 0; j < height(); ++j)
         for (unsigned int i = 0; i < width(); ++i, ++o)
         {
            tmp[o * 4 + 0] = src[o * 3 + 2]; // B
            tmp[o * 4 + 1] = src[o * 3 + 1]; // G
            tmp[o * 4 + 2] = src[o * 3 + 0]; // R
            tmp[o * 4 + 3] = 255; // A
         }
   }
   else if (m_format == BaseTexture::RGBA || m_format == BaseTexture::SRGBA)
   {
      const BYTE* const __restrict psrc = data();
      size_t o = 0;
      const bool isWinXP = GetWinVersion() < 2600;
      for (unsigned int j = 0; j < height(); ++j)
      {
         for (unsigned int i = 0; i < width(); ++i, ++o)
         {
            int r = psrc[o * 4 + 0];
            int g = psrc[o * 4 + 1];
            int b = psrc[o * 4 + 2];
            const int alpha = psrc[o * 4 + 3];
            if (!isWinXP) // For everything newer than Windows XP: use the alpha in the bitmap, thus RGB needs to be premultiplied with alpha, due to how AlphaBlend() works
            {
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
            }
            else
            {
               if (alpha != 255)
               {
                  const unsigned int c = (((((i >> 4) ^ (j >> 4)) & 1) << 7) + 127) * (255 - alpha);
                  r = (r * alpha + c) >> 8;
                  g = (g * alpha + c) >> 8;
                  b = (b * alpha + c) >> 8;
               }
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

////////////////////////////////////////////////////////////////////////////////


Texture::Texture()
{
   m_pdsBuffer = nullptr;
   m_hbmGDIVersion = nullptr;
   m_ppb = nullptr;
   m_alphaTestValue = 1.0f;
   m_resize_on_low_mem = true;
}

Texture::Texture(BaseTexture * const base)
{
   m_pdsBuffer = base;
   SetSizeFrom(base);

   m_hbmGDIVersion = nullptr;
   m_ppb = nullptr;
   m_alphaTestValue = 1.0f;
   m_resize_on_low_mem = true;
}

Texture::~Texture()
{
   FreeStuff();
}

HRESULT Texture::SaveToStream(IStream *pstream, const PinTable *pt)
{
   BiffWriter bw(pstream, 0);

   bw.WriteString(FID(NAME), m_szName);
   bw.WriteString(FID(PATH), m_szPath);
   bw.WriteInt(FID(WDTH), m_width);
   bw.WriteInt(FID(HGHT), m_height);

   if (!m_ppb)
   {
      bw.WriteTag(FID(BITS));

      // 32-bit picture BGRA
      BaseTexture* bgra = m_pdsBuffer->ToBGRA();
      LZWWriter lzwwriter(pstream, (int*)bgra->data(), m_width * 4, m_height, bgra->pitch());
      lzwwriter.CompressBits(8 + 1);
      delete bgra;
   }
   else // JPEG (or other binary format)
   {
      if (!pt->GetImageLink(this))
      {
         bw.WriteTag(FID(JPEG));
         m_ppb->SaveToStream(pstream);
      }
      else
         bw.WriteInt(FID(LINK), 1);
   }
   bw.WriteFloat(FID(ALTV), m_alphaTestValue);
   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Texture::LoadFromStream(IStream* pstream, int version, PinTable* pt, bool resize_on_low_mem)
{
   BiffReader br(pstream, this, pt, version, 0, 0);
   bool tmp = m_resize_on_low_mem;
   m_resize_on_low_mem = resize_on_low_mem;
   br.Load();
   m_resize_on_low_mem = tmp;
   return ((m_pdsBuffer != nullptr) ? S_OK : E_FAIL);
}

bool Texture::LoadFromMemory(BYTE * const data, const DWORD size)
{
   if (m_pdsBuffer)
      FreeStuff();

   const int maxTexDim = LoadValueIntWithDefault(regKey[RegName::Player], "MaxTexDimension"s, 0); // default: Don't resize textures
   if(maxTexDim <= 0) // only use fast JPG path via stbi if no texture resize must be triggered
   {
      int ok, x, y, channels_in_file;
      ok = stbi_info_from_memory(data, size, &x, &y, &channels_in_file); // Request stbi to convert image to SRGB or SRGBA
      unsigned char * const __restrict stbi_data = stbi_load_from_memory(data, size, &x, &y, &channels_in_file, (ok && channels_in_file <= 3) ? 3 : 4);
      if (stbi_data) // will only enter this path for JPG files
      {
         BaseTexture* tex = nullptr;
         try
         {
            tex = new BaseTexture(x, y, channels_in_file == 4 ? BaseTexture::SRGBA : BaseTexture::SRGB);
         }
         // failed to get mem?
         catch(...)
         {
            delete tex;

            goto freeimage_fallback;
         }

         BYTE* const __restrict pdst = (BYTE*)tex->data();
         const BYTE* const __restrict psrc = (BYTE*)stbi_data;
         memcpy(pdst, psrc, x * y * channels_in_file);
         stbi_image_free(stbi_data);

         tex->m_realWidth = x;
         tex->m_realHeight = y;

         m_pdsBuffer = tex;
         SetSizeFrom(m_pdsBuffer);

         return true;
      }
   }

freeimage_fallback:

   FIMEMORY * const hmem = FreeImage_OpenMemory(data, size);
   if (!hmem)
      return false;
   const FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
   FIBITMAP * const dib = FreeImage_LoadFromMemory(fif, hmem, 0);
   FreeImage_CloseMemory(hmem);
   if (!dib)
      return false;

   m_pdsBuffer = BaseTexture::CreateFromFreeImage(dib, m_resize_on_low_mem);
   if (!m_pdsBuffer)
      return false;

   SetSizeFrom(m_pdsBuffer);

   return true;
}

bool Texture::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(NAME): pbr->GetString(m_szName); break;
   case FID(PATH): pbr->GetString(m_szPath); break;
   case FID(WDTH): pbr->GetInt(m_width); break;
   case FID(HGHT): pbr->GetInt(m_height); break;
   case FID(ALTV): pbr->GetFloat(m_alphaTestValue); break;
   case FID(BITS):
   {
      if (m_pdsBuffer)
         FreeStuff();

      // BMP stored as a 32-bit SBGRA picture
      BYTE* const __restrict tmp = new BYTE[m_width * m_height * 4];
      LZWReader lzwreader(pbr->m_pistream, (int *)tmp, m_width * 4, m_height, m_width * 4);
      lzwreader.Decoder();

      // Find out if all alpha values are 0x00 or 0xFF
      bool has_alpha = false;
      for (unsigned int i = 0; i < m_height && !has_alpha; i++)
      {
         unsigned int o = i * m_width * 4 + 3;
         for (unsigned int l = 0; l < m_width; l++,o+=4)
         {
            if (tmp[o] != 0 && tmp[o] != 255)
            {
               has_alpha = true;
               break;
            }
         }
      }

      m_pdsBuffer = new BaseTexture(m_width, m_height, has_alpha ? BaseTexture::SRGBA : BaseTexture::SRGB);

      // copy, converting from SBGR to SRGB, and eventually dropping the alpha channel
      BYTE* const __restrict pdst = m_pdsBuffer->data();
      unsigned int o1 = 0, o2 = 0;
      const unsigned int o2_step = has_alpha ? 4 : 3;
      for (unsigned int yo = 0; yo < m_height; yo++)
         for (unsigned int xo = 0; xo < m_width; xo++, o1+=4, o2+=o2_step)
         {
            pdst[o2    ] = tmp[o1 + 2];
            pdst[o2 + 1] = tmp[o1 + 1];
            pdst[o2 + 2] = tmp[o1    ];
            if (has_alpha) pdst[o2 + 3] = tmp[o1 + 3];
         }

      delete[] tmp;
      SetSizeFrom(m_pdsBuffer);

      break;
   }
   case FID(JPEG):
   {
      m_ppb = new PinBinary();
      m_ppb->LoadFromStream(pbr->m_pistream, pbr->m_version);
      // m_ppb->m_szPath has the original filename
      // m_ppb->m_pdata() is the buffer
      // m_ppb->m_cdata() is the filesize
      return LoadFromMemory((BYTE*)m_ppb->m_pdata, m_ppb->m_cdata);
      //break;
   }
   case FID(LINK):
   {
      int linkid;
      pbr->GetInt(linkid);
      PinTable * const pt = (PinTable *)pbr->m_pdata;
      m_ppb = pt->GetImageLinkBinary(linkid);
      if (!m_ppb)
      {
         assert(!"Invalid PinBinary");
         return false;
      }
      return LoadFromMemory((BYTE*)m_ppb->m_pdata, m_ppb->m_cdata);
      //break;
   }
   }
   return true;
}

void Texture::FreeStuff()
{
   delete m_pdsBuffer;
   m_pdsBuffer = nullptr;
#ifndef __STANDALONE__
   if (m_hbmGDIVersion)
   {
      if(m_hbmGDIVersion != g_pvp->m_hbmInPlayMode)
          DeleteObject(m_hbmGDIVersion);
      m_hbmGDIVersion = nullptr;
   }
#endif
   if (m_ppb)
   {
      delete m_ppb;
      m_ppb = nullptr;
   }
}

void Texture::CreateGDIVersion()
{
#ifndef __STANDALONE__
   if (m_hbmGDIVersion)
      return;

   if (g_pvp->m_table_played_via_command_line || g_pvp->m_table_played_via_SelectTableOnStart) // only do anything in here (and waste memory/time on it) if UI needed (i.e. if not just -Play via command line is triggered or selected on VPX start with the file popup!)
   {
      m_hbmGDIVersion = g_pvp->m_hbmInPlayMode;
      return;
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

   BaseTexture* bgr32bits = m_pdsBuffer->ToBGRA();
   SetStretchBltMode(hdcNew, COLORONCOLOR);
   StretchDIBits(hdcNew,
      0, 0, m_width, m_height,
      0, 0, m_width, m_height,
      bgr32bits->data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
   delete bgr32bits;

   SelectObject(hdcNew, hbmOld);
   DeleteDC(hdcNew);
   ReleaseDC(nullptr, hdcScreen);
#endif
}

void Texture::GetTextureDC(HDC *pdc)
{
#ifndef __STANDALONE__
   CreateGDIVersion();
   *pdc = CreateCompatibleDC(nullptr);
   m_oldHBM = (HBITMAP)SelectObject(*pdc, m_hbmGDIVersion);
#endif
}

void Texture::ReleaseTextureDC(HDC dc)
{
#ifndef __STANDALONE__
   SelectObject(dc, m_oldHBM);
   DeleteDC(dc);
#endif
}

void Texture::CreateFromResource(const int id)
{
#ifndef __STANDALONE__
   const HBITMAP hbm = (HBITMAP)LoadImage(g_pvp->theInstance, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

   if (m_pdsBuffer)
      FreeStuff();

   if (hbm == nullptr)
      return;

   m_pdsBuffer = CreateFromHBitmap(hbm);
#endif
}

#ifdef __STANDALONE__
void Texture::CreateFromResource(const string szName)
{
   if (m_pdsBuffer)
      FreeStuff();

   std::ifstream stream(szName);
   if (stream.fail()) 
      return;

   stream.seekg(0, std::ios::end);
   size_t size = stream.tellg();
   stream.seekg(0);
   BYTE* data = (BYTE*)malloc(size);
   stream.read((char*)data, size);
   stream.close();

   FIMEMORY* const hmem = FreeImage_OpenMemory(data, size);
   if (!hmem) {
      free(data);
      return;
}
   const FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
   FIBITMAP* const dib = FreeImage_LoadFromMemory(fif, hmem, 0);

   FreeImage_CloseMemory(hmem);
   free(data);

   if (!dib)
      return;

   m_pdsBuffer = BaseTexture::CreateFromFreeImage(dib, true);
   SetSizeFrom(m_pdsBuffer);
}
#endif

BaseTexture* Texture::CreateFromHBitmap(const HBITMAP hbm, bool with_alpha)
{
   BaseTexture* const pdds = BaseTexture::CreateFromHBitmap(hbm, with_alpha);
   SetSizeFrom(pdds);

   return pdds;
}
