#pragma once

#define MIN_TEXTURE_SIZE 8u

#if !defined(_MSC_VER) && !defined(_rotl) //!!
#define _rotl(x,y)  (((x) << (y)) | ((x) >> (-(y) & 31)))
#endif

struct FIBITMAP;

// texture stored in main memory in 24/32bit RGB/RGBA uchar format or 48/96bit RGB float
class BaseTexture final
{
public:
   enum Format
   { // RGB/RGBA formats must be ordered R, G, B (and optionally A)
      BW,        // Linear BW image, 1 byte per texel
      RGB,       // Linear RGB without alpha channel, 1 byte per channel
      RGBA,      // Linear RGB with alpha channel, 1 byte per channel
      SRGB,      // sRGB without alpha channel, 1 byte per channel
      SRGBA,     // sRGB with alpha channel, 1 byte per channel
      RGB_FP16,  // Linear RGB, 1 half float per channel
      RGBA_FP16, // Linear RGB with alpha channel, 1 half float per channel
      RGB_FP32   // Linear RGB, 1 float per channel
   };

   BaseTexture(const unsigned int w, const unsigned int h, const Format format);
   ~BaseTexture();

   unsigned int width() const  { return m_width; }
   unsigned int height() const { return m_height; }
   unsigned int pitch() const  { return (m_format == BW ? 1 : (has_alpha() ? 4 : 3)) * (m_format == RGB_FP32 ? 4 : (m_format == RGB_FP16 || m_format == RGBA_FP16) ? 2 : 1) * m_width; } // pitch in bytes
   BYTE* data()                { return m_data; }
   bool has_alpha() const      { return m_format == RGBA || m_format == SRGBA || m_format == RGBA_FP16; }

   BaseTexture *ToBGRA(); // swap R and B channels, also tonemaps floating point buffers during conversion and adds an opaque alpha channel (if format with missing alpha)
   void AddAlpha();
   void RemoveAlpha();

   unsigned int m_realWidth, m_realHeight;
   Format m_format;

   static BaseTexture *CreateFromHBitmap(const HBITMAP hbm, unsigned int maxTexDim, bool with_alpha = true);
   static BaseTexture *CreateFromFile(const string &filename, unsigned int maxTexDim);
   static BaseTexture *CreateFromData(const void *data, const size_t size, unsigned int maxTexDim);
   static BaseTexture *CreateFromFreeImage(FIBITMAP *dib, bool resize_on_low_mem, unsigned int maxTexDim); // also free's/delete's the dib inside!

   uint8_t* GetMD5Hash() { UpdateMD5(); return m_md5Hash; }
   bool IsOpaque() const { UpdateOpaque(); return m_isOpaque; }
   bool IsSigned() const { return m_isSigned; }

   bool IsMD5HashComputed() const { return !m_isMD5Dirty; }
   bool IsOpaqueComputed() const { return !m_isOpaqueDirty; }
   void SetMD5Hash(uint8_t* md5) { memcpy(m_md5Hash, md5, sizeof(m_md5Hash)); m_isMD5Dirty = false; }
   void SetIsOpaque(const bool v) { m_isOpaque = v; m_isOpaqueDirty = false; }
   void SetIsSigned(const bool v) { m_isSigned = v; }

private:
   void UpdateMD5() const;
   void UpdateOpaque() const;

   const unsigned int m_width, m_height;
   BYTE* m_data;

   // These field are (lazily) computed from the data, therefore they do not impact the constness of the object
   mutable bool m_isSigned = false;
   mutable bool m_isMD5Dirty = true;
   mutable uint8_t m_md5Hash[16];
   mutable bool m_isOpaqueDirty = true;
   mutable bool m_isOpaque = true;
};

class Texture final : public ILoadable
{
public:
   Texture();
   Texture(BaseTexture * const base);
   ~Texture();

   // ILoadable callback
   bool LoadToken(const int id, BiffReader * const pbr) final;

   HRESULT SaveToStream(IStream *pstream, const PinTable *pt);
   HRESULT LoadFromStream(IStream *pstream, int version, PinTable *pt, bool resize_on_low_mem);
   bool LoadFromMemory(BYTE *const data, const DWORD size);
   bool LoadFromFile(const string &filename, const bool setName = false);

