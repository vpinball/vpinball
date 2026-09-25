// license:GPLv3+

#include "core/stdafx.h"

#if defined(ENABLE_BGFX)

#include "TextureCompressor.h"
#include "Texture.h"

#include <bx/allocator.h>
#include <bx/error.h>
#include <bimg/bimg.h>
#include <bimg/encode.h>

#include <atomic>
#include <chrono>
#include <fstream>

namespace
{
   struct CacheHeader
   {
      char magic[4] = { 'V', 'P', 'X', 'T' };
      uint32_t version = 3;
      uint32_t format = 0;
      uint32_t srcFormat = 0;
      uint32_t opaque = 0;
      uint32_t width = 0;
      uint32_t height = 0;
      uint32_t numMips = 0;
      uint64_t dataSize = 0;
   };

   std::shared_ptr<const CompressedTexture> ReadCacheFile(const std::filesystem::path& cacheFile, CacheHeader& header)
   {
      std::ifstream in(cacheFile, std::ios::binary);
      if (!in || !in.read(reinterpret_cast<char*>(&header), sizeof(header)) || memcmp(header.magic, CacheHeader().magic, 4) != 0 || header.version != CacheHeader().version)
         return nullptr;
      if (header.format >= bgfx::TextureFormat::Count || header.width == 0 || header.height == 0 || header.width > 16384 || header.height > 16384
         || header.numMips != static_cast<uint32_t>(1 + static_cast<int>(floor(log2(max(header.width, header.height)))))
         || header.dataSize != bimg::imageGetSize(nullptr, header.width, header.height, 1, false, true, 1, static_cast<bimg::TextureFormat::Enum>(header.format)))
         return nullptr;
      auto result = std::make_shared<CompressedTexture>();
      result->format = static_cast<bgfx::TextureFormat::Enum>(header.format);
      result->width = header.width;
      result->height = header.height;
      result->numMips = static_cast<uint8_t>(header.numMips);
      result->data.resize(header.dataSize);
      if (!in.read(reinterpret_cast<char*>(result->data.data()), static_cast<std::streamsize>(header.dataSize)))
         return nullptr;
      return result;
   }

   std::atomic<uint64_t> s_nCompressed = 0;
   std::atomic<uint64_t> s_nLoaded = 0;
   std::atomic<uint64_t> s_compressMs = 0;
   std::atomic<uint64_t> s_loadMs = 0;
   std::atomic<uint64_t> s_rawBytes = 0;
   std::atomic<uint64_t> s_compressedBytes = 0;

   float s_srgbToLinear[256];
   const bool s_lutInit = []()
   {
      for (int i = 0; i < 256; i++)
      {
         const float c = static_cast<float>(i) / 255.f;
         s_srgbToLinear[i] = c <= 0.04045f ? c / 12.92f : powf((c + 0.055f) / 1.055f, 2.4f);
      }
      return true;
   }();

   uint8_t LinearToSrgb(float c)
   {
      c = c <= 0.0031308f ? c * 12.92f : 1.055f * powf(c, 1.f / 2.4f) - 0.055f;
      return static_cast<uint8_t>(clamp(c * 255.f + 0.5f, 0.f, 255.f));
   }

