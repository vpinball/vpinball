// license:GPLv3+

#include "core/stdafx.h"

#if defined(ENABLE_BGFX)

#include "TextureCompressor.h"
#include "Texture.h"

#include <bx/allocator.h>
#include <bx/error.h>
#include <bimg/bimg.h>
#include <bimg/encode.h>
#include <bc6h/basisu_bc6h.h>
#include <bc7f/basisu_bc7f.h>
#include <miniz/miniz.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace
{
   struct CacheHeader
   {
      char magic[4] = { 'V', 'P', 'X', 'T' };
      uint32_t version = 5;
      uint32_t format = 0;
      uint32_t srcFormat = 0;
      uint32_t opaque = 0;
      uint32_t width = 0;
      uint32_t height = 0;
      uint32_t numMips = 0;
      uint64_t dataSize = 0; // Size of the uncompressed texture data
      uint64_t packedSize = 0; // Size of the deflated data stored in the file
   };

   bool IsCacheFormat(uint32_t format)
   {
      switch (static_cast<bgfx::TextureFormat::Enum>(format))
      {
      case bgfx::TextureFormat::BC1:
      case bgfx::TextureFormat::BC3:
      case bgfx::TextureFormat::BC6HU:
      case bgfx::TextureFormat::BC7:
      case bgfx::TextureFormat::ASTC4x4:
      case bgfx::TextureFormat::ASTC6x6:
      case bgfx::TextureFormat::ETC2:
      case bgfx::TextureFormat::ETC2A:
      case bgfx::TextureFormat::RGB9E5F:
      case bgfx::TextureFormat::RG11B10F: return true;
      default: return false;
      }
   }

   std::shared_ptr<const CompressedTexture> ReadCacheFile(const std::filesystem::path& cacheFile, CacheHeader& header)
   {
      std::ifstream in(cacheFile, std::ios::binary);
      if (!in || !in.read(reinterpret_cast<char*>(&header), sizeof(header)) || memcmp(header.magic, CacheHeader().magic, 4) != 0 || header.version != CacheHeader().version)
         return nullptr;
      if (!IsCacheFormat(header.format) || header.width == 0 || header.height == 0 || header.width > 16384 || header.height > 16384
         || header.numMips != static_cast<uint32_t>(1 + static_cast<int>(floor(log2(max(header.width, header.height)))))
         || header.dataSize != bimg::imageGetSize(nullptr, header.width, header.height, 1, false, true, 1, static_cast<bimg::TextureFormat::Enum>(header.format))
         || header.dataSize > std::numeric_limits<mz_ulong>::max() || header.packedSize == 0 || header.packedSize > compressBound(static_cast<mz_ulong>(header.dataSize)))
         return nullptr;
      vector<uint8_t> packed(static_cast<size_t>(header.packedSize));
      if (!in.read(reinterpret_cast<char*>(packed.data()), static_cast<std::streamsize>(header.packedSize)))
         return nullptr;
      auto result = std::make_shared<CompressedTexture>();
      result->format = static_cast<bgfx::TextureFormat::Enum>(header.format);
      result->width = header.width;
      result->height = header.height;
      result->numMips = static_cast<uint8_t>(header.numMips);
      result->data.resize(header.dataSize);
      mz_ulong unpackedSize = static_cast<mz_ulong>(header.dataSize);
      if (uncompress(result->data.data(), &unpackedSize, packed.data(), static_cast<mz_ulong>(header.packedSize)) != Z_OK || unpackedSize != header.dataSize)
         return nullptr;
      return result;
   }

   const bool s_encodersInit = []()
   {
      basist::bc7f::init();
      basist::astc_6x6_hdr::fast_encode_bc6h_init();
      return true;
   }();

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
#ifdef ENABLE_SSE_OPTIMIZATIONS
            const __m128 sum = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_loadu_ps(p[0]), _mm_loadu_ps(p[1])), _mm_loadu_ps(p[2])), _mm_loadu_ps(p[3]));
            _mm_storeu_ps(d, _mm_mul_ps(sum, _mm_set1_ps(0.25f)));
#else
            for (int c = 0; c < 4; c++)
               d[c] = 0.25f * (p[0][c] + p[1][c] + p[2][c] + p[3][c]);