   void FreeStuff();

   void CreateGDIVersion();

   BaseTexture *CreateFromHBitmap(const HBITMAP hbm, bool with_alpha = true);

   bool IsHDR() const
   {
      return m_pdsBuffer != nullptr && (m_pdsBuffer->m_format == BaseTexture::RGB_FP16 || m_pdsBuffer->m_format == BaseTexture::RGBA_FP16 || m_pdsBuffer->m_format == BaseTexture::RGB_FP32);
   }

   void SetSizeFrom(const BaseTexture* const tex)
   {
      m_width = tex->width();
      m_height = tex->height();
      m_realWidth = tex->m_realWidth;
      m_realHeight = tex->m_realHeight;
   }

   // create/release a DC which contains a (read-only) copy of the texture; for editor use
   void GetTextureDC(HDC *pdc);
   void ReleaseTextureDC(HDC dc);

private:
   bool m_resize_on_low_mem = true;

public:
   unsigned int m_maxTexDim = 0;
   
   // width and height of texture can be different than width and height
   // of m_pdsBuffer, since the surface can be limited to smaller sizes by the user
   unsigned int m_width = 0, m_height = 0;
   unsigned int m_realWidth = 0, m_realHeight = 0;
   float m_alphaTestValue = (float)(-1.0 / 255.0);
   BaseTexture *m_pdsBuffer = nullptr;

   HBITMAP m_hbmGDIVersion = nullptr; // HBitmap at screen depth and converted/visualized alpha so GDI draws it fast
   PinBinary *m_ppb = nullptr; // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   string m_szName;
   string m_szPath;

private:
   HBITMAP m_oldHBM = nullptr;        // this is to cache the result of SelectObject()
};

template<bool opaque>
inline void copy_bgra_rgba(unsigned int* const __restrict dst, const unsigned int* const __restrict src, const size_t size)
{
    size_t o = 0;

#ifdef ENABLE_SSE_OPTIMIZATIONS
    // align output writes
    for (; ((reinterpret_cast<size_t>(dst+o) & 15) != 0) && o < size; ++o)
    {
       const unsigned int rgba = src[o];
       unsigned int tmp = (_rotl(rgba, 16) & 0x00FF00FFu) | (rgba & 0xFF00FF00u);
       if (opaque)
          tmp |= 0xFF000000u;
       dst[o] = tmp;
    }

    const __m128i brMask  = _mm_set1_epi32(0x00FF00FFu);
    const __m128i alphaFF = _mm_set1_epi32(0xFF000000u);
    for (; o+3 < size; o+=4)
    {
       const __m128i srco = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src+o));
       __m128i ga = _mm_andnot_si128(brMask, srco);    // mask g and a
       if (opaque)
          ga = _mm_or_si128(ga, alphaFF);
       const __m128i br = _mm_and_si128(brMask, srco); // mask b and r
       // swap b and r, then or g and a back again
       const __m128i rb = _mm_shufflehi_epi16(_mm_shufflelo_epi16(br, _MM_SHUFFLE(2, 3, 0, 1)), _MM_SHUFFLE(2, 3, 0, 1));
       _mm_store_si128(reinterpret_cast<__m128i*>(dst+o), _mm_or_si128(ga, rb));
    }
    // leftover writes below
#else
#pragma message ("Warning: No SSE texture conversion")
#endif

    for (; o < size; ++o)
    {
       const unsigned int rgba = src[o];
       unsigned int tmp = (_rotl(rgba, 16) & 0x00FF00FFu) | (rgba & 0xFF00FF00u);
       if (opaque)
          tmp |= 0xFF000000u;
       dst[o] = tmp;
    }
}

