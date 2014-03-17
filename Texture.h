#include "stdafx.h"
#pragma once

#ifdef RGB
#undef RGB
#endif
#define BGR(b,g,r) ((COLORREF)(((DWORD)(b)) | (((DWORD)(g))<<8) | (((DWORD)(r))<<16)))
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

#define NOTRANSCOLOR  RGB(123,123,123)

static inline D3DCOLOR COLORREF_to_D3DCOLOR(COLORREF c)
{
    // COLORREF: 0x00BBGGRR
    // D3DCOLOR: 0xAARRGGBB
    const COLORREF r = (c & 0x000000ff);
    const COLORREF g = (c & 0x0000ff00) >> 8;
    const COLORREF b = (c & 0x00ff0000) >> 16;

    return b | (g << 8) | (r << 16) | 0xff000000;
}


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
   void EnsureBackdrop(const COLORREF color);
   void FreeStuff();
   void SetTransparentColor(const COLORREF color);

   static void SetRenderDevice( RenderDevice *_device );
   void Set(DWORD textureChannel);
   void SetBackDrop(DWORD textureChannel);
   void Unset(DWORD textureChannel);

   void Release();
   void EnsureHBitmap();
   void CreateGDIVersion();

   void CreateTextureOffscreen(const int width, const int height);
   BaseTexture *CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);
   void CreateFromResource(const int id, int * const pwidth = NULL, int * const pheight = NULL);

   BOOL SetAlpha(const COLORREF rgbTransparent);

   static void SetOpaque(BaseTexture* pdds);
   static void SetOpaqueBackdrop(BaseTexture* pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop);
   static BOOL SetAlpha(BaseTexture* pdds, const COLORREF rgbTransparent);
   static void Blur(BaseTexture* pdds, const BYTE * const pbits, const int shadwidth, const int shadheight);

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

   COLORREF m_rgbTransparent;
   BOOL m_fTransparent; // Whether this picture actually contains transparent bits

   BaseTexture* m_pdsBuffer;
   BaseTexture* m_pdsBufferColorKey;

   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth so GDI draws it fast
   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

private:
   COLORREF m_rgbBackdropCur;

   BaseTexture* m_pdsBufferBackdrop;

   static RenderDevice *renderDevice;

   HBITMAP m_oldHBM;        // this is to cache the result of SelectObject()
};