#endif
         }
      }
   }

#ifdef ENABLE_SSE_OPTIMIZATIONS
   // Expands a RGB half-float pixel to 4 float lanes (lane 3 is 0). Matches half2float exactly,
   // including denormals, Inf and NaN.
   inline __m128 Half3ToFloat(const uint16_t* s)
   {
      int32_t lo; // 6 byte read: avoids reading past the end of the buffer on the last pixel
      memcpy(&lo, s, sizeof(lo));
      const __m128i d = _mm_unpacklo_epi16(_mm_insert_epi16(_mm_cvtsi32_si128(lo), s[2], 2), _mm_setzero_si128());
      const __m128i shifted = _mm_slli_epi32(_mm_and_si128(d, _mm_set1_epi32(0x7FFF)), 13);
      __m128 f = _mm_mul_ps(_mm_castsi128_ps(shifted), _mm_castsi128_ps(_mm_set1_epi32(0x77800000))); // * 2^112
      const __m128i isInfNan = _mm_cmpeq_epi32(_mm_and_si128(d, _mm_set1_epi32(0x7C00)), _mm_set1_epi32(0x7C00));
      const __m128 fInfNan = _mm_castsi128_ps(_mm_or_si128(_mm_and_si128(shifted, _mm_set1_epi32(0x007FFFFF)), _mm_set1_epi32(0x7F800000)));
      f = _mm_or_ps(_mm_and_ps(_mm_castsi128_ps(isInfNan), fInfNan), _mm_andnot_ps(_mm_castsi128_ps(isInfNan), f));
      return _mm_or_ps(f, _mm_castsi128_ps(_mm_slli_epi32(_mm_and_si128(d, _mm_set1_epi32(0x8000)), 16)));
   }
