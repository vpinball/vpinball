#pragma once

#define MIN_TEXTURE_SIZE 8

struct FIBITMAP;

// texture stored in main memory in 32bit ARGB uchar format or 96bit RGB float
class BaseTexture
{
public:
   enum Format
   {
      RGBA,
      RGB_FP
   };

   BaseTexture()
      : m_width(0), m_height(0), m_realWidth(0), m_realHeight(0), m_format(RGBA), m_has_alpha(false)
   { }

   BaseTexture(const int w, const int h, const Format format, const bool has_alpha)
      : m_width(w), m_height(h), m_data((format == RGBA ? 4 : 3*4) * (w*h)), m_realWidth(w), m_realHeight(h), m_format(format), m_has_alpha(has_alpha)
   { }

   int width() const   { return m_width; }
   int height() const  { return m_height; }
   int pitch() const   { return (m_format == RGBA ? 4 : 3*4) * m_width; } // pitch in bytes
   BYTE* data()        { return m_data.data(); }

private:
   int m_width;
   int m_height;

public:
   std::vector<BYTE> m_data;
   int m_realWidth, m_realHeight;
   Format m_format;
   bool m_has_alpha;

   bool Needs_ConvertAlpha_Tonemap() const { return (m_format == RGB_FP) || ((m_format == RGBA) && m_has_alpha); }

   void CopyTo_ConvertAlpha_Tonemap(BYTE* const __restrict bits) const // premultiplies alpha (as Win32 AlphaBlend() wants it like that) OR converts rgb_fp format to 32bits
   {
     if(m_format == RGB_FP) // Tonemap for 8bpc-Display
     {
        const float * const __restrict src = (float*)m_data.data();
        unsigned int o = 0;
        for (int j = 0; j < m_height; ++j)
			  for (int i = 0; i < m_width; ++i, ++o)
			  {
				  const float r = src[o * 3];
				  const float g = src[o * 3 + 1];
				  const float b = src[o * 3 + 2];
				  const float l = r*0.176204f + g*0.812985f + b*0.0108109f;
				  const float n = (l*(float)(255.*0.25) + 255.0f) / (l + 1.0f); // simple tonemap and scale by 255, overflow is handled by clamp below
				  ((DWORD*)bits)[o] =  (int)clamp(b*n, 0.f, 255.f)      |
				                      ((int)clamp(g*n, 0.f, 255.f)<< 8) |
				                      ((int)clamp(r*n, 0.f, 255.f)<<16) |
				                      (                     255u  <<24);
			  }
      }
	  else
	  {
          if (!m_has_alpha)
              memcpy(bits, m_data.data(), m_height * pitch());
          else
		  if (GetWinVersion() >= 2600) // For everything newer than Windows XP: use the alpha in the bitmap, thus RGB needs to be premultiplied with alpha, due to how AlphaBlend() works
		  {
			  unsigned int o = 0;
			  for (int j = 0; j < m_height; ++j)
				  for (int i = 0; i < m_width; ++i, ++o)
				  {
					  const unsigned int src = ((DWORD*)m_data.data())[o];
					  const unsigned int alpha = src>>24;
					  if (alpha == 0) // adds a checkerboard where completely transparent (for the image manager display)
					  {
						  const DWORD c = ((((i >> 4) ^ (j >> 4)) & 1) << 7) + 127;
						  ((DWORD*)bits)[o] = c | (c<<8) | (c<<16) | (0<<24);
					  }
					  else if (alpha != 255) // premultiply alpha for win32 AlphaBlend()
					  {
						  ((DWORD*)bits)[o] =  (( (src     &0xFF) * alpha) >> 8)      |
						                      (((((src>> 8)&0xFF) * alpha) >> 8)<< 8) |
						                      (((((src>>16)&0xFF) * alpha) >> 8)<<16) |
						                      (                           alpha <<24);
					  }
					  else
						  ((DWORD*)bits)[o] = src;
				  }
		  }
		  else // adds a checkerboard pattern where alpha is set to output bits
		  {
			  unsigned int o = 0;
			  for (int j = 0; j < m_height; ++j)
				  for (int i = 0; i < m_width; ++i, ++o)
				  {
					  const unsigned int src = ((DWORD*)m_data.data())[o];
					  const unsigned int alpha = src>>24;
					  if (alpha != 255)
					  {
						  const unsigned int c = (((((i >> 4) ^ (j >> 4)) & 1) << 7) + 127) * (255 - alpha);
						  ((DWORD*)bits)[o] =  (( (src     &0xFF) * alpha + c) >> 8)      |
						                      (((((src>> 8)&0xFF) * alpha + c) >> 8)<< 8) |
						                      (((((src>>16)&0xFF) * alpha + c) >> 8)<<16) |
						                      (                               alpha <<24);
					  }
					  else
						  ((DWORD*)bits)[o] = src;
				  }
		  }
	  }
   }

   static BaseTexture *CreateFromHBitmap(const HBITMAP hbm);
   static BaseTexture *CreateFromFile(const string& filename);
   static BaseTexture *CreateFromFreeImage(FIBITMAP* dib); // also free's/delete's the dib inside!
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

   HRESULT SaveToStream(IStream *pstream, PinTable *pt);
   HRESULT LoadFromStream(IStream *pstream, int version, PinTable *pt);

   void FreeStuff();

   void CreateGDIVersion();

   BaseTexture *CreateFromHBitmap(const HBITMAP hbm);
   void CreateFromResource(const int id);

   bool IsHDR() const
   {
       if (m_pdsBuffer == NULL)
           return false;
       else
           return (m_pdsBuffer->m_format == BaseTexture::RGB_FP);
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
   bool LoadFromMemory(BYTE * const data, const DWORD size);

public:

   // width and height of texture can be different than width and height
   // of m_pdsBuffer, since the surface can be limited to smaller sizes by the user
   int m_width, m_height;
   int m_realWidth, m_realHeight;
   float m_alphaTestValue;
   BaseTexture* m_pdsBuffer;

   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth and converted/visualized alpha so GDI draws it fast
   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   string m_szName;
   string m_szPath;

private:
   HBITMAP m_oldHBM;        // this is to cache the result of SelectObject()
};
