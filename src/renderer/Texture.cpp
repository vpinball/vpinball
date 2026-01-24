// license:GPLv3+

#include "core/stdafx.h"
#include "Texture.h"

#ifndef __STANDALONE__
#include "FreeImage.h"
#else
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_surface.h>
#include "standalone/FreeImage.h"
#endif

#include "math/math.h"

#include "utils/lzwreader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG // only use the SSE2-JPG path from stbi, as all others are not faster than FreeImage //!! can remove stbi again if at some point FreeImage incorporates libjpeg-turbo or something similar
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#ifdef __STANDALONE__
#include <fstream>
#include <iostream>
#endif

#define QOI_API static
#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include "qoi/qoi.h"
static inline int GetPixelSize(const BaseTexture::Format format)
{
   switch (format)
   {
   case BaseTexture::BW: return 1;
   case BaseTexture::BW_FP32: return 4;
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
   , m_liveHash(((uint64_t)this) ^ usec() ^ ((uint64_t)w << 16) ^ ((uint64_t)h << 32) ^ format)
   , m_data(reinterpret_cast<uint8_t*>(SDL_aligned_alloc(16, w * h * GetPixelSize(format))))
{
}

BaseTexture::~BaseTexture()
{
   SDL_aligned_free(m_data);
}

unsigned int BaseTexture::pitch() const
{
   return m_width * GetPixelSize(m_format);
}

std::shared_ptr<BaseTexture> BaseTexture::Create(const unsigned int w, const unsigned int h, const Format format) noexcept
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
   auto result = std::shared_ptr<BaseTexture>(tex);
   tex->m_selfPointer = result;
   return result;
}

std::shared_ptr<BaseTexture> BaseTexture::CreateFromFile(const string& filename, unsigned int maxTexDimension, bool resizeOnLowMem) noexcept
{
   if (filename.empty())
      return nullptr;
   PinBinary ppb;
   ppb.ReadFromFile(filename);
   return CreateFromData(ppb.m_buffer.data(), ppb.m_buffer.size(), true, maxTexDimension, resizeOnLowMem);
}

std::shared_ptr<BaseTexture> BaseTexture::CreateFromData(const void* data, const size_t size, const bool isImageData, unsigned int maxTexDimension, bool resizeOnLowMem) noexcept
{
   std::shared_ptr<BaseTexture> tex;

   if (data == nullptr || size == 0)
      return nullptr;
   
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
         Format format = channels_in_file == 4 ? BaseTexture::SRGBA : ((channels_in_file == 1) ? BaseTexture::BW : BaseTexture::SRGB);
         if (!isImageData)
         {
            switch (format)
            {
            case BaseTexture::SRGB: format = BaseTexture::RGB; break;
            case BaseTexture::SRGBA: format = BaseTexture::RGBA; break;
            default: break;
            }
         }
         tex = BaseTexture::Create(x, y, format);
         if (tex)
         {
            uint8_t* const __restrict pdst = static_cast<uint8_t*>(tex->data());
            const uint8_t* const __restrict psrc = (uint8_t*)stbi_data;
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
      tex = dib ? BaseTexture::CreateFromFreeImage(dib, isImageData, maxTexDimension, resizeOnLowMem) : nullptr;
   }
   
   #ifdef __OPENGLES__
   if (tex && (tex->m_format == SRGB || tex->m_format == RGB_FP16 || tex->m_format == RGB_FP32))
      tex = tex->NewWithAlpha();
   #endif
   
   return tex;
}

std::shared_ptr<BaseTexture> BaseTexture::CreateFromFreeImage(FIBITMAP* dib, const bool isImageData, unsigned int maxTexDim, bool resizeOnLowMem) noexcept
{
   // check if Textures exceed the maximum texture dimension
   if (maxTexDim <= 0)
      maxTexDim = 65536;

   const unsigned int pictureWidth  = FreeImage_GetWidth(dib);
   const unsigned int pictureHeight = FreeImage_GetHeight(dib);

   FIBITMAP* dibResized = dib;
   FIBITMAP* dibConv = dib;
   std::shared_ptr<BaseTexture> tex;

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
         const uint8_t* __restrict bits = (uint8_t*)FreeImage_GetBits(dibConv);
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
         format = isImageData ? (has_alpha ? SRGBA : SRGB) : (has_alpha ? RGBA : RGB);
      }

      tex = BaseTexture::Create(tex_w, tex_h, format);
      success = tex != nullptr;
      if (!success)
      { // failed to get mem?
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

   assert(tex->data());

   tex->m_realWidth = pictureWidth;
   tex->m_realHeight = pictureHeight;

   // Copy, applying channel and data format conversion, as well as flipping upside down
   // Note that free image uses RGB for float image, and the FI_RGBA_xxx for others
   if (tex->m_format == RGB_FP16 || tex->m_format == RGBA_FP16)
   {
      const FREE_IMAGE_TYPE img_type = FreeImage_GetImageType(dibConv);
      const uint8_t* __restrict bits = (uint8_t*)FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      const unsigned components = tex->m_format == RGB_FP16 ? 3 : 4;
      uint16_t* const __restrict pdst = (uint16_t*)tex->data();
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const size_t offs = (size_t)(tex->m_height - y - 1) * (tex->m_width*components);
         if (img_type == FIT_RGB16F || img_type == FIT_RGBA16F)
            memcpy(pdst+offs, bits, tex->m_width*components*sizeof(uint16_t));
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
      const uint8_t* __restrict bits = (uint8_t*)FreeImage_GetBits(dibConv);
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
      const uint8_t* __restrict bits = (uint8_t*)FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      uint8_t* const __restrict pdst = static_cast<uint8_t*>(tex->data());
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
      const uint8_t* __restrict bits = (uint8_t*)FreeImage_GetBits(dibConv);
      const unsigned pitch = FreeImage_GetPitch(dibConv);
      uint8_t* const __restrict pdst = static_cast<uint8_t*>(tex->data());
      const bool has_alpha = tex->HasAlpha();
      const unsigned int stride = has_alpha ? 4 : 3;
      #if (!((FI_RGBA_RED == 2) && (FI_RGBA_GREEN == 1) && (FI_RGBA_BLUE == 0) && (FI_RGBA_ALPHA == 3)))
         bool opaque = true;
      #endif
      for (unsigned int y = 0; y < tex->m_height; ++y)
      {
         const uint8_t* __restrict pixel = bits;
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

std::shared_ptr<BaseTexture> BaseTexture::CreateFromHBitmap(const HBITMAP hbmp, unsigned int maxTexDim, bool with_alpha) noexcept
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
      return BaseTexture::CreateFromFreeImage(dib, true, maxTexDim, true);
   #endif
}

void BaseTexture::Update(std::shared_ptr<BaseTexture>& tex, const unsigned int width, const unsigned int height, const Format texFormat, const void* image)
{
   const int pixelSize = GetPixelSize(texFormat);
   string name;
   if (tex != nullptr)
   {
      name = tex->GetName();
      if ((tex->m_width == width) && (tex->m_height == height) && (tex->m_format == texFormat))
      {
         assert(tex->pitch() * tex->height() == width * height * pixelSize);
         if (tex->data() != image && image)
            memcpy(tex->data(), image, width * height * pixelSize);
         tex->m_aliases.clear();
         if (g_pplayer)
            g_pplayer->m_renderer->m_renderDevice->m_texMan.SetDirty(tex.get());
         return;
      }
      if (g_pplayer)
         g_pplayer->m_renderer->m_renderDevice->m_texMan.UnloadTexture(tex.get());
   }
   tex = BaseTexture::Create(width, height, texFormat);
   if (tex)
   {
      tex->SetName(name);
      if (image)
         memcpy(tex->data(), image, (size_t)width * height * pixelSize);
   }
}

void BaseTexture::FlipY()
{
   const int pitch = this->pitch();
   vector<uint8_t> buf(pitch);
   uint8_t* __restrict bits = buf.data();
   for (unsigned int i = 0; i < m_height / 2; i++)
   {
      memcpy(bits, m_data + i * pitch, pitch);
      memcpy(m_data + i * pitch, m_data + (m_height - 1 - i) * pitch, pitch);
      memcpy(m_data + (m_height - 1 - i) * pitch, bits, pitch);
   }
   m_aliases.clear();
}

bool BaseTexture::Save(const string& filepath) const
{
   if ((m_format != SRGBA) && (m_format != SRGB))
      return false;

   const string ext = extension_from_path(filepath);
   bool success = false;

   // Create parent directory if needed
   std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());

   if (ext == "bmp")
   {
      if (SDL_Surface* pSurface = ToSDLSurface(); pSurface)
      {
         success = SDL_SaveBMP(pSurface, filepath.c_str());
         SDL_DestroySurface(pSurface);
      }
   }
   else if (ext == "qoi")
   {
      qoi_desc desc { .width = m_width, .height = m_height, .channels = static_cast<unsigned char>(m_format == SRGB ? 3 :4), .colorspace = QOI_SRGB };
      int size;
      void* encoded = qoi_encode(m_data, &desc, &size);
      if (encoded)
      {
         try
         {
            if (std::ofstream file(filepath, std::ios::binary | std::ios::trunc); file)
            {
                  file.write(reinterpret_cast<const char*>(encoded), size);
                  file.close();
                  success = true;
            }
         }
         catch (const std::filesystem::filesystem_error& e)
         {
            PLOGE << "Failed to save file " << filepath.c_str() << ": " << e.what();
         }
         QOI_FREE(encoded);
      }
   }
   else
   {
   #ifdef __STANDALONE__
      if (SDL_Surface* pSurface = ToSDLSurface(); pSurface)
      {
         if (ext == "png")
            success = IMG_SavePNG(pSurface, filepath.c_str());
         else if (ext == "jpg" || ext == "jpeg")
            success = IMG_SaveJPG(pSurface, filepath.c_str(), 75);
         // Needs latest SDL3_image for WEBP support
         //else if (ext == "webp")
         //   success = IMG_SaveWEBP(pSurface, filepath.c_str(), 75);
         SDL_DestroySurface(pSurface);
      }

   #else
      FIBITMAP* bitmap = FreeImage_Allocate(m_width, m_height, m_format == SRGB ? 24 : 32);
      if (bitmap)
      {
         uint8_t* __restrict bits = (uint8_t*)FreeImage_GetBits(bitmap);
         memcpy(bits, m_data, pitch() * m_height);
         FreeImage_FlipVertical(bitmap);
         if (ext == "png")
            success = FreeImage_Save(FIF_PNG, bitmap, filepath.c_str(), 0);
         else if (ext == "jpg" || ext == "jpeg")
            success = FreeImage_Save(FIF_JPEG, bitmap, filepath.c_str(), 0);
         else if (ext == "webp")
            //success = FreeImage_Save(FIF_WEBP, bitmap, _filePath, WEBP_LOSSLESS); // Very slow and very large files (but would better for our regression tests)
            success = FreeImage_Save(FIF_WEBP, bitmap, filepath.c_str(), WBMP_DEFAULT);
         FreeImage_Unload(bitmap);
      }
   #endif
   }

   return success;
}

std::shared_ptr<BaseTexture> BaseTexture::GetAlias(Format format) const
{
   auto it = m_aliases.find(format);
   if (it == m_aliases.end())
   {
      std::shared_ptr<BaseTexture> alias = Convert(format);
      if (!m_isOpaqueDirty)
         alias->SetIsOpaque(IsOpaque());
      if (!m_isMD5Dirty)
         alias->SetMD5Hash(GetMD5Hash());
      m_aliases[format] = alias;
      return alias;
   }
   else
   {
      return it->second;
   }
}

std::shared_ptr<BaseTexture> BaseTexture::Convert(Format format) const
{
   std::shared_ptr<BaseTexture> tex = nullptr;

   switch (m_format)
   {
   case RGB:
      switch (format)
      {
      case RGBA:
         tex = BaseTexture::Create(m_width, m_height, RGBA);
         if (tex == nullptr)
            return nullptr;
         copy_rgb_rgba<false>((unsigned int*)tex->data(), static_cast<const uint8_t*>(datac()), (size_t)width() * height());
         break;
      default: break;
      }
      break;

   case SRGB:
      switch (format)
      {
      case SRGBA:
         tex = BaseTexture::Create(m_width, m_height, SRGBA);
         if (tex == nullptr)
            return nullptr;
         copy_rgb_rgba<false>((unsigned int*)tex->data(), static_cast<const uint8_t*>(datac()), (size_t)width() * height());
         break;
      default: break;
      }
      break;

   case SRGBA:
      switch (format)
      {
      case SRGB:
         tex = BaseTexture::Create(m_width, m_height, SRGB);
         if (tex == nullptr)
            return nullptr;
         {
            const uint32_t* const __restrict src_data = reinterpret_cast<const uint32_t*>(datac());
            uint8_t* const __restrict dest_data = static_cast<uint8_t*>(tex->data());
            for (size_t o = 0; o < (size_t)width() * height(); ++o)
            {
               const uint32_t rgba = src_data[o];
               dest_data[o * 3 + 0] =  rgba        & 0xFF;
               dest_data[o * 3 + 1] = (rgba >>  8) & 0xFF;
               dest_data[o * 3 + 2] = (rgba >> 16) & 0xFF;
            }
         }
         break;
      default: break;
      }
      break;

   case SRGB565:
      switch (format)
      {
      case SRGBA:
         {
            tex = BaseTexture::Create(m_width, m_height, SRGBA);
            if (tex == nullptr)
               return nullptr;
            static constexpr uint8_t lum32[] = { 0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255 };
            static constexpr uint8_t lum64[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138,
               142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255 };
            uint32_t* const __restrict data = reinterpret_cast<uint32_t*>(tex->data());
            const uint16_t* const __restrict frame = reinterpret_cast<const uint16_t*>(datac());
            const size_t size = (size_t)width() * height();
            for (size_t ofs = 0; ofs < size; ++ofs)
            {
               const uint16_t rgb565 = frame[ofs];
               data[ofs] = 0xFF000000 | (lum32[rgb565 & 0x1F] << 16) | (lum64[(rgb565 >> 5) & 0x3F] << 8) | lum32[(rgb565 >> 11) & 0x1F];
            }
            tex->SetIsOpaque(true);
         }
         break;
      default: break;
      }
      break;

   case RGB_FP16:
      switch (format)
      {
         case RGBA_FP16:
         {
            tex = BaseTexture::Create(m_width, m_height, RGBA_FP16);
            if (tex == nullptr)
               return nullptr;
            uint16_t* const __restrict dest_data16 = reinterpret_cast<uint16_t*>(tex->data());
            const uint16_t* const __restrict src_data16 = reinterpret_cast<const uint16_t*>(datac());
            const size_t size = (size_t)width() * height();
            for (size_t o = 0; o < size; ++o)
            {
               dest_data16[o * 4 + 0] = src_data16[o * 3 + 0];
               dest_data16[o * 4 + 1] = src_data16[o * 3 + 1];
               dest_data16[o * 4 + 2] = src_data16[o * 3 + 2];
               dest_data16[o * 4 + 3] = 0x3C00; //=1.f
            }
         }
         break;
         default: break;
      }
      break;
   
   case RGB_FP32:
      switch (format)
      {
         case RGBA_FP32:
            {
            tex = BaseTexture::Create(m_width, m_height, RGBA_FP32);
            if (tex == nullptr)
               return nullptr;
            uint32_t* const __restrict dest_data32 = reinterpret_cast<uint32_t*>(tex->data());
            const uint32_t* const __restrict src_data32 = reinterpret_cast<const uint32_t*>(datac());
            const size_t size = (size_t)width() * height();
            for (size_t o = 0; o < size; ++o)
            {
               dest_data32[o * 4 + 0] = src_data32[o * 3 + 0];
               dest_data32[o * 4 + 1] = src_data32[o * 3 + 1];
               dest_data32[o * 4 + 2] = src_data32[o * 3 + 2];
               dest_data32[o * 4 + 3] = 0x3f800000; //=1.f
            }
         }
         break;
         default: break;
      }
      break;

   default: break;

   }

   // Copy without conversion
   if (m_format == format)
   {
      assert(tex == nullptr);
      tex = BaseTexture::Create(m_width, m_height, m_format);
      memcpy(tex->data(), datac(), (size_t)m_width * m_height * GetPixelSize(m_format));
   }

   if (tex)
   {
      if (!m_isOpaqueDirty)
         tex->SetIsOpaque(IsOpaque());
      if (!m_isMD5Dirty)
         tex->SetMD5Hash(GetMD5Hash());
   }

   return tex;
}

std::shared_ptr<BaseTexture> BaseTexture::ToBGRA() const
{
   std::shared_ptr<BaseTexture> tex = BaseTexture::Create(m_width, m_height, RGBA);
   if (tex == nullptr)
      return nullptr;

   tex->m_realWidth = m_realWidth;
   tex->m_realHeight = m_realHeight;
   if (IsOpaqueComputed())
      tex->SetIsOpaque(IsOpaque());
   uint8_t* const __restrict tmp = static_cast<uint8_t*>(tex->data());

   if (m_format == BaseTexture::RGB_FP32) // Tonemap for 8bpc-Display
   {
      const float* const __restrict src = (const float*)datac();
      const size_t e = (size_t)width() * height();
      for (size_t o = 0; o < e; ++o)
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
      const uint16_t* const __restrict src = (const uint16_t*)datac();
      const size_t e = (size_t)width() * height();
      for (size_t o = 0; o < e; ++o)
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
      const uint16_t* const __restrict src = (const uint16_t*)datac();
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
      const uint8_t* const __restrict src = static_cast<const uint8_t*>(datac());
      const size_t e = (size_t)width() * height();
      for (size_t o = 0; o < e; ++o)
      {
         tmp[o * 4 + 0] =
         tmp[o * 4 + 1] =
         tmp[o * 4 + 2] = src[o];
         tmp[o * 4 + 3] = 255; // A
      }
   }
   else if (m_format == BaseTexture::RGB || m_format == BaseTexture::SRGB)
   {
      copy_rgb_rgba<true>((unsigned int*)tmp, static_cast<const uint8_t*>(datac()), (size_t)width() * height());
   }
   else if (m_format == BaseTexture::RGBA || m_format == BaseTexture::SRGBA)
   {
      const uint8_t* const __restrict psrc = static_cast<const uint8_t*>(datac());
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

SDL_Surface* BaseTexture::ToSDLSurface() const
{
   SDL_PixelFormat format;
   switch (m_format)
   {
   case BW: format = SDL_PIXELFORMAT_INDEX8; break;
   case RGB: format = SDL_PIXELFORMAT_RGB24; break;
   case RGBA: format = SDL_PIXELFORMAT_ARGB8888; break;
   case SRGB: format = SDL_PIXELFORMAT_RGB24; break;
   case SRGBA: format = SDL_PIXELFORMAT_ARGB8888; break;
   case SRGB565: format = SDL_PIXELFORMAT_RGB565; break;
   case RGB_FP16: format = SDL_PIXELFORMAT_RGB48_FLOAT; break;
   case RGBA_FP16: format = SDL_PIXELFORMAT_RGBA64_FLOAT; break;
   case RGB_FP32: format = SDL_PIXELFORMAT_RGB96_FLOAT; break;
   case RGBA_FP32: format = SDL_PIXELFORMAT_RGBA128_FLOAT; break;
   default: format = SDL_PIXELFORMAT_UNKNOWN; break;
   }
   return format == SDL_PIXELFORMAT_UNKNOWN ? nullptr : SDL_CreateSurfaceFrom(m_width, m_height, format, const_cast<uint8_t*>(static_cast<const uint8_t*>(datac())), pitch());
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
      uint8_t* const __restrict pdst = m_data;
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

Texture::Texture(string name, PinBinary* ppb, unsigned int width, unsigned int height)
   : m_name(std::move(name))
   , m_width(width)
   , m_height(height)
   , m_ppb(ppb)
   , m_liveHash(((uint64_t)this) ^ ((uint64_t)ppb) ^ usec() ^ ((uint64_t)width << 16) ^ ((uint64_t)height << 32))
{
   assert(m_ppb != nullptr);
   assert(m_width > 0);
   assert(m_height > 0);
}

Texture* Texture::CreateFromStream(IStream * const pstream, int version, PinTable * const pt)
{
   string name;
   string path;
   unsigned int width = 0;
   unsigned int height = 0;
   float alphaTestValue = static_cast<float>(-1.0 / 255.0);
   PinBinary* ppb = nullptr;
   bool isMD5Dirty = true;
   uint8_t md5Hash[16] = {};
   bool isOpaqueDirty = true;
   bool isOpaque = true;
   BiffReader br(pstream, nullptr, version, 0, 0);
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
         // Old files used to store some bitmaps as a 32-bit SBGRA picture, we now (10.8.1+) always use a compressed file format. Convert here to simplify the code
         const size_t size = (size_t)height * width;
         assert(ppb == nullptr && size != 0);

         uint8_t* const __restrict tmp = new uint8_t[size * 4];
         const LZWReader lzwreader(pbr->m_pistream, tmp, width * 4);

         // Find out if all alpha values are 0x00 or 0xFF
         #ifdef __OPENGLES__
            constexpr bool has_alpha = true;
         #else
            bool has_alpha = false;
            for (size_t o = 3; o < size * 4; o += 4)
               if (tmp[o] != 0 && tmp[o] != 255)
               {
                  has_alpha = true;
                  break;
               }
         #endif

         // Create a FreeImage from LZW data, optionally dropping a constant (0 or 255) alpha channel
         FIBITMAP* dib = FreeImage_Allocate(width, height, has_alpha ? 32 : 24);
         uint8_t* const pdst = (uint8_t*)FreeImage_GetBits(dib);
         const unsigned int pitch = width * 4;
         const unsigned int pitch_dst = FreeImage_GetPitch(dib);
         const uint8_t* spch = tmp + (height * pitch);
         for (unsigned int i = 0; i < height; i++)
         {
            const uint32_t* const __restrict src = (const uint32_t*)(spch -= pitch); // start on previous previous line
            uint8_t* __restrict dst = pdst + i * pitch_dst;
            if (has_alpha)
               memcpy(dst, src, pitch);
            else
               for (unsigned int x = 0; x < width; x++, dst += 3) // copy without alpha channel
               {
                  const unsigned int rgba = src[x];
                  dst[0] = rgba;
                  dst[1] = rgba >> 8;
                  dst[2] = rgba >> 16;
               }
         }

         // Convert to a lossless webp
         auto memStream = FreeImage_OpenMemory();
         FreeImage_SaveToMemory(FREE_IMAGE_FORMAT::FIF_WEBP, dib, memStream, WEBP_LOSSLESS);
         ppb = new PinBinary();
         ppb->m_buffer.resize(FreeImage_TellMemory(memStream));
         ppb->m_name = name;
         string ext = extension_from_path(path);
         if (!ext.empty())
         {
            path.erase(path.length() - ext.length());
            path += "webp";
         }
         ppb->m_path = path;
         FreeImage_SeekMemory(memStream, 0, SEEK_SET);
         FreeImage_ReadMemory(ppb->m_buffer.data(), 1, static_cast<unsigned int>(ppb->m_buffer.size()), memStream);
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

   Texture* const tex = new Texture(name, ppb, width, height);
   tex->m_alphaTestValue = alphaTestValue;
   if (!isOpaqueDirty)
      tex->SetIsOpaque(isOpaque);
   if (!isMD5Dirty)
      tex->SetMD5Hash(md5Hash);

   return tex;
}

Texture* Texture::CreateFromFile(const string& filename, const bool isImageData)
{
   PinBinary* const ppb = new PinBinary();
   ppb->ReadFromFile(filename);

   std::shared_ptr<BaseTexture> const imageBuffer = BaseTexture::CreateFromData(ppb->m_buffer.data(), ppb->m_buffer.size(), isImageData);
   if (imageBuffer == nullptr)
   {
      delete ppb;
      return nullptr;
   }

   Texture* tex = new Texture(TitleFromFilename(filename), ppb, imageBuffer->m_realWidth, imageBuffer->m_realHeight);
   tex->m_imageBuffer = imageBuffer;
   tex->UpdateMD5();
   tex->UpdateOpaque();
   return tex;
}

Texture::~Texture()
{
   delete m_ppb;
   #ifndef __STANDALONE__
      if (m_hbmGDIVersion)
      {
         if(m_hbmGDIVersion != g_pvp->m_hbmInPlayMode)
             DeleteObject(m_hbmGDIVersion);
      }
   #endif
}

HRESULT Texture::SaveToStream(IStream *pstream, const PinTable *pt) const
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

bool Texture::IsHDR() const
{
   auto buffer = m_imageBuffer.lock();
   if (buffer)
      return buffer->m_format == BaseTexture::RGB_FP16 || buffer->m_format == BaseTexture::RGBA_FP16
          || buffer->m_format == BaseTexture::RGB_FP32 || buffer->m_format == BaseTexture::RGBA_FP32;
   string ext = extension_from_path(m_ppb->m_path);
   return (ext == "exr") || (ext == "hdr");
}

size_t Texture::GetEstimatedGPUSize() const
{
   size_t estimatedSize;
   auto buffer = m_imageBuffer.lock();
   if (buffer)
      estimatedSize = static_cast<size_t>(buffer->height()) * static_cast<size_t>(buffer->pitch());
   else
      estimatedSize = static_cast<size_t>(m_width) * static_cast<size_t>(m_height) * (IsHDR() ? 8 : 4); // 8 bytes per pixel for HDR (RGBA_FP16) and 4 bytes per pixel for non-HDR (RGBA)
   // Add mipmaps (+1/3).
   return (4 * estimatedSize) / 3;
}

std::shared_ptr<const BaseTexture> Texture::GetRawBitmap(bool resizeOnLowMem, unsigned int maxTexDimension) const
{
   auto buffer = m_imageBuffer.lock();
   if (buffer)
      return buffer;
   //PLOGD << "Decoding image " << m_name;
   buffer = std::shared_ptr<BaseTexture>(BaseTexture::CreateFromData(m_ppb->m_buffer.data(), m_ppb->m_buffer.size(), true, maxTexDimension, resizeOnLowMem));
   if (buffer && m_width != buffer->width())
   {
      PLOGE << "Invalid file: image '" << m_name << "' width does not match the width of the image datablock.";
      const_cast<Texture*>(this)->m_width = buffer->width();
   }
   if (buffer && m_height != buffer->height())
   {
      PLOGE << "Invalid file: image '" << m_name << "' height does not match the height of the image datablock.";
      const_cast<Texture*>(this)->m_height = buffer->height();
   }
   m_imageBuffer = buffer;
   UpdateOpaque();
   return buffer;
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

   const auto buffer = GetRawBitmap(false, 0);
   if (buffer == nullptr)
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

   std::shared_ptr<BaseTexture> bgr32bits = buffer->ToBGRA();
   SetStretchBltMode(hdcNew, COLORONCOLOR);
   StretchDIBits(hdcNew,
      0, 0, m_width, m_height,
      0, 0, m_width, m_height,
      bgr32bits->data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
   bgr32bits = nullptr;

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
   generateMD5(m_ppb->m_buffer.data(), m_ppb->m_buffer.size(), m_md5Hash);
   SetMD5Hash(m_md5Hash);
}

void Texture::SetMD5Hash(uint8_t* md5) const
{
   memcpy(m_md5Hash, md5, sizeof(m_md5Hash));
   m_isMD5Dirty = false;
   auto buffer = m_imageBuffer.lock();
   if (buffer)
      buffer->SetMD5Hash(md5);
}

void Texture::UpdateOpaque() const
{
   if (!m_isOpaqueDirty)
      return;
   m_isOpaqueDirty = false;
   auto bitmap = GetRawBitmap(false, 0);
   m_isOpaque = bitmap ? bitmap->IsOpaque() : false;
}

void Texture::SetIsOpaque(const bool v) const
{
   m_isOpaque = v;
   m_isOpaqueDirty = false;
   auto buffer = m_imageBuffer.lock();
   if (buffer)
      buffer->SetIsOpaque(v);
}
