#include "stdafx.h"
#include "RenderDevice.h"
#pragma once

#ifdef RGB
#undef RGB
#endif
#define BGR(b,g,r) ((COLORREF)(((DWORD)(b)) | (((DWORD)(g))<<8) | (((DWORD)(r))<<16)))
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

#define NOTRANSCOLOR  RGB(123,123,123)

//class RenderDevice;

class BaseTexture : public IDirectDrawSurface7
{
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

   void CreateAlphaChannel();
   void EnsureBackdrop(const COLORREF color);
   void FreeStuff();
   void SetTransparentColor(const COLORREF color);
   void EnsureMaxTextureCoordinates();

   static void SetRenderDevice( RenderDevice *_device );
   void SetBackDrop( const DWORD textureChannel );
   void Set( const DWORD textureChannel )
   {
      renderDevice->SetTexture( textureChannel, (LPDIRECTDRAWSURFACE7)m_pdsBufferColorKey);
   }

   void Release();
   void EnsureHBitmap();
   void CreateGDIVersion();
   void CreateFromResource(const int id, int * const pwidth, int * const pheight);
   BaseTexture *CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);
   void CreateTextureOffscreen(const int width, const int height);
   BaseTexture *CreateBaseTexture(const int width, const int height);
   void CreateMipMap();
   BOOL SetAlpha(const COLORREF rgbTransparent, const int width, const int height);
   void CreateNextMipMapLevel(BaseTexture* pdds);
   void SetOpaqueBackdrop(BaseTexture* pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop, const int width, const int height);
   void Lock();
   void Unlock();

   void Unset( const DWORD textureChannel );

   // width and height of texture can be different than width and height
   // of dd surface, since the surface has to be in powers of 2
   int m_width, m_height;
   int m_originalWidth, m_originalHeight;

   LONG pitch;
   // only valid if Lock() was called before
   BYTE *surfaceData;

   // Filled at runtime, accounts for buffer space to meet the power of 2
   // requirement
   float m_maxtu, m_maxtv;

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

   COLORREF m_rgbTransparent;
   BOOL m_fTransparent; // Whether this picture actually contains transparent bits

   COLORREF m_rgbBackdropCur;

   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth so GDI draws it fast

   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   BaseTexture* m_pdsBuffer;
   BaseTexture* m_pdsBufferColorKey;
   BaseTexture* m_pdsBufferBackdrop;

   static RenderDevice *renderDevice;
};