   void Downsample(const uint8_t* src, unsigned int sw, unsigned int sh, uint8_t* dst, unsigned int dw, unsigned int dh, bool isSrgb)
   {
      for (unsigned int y = 0; y < dh; y++)
      {
         const unsigned int y0 = min(y * 2, sh - 1);
         const unsigned int y1 = min(y * 2 + 1, sh - 1);
         for (unsigned int x = 0; x < dw; x++)
         {
            const unsigned int x0 = min(x * 2, sw - 1);
            const unsigned int x1 = min(x * 2 + 1, sw - 1);
            const uint8_t* p[4] = { &src[(y0 * sw + x0) * 4], &src[(y0 * sw + x1) * 4], &src[(y1 * sw + x0) * 4], &src[(y1 * sw + x1) * 4] };
            uint8_t* d = &dst[(y * dw + x) * 4];
            for (int c = 0; c < 3; c++)
            {
               if (isSrgb)
                  d[c] = LinearToSrgb(0.25f * (s_srgbToLinear[p[0][c]] + s_srgbToLinear[p[1][c]] + s_srgbToLinear[p[2][c]] + s_srgbToLinear[p[3][c]]));
               else
                  d[c] = static_cast<uint8_t>((p[0][c] + p[1][c] + p[2][c] + p[3][c] + 2) >> 2);
            }
            d[3] = static_cast<uint8_t>((p[0][3] + p[1][3] + p[2][3] + p[3][3] + 2) >> 2);
         }
      }
   }

   void DownsampleFloat(const float* src, unsigned int sw, unsigned int sh, float* dst, unsigned int dw, unsigned int dh)
   {
      for (unsigned int y = 0; y < dh; y++)
      {
         const unsigned int y0 = min(y * 2, sh - 1);
         const unsigned int y1 = min(y * 2 + 1, sh - 1);
         for (unsigned int x = 0; x < dw; x++)
         {
            const unsigned int x0 = min(x * 2, sw - 1);
            const unsigned int x1 = min(x * 2 + 1, sw - 1);
            const float* p[4] = { &src[(y0 * sw + x0) * 4], &src[(y0 * sw + x1) * 4], &src[(y1 * sw + x0) * 4], &src[(y1 * sw + x1) * 4] };
            float* d = &dst[(y * dw + x) * 4];
            for (int c = 0; c < 4; c++)
               d[c] = 0.25f * (p[0][c] + p[1][c] + p[2][c] + p[3][c]);
         }
      }
   }

   uint32_t PackRgb9E5(const float* rgb)
   {
      constexpr float maxValue = 65408.f;
      const float r = clamp(rgb[0], 0.f, maxValue);
      const float g = clamp(rgb[1], 0.f, maxValue);
      const float b = clamp(rgb[2], 0.f, maxValue);
      const float maxc = max(r, max(g, b));
      if (maxc < 1e-10f)
         return 0;
      int exponent = max(-16, static_cast<int>(floorf(log2f(maxc)))) + 16;
      float scale = exp2f(static_cast<float>(exponent - 24));
      if (static_cast<int>(floorf(maxc / scale + 0.5f)) == 512)
      {
         scale *= 2.f;
         exponent++;
      }
      const uint32_t rm = static_cast<uint32_t>(floorf(r / scale + 0.5f));
      const uint32_t gm = static_cast<uint32_t>(floorf(g / scale + 0.5f));
      const uint32_t bm = static_cast<uint32_t>(floorf(b / scale + 0.5f));
      return rm | (gm << 9) | (bm << 18) | (static_cast<uint32_t>(exponent) << 27);
   }

   bool IsHdr(BaseTexture::Format format) { return format == BaseTexture::RGB_FP16 || format == BaseTexture::RGB_FP32; }

   bool IsFormatUsable(bgfx::TextureFormat::Enum format, bool needSrgb)
   {
      const uint32_t needed = BGFX_CAPS_FORMAT_TEXTURE_2D | (needSrgb ? BGFX_CAPS_FORMAT_TEXTURE_2D_SRGB : 0);
      return (bgfx::getCaps()->formats[format] & needed) == needed;
   }

   bgfx::TextureFormat::Enum SelectFormatFor(BaseTexture::Format srcFormat, bool opaque)
   {
      if (!IsHdr(srcFormat))
         return TextureCompressor::SelectFormat(!opaque);
      for (const auto format : { bgfx::TextureFormat::RGB9E5F, bgfx::TextureFormat::RG11B10F })
         if (IsFormatUsable(format, false))
            return format;
      return bgfx::TextureFormat::Unknown;
   }
}

