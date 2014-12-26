#pragma once

#define NOTRANSCOLOR  RGB(123,123,123)

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

   void CreateAlphaChannel();
   void FreeStuff();
   void SetTransparentColor(const COLORREF color);

   void Set(DWORD textureChannel);
   void Unset(DWORD textureChannel);

   void Release();
   void EnsureHBitmap();
   void CreateGDIVersion();

   void CreateTextureOffscreen(const int width, const int height);
   BaseTexture *CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);
   void CreateFromResource(const int id, int * const pwidth = NULL, int * const pheight = NULL);

   void SetAlpha(const COLORREF rgbTransparent);

   static void SetOpaque(BaseTexture* pdds);
   static bool SetAlpha(BaseTexture* pdds, const COLORREF rgbTransparent);

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
   BaseTexture* m_pdsBufferColorKey;

   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth so GDI draws it fast
   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

   COLORREF m_rgbTransparent; // if NOTRANSCOLOR then no transparency in picture

private:
   HBITMAP m_oldHBM;        // this is to cache the result of SelectObject()
};