template<bool bgr>
inline void copy_rgb_rgba(unsigned int* const __restrict dst, const unsigned char* const __restrict src, const size_t size)
{
#ifdef ENABLE_SSE_OPTIMIZATIONS // actually uses SSSE3
#if !(defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && defined(_MSC_VER)
    static int ssse3_supported = -1;
    if (ssse3_supported == -1)
    {
       int cpuInfo[4];
       __cpuid(cpuInfo, 1);
       ssse3_supported = (cpuInfo[2] & (1 << 9));
    }
#else
    constexpr bool ssse3_supported = true;
#endif
#endif

    size_t o = 0;

#ifdef ENABLE_SSE_OPTIMIZATIONS // actually uses SSSE3
    if (ssse3_supported)
    {
       // align output writes
       if (!bgr)
       {
          for (; ((reinterpret_cast<size_t>(dst + o) & 15) != 0) && o < size; ++o)
             dst[o] = (unsigned int)src[o*3] | ((unsigned int)src[o*3 + 1] << 8) | ((unsigned int)src[o*3 + 2] << 16) | (255u << 24);
       }
       else
          for (; ((reinterpret_cast<size_t>(dst + o) & 15) != 0) && o < size; ++o)
             dst[o] = (unsigned int)src[o*3 + 2] | ((unsigned int)src[o*3 + 1] << 8) | ((unsigned int)src[o*3] << 16) | (255u << 24);

       const __m128i mask  = bgr ? _mm_setr_epi8(2, 1, 0, -1, 5, 4, 3, -1, 8, 7, 6, -1, 11, 10, 9, -1) : _mm_setr_epi8(0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1);
       const __m128i alpha = _mm_set1_epi32(0xFF000000u); // alpha set to be 255
       for (; o+15 < size; o+=16)
       {
          const __m128i c[3] = { _mm_loadu_si128((__m128i *)(src + o*3)), _mm_loadu_si128((__m128i *)(src + o*3 + 16)), _mm_loadu_si128((__m128i *)(src + o*3 + 32)) };
          _mm_store_si128((__m128i *)(dst + o     ), _mm_or_si128(_mm_shuffle_epi8(                           c[0], mask), alpha));
          _mm_store_si128((__m128i *)(dst + o +  4), _mm_or_si128(_mm_shuffle_epi8(_mm_alignr_epi8(c[1], c[0], 12), mask), alpha));
          _mm_store_si128((__m128i *)(dst + o +  8), _mm_or_si128(_mm_shuffle_epi8(_mm_alignr_epi8(c[2], c[1],  8), mask), alpha));
          _mm_store_si128((__m128i *)(dst + o + 12), _mm_or_si128(_mm_shuffle_epi8(_mm_alignr_epi8(c[2], c[2],  4), mask), alpha));
       }
    }
    else
#endif
    {
       for (; o+3 < size; o+=4)
       {
          const unsigned int tmpa = *((unsigned int *)&src[o*3]);
          const unsigned int tmpb = *((unsigned int *)&src[o*3 + 4]);
          const unsigned int tmpc = *((unsigned int *)&src[o*3 + 8]);

          const unsigned tmp0 = (tmpa & 0xFFFFFFu) | (255u << 24);
          const unsigned tmp1 = (((tmpa >> 24) | (tmpb << 8)) & 0xFFFFFFu) | (255u << 24);
          const unsigned tmp2 = (((tmpb >> 16) | (tmpc << 16)) & 0xFFFFFFu) | (255u << 24);
          const unsigned tmp3 = (tmpc >> 8) | (255u << 24);

          if (bgr)
          {
             dst[o    ] = (_rotl(tmp0, 16) & 0x00FF00FFu) | (tmp0 & 0xFF00FF00u);
             dst[o + 1] = (_rotl(tmp1, 16) & 0x00FF00FFu) | (tmp1 & 0xFF00FF00u);
             dst[o + 2] = (_rotl(tmp2, 16) & 0x00FF00FFu) | (tmp2 & 0xFF00FF00u);
             dst[o + 3] = (_rotl(tmp3, 16) & 0x00FF00FFu) | (tmp3 & 0xFF00FF00u);
          }
          else
          {
             dst[o    ] = tmp0;
             dst[o + 1] = tmp1;
             dst[o + 2] = tmp2;
             dst[o + 3] = tmp3;
          }
       }
    }

    if (!bgr)
    {
       for (; o < size; ++o)
          dst[o] = (unsigned int)src[o*3] | ((unsigned int)src[o*3 + 1] << 8) | ((unsigned int)src[o*3 + 2] << 16) | (255u << 24);
    }
    else
       for (; o < size; ++o)
          dst[o] = (unsigned int)src[o*3 + 2] | ((unsigned int)src[o*3 + 1] << 8) | ((unsigned int)src[o*3] << 16) | (255u << 24);
}

