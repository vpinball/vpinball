#pragma once

struct FIBITMAP;
class RenderDevice;

// texture stored in main memory in 32bit ARGB format
struct MemTexture
{
public:
    MemTexture()
        : m_width(0), m_height(0)
    { }

    MemTexture(int w, int h)
        : m_width(w), m_height(h), m_data(4*w*h)
    { }

    int width() const   { return m_width; }
    int height() const  { return m_height; }
    int pitch() const   { return 4*m_width; }
    BYTE* data()        { return &m_data[0]; }

    int m_width;
    int m_height;
    std::vector<BYTE> m_data;

    void CopyBits(const void* bits)      // copy bits which are already in the right format
    { memcpy( data(), bits, m_data.size() ); }

	void CopyTo_ConvertAlpha(void* bits) // adds checkerboard pattern where alpha is set to output bits
	{
		unsigned int o = 0;
		for(unsigned int j = 0; j < m_height; ++j)
		for(unsigned int i = 0; i < m_width; ++i,++o)
		{
			if(m_data[o*4+3] != 0xFF)
			{
				const BYTE c = (((i>>4)^(j>>4))&1)*128+127;
				((BYTE*)bits)[o*4  ] = c;
				((BYTE*)bits)[o*4+1] = c;
				((BYTE*)bits)[o*4+2] = c;
				((BYTE*)bits)[o*4+3] = 0xFF;
			}
			else
				((DWORD*)bits)[o] = ((DWORD*)data())[o];
		}
	}

    static MemTexture *CreateFromHBitmap(HBITMAP hbm);
    static MemTexture *CreateFromFile(const char *filename);
    static MemTexture *CreateFromFreeImage(FIBITMAP* dib);
};

typedef struct MemTexture BaseTexture;

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

   void Set(const DWORD textureChannel);
   void Unset(const DWORD textureChannel);

   void Release();
   void EnsureHBitmap();
   void CreateGDIVersion();

   void CreateTextureOffscreen(const int width, const int height);
   BaseTexture *CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);
   void CreateFromResource(const int id, int * const pwidth = NULL, int * const pheight = NULL);

   static void SetOpaque(BaseTexture* const pdds);

   // create/release a DC which contains a (read-only) copy of the texture; for editor use
   void GetTextureDC(HDC *pdc);
   void ReleaseTextureDC(HDC dc);

private:
   bool LoadFromMemory(BYTE *data, DWORD size);

   void SetSizeFrom(MemTexture* tex)
   {
      m_width = tex->width();
      m_height = tex->height();
   }

public:

   // width and height of texture can be different than width and height
   // of dd surface, since the surface can be limited to smaller sizes by the user
   int m_width, m_height;

   BaseTexture* m_pdsBuffer;

   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth so GDI draws it fast
   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

private:
   HBITMAP m_oldHBM;        // this is to cache the result of SelectObject()
};