size_t CompressedTexture::GetMipSize(unsigned int mip) const
{
   return static_cast<size_t>(bimg::imageGetSize(nullptr, max(1u, width >> mip), max(1u, height >> mip), 1, false, false, 1, static_cast<bimg::TextureFormat::Enum>(format)));
}

bool TextureCompressor::IsSupported(const BaseTexture& tex)
{
   switch (tex.m_format)
   {
   case BaseTexture::RGB:
   case BaseTexture::RGBA:
   case BaseTexture::SRGB:
   case BaseTexture::SRGBA:
   case BaseTexture::SRGB565:
   case BaseTexture::RGB_FP16:
   case BaseTexture::RGB_FP32:
      return tex.width() >= 64 && tex.height() >= 64;
   default:
      return false;
   }
}

bgfx::TextureFormat::Enum TextureCompressor::SelectFormat(bool hasAlpha)
{
   static constexpr bgfx::TextureFormat::Enum opaqueFormats[] = { bgfx::TextureFormat::BC1, bgfx::TextureFormat::ASTC6x6, bgfx::TextureFormat::ETC2 };
   static constexpr bgfx::TextureFormat::Enum alphaFormats[] = { bgfx::TextureFormat::BC3, bgfx::TextureFormat::ASTC4x4, bgfx::TextureFormat::ETC2A };
   for (const auto format : hasAlpha ? alphaFormats : opaqueFormats)
      if (IsFormatUsable(format, true))
         return format;
   return bgfx::TextureFormat::Unknown;
}

const char* TextureCompressor::GetFormatName(bgfx::TextureFormat::Enum format)
{
   return bimg::getName(static_cast<bimg::TextureFormat::Enum>(format));
}

static std::shared_ptr<const CompressedTexture> CompressHdr(const BaseTexture& tex, bgfx::TextureFormat::Enum format)
{
   const auto start = std::chrono::steady_clock::now();
   auto result = std::make_shared<CompressedTexture>();
   result->format = format;
   result->width = tex.width();
   result->height = tex.height();
   result->numMips = static_cast<uint8_t>(1 + static_cast<int>(floor(log2(max(tex.width(), tex.height())))));
   result->data.resize(static_cast<size_t>(bimg::imageGetSize(nullptr, tex.width(), tex.height(), 1, false, true, 1, static_cast<bimg::TextureFormat::Enum>(format))));

   const size_t nPixels = static_cast<size_t>(tex.width()) * tex.height();
   vector<float> mipA(nPixels * 4), mipB;
   if (tex.m_format == BaseTexture::RGB_FP16)
   {
      const uint16_t* src = static_cast<const uint16_t*>(tex.datac());
      for (size_t i = 0; i < nPixels; i++)
      {
         mipA[i * 4 + 0] = half2float(src[i * 3 + 0]);
         mipA[i * 4 + 1] = half2float(src[i * 3 + 1]);
         mipA[i * 4 + 2] = half2float(src[i * 3 + 2]);
         mipA[i * 4 + 3] = 1.f;
      }
   }
   else
   {
      const float* src = static_cast<const float*>(tex.datac());
      for (size_t i = 0; i < nPixels; i++)
      {
         mipA[i * 4 + 0] = src[i * 3 + 0];
         mipA[i * 4 + 1] = src[i * 3 + 1];
         mipA[i * 4 + 2] = src[i * 3 + 2];
         mipA[i * 4 + 3] = 1.f;
      }
   }

   bx::DefaultAllocator allocator;
   const float* src = mipA.data();
   unsigned int w = tex.width(), h = tex.height();
   size_t offset = 0;
   for (unsigned int mip = 0; mip < result->numMips; mip++)
   {
      if (mip > 0)
      {
         const unsigned int nw = max(1u, w >> 1), nh = max(1u, h >> 1);
         vector<float>& dst = (mip & 1) ? mipB : mipA;
         dst.resize(static_cast<size_t>(nw) * nh * 4);
         DownsampleFloat(src, w, h, dst.data(), nw, nh);
         src = dst.data();
         w = nw;
         h = nh;
      }
      bool ok;
      if (format == bgfx::TextureFormat::RGB9E5F)
      {
         uint32_t* dst = reinterpret_cast<uint32_t*>(result->data.data() + offset);
         for (size_t i = 0; i < static_cast<size_t>(w) * h; i++)
            dst[i] = PackRgb9E5(&src[i * 4]);
         ok = true;
      }
      else
         ok = bimg::imageConvert(&allocator, result->data.data() + offset, static_cast<bimg::TextureFormat::Enum>(format), src, bimg::TextureFormat::RGBA32F, w, h, 1);
      if (!ok)
      {
         PLOGE << "Failed to compress HDR texture '" << tex.GetName() << "' to " << bimg::getName(static_cast<bimg::TextureFormat::Enum>(format));
         return nullptr;
      }
      offset += result->GetMipSize(mip);
   }
   assert(offset == result->data.size());

   s_nCompressed++;
   s_compressMs += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
   return result;
}