inline void copy_bgr_rgb(unsigned char* const __restrict dst, const unsigned char* const __restrict src, const size_t size)
{
#ifdef ENABLE_SSE_OPTIMIZATIONS // actually uses SSSE3
#if !(defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && defined(_MSC_VER)
    static int ssse3_supported = -1;
    if (ssse3_supported == -1)
    {
       int cpuInfo[4];
       __cpuid(cpuInfo, 1);
       ssse3_supported = (cpuInfo[2] & (1 << 9));
    }
#else
    constexpr bool ssse3_supported = true;
#endif
#endif

    size_t o = 0;

#ifdef ENABLE_SSE_OPTIMIZATIONS // actually uses SSSE3
    if (ssse3_supported)
    {
       // align output writes
       for (; ((reinterpret_cast<size_t>(dst + o*3) & 15) != 0) && o < size; ++o)
       {
          dst[o*3    ] = src[o*3 + 2];
          dst[o*3 + 1] = src[o*3 + 1];
          dst[o*3 + 2] = src[o*3 + 0];
       }

       const __m128i mask0a = _mm_setr_epi8(2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, -1);
       const __m128i mask0b = _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1);
       const __m128i mask1a = _mm_setr_epi8(-1, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
       const __m128i mask1b = _mm_setr_epi8(0, -1, 4, 3, 2, 7, 6, 5, 10, 9, 8, 13, 12, 11, -1, 15);
       const __m128i mask1c = _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1);
       const __m128i mask2a = _mm_setr_epi8(14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
       const __m128i mask2b = _mm_setr_epi8(-1, 3, 2, 1, 6, 5, 4, 9, 8, 7, 12, 11, 10, 15, 14, 13);
       for (; o+15 < size; o+=16)
       {
          const __m128i c[3] = { _mm_loadu_si128((__m128i *)(src + o*3)), _mm_loadu_si128((__m128i *)(src + o*3 + 16)), _mm_loadu_si128((__m128i *)(src + o*3 + 32)) };
          _mm_store_si128((__m128i *)(dst + o*3     ), _mm_or_si128(_mm_shuffle_epi8(c[0], mask0a), _mm_shuffle_epi8(c[1], mask0b)));
          _mm_store_si128((__m128i *)(dst + o*3 + 16), _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(c[0], mask1a), _mm_shuffle_epi8(c[1], mask1b)), _mm_shuffle_epi8(c[2], mask1c)));
          _mm_store_si128((__m128i *)(dst + o*3 + 32), _mm_or_si128(_mm_shuffle_epi8(c[1], mask2a), _mm_shuffle_epi8(c[2], mask2b)));
       }
    }
#endif

    for (; o < size; ++o)
    {
       dst[o*3    ] = src[o*3 + 2];
       dst[o*3 + 1] = src[o*3 + 1];
       dst[o*3 + 2] = src[o*3 + 0];
    }
}

