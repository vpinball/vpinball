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
   ~BaseTexture() = default;

   unsigned int width() const  { return m_width; }
   unsigned int height() const { return m_height; }
   unsigned int pitch() const  { return (m_format == BW ? 1 : (has_alpha() ? 4 : 3)) * (m_format == RGB_FP32 ? 4 : (m_format == RGB_FP16 || m_format == RGBA_FP16) ? 2 : 1) * m_width; } // pitch in bytes
   BYTE* data()                { return m_data.data(); }
   bool has_alpha() const      { return m_format == RGBA || m_format == SRGBA || m_format == RGBA_FP16; }

   BaseTexture *ToBGRA(); // swap R and B channels, also tonemaps floating point buffers during conversion and adds an opaque alpha channel (if format with missing alpha)
   void AddAlpha();
   void RemoveAlpha();

private:
   unsigned int m_width, m_height;
   vector<BYTE> m_data;

public:
   unsigned int m_realWidth, m_realHeight;
   Format m_format;

   static BaseTexture *CreateFromHBitmap(const HBITMAP hbm, bool with_alpha = true);
   static BaseTexture *CreateFromFile(const string& filename);
   static BaseTexture *CreateFromFreeImage(FIBITMAP *dib, bool resize_on_low_mem); // also free's/delete's the dib inside!
   static BaseTexture *CreateFromData(const void *data, const size_t size);
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

   void FreeStuff();

   void CreateGDIVersion();

   BaseTexture *CreateFromHBitmap(const HBITMAP hbm, bool with_alpha = true);
   void CreateFromResource(const int id);

#ifdef __STANDALONE__
   void CreateFromResource(const string szName);
#endif

   bool IsHDR() const
   {
      return m_pdsBuffer != nullptr && (m_pdsBuffer->m_format == BaseTexture::RGB_FP16 || m_pdsBuffer->m_format == BaseTexture::RGB_FP32);
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
   bool m_resize_on_low_mem;

public:

   // width and height of texture can be different than width and height
   // of m_pdsBuffer, since the surface can be limited to smaller sizes by the user
   unsigned int m_width, m_height;
   unsigned int m_realWidth, m_realHeight;
   float m_alphaTestValue;
   BaseTexture* m_pdsBuffer;

   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth and converted/visualized alpha so GDI draws it fast
   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   string m_szName;
   string m_szPath;

private:
   HBITMAP m_oldHBM;        // this is to cache the result of SelectObject()
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