std::shared_ptr<const CompressedTexture> TextureCompressor::Compress(const BaseTexture& tex, bgfx::TextureFormat::Enum format)
{
   if (format == bgfx::TextureFormat::Unknown || !IsSupported(tex))
      return nullptr;

   if (IsHdr(tex.m_format))
      return CompressHdr(tex, format);

   const auto start = std::chrono::steady_clock::now();
   const bool isSrgb = !BaseTexture::IsLinearFormat(tex.m_format);
   std::shared_ptr<const BaseTexture> converted;
   if (tex.m_format != BaseTexture::RGBA && tex.m_format != BaseTexture::SRGBA)
   {
      converted = tex.Convert(isSrgb ? BaseTexture::SRGBA : BaseTexture::RGBA);
      if (converted == nullptr)
         return nullptr;
   }

   auto result = std::make_shared<CompressedTexture>();
   result->format = format;
   result->width = tex.width();
   result->height = tex.height();
   result->numMips = static_cast<uint8_t>(1 + static_cast<int>(floor(log2(max(tex.width(), tex.height())))));
   result->data.resize(static_cast<size_t>(bimg::imageGetSize(nullptr, tex.width(), tex.height(), 1, false, true, 1, static_cast<bimg::TextureFormat::Enum>(format))));

   bx::DefaultAllocator allocator;
   bx::Error err;
   vector<uint8_t> mipA, mipB;
   const uint8_t* src = static_cast<const uint8_t*>(converted ? converted->datac() : tex.datac());
   unsigned int w = tex.width(), h = tex.height();
   size_t offset = 0;
   for (unsigned int mip = 0; mip < result->numMips; mip++)
   {
      if (mip > 0)
      {
         const unsigned int nw = max(1u, w >> 1), nh = max(1u, h >> 1);
         vector<uint8_t>& dst = (mip & 1) ? mipA : mipB;
         dst.resize(static_cast<size_t>(nw) * nh * 4);
         Downsample(src, w, h, dst.data(), nw, nh, isSrgb);
         src = dst.data();
         w = nw;
         h = nh;
      }
      bimg::imageEncodeFromRgba8(&allocator, result->data.data() + offset, src, w, h, 1, static_cast<bimg::TextureFormat::Enum>(format), bimg::Quality::Fastest, &err);
      if (!err.isOk())
      {
         PLOGE << "Failed to compress texture '" << tex.GetName() << "' to " << bimg::getName(static_cast<bimg::TextureFormat::Enum>(format));
         return nullptr;
      }
      offset += result->GetMipSize(mip);
   }
   assert(offset == result->data.size());

   s_nCompressed++;
   s_compressMs += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
   return result;
}