inline void float2half(unsigned short* const __restrict dst, const float* const __restrict src, const size_t size)
{
    size_t o = 0;

#if !(defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) || defined(__RPI__) || defined(__RK3588__)
#ifdef ENABLE_SSE_OPTIMIZATIONS
    // align output writes
    for (; ((reinterpret_cast<size_t>(dst+o) & 15) != 0) && o < size; ++o)
       dst[o] = float2half_noLUT(src[o]);

    // see https://gist.github.com/rygorous/2156668, extended to 2 values/loop-iteration
    const __m128 mabs = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
    const __m128 f32infty = _mm_castsi128_ps(_mm_set1_epi32(255 << 23));
    const __m128 expinf = _mm_castsi128_ps(_mm_set1_epi32((255 ^ 31) << 23));
    const __m128 f16max = _mm_castsi128_ps(_mm_set1_epi32((127 + 16) << 23));
    const __m128 magic = _mm_castsi128_ps(_mm_set1_epi32(15 << 23));

    for (; o+7 < size; o+=8)
    {
       const __m128 srco[2] = { _mm_loadu_ps(src + o), _mm_loadu_ps(src + o + 4) };

       const __m128 fabs[2] = { _mm_and_ps(mabs, srco[0]), _mm_and_ps(mabs, srco[1]) };
       const __m128 justsign[2] = { _mm_andnot_ps(mabs, srco[0]), _mm_andnot_ps(mabs, srco[1]) };

       const __m128 infnancase[2] = { _mm_xor_ps(expinf, fabs[0]), _mm_xor_ps(expinf, fabs[1]) };
       const __m128 clamped[2] = { _mm_min_ps(f16max, fabs[0]), _mm_min_ps(f16max, fabs[1]) };
       const __m128 b_notnormal[2] = { _mm_cmpnlt_ps(fabs[0], f32infty), _mm_cmpnlt_ps(fabs[1], f32infty) };
       const __m128 scaled[2] = { _mm_mul_ps(clamped[0], magic), _mm_mul_ps(clamped[1], magic) };

       const __m128 merge1[2] = { _mm_and_ps(b_notnormal[0], infnancase[0]), _mm_and_ps(b_notnormal[1], infnancase[1]) };
       const __m128 merge2[2] = { _mm_andnot_ps(b_notnormal[0], scaled[0]), _mm_andnot_ps(b_notnormal[1], scaled[1]) };
       const __m128 merged[2] = { _mm_or_ps(merge1[0], merge2[0]), _mm_or_ps(merge1[1], merge2[1]) };

       const __m128i shifted[2] = { _mm_slli_epi32(_mm_castps_si128(merged[0]), 3), _mm_slli_epi32(_mm_castps_si128(merged[1]), 3) };
       __m128i final[2] = { _mm_or_si128(shifted[0], _mm_castps_si128(justsign[0])), _mm_or_si128(shifted[1], _mm_castps_si128(justsign[1])) };

       final[0] = _mm_shufflelo_epi16(final[0], _MM_SHUFFLE(3, 1, 3, 1));
       final[1] = _mm_shufflelo_epi16(final[1], _MM_SHUFFLE(3, 1, 3, 1));
       final[0] = _mm_shufflehi_epi16(final[0], _MM_SHUFFLE(3, 1, 3, 1));
       final[1] = _mm_shufflehi_epi16(final[1], _MM_SHUFFLE(3, 1, 3, 1));
       const __m128 finalc = _mm_shuffle_ps(_mm_castsi128_ps(final[0]), _mm_castsi128_ps(final[1]), _MM_SHUFFLE(2, 0, 2, 0));
       _mm_store_ps((float*)(dst+o), finalc);
    }
    // leftover writes below
#else
#pragma message ("Warning: No SSE texture conversion")
#endif
#endif

    for (; o < size; ++o)
       dst[o] = float2half_noLUT(src[o]);
}

inline void float2half_noF16MaxInfNaN(unsigned short* const __restrict dst, const float* const __restrict src, const size_t size)
{
    size_t o = 0;

#if !(defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) || defined(__RPI__) || defined(__RK3588__)
#ifdef ENABLE_SSE_OPTIMIZATIONS
    // align output writes
    for (; ((reinterpret_cast<size_t>(dst+o) & 15) != 0) && o < size; ++o)
       dst[o] = float2half_noLUT(src[o]);

    // see https://gist.github.com/rygorous/2156668, extended to 2 values/loop-iteration, removed f16max/inf/nan handling
    const __m128 sign = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
    const __m128 magic = _mm_castsi128_ps(_mm_set1_epi32(15 << 23));

    for (; o+7 < size; o+=8)
    {
       const __m128 srco[2] = { _mm_loadu_ps(src + o), _mm_loadu_ps(src + o + 4) };

       const __m128 scaled[2] = { _mm_mul_ps(srco[0], magic), _mm_mul_ps(srco[1], magic) };
       const __m128 justsign[2] = { _mm_and_ps(srco[0], sign), _mm_and_ps(srco[1], sign) };
       const __m128i shifted[2] = { _mm_slli_epi32(_mm_castps_si128(scaled[0]), 3), _mm_slli_epi32(_mm_castps_si128(scaled[1]), 3) };
       __m128i final[2] = { _mm_or_si128(shifted[0], _mm_castps_si128(justsign[0])), _mm_or_si128(shifted[1], _mm_castps_si128(justsign[1])) };

       final[0] = _mm_shufflelo_epi16(final[0], _MM_SHUFFLE(3, 1, 3, 1));
       final[1] = _mm_shufflelo_epi16(final[1], _MM_SHUFFLE(3, 1, 3, 1));
       final[0] = _mm_shufflehi_epi16(final[0], _MM_SHUFFLE(3, 1, 3, 1));
       final[1] = _mm_shufflehi_epi16(final[1], _MM_SHUFFLE(3, 1, 3, 1));
       const __m128 finalc = _mm_shuffle_ps(_mm_castsi128_ps(final[0]), _mm_castsi128_ps(final[1]), _MM_SHUFFLE(2, 0, 2, 0));
       _mm_store_ps((float*)(dst+o), finalc);
    }
    // leftover writes below
#else
#pragma message ("Warning: No SSE texture conversion")
#endif
#endif

    for (; o < size; ++o)
       dst[o] = float2half_noLUT(src[o]);
}

