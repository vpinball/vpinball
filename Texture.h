#pragma once

#define MIN_TEXTURE_SIZE 8u

struct FIBITMAP;

// texture stored in main memory in 24/32bit RGB/RGBA uchar format or 48/96bit RGB float
class BaseTexture
{
public:
   enum Format
   { // RGB/RGBA formats must be ordered R, G, B (and eventually A)
      RGB,			// Linear RGB without alpha channel, 1 byte per channel
      RGBA,			// Linear RGB with alpha channel, 1 byte per channel
      SRGB,			// sRGB without alpha channel, 1 byte per channel
      SRGBA,		// sRGB with alpha channel, 1 byte per channel
      RGB_FP16,		// Linear RGB, 1 half float per channel
      RGB_FP32		// Linear RGB, 1 float per channel
   };

   BaseTexture(const unsigned int w, const unsigned int h, const Format format)
      : m_width(w), m_height(h), m_data((format == RGBA || format == SRGBA ? 4 : 3) * (format == RGB_FP32 ? 4 : format == RGB_FP16 ? 2 : 1) * w * h), m_realWidth(w), m_realHeight(h), m_format(format)
   { }

   unsigned int width() const  { return m_width; }
   unsigned int height() const { return m_height; }
   unsigned int pitch() const  { return (has_alpha() ? 4 : 3) * (m_format == RGB_FP32 ? 4 : m_format == RGB_FP16 ? 2 : 1) * m_width; } // pitch in bytes
   BYTE* data()                { return m_data.data(); }
   bool has_alpha() const      { return m_format == RGBA || m_format == SRGBA; }

   BaseTexture *ToBGRA();

private:
   unsigned int m_width, m_height;
   std::vector<BYTE> m_data;

public:
   unsigned int m_realWidth, m_realHeight;
   Format m_format;

   static BaseTexture *CreateFromHBitmap(const HBITMAP hbm, bool with_alpha = true);
   static BaseTexture *CreateFromFile(const string& filename);
   static BaseTexture *CreateFromFreeImage(FIBITMAP *dib, bool resize_on_low_mem); // also free's/delete's the dib inside!
   static BaseTexture *CreateFromData(const void *data, const size_t size);
};

class Texture : public ILoadable
{
public:
   Texture();
   Texture(BaseTexture * const base);
   virtual ~Texture();

   // ILoadable callback
   virtual bool LoadToken(const int id, BiffReader * const pbr);

   HRESULT SaveToStream(IStream *pstream, const PinTable *pt);
   HRESULT LoadFromStream(IStream *pstream, int version, PinTable *pt, bool resize_on_low_mem);

   void FreeStuff();

   void CreateGDIVersion();

   BaseTexture *CreateFromHBitmap(const HBITMAP hbm, bool with_alpha = true);
   void CreateFromResource(const int id);

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
   bool LoadFromMemory(BYTE * const data, const DWORD size);

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