std::shared_ptr<BaseTexture> TextureCompressor::LoadCached(const std::filesystem::path& cacheFile)
{
   const auto start = std::chrono::steady_clock::now();
   CacheHeader header;
   std::shared_ptr<const CompressedTexture> compressed = ReadCacheFile(cacheFile, header);
   if (compressed == nullptr || header.format != static_cast<uint32_t>(SelectFormatFor(static_cast<BaseTexture::Format>(header.srcFormat), header.opaque != 0)))
      return nullptr;
   s_nLoaded++;
   s_loadMs += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
   s_rawBytes += (static_cast<uint64_t>(header.width) * header.height * (IsHdr(static_cast<BaseTexture::Format>(header.srcFormat)) ? 8 : 4) * 4) / 3;
   s_compressedBytes += compressed->data.size();
   return BaseTexture::CreateCompressedOnly(std::move(compressed), static_cast<BaseTexture::Format>(header.srcFormat), header.opaque != 0);
}

std::shared_ptr<const CompressedTexture> TextureCompressor::LoadOrCompress(const BaseTexture& tex, const std::filesystem::path& cacheFile)
{
   if (!IsSupported(tex))
      return nullptr;
   const bool opaque = tex.IsOpaque();
   const bgfx::TextureFormat::Enum format = SelectFormatFor(tex.m_format, opaque);
   if (format == bgfx::TextureFormat::Unknown)
      return nullptr;
   const uint64_t rawBytes = (static_cast<uint64_t>(tex.width()) * tex.height() * (IsHdr(tex.m_format) ? 8 : 4) * 4) / 3;

   if (!cacheFile.empty())
   {
      const auto start = std::chrono::steady_clock::now();
      CacheHeader header;
      std::shared_ptr<const CompressedTexture> result = ReadCacheFile(cacheFile, header);
      if (result && result->format == format && result->width == tex.width() && result->height == tex.height())
      {
         s_nLoaded++;
         s_loadMs += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
         s_rawBytes += rawBytes;
         s_compressedBytes += result->data.size();
         return result;
      }
   }

   auto result = Compress(tex, format);
   if (result == nullptr)
      return nullptr;
   s_rawBytes += rawBytes;
   s_compressedBytes += result->data.size();

   if (!cacheFile.empty())
   {
      CacheHeader header;
      header.format = static_cast<uint32_t>(result->format);
      header.srcFormat = static_cast<uint32_t>(tex.m_format);
      header.opaque = opaque ? 1 : 0;
      header.width = result->width;
      header.height = result->height;
      header.numMips = result->numMips;
      header.dataSize = result->data.size();
      const std::filesystem::path tmpFile = std::filesystem::path(cacheFile).concat(".tmp");
      {
         std::ofstream out(tmpFile, std::ios::binary | std::ios::trunc);
         out.write(reinterpret_cast<const char*>(&header), sizeof(header));
         out.write(reinterpret_cast<const char*>(result->data.data()), static_cast<std::streamsize>(result->data.size()));
      }
      std::error_code ec;
      std::filesystem::rename(tmpFile, cacheFile, ec);
      if (ec)
      {
         PLOGE << "Failed to write compressed texture cache " << cacheFile;
      }
   }
   return result;
}

void TextureCompressor::ResetStats()
{
   s_nCompressed = 0;
   s_nLoaded = 0;
   s_compressMs = 0;
   s_loadMs = 0;
   s_rawBytes = 0;
   s_compressedBytes = 0;
}

void TextureCompressor::LogStats()
{
   PLOGI << "Texture compression: " << s_nCompressed << " compressed (" << s_compressMs << "ms cumulated), " << s_nLoaded << " loaded from cache (" << s_loadMs << "ms cumulated), GPU memory "
         << (s_rawBytes / (1024 * 1024)) << "MB uncompressed -> " << (s_compressedBytes / (1024 * 1024)) << "MB compressed";
}

#endif