#endif

   void DownsampleHalf(const uint16_t* src, unsigned int sw, unsigned int sh, uint16_t* dst, unsigned int dw, unsigned int dh)
   {
      for (unsigned int y = 0; y < dh; y++)
      {
         const unsigned int y0 = min(y * 2, sh - 1);
         const unsigned int y1 = min(y * 2 + 1, sh - 1);
         for (unsigned int x = 0; x < dw; x++)
         {
            const unsigned int x0 = min(x * 2, sw - 1);
            const unsigned int x1 = min(x * 2 + 1, sw - 1);
            const uint16_t* p[4] = { &src[(y0 * sw + x0) * 3], &src[(y0 * sw + x1) * 3], &src[(y1 * sw + x0) * 3], &src[(y1 * sw + x1) * 3] };
            uint16_t* d = &dst[(y * dw + x) * 3];
#ifdef ENABLE_SSE_OPTIMIZATIONS
            float f[4];
            const __m128 sum = _mm_add_ps(_mm_add_ps(_mm_add_ps(Half3ToFloat(p[0]), Half3ToFloat(p[1])), Half3ToFloat(p[2])), Half3ToFloat(p[3]));
            _mm_storeu_ps(f, _mm_mul_ps(sum, _mm_set1_ps(0.25f)));
            for (int c = 0; c < 3; c++)
               d[c] = float2half(f[c]);
#else
            for (int c = 0; c < 3; c++)
               d[c] = float2half(0.25f * (half2float(p[0][c]) + half2float(p[1][c]) + half2float(p[2][c]) + half2float(p[3][c])));
#endif
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

   // Bytes per pixel of the uncompressed GPU upload (LDR goes to RGBA8, RGB_FP16 to RGBA16F, RGB_FP32 to RGBA32F)
   uint64_t UncompressedPixelSize(BaseTexture::Format format) { return format == BaseTexture::RGB_FP32 ? 16 : IsHdr(format) ? 8 : 4; }

   bool IsFormatUsable(bgfx::TextureFormat::Enum format, unsigned int width, unsigned int height, bool needSrgb)
   {
      if (format == bgfx::TextureFormat::Unknown)
         return false;

      const uint32_t needed = BGFX_CAPS_FORMAT_TEXTURE_2D | (needSrgb ? BGFX_CAPS_FORMAT_TEXTURE_2D_SRGB : 0);
      if ((bgfx::getCaps()->formats[format] & needed) != needed)
         return false;

      // D3D11/D3D12 cannot create block-compressed textures whose base dimensions are not multiples of the block size
      switch (bgfx::getRendererType())
      {
      case bgfx::RendererType::Direct3D11:
      case bgfx::RendererType::Direct3D12:
      {
         const bimg::ImageBlockInfo& block = bimg::getBlockInfo(static_cast<bimg::TextureFormat::Enum>(format));
         return width % block.blockWidth == 0 && height % block.blockHeight == 0;
      }
      default: return true;
      }
   }

   }

   bgfx::TextureFormat::Enum TextureCompressor::SelectFormatFor(BaseTexture::Format srcFormat, bool opaque, unsigned int width, unsigned int height)
   {
      // Selection logic:
      // - Desktop: BC7/BC6HU, BC3/BC1 as a fallback.
      // - Mobile: ASTC 4x4 (equiv as BC7), could use ASTC 5x5, ASTC 6x6, ASTC 8x8 or even ASTC 10x10/12x12 as a user selection for highly memory constrained devices
      // - Mobile Legacy: ETC2/ETC2A as a fallback format
      if (IsHdr(srcFormat))
      {
         for (const auto format : { bgfx::TextureFormat::BC6HU, bgfx::TextureFormat::RGB9E5F, bgfx::TextureFormat::RG11B10F })
            if (IsFormatUsable(format, width, height, false))
               return format;
      }
      else if (opaque)
      {
         for (const auto format : { bgfx::TextureFormat::BC7, bgfx::TextureFormat::BC1, bgfx::TextureFormat::ASTC4x4, bgfx::TextureFormat::ETC2 })
            if (IsFormatUsable(format, width, height, !BaseTexture::IsLinearFormat(srcFormat)))
               return format;
      }
      else
      {
         for (const auto format : { bgfx::TextureFormat::BC7, bgfx::TextureFormat::BC3, bgfx::TextureFormat::ASTC4x4, bgfx::TextureFormat::ETC2A })
            if (IsFormatUsable(format, width, height, !BaseTexture::IsLinearFormat(srcFormat)))
               return format;
      }
      return bgfx::TextureFormat::Unknown;
   }

size_t CompressedTexture::GetMipSize(unsigned int mip) const
{
   return static_cast<size_t>(bimg::imageGetSize(nullptr, max(1u, width >> mip), max(1u, height >> mip), 1, false, false, 1, static_cast<bimg::TextureFormat::Enum>(format)));
}

bool TextureCompressor::IsSupported(const BaseTexture& tex)
{
   if (tex.datac() == nullptr) // Compressed-only textures have no pixel data to encode
      return false;
   switch (tex.m_format)
   {
   case BaseTexture::RGB:
   case BaseTexture::RGBA:
   case BaseTexture::SRGB:
   case BaseTexture::SRGBA:
   case BaseTexture::SRGB565:
   case BaseTexture::RGB_FP16:
   case BaseTexture::RGB_FP32:
      return tex.width() >= 64 && tex.height() >= 64 && SelectFormatFor(tex.m_format, tex.IsOpaque(), tex.width(), tex.height()) != bgfx::TextureFormat::Unknown;
   default:
      return false;
   }
}

const char* TextureCompressor::GetFormatName(bgfx::TextureFormat::Enum format)
{
   return bimg::getName(static_cast<bimg::TextureFormat::Enum>(format));
}

// Dedicated RGB16F -> BC6HU path: mip generation and block extraction are done in half-float,
// avoiding the FP16 -> FP32 -> FP16 conversions of the generic HDR compression path.
static std::shared_ptr<const CompressedTexture> CompressRGB16FToBC6H(const BaseTexture& tex, const std::function<bool()>& isCompressionDiscarded)
{
   auto result = std::make_shared<CompressedTexture>();
   result->format = bgfx::TextureFormat::BC6HU;
   result->width = tex.width();
   result->height = tex.height();
   result->numMips = static_cast<uint8_t>(1 + static_cast<int>(floor(log2(max(tex.width(), tex.height())))));
   result->data.resize(static_cast<size_t>(bimg::imageGetSize(nullptr, tex.width(), tex.height(), 1, false, true, 1, static_cast<bimg::TextureFormat::Enum>(bgfx::TextureFormat::BC6HU))));

   vector<uint16_t> mipA, mipB;
   const uint16_t* src = static_cast<const uint16_t*>(tex.datac());
   unsigned int w = tex.width(), h = tex.height();
   size_t offset = 0;
   for (unsigned int mip = 0; mip < result->numMips; mip++)
   {
      if (isCompressionDiscarded && isCompressionDiscarded())
         return nullptr;
      if (mip > 0)
      {
         const unsigned int nw = max(1u, w >> 1), nh = max(1u, h >> 1);
         vector<uint16_t>& dst = (mip & 1) ? mipB : mipA;
         dst.resize(static_cast<size_t>(nw) * nh * 3);
         DownsampleHalf(src, w, h, dst.data(), nw, nh);
         src = dst.data();
         w = nw;
         h = nh;
      }
      // bc6hf takes one 4x4 RGB half-float block per output block: extract with clamped edge pixels
      const unsigned int bw = (w + 3) / 4, bh = (h + 3) / 4;
      vector<basist::half_float> blockPixels(static_cast<size_t>(bw) * bh * 48);
      basist::half_float* bp = blockPixels.data();
      for (unsigned int by = 0; by < bh; by++)
         for (unsigned int bx = 0; bx < bw; bx++)
            for (unsigned int py = 0; py < 4; py++)
               for (unsigned int px = 0; px < 4; px++)
               {
                  const uint16_t* s = &src[(min(by * 4 + py, h - 1) * w + min(bx * 4 + px, w - 1)) * 3];
                  for (int c = 0; c < 3; c++)
                  {
                     // The encoder only accepts unsigned finite halfs: halfs >= 0x7C00 (Inf/NaN and all
                     // signed values, also normalizing -0.0 to +0.0) map to +0.0
                     const uint16_t v = s[c];
                     *bp++ = v < 0x7C00u ? v : static_cast<basist::half_float>(0);
                  }
               }
      // Encode one row of blocks at a time to be able to abort an ongoing compression
      const basist::astc_6x6_hdr::fast_bc6h_params params;
      basist::bc6h_block* const dst = reinterpret_cast<basist::bc6h_block*>(result->data.data() + offset);
      for (unsigned int by = 0; by < bh; by++)
      {
         if (isCompressionDiscarded && isCompressionDiscarded())
            return nullptr;
         for (unsigned int bx = 0; bx < bw; bx++)
            basist::astc_6x6_hdr::fast_encode_bc6h(blockPixels.data() + (static_cast<size_t>(by) * bw + bx) * 48, dst + static_cast<size_t>(by) * bw + bx, params);
      }
      offset += result->GetMipSize(mip);
   }
   assert(offset == result->data.size());

   return result;
}

static std::shared_ptr<const CompressedTexture> CompressHDR(const BaseTexture& tex, bgfx::TextureFormat::Enum format, const std::function<bool()>& isCompressionDiscarded)
{
   if (tex.m_format == BaseTexture::RGB_FP16 && format == bgfx::TextureFormat::BC6HU)
      return CompressRGB16FToBC6H(tex, isCompressionDiscarded);

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
      if (isCompressionDiscarded && isCompressionDiscarded())
         return nullptr;
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
         {
            if ((i & 0xFFFF) == 0 && isCompressionDiscarded && isCompressionDiscarded())
               return nullptr;
            dst[i] = PackRgb9E5(&src[i * 4]);
         }
         ok = true;
      }
      else if (format == bgfx::TextureFormat::BC6HU)
      {
         // bc6hf takes one 4x4 RGB half-float block per output block: extract with clamped edge pixels
         const unsigned int bw = (w + 3) / 4, bh = (h + 3) / 4;
         vector<basist::half_float> blockPixels(static_cast<size_t>(bw) * bh * 48);
         basist::half_float* bp = blockPixels.data();
         for (unsigned int by = 0; by < bh; by++)
            for (unsigned int bx = 0; bx < bw; bx++)
               for (unsigned int py = 0; py < 4; py++)
                  for (unsigned int px = 0; px < 4; px++)
                  {
                     const float* s = &src[(min(by * 4 + py, h - 1) * w + min(bx * 4 + px, w - 1)) * 4];
                     for (int c = 0; c < 3; c++)
                     {
                        // Bit test for Inf/NaN on purpose: the GNU build uses -ffast-math which makes isfinite() unreliable
                        const float v = s[c];
                        uint32_t bits;
                        memcpy(&bits, &v, sizeof(bits));
                        const float clamped = (bits & 0x7F800000u) == 0x7F800000u ? 0.f : clamp(v, 0.f, static_cast<float>(basist::MAX_HALF_FLOAT));
                        *bp++ = float2half(clamped == 0.f ? 0.f : clamped); // normalize -0.0f to +0.0f: half 0x8000 is signed and rejected by the encoder
                     }
                  }
         // Encode one row of blocks at a time to be able to abort an ongoing compression
         const basist::astc_6x6_hdr::fast_bc6h_params params;
         basist::bc6h_block* const dst = reinterpret_cast<basist::bc6h_block*>(result->data.data() + offset);
         for (unsigned int by = 0; by < bh; by++)
         {
            if (isCompressionDiscarded && isCompressionDiscarded())
               return nullptr;
            for (unsigned int bx = 0; bx < bw; bx++)
               basist::astc_6x6_hdr::fast_encode_bc6h(blockPixels.data() + (static_cast<size_t>(by) * bw + bx) * 48, dst + static_cast<size_t>(by) * bw + bx, params);
         }
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

   return result;
}

std::shared_ptr<const CompressedTexture> TextureCompressor::Compress(const BaseTexture& tex, bgfx::TextureFormat::Enum format, const std::function<bool()>& isCompressionDiscarded)
{
   if (!IsFormatUsable(format, tex.width(), tex.height(), !BaseTexture::IsLinearFormat(tex.m_format)))
      return nullptr;

   if (isCompressionDiscarded && isCompressionDiscarded())
      return nullptr;

   if (IsHdr(tex.m_format))
      return CompressHDR(tex, format, isCompressionDiscarded);

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
      if (isCompressionDiscarded && isCompressionDiscarded())
         return nullptr;
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
      if (format == bgfx::TextureFormat::BC7)
      {
         // bc7f takes one 4x4 RGBA block per output block: extract with clamped edge pixels
         // (NVTT's encoder is far slower and reads out of bounds on partial edge tiles)
         const unsigned int bw = (w + 3) / 4, bh = (h + 3) / 4;
         vector<basist::color_rgba> blockPixels(static_cast<size_t>(bw) * bh * 16);
         basist::color_rgba* bp = blockPixels.data();
         for (unsigned int by = 0; by < bh; by++)
            for (unsigned int bx = 0; bx < bw; bx++)
               for (unsigned int py = 0; py < 4; py++)
                  for (unsigned int px = 0; px < 4; px++)
                  {
                     const uint8_t* s = &src[(min(by * 4 + py, h - 1) * w + min(bx * 4 + px, w - 1)) * 4];
                     bp->r = s[0];
                     bp->g = s[1];
                     bp->b = s[2];
                     bp->a = s[3];
                     bp++;
                  }
         // Encode one row of blocks at a time to be able to abort an ongoing compression
         uint8_t* const dst = result->data.data() + offset;
         for (unsigned int by = 0; by < bh; by++)
         {
            if (isCompressionDiscarded && isCompressionDiscarded())
               return nullptr;
            for (unsigned int bx = 0; bx < bw; bx++)
               basist::bc7f::fast_pack_bc7_auto_rgba(
                  dst + (static_cast<size_t>(by) * bw + bx) * 16, blockPixels.data() + (static_cast<size_t>(by) * bw + bx) * 16, basist::bc7f::cPackBC7FlagDefault & ~basist::bc7f::cPackBC7FlagPBitOptMode6);
         }
      }
      else
         bimg::imageEncodeFromRgba8(&allocator, result->data.data() + offset, src, w, h, 1, static_cast<bimg::TextureFormat::Enum>(format), bimg::Quality::Fastest, &err);
      if (!err.isOk())
      {
         PLOGE << "Failed to compress texture '" << tex.GetName() << "' to " << bimg::getName(static_cast<bimg::TextureFormat::Enum>(format));
         return nullptr;
      }
      offset += result->GetMipSize(mip);
   }
   assert(offset == result->data.size());

   return result;
}

std::shared_ptr<BaseTexture> TextureCompressor::LoadCached(const std::filesystem::path& cacheFile)
{
   const auto start = std::chrono::steady_clock::now();
   CacheHeader header;
   std::shared_ptr<const CompressedTexture> compressed = ReadCacheFile(cacheFile, header);
   if (compressed == nullptr || header.format != static_cast<uint32_t>(SelectFormatFor(static_cast<BaseTexture::Format>(header.srcFormat), header.opaque != 0, header.width, header.height)))
      return nullptr;
   m_nLoaded++;
   m_loadMs += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
   m_rawBytes += (static_cast<uint64_t>(header.width) * header.height * UncompressedPixelSize(static_cast<BaseTexture::Format>(header.srcFormat)) * 4) / 3;
   m_compressedBytes += compressed->data.size();
   return BaseTexture::CreateCompressedOnly(std::move(compressed), static_cast<BaseTexture::Format>(header.srcFormat), header.opaque != 0);
}

std::shared_ptr<const CompressedTexture> TextureCompressor::LoadOrCompress(const BaseTexture& tex, const std::filesystem::path& cacheFile, const std::function<bool()>& isCompressionDiscarded)
{
   if (!IsSupported(tex))
      return nullptr;

   const bgfx::TextureFormat::Enum format = SelectFormatFor(tex.m_format, tex.IsOpaque(), tex.width(), tex.height());
   if (format == bgfx::TextureFormat::Unknown)
      return nullptr;
   const uint64_t rawBytes = (static_cast<uint64_t>(tex.width()) * tex.height() * UncompressedPixelSize(tex.m_format) * 4) / 3;

   if (!cacheFile.empty())
   {
      const auto start = std::chrono::steady_clock::now();
      CacheHeader header;
      std::shared_ptr<const CompressedTexture> result = ReadCacheFile(cacheFile, header);
      if (result && result->format == format && result->width == tex.width() && result->height == tex.height())
      {
         m_nLoaded++;
         m_loadMs += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
         m_rawBytes += rawBytes;
         m_compressedBytes += result->data.size();
         return result;
      }
   }

   const auto start = std::chrono::steady_clock::now();
   auto result = Compress(tex, format, isCompressionDiscarded);
   if (result == nullptr)
      return nullptr;
   m_nCompressed++;
   m_compressMs += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
   m_rawBytes += rawBytes;
   m_compressedBytes += result->data.size();
   PLOGI << std::format("Texture '{}' was compressed ({}x{} {})", tex.GetName(), tex.width(), tex.height(), GetFormatName(format));

   if (!cacheFile.empty())
   {
      // The payload is stored deflated to limit disk space (f.e. VPW Jurassic Park goes from 1.89Go to 0.92Go)
      if (result->data.size() <= std::numeric_limits<mz_ulong>::max())
      {
         const mz_ulong dataSize = static_cast<mz_ulong>(result->data.size());
         mz_ulong packedSize = compressBound(dataSize);
         vector<uint8_t> packed(packedSize);
         if (compress2(packed.data(), &packedSize, result->data.data(), dataSize, MZ_BEST_COMPRESSION) == Z_OK)
         {
            CacheHeader header;
            header.format = static_cast<uint32_t>(result->format);
            header.srcFormat = static_cast<uint32_t>(tex.m_format);
            header.opaque = tex.IsOpaque() ? 1 : 0;
            header.width = result->width;
            header.height = result->height;
            header.numMips = result->numMips;
            header.dataSize = result->data.size();
            header.packedSize = packedSize;
            const std::filesystem::path tmpFile = std::filesystem::path(cacheFile).concat(".tmp");
            {
               std::ofstream out(tmpFile, std::ios::binary | std::ios::trunc);
               out.write(reinterpret_cast<const char*>(&header), sizeof(header));
               out.write(reinterpret_cast<const char*>(packed.data()), static_cast<std::streamsize>(packedSize));
            }
            std::error_code ec;
            std::filesystem::rename(tmpFile, cacheFile, ec);
            if (ec)
	    {
               PLOGE << "Failed to write compressed texture cache " << cacheFile;
	    }
         }
         else
         {
            PLOGE << "Failed to deflate compressed texture cache data for '" << tex.GetName() << '\'';
         }
      }
      else
      {
         PLOGW << "Compressed texture '" << tex.GetName() << "' is too large to be cached";
      }
   }
   return result;
}

TextureCompressor::TextureCompressor(std::filesystem::path cacheFolder, unsigned int maxTexDim)
   : m_cacheFolder(std::move(cacheFolder))
   , m_maxTexDim(maxTexDim)
{
   // Statistics are scoped to the compressor lifetime, so a texture loading session starts with clean stats
}

std::filesystem::path TextureCompressor::GetCacheFile(const Texture* image) const
{
   std::filesystem::path path;
   if (!m_cacheFolder.empty())
   {
      std::stringstream key;
      key << std::hex << std::setfill('0');
      for (int i = 0; i < 16; i++)
         key << std::setw(2) << static_cast<int>(image->GetMD5Hash()[i]);
      key << std::dec;
      // Only downscaled textures have their compressed content depend on the maxTexDim setting
      if (m_maxTexDim > 0 && (image->m_width > m_maxTexDim || image->m_height > m_maxTexDim))
         key << "_MaxTex" << m_maxTexDim;
      key << ".vpxtex";
      path = m_cacheFolder / key.str();
   }
   return path;
}

std::shared_ptr<const BaseTexture> TextureCompressor::Load(Texture* image, bool resizeOnLowMem, const std::function<bool()>& isCompressionDiscarded)
{
   std::shared_ptr<const BaseTexture> buffer;
   const std::filesystem::path cacheFile = GetCacheFile(image);
   if (!cacheFile.empty())
   {
      {
         const std::lock_guard lock(m_usedCacheFilesMutex);
         m_usedCacheFiles.insert(cacheFile.filename());
      }
      if (std::shared_ptr<BaseTexture> cached = LoadCached(cacheFile))
      {
         cached->SetName(image->m_name);
         image->SetIsOpaque(cached->IsOpaque());
         buffer = std::move(cached);
      }
   }
   if (buffer == nullptr)
      buffer = image->GetRawBitmap(resizeOnLowMem, m_maxTexDim);
   if (buffer && buffer->m_compressed == nullptr && IsSupported(*buffer) && (!isCompressionDiscarded || !isCompressionDiscarded()))
      buffer->m_compressed = LoadOrCompress(*buffer, buffer->m_resizedOnLowMem ? std::filesystem::path() : cacheFile, isCompressionDiscarded);
   return buffer;
}

void TextureCompressor::LogStats() const
{
   PLOGI << "Texture compression: " << m_nCompressed << " compressed (" << m_compressMs << "ms cumulated), " << m_nLoaded << " loaded from cache (" << m_loadMs
         << "ms cumulated), GPU memory " << (m_rawBytes / (1024 * 1024)) << "MB uncompressed -> " << (m_compressedBytes / (1024 * 1024)) << "MB compressed";
}

void TextureCompressor::CleanCache()
{
   if (m_cacheFolder.empty())
      return;
   std::unordered_set<std::filesystem::path> expected;
   {
      const std::lock_guard lock(m_usedCacheFilesMutex);
      expected = m_usedCacheFiles;
   }
   uint64_t nRemoved = 0;
   uint64_t freedBytes = 0;
   std::error_code ec;
   for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_cacheFolder, ec))
   {
      const std::filesystem::path& path = entry.path();
      if (!entry.is_regular_file(ec) || (path.extension() != ".tmp" && (path.extension() != ".vpxtex" || expected.contains(path.filename()))))
         continue;
      const uint64_t size = entry.file_size(ec);
      if (std::filesystem::remove(path, ec))
      {
         ++nRemoved;
         freedBytes += size;
      }
   }
   if (nRemoved > 0)
   {
      PLOGI << "Texture cache: removed " << nRemoved << " stale file(s), freeing " << (freedBytes / (1024 * 1024)) << "MB in " << m_cacheFolder;
   }
}

#endif