inline void float2half_pos_noF16MaxInfNaN(unsigned short* const __restrict dst, const float* const __restrict src, const size_t size)
{
    size_t o = 0;

#if !(defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) || defined(__RPI__) || defined(__RK3588__)
#ifdef ENABLE_SSE_OPTIMIZATIONS
    // align output writes
    for (; ((reinterpret_cast<size_t>(dst+o) & 15) != 0) && o < size; ++o)
       dst[o] = float2half_noLUT(src[o]);

    // see https://gist.github.com/rygorous/2156668, extended to 2 values/loop-iteration, removed signed/f16max/inf/nan handling
    const __m128 magic = _mm_castsi128_ps(_mm_set1_epi32(15 << 23));

    for (; o+7 < size; o+=8)
    {
       const __m128 srco[2] = { _mm_loadu_ps(src + o), _mm_loadu_ps(src + o + 4) };

       const __m128 scaled[2] = { _mm_mul_ps(srco[0], magic), _mm_mul_ps(srco[1], magic) };
       __m128i final[2] = { _mm_slli_epi32(_mm_castps_si128(scaled[0]), 3), _mm_slli_epi32(_mm_castps_si128(scaled[1]), 3) };

       final[0] = _mm_shufflelo_epi16(final[0], _MM_SHUFFLE(3, 1, 3, 1));
       final[1] = _mm_shufflelo_epi16(final[1], _MM_SHUFFLE(3, 1, 3, 1));
       final[0] = _mm_shufflehi_epi16(final[0], _MM_SHUFFLE(3, 1, 3, 1));
       final[1] = _mm_shufflehi_epi16(final[1], _MM_SHUFFLE(3, 1, 3, 1));
       const __m128 finalc = _mm_shuffle_ps(_mm_castsi128_ps(final[0]), _mm_castsi128_ps(final[1]), _MM_SHUFFLE(2, 0, 2, 0));
       _mm_store_ps((float*)(dst+o), finalc);
    }
    // leftover writes below
#else
#pragma message ("Warning: No SSE texture conversion")
#endif
#endif

    for (; o < size; ++o)
       dst[o] = float2half_noLUT(src[o]);
}

inline Vertex2D min_max(const float* const __restrict src, const size_t size)
{
    Vertex2D minmax(FLT_MAX,-FLT_MAX);
    size_t o = 0;

#ifdef ENABLE_SSE_OPTIMIZATIONS
    // align
    for (; ((reinterpret_cast<size_t>(src+o) & 15) != 0) && o < size; ++o)
    {
       const float f = src[o];
       if (f < minmax.x)
          minmax.x = f;
       if (f > minmax.y)
          minmax.y = f;
    }

    __m128 min128 = _mm_set1_ps(minmax.x);
    __m128 max128 = _mm_set1_ps(minmax.y);
    for (; o + 3 < size; o += 4)
    {
       const __m128 f = _mm_load_ps(src+o);
       min128 = _mm_min_ps(min128, f);
       max128 = _mm_max_ps(max128, f);
    }

    minmax.x = _mm_cvtss_f32(sseHorizontalMin(min128));
    minmax.y = _mm_cvtss_f32(sseHorizontalMax(max128));
    // leftovers below
#else
#pragma message ("Warning: No SSE texture conversion")
#endif

    for (; o < size; ++o)
    {
       const float f = src[o];
       if (f < minmax.x)
          minmax.x = f;
       if (f > minmax.y)
          minmax.y = f;
    }

    return minmax;
}
