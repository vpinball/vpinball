#pragma once

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
      : m_width(0), m_height(0), m_realWidth(0), m_realHeight(0), m_format(RGBA)
   { }

   BaseTexture(const int w, const int h, const Format format = RGBA)
      : m_width(w), m_height(h), m_realWidth(w), m_realHeight(h), m_format(format), m_data((format == RGBA ? 4 : 3*4) * (w*h))
   { }

   int width() const   { return m_width; }
   int height() const  { return m_height; }
   int pitch() const   { return (m_format == RGBA ? 4 : 3*4) * m_width; } // pitch in bytes
   BYTE* data()        { return m_data.data(); }

   int m_width;
   int m_height;
   int m_realWidth, m_realHeight;
   Format m_format;
   std::vector<BYTE> m_data;

   void CopyFrom_Raw(const void* bits)  // copy bits which are already in the right format
   {
      memcpy(data(), bits, m_data.size());
   }

   void CopyTo_ConvertAlpha(BYTE* const bits) // premultiplies alpha (as Win32 AlphaBlend() wants it like that) OR converts rgb_fp format to 32bits
   {
     if(m_format == RGB_FP) // Tonemap for 8bpc-Display
     {
        unsigned int o = 0;
        for (int j = 0; j < m_height; ++j)
			  for (int i = 0; i < m_width; ++i, ++o)
			  {
				  const float r = ((float*)m_data.data())[o * 3];
				  const float g = ((float*)m_data.data())[o * 3 + 1];
				  const float b = ((float*)m_data.data())[o * 3 + 2];
				  const float l = r*0.176204f + g*0.812985f + b*0.0108109f;
				  const float n = (l*0.25f + 1.0f) / (l + 1.0f); // overflow is handled by clamp
				  bits[o * 4    ] = (BYTE)(clamp(b*n, 0.f, 1.f) * 255.f);
				  bits[o * 4 + 1] = (BYTE)(clamp(g*n, 0.f, 1.f) * 255.f);
				  bits[o * 4 + 2] = (BYTE)(clamp(r*n, 0.f, 1.f) * 255.f);
				  bits[o * 4 + 3] = 255;
			  }
      }
	  else
	  {
		  if (GetWinVersion() >= 2600) // For everything newer than Windows XP: use the alpha in the bitmap, thus RGB needs to be premultiplied with alpha, due to how AlphaBlend() works
		  {
			  unsigned int o = 0;
			  for (int j = 0; j < m_height; ++j)
				  for (int i = 0; i < m_width; ++i, ++o)
				  {
					  const unsigned int alpha = m_data[o * 4 + 3];
					  if (alpha == 0) // adds a checkerboard where completely transparent (for the image manager display)
					  {
						  const BYTE c = ((((i >> 4) ^ (j >> 4)) & 1) << 7) + 127;
						  bits[o * 4    ] = c;
						  bits[o * 4 + 1] = c;
						  bits[o * 4 + 2] = c;
						  bits[o * 4 + 3] = 0;
					  }
					  else if (alpha != 255) // premultiply alpha for win32 AlphaBlend()
					  {
						  bits[o * 4    ] = ((unsigned int)m_data[o * 4    ] * alpha) >> 8;
						  bits[o * 4 + 1] = ((unsigned int)m_data[o * 4 + 1] * alpha) >> 8;
						  bits[o * 4 + 2] = ((unsigned int)m_data[o * 4 + 2] * alpha) >> 8;
						  bits[o * 4 + 3] = alpha;
					  }
					  else
						  ((DWORD*)bits)[o] = ((DWORD*)m_data.data())[o];
				  }
		  }
		  else // adds a checkerboard pattern where alpha is set to output bits
		  {
			  unsigned int o = 0;
			  for (int j = 0; j < m_height; ++j)
				  for (int i = 0; i < m_width; ++i, ++o)
				  {
					  const unsigned int alpha = m_data[o * 4 + 3];
					  if (alpha != 255)
					  {
						  const unsigned int c = (((((i >> 4) ^ (j >> 4)) & 1) << 7) + 127) * (255 - alpha);
						  bits[o * 4    ] = ((unsigned int)m_data[o * 4    ] * alpha + c) >> 8;
						  bits[o * 4 + 1] = ((unsigned int)m_data[o * 4 + 1] * alpha + c) >> 8;
						  bits[o * 4 + 2] = ((unsigned int)m_data[o * 4 + 2] * alpha + c) >> 8;
						  bits[o * 4 + 3] = alpha;
					  }
					  else
						  ((DWORD*)bits)[o] = ((DWORD*)m_data.data())[o];
				  }
		  }
	  }
   }

   static BaseTexture *CreateFromHBitmap(HBITMAP hbm);
   static BaseTexture *CreateFromFile(const char *filename);
   static BaseTexture *CreateFromFreeImage(FIBITMAP* dib);
};

class Texture : public ILoadable
{
public:
   Texture();
   virtual ~Texture();

   // ILoadable callback
   virtual BOOL LoadToken(int id, BiffReader *pbr);

   HRESULT SaveToStream(IStream *pstream, PinTable *pt);
   HRESULT LoadFromStream(IStream *pstream, int version, PinTable *pt);

   void FreeStuff();

   void EnsureHBitmap();
   void CreateGDIVersion();

   void CreateTextureOffscreen(const int width, const int height);
   BaseTexture *CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);
   void CreateFromResource(const int id, int * const pwidth = NULL, int * const pheight = NULL);

   static void SetOpaque(BaseTexture* const pdds);

   bool IsHDR() const
   {
       if(m_pdsBuffer == NULL)
	   return false;
       else
	   return (m_pdsBuffer->m_format == BaseTexture::RGB_FP);
   }

   // create/release a DC which contains a (read-only) copy of the texture; for editor use
   void GetTextureDC(HDC *pdc);
   void ReleaseTextureDC(HDC dc);

private:
   bool LoadFromMemory(BYTE *data, DWORD size);

   void SetSizeFrom(const BaseTexture* const tex)
   {
      m_width = tex->width();
      m_height = tex->height();
      m_realWidth = tex->m_realWidth;
      m_realHeight = tex->m_realHeight;
   }

public:

   // width and height of texture can be different than width and height
   // of m_pdsBuffer, since the surface can be limited to smaller sizes by the user
   int m_width, m_height;
   int m_realWidth, m_realHeight;
   float m_alphaTestValue;
   BaseTexture* m_pdsBuffer;

   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth and converted/visualized alpha so GDI draws it fast
   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

private:
   HBITMAP m_oldHBM;        // this is to cache the result of SelectObject()
};
