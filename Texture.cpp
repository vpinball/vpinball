#include "stdafx.h"
#include "Texture.h"
#include "freeimage.h"

RenderDevice *Texture::renderDevice=0;

Texture::Texture()
{
   m_pdsBuffer = NULL;
   m_pdsBufferColorKey = NULL;
   m_pdsBufferBackdrop = NULL;
   m_rgbTransparent = RGB(255,255,255);
   m_hbmGDIVersion = NULL;
   m_ppb = NULL;
}

Texture::~Texture()
{
   FreeStuff();
}

void Texture::SetRenderDevice( RenderDevice *_device )
{
   renderDevice = _device;
}

void Texture::Release()
{
}

void Texture::SetBackDrop( DWORD textureChannel )
{
    renderDevice->SetTexture( textureChannel, m_pdsBufferBackdrop ? (LPDIRECTDRAWSURFACE7)m_pdsBufferBackdrop : NULL);
}

/*
void Texture::Set( const DWORD textureChannel )
{
    renderDevice->SetTexture( textureChannel, m_pdsBufferColorKey ? (LPDIRECTDRAWSURFACE7)m_pdsBufferColorKey : NULL);
}
*/
void Texture::Unset( const DWORD textureChannel )
{
   renderDevice->SetTexture( textureChannel, NULL );
}

HRESULT Texture::SaveToStream(IStream *pstream, PinTable *pt)
{
   BiffWriter bw(pstream, NULL, NULL);

   bw.WriteString(FID(NAME), m_szName);

   bw.WriteString(FID(INME), m_szInternalName);

   bw.WriteString(FID(PATH), m_szPath);

   bw.WriteInt(FID(WDTH), m_originalWidth);
   bw.WriteInt(FID(HGHT), m_originalHeight);

   bw.WriteInt(FID(TRNS), m_rgbTransparent);

   if (!m_ppb)
   {
      DDSURFACEDESC2 ddsd;
      ddsd.dwSize = sizeof(ddsd);

      m_pdsBuffer->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

      bw.WriteTag(FID(BITS));

      // 32-bit picture
      LZWWriter lzwwriter(pstream, (int *)ddsd.lpSurface, m_width*4, m_height, ddsd.lPitch);

      lzwwriter.CompressBits(8+1);

      m_pdsBuffer->Unlock(NULL);
   }
   else // JPEG (or other binary format)
   {
      const int linkid = pt->GetImageLink(this);
      if (linkid == 0)
      {
         bw.WriteTag(FID(JPEG));
         m_ppb->SaveToStream(pstream);
      }
      else
      {
         bw.WriteInt(FID(LINK), linkid);
      }
   }

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Texture::LoadFromStream(IStream *pstream, int version, PinTable *pt)
{
   BiffReader br(pstream, this, pt, version, NULL, NULL);

   br.Load();

   EnsureMaxTextureCoordinates();

   return ((m_pdsBuffer != NULL) ? S_OK : E_FAIL);
}

BOOL Texture::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(NAME))
   {
      pbr->GetString(m_szName);
   }
   else if (id == FID(INME))
   {
      pbr->GetString(m_szInternalName);
   }
   else if (id == FID(PATH))
   {
      pbr->GetString(m_szPath);
   }
   else if (id == FID(TRNS))
   {
      pbr->GetInt(&m_rgbTransparent);
   }
   else if (id == FID(WDTH))
   {
      pbr->GetInt(&m_width);
      m_originalWidth = m_width;
   }
   else if (id == FID(HGHT))
   {
      pbr->GetInt(&m_height);
      m_originalHeight = m_height;
   }
   else if (id == FID(BITS))
   {

      m_pdsBuffer = g_pvp->m_pdd.CreateTextureOffscreen(m_width, m_height);

      if (m_pdsBuffer == NULL)
      {
         return fFalse;
      }

      DDSURFACEDESC2 ddsd;
      ddsd.dwSize = sizeof(ddsd);

      /*const HRESULT hr =*/ m_pdsBuffer->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

      // 32-bit picture
      LZWReader lzwreader(pbr->m_pistream, (int *)ddsd.lpSurface, m_width*4, m_height, ddsd.lPitch);

      lzwreader.Decoder();

      const int pitch = ddsd.lPitch;

      // Assume our 32 bit color structure
      BYTE *pch = (BYTE *)ddsd.lpSurface;
      unsigned char min = 0xff;
      unsigned char max = 0;
      for (int i=0;i<m_height;i++)
      {
         for (int l=0;l<m_width;l++)
         {				
            if (min > pch[3])
               min = pch[3];
            if (max < pch[3])
               max = pch[3];					
            pch += 4;
         }
         pch += pitch - m_width*4;
      }

      pch = (BYTE *)ddsd.lpSurface;
      if ((min == max) && (min == 0))
         for (int i=0;i<m_height;i++)
         {
            for (int l=0;l<m_width;l++)
            {				
               pch[3] = 0xff;
               pch += 4;
            }
            pch += pitch-(m_width*4);
         }

         m_pdsBuffer->Unlock(NULL);

   }
   else if (id == FID(JPEG))
   {
      m_ppb = new PinBinary();
      m_ppb->LoadFromStream(pbr->m_pistream, pbr->m_version);

      // m_ppb->m_szPath has the original filename	
      // m_ppb->m_pdata() is the buffer
      // m_ppb->m_cdata() is the filesize
      FIMEMORY *hmem = FreeImage_OpenMemory((BYTE *)m_ppb->m_pdata, m_ppb->m_cdata);
      FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
      FIBITMAP *dib = FreeImage_LoadFromMemory(fif, hmem, 0);
      int bitsPerPixel = FreeImage_GetBPP(dib);

      // check if Textures exceed the maximum texture dimension
      int maxTexDim;
      HRESULT hrMaxTex = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
      if (hrMaxTex != S_OK)
      {
         maxTexDim = 0; // default: Don't resize textures
      }
      if((maxTexDim <= 0) || (maxTexDim > MAX_TEXTURE_SIZE))
      {
         maxTexDim = MAX_TEXTURE_SIZE;
      }
      int pictureWidth = FreeImage_GetWidth(dib);
      int pictureHeight = FreeImage_GetHeight(dib);
      // save original width and height, if the texture is rescaled
      m_originalWidth = pictureWidth;
      m_originalHeight = pictureHeight;
      if ((pictureHeight > maxTexDim) || (pictureWidth > maxTexDim))
      {
         dib = FreeImage_Rescale(dib, min(pictureWidth,maxTexDim), min(pictureHeight,maxTexDim), FILTER_BILINEAR);
         m_width = min(pictureWidth,maxTexDim);
         m_height = min(pictureHeight,maxTexDim);
      }

      HDC hDC = GetDC(NULL);

      HBITMAP hbm = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);

      int dibWidth = FreeImage_GetWidth(dib);
      int dibHeight = FreeImage_GetHeight(dib);

      FreeImage_Unload(dib);

      m_pdsBuffer =  g_pvp->m_pdd.CreateFromHBitmap(hbm, &dibWidth, &dibHeight );

      if (bitsPerPixel == 24)
         g_pvp->m_pdd.SetOpaque(m_pdsBuffer, dibWidth, dibHeight);

      //m_pdsBuffer = g_pvp->m_pdd.DecompressJPEG(this, m_ppb, &m_width, &m_height);
      if (m_pdsBuffer == NULL)
      {
         return fFalse;
      }
   }
   else if (id == FID(LINK))
   {
      int linkid;
      PinTable * const pt = (PinTable *)pbr->m_pdata;
      pbr->GetInt(&linkid);
      m_ppb = pt->GetImageLinkBinary(linkid);
      FIMEMORY *hmem = FreeImage_OpenMemory((BYTE *)m_ppb->m_pdata, m_ppb->m_cdata);
      FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
      FIBITMAP *dib = FreeImage_LoadFromMemory(fif, hmem, 0);

      // check if Textures exceed the maximum texture dimension
      int maxTexDim;
      HRESULT hrMaxTex = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
      if (hrMaxTex != S_OK)
      {
         maxTexDim = 0; // default: Don't resize textures
      }
      if((maxTexDim <= 0) || (maxTexDim > MAX_TEXTURE_SIZE))
      {
         maxTexDim = MAX_TEXTURE_SIZE;
      }
      int pictureWidth = FreeImage_GetWidth(dib);
      int pictureHeight = FreeImage_GetHeight(dib);
      // save original width and height, if the texture is rescaled
      m_originalWidth = pictureWidth;
      m_originalHeight = pictureHeight;
      if ((pictureHeight > maxTexDim) || (pictureWidth > maxTexDim))
      {
         dib = FreeImage_Rescale(dib, min(pictureWidth,maxTexDim), min(pictureHeight,maxTexDim), FILTER_BILINEAR);
         m_width = min(pictureWidth,maxTexDim);
         m_height = min(pictureHeight,maxTexDim);
      }

      HDC hDC = GetDC(NULL);
      HBITMAP hbm = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);

      int bitsPerPixel = FreeImage_GetBPP(dib);
      int dibWidth = FreeImage_GetWidth(dib);
      int dibHeight = FreeImage_GetHeight(dib);

      FreeImage_Unload(dib);

      m_pdsBuffer =  g_pvp->m_pdd.CreateFromHBitmap(hbm, 0, 0);

      if (bitsPerPixel == 24)
         g_pvp->m_pdd.SetOpaque(m_pdsBuffer, dibWidth, dibHeight);

      if (m_pdsBuffer == NULL)
      {
         return fFalse;
      }
   }
   return fTrue;
}

void Texture::SetTransparentColor(const COLORREF color)
{
   m_fTransparent = fFalse;
   if (m_rgbTransparent != color)
   {
      m_rgbTransparent = color;
      SAFE_RELEASE(m_pdsBufferColorKey);
      SAFE_RELEASE(m_pdsBufferBackdrop);
   }
}

void Texture::CreateAlphaChannel()
{
   if (!m_pdsBufferColorKey)
   {
      DDSURFACEDESC2 ddsd;
      ddsd.dwSize = sizeof(ddsd);
      m_pdsBuffer->GetSurfaceDesc(&ddsd);
      m_pdsBufferColorKey = g_pvp->m_pdd.CreateTextureOffscreen(ddsd.dwWidth, ddsd.dwHeight);
      m_pdsBufferColorKey->Blt(NULL,m_pdsBuffer,NULL,DDBLT_WAIT,NULL);
      m_fTransparent = g_pvp->m_pdd.SetAlpha(m_pdsBufferColorKey, m_rgbTransparent, m_width, m_height);
      if (!m_fTransparent) m_rgbTransparent = NOTRANSCOLOR; // set to magic color to disable future checking
      CreateNextMipMapLevel(m_pdsBufferColorKey);
   }
}

void Texture::EnsureBackdrop(const COLORREF color)
{
   if (!m_pdsBufferBackdrop || color != m_rgbBackdropCur)
   {
      DDSURFACEDESC2 ddsd;
      ddsd.dwSize = sizeof(ddsd);
      m_pdsBuffer->GetSurfaceDesc(&ddsd);
      if (!m_pdsBufferBackdrop)
      {
         m_pdsBufferBackdrop = g_pvp->m_pdd.CreateTextureOffscreen(ddsd.dwWidth, ddsd.dwHeight);
      }
      m_pdsBufferBackdrop->Blt(NULL,m_pdsBuffer,NULL,DDBLT_WAIT,NULL);
      SetOpaqueBackdrop(m_pdsBufferBackdrop, m_rgbTransparent, color, ddsd.dwWidth, ddsd.dwHeight);
      CreateNextMipMapLevel(m_pdsBufferBackdrop);

      m_rgbBackdropCur = color;
   }
}

void Texture::EnsureMaxTextureCoordinates()
{
   DDSURFACEDESC2 ddsd;
   ddsd.dwSize = sizeof(ddsd);

   m_pdsBuffer->GetSurfaceDesc(&ddsd);

   m_maxtu = (float)m_width / (float)ddsd.dwWidth;
   m_maxtv = (float)m_height / (float)ddsd.dwHeight;
}

void Texture::FreeStuff()
{
   SAFE_RELEASE(m_pdsBuffer);
   SAFE_RELEASE(m_pdsBufferColorKey);
   SAFE_RELEASE(m_pdsBufferBackdrop);
   if (m_hbmGDIVersion)
   {
      DeleteObject(m_hbmGDIVersion);
      m_hbmGDIVersion = NULL;
   }
   if (m_ppb)
   {
      delete m_ppb;
      m_ppb = NULL;
   }
}

void Texture::EnsureHBitmap()
{
   if (!m_hbmGDIVersion)
   {
      CreateGDIVersion();
   }
}

void Texture::CreateGDIVersion()
{
   HDC hdcImage;
   m_pdsBuffer->GetDC(&hdcImage);

   HDC hdcScreen = GetDC(NULL);
   m_hbmGDIVersion = CreateCompatibleBitmap(hdcScreen, m_width, m_height);
   HDC hdcNew = CreateCompatibleDC(hdcScreen);
   HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, m_hbmGDIVersion);

   StretchBlt(hdcNew, 0, 0, m_width, m_height, hdcImage, 0, 0, m_width, m_height, SRCCOPY);

   SelectObject(hdcNew, hbmOld);
   DeleteDC(hdcNew);
   ReleaseDC(NULL,hdcScreen);

   m_pdsBuffer->ReleaseDC(hdcImage);
}

void Texture::CreateFromResource(const int id, int * const pwidth, int * const pheight)
{
   HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

   if (hbm == NULL)
   {
      m_pdsBufferColorKey=NULL;
      return;
   }

   m_pdsBufferColorKey = CreateFromHBitmap(hbm, pwidth, pheight);
}

BaseTexture* Texture::CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight)
{
   BITMAP bm;
   GetObject(hbm, sizeof(bm), &bm);

   if (pwidth)
   {
      *pwidth = bm.bmWidth;
   }

   if (pheight)
   {
      *pheight = bm.bmHeight;
   }

   if (bm.bmWidth > MAX_TEXTURE_SIZE || bm.bmHeight > MAX_TEXTURE_SIZE)
   {
      return NULL; // MAX_TEXTURE_SIZE is the limit for directx7 textures
   }

   BaseTexture* pdds = CreateBaseTexture(bm.bmWidth, bm.bmHeight);

   HDC hdc;
   pdds->GetDC(&hdc);

   HDC hdcFoo = CreateCompatibleDC(hdc);

   HBITMAP hbmOld = (HBITMAP)SelectObject(hdcFoo, hbm);

   BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcFoo, 0, 0, SRCCOPY);

   SelectObject(hdcFoo, hbmOld);

   DeleteDC(hdcFoo);

   DeleteObject(hbm);

   pdds->ReleaseDC(hdc); 

   //bm.bmBitsPixel
   //SetAlpha(pdds, RGB(0,0,0));

   if (bm.bmBitsPixel != 32) 
      g_pvp->m_pdd.SetOpaque(pdds, bm.bmWidth, bm.bmHeight);

   return pdds;
}

void Texture::CreateTextureOffscreen(const int width, const int height)
{
   m_pdsBufferColorKey = CreateBaseTexture( width, height );
}

BaseTexture* Texture::CreateBaseTexture(const int width, const int height)
{
   DDSURFACEDESC2 ddsd;
   ZeroMemory( &ddsd, sizeof(ddsd) );
   ddsd.dwSize = sizeof(ddsd);

   //!! power of two only necessary because of mipmaps?
   int texwidth = 8; // Minimum size 8
   while(texwidth < width)
      texwidth <<= 1;

   int texheight = 8;
   while(texheight < height)
      texheight <<= 1;

   // D3D7 does not support textures greater than 4096 in either dimension
   if (texwidth > MAX_TEXTURE_SIZE)
   {
      texwidth = MAX_TEXTURE_SIZE;
   }

   if (texheight > MAX_TEXTURE_SIZE)
   {
      texheight = MAX_TEXTURE_SIZE;
   }

   ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | /*DDSD_CKSRCBLT |*/ DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
   ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = 0;
   ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = 0;
   ddsd.dwWidth        = texwidth;
   ddsd.dwHeight       = texheight;
   ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP; 
   ddsd.dwMipMapCount	= 4;

   if ( RenderDevice::instance()->getHardwareAccelerated() )
   {
#if 1
      // Create the texture and let D3D driver decide where it store it.
      ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#else
      // Create the texture in video memory.
      ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
#endif
   }
   else
   {
      // Create the texture in system memory.
      ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
   }

   ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
   ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
   ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
   ddsd.ddpfPixelFormat.dwRBitMask        = 0x00ff0000;
   ddsd.ddpfPixelFormat.dwGBitMask        = 0x0000ff00;
   ddsd.ddpfPixelFormat.dwBBitMask        = 0x000000ff;
   ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;

   BaseTexture* pdds;
   HRESULT hr;
   bool retryflag = (RenderDevice::instance()->getHardwareAccelerated() != 0);
retryimage:
   if( FAILED( hr = g_pvp->m_pdd.m_pDD->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&pdds, NULL ) ) )
   {
      if(retryflag)
      {
         ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM;
         ddsd.ddsCaps.dwCaps2 = 0;
         retryflag = false;
         goto retryimage;
      }
      ShowError("Could not create texture offscreen surface.");
      return NULL;
   }

   // Update the count (including mipmaps).
   NumVideoBytes += (unsigned int)((float)(ddsd.dwWidth * ddsd.dwHeight * 4) * (float)(4.0/3.0));
   m_width = ddsd.dwWidth;
   m_height = ddsd.dwHeight;
   m_maxtu = (float)m_width / (float)ddsd.dwWidth;
   m_maxtv = (float)m_height / (float)ddsd.dwHeight;
   pitch = ddsd.lPitch;

   pdds->SetLOD(0);

   return pdds;
}


void Texture::SetOpaqueBackdrop(BaseTexture* pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop, const int width, const int height)
{
   DDSURFACEDESC2 ddsd;
   ddsd.dwSize = sizeof(ddsd);
   pdds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

   const int pitch = ddsd.lPitch;

   const unsigned int rback = (rgbBackdrop & 0x00ff0000) >> 16;
   const unsigned int gback = (rgbBackdrop & 0x0000ff00) >> 8;
   const unsigned int bback = (rgbBackdrop & 0x000000ff);

   const unsigned int rgbBd = rback | (gback << 8) | (bback << 16) | ((unsigned int)0xff << 24);

   // Assume our 32 bit color structure	
   BYTE *pch = (BYTE *)ddsd.lpSurface;	

   for (int i=0;i<height;i++)
   {
      for (int l=0;l<width;l++)
      {
         if ((*(unsigned int *)pch & 0xffffff) != rgbTransparent)
         {
            pch[3] = 0xff;
         }
         else
         {
            *(unsigned int *)pch = rgbBd;
         }
         pch += 4;
      }
      pch += pitch-(width*4);
   }

   pdds->Unlock(NULL);
}

void Texture::CreateMipMap()
{
   CreateNextMipMapLevel( m_pdsBufferColorKey );
}

void Texture::CreateNextMipMapLevel(BaseTexture* pdds)
{
   DDSURFACEDESC2 ddsd, ddsdNext;
   ddsd.dwSize = sizeof(ddsd);
   ddsdNext.dwSize = sizeof(ddsd);
   DDSCAPS2 ddsCaps;
   ddsCaps.dwCaps2 = 0;
   ddsCaps.dwCaps3 = 0;
   ddsCaps.dwCaps4 = 0;
   ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;	

   BaseTexture* pddsNext;
   HRESULT hr = pdds->GetAttachedSurface(&ddsCaps, (LPDIRECTDRAWSURFACE7*)&pddsNext);

   if (hr == S_OK)
   {
      pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
      pddsNext->Lock(NULL, &ddsdNext, DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

      const int pitch = ddsd.lPitch;
      const int pitchNext = ddsdNext.lPitch;
      const int width = ddsdNext.dwWidth;
      const int height = ddsdNext.dwHeight;
      const BYTE * const pch = (BYTE *)ddsd.lpSurface;
      BYTE *pchNext = (BYTE *)ddsdNext.lpSurface;

      const BYTE* pbytes1 = pch;
      const BYTE* pbytes2 = pch + pitch;

      const int addtoouterpitch = pitch*2 - (width*2*4);

      for (int y=0;y<height;y++)
      {
         for (int x=0;x<width;x++)
         {
            unsigned int rtotal = 0;
            unsigned int gtotal = 0;
            unsigned int btotal = 0;
            unsigned int count = 0;
            const unsigned int a0 = pbytes1[3];
            if (a0) 
            { 
               count++; 
               rtotal+=pbytes1[2]; 
               gtotal+=pbytes1[1]; 
               btotal+=pbytes1[0]; 
            } //!! opt.
            const unsigned int a1 = pbytes1[7];
            if (a1) 
            { 
               count++; 
               rtotal+=pbytes1[6]; 
               gtotal+=pbytes1[5]; 
               btotal+=pbytes1[4]; 
            }
            const unsigned int a2 = pbytes2[3];
            if (a2) 
            { 
               count++; 
               rtotal+=pbytes2[2]; 
               gtotal+=pbytes2[1]; 
               btotal+=pbytes2[0]; 
            }
            const unsigned int a3 = pbytes2[7];
            if (a3) 
            { 
               count++; 
               rtotal+=pbytes2[6]; 
               gtotal+=pbytes2[5]; 
               btotal+=pbytes2[4]; 
            }

            if (count == 0) // all pixels are transparent - do whatever
            {
               *(unsigned int*)pchNext = 0;
            } 
            else 
            {
               const unsigned int atotal = ((a0 + a1 + a2 + a3 + 2)>>2)<<24;
               const unsigned int round = count>>1;
               btotal += round;
               gtotal += round;
               rtotal += round;
               if(count == 2) 
               {
                  btotal >>= 1;
                  gtotal >>= 1;
                  rtotal >>= 1;
               } else if(count == 4) 
               {
                  btotal >>= 2;
                  gtotal >>= 2;
                  rtotal >>= 2;
               } else if(count == 3) 
               {
                  btotal /= 3; // compiler optimizations will turn this into a mul
                  gtotal /= 3;
                  rtotal /= 3;
               }
               *(unsigned int*)pchNext = btotal | (gtotal<<8) | (rtotal<<16) | atotal;
            }
            pchNext += 4;

            pbytes1 += 8;
            pbytes2 += 8;
         }
         pbytes1 += addtoouterpitch;
         pbytes2 += addtoouterpitch;
         pchNext += pitchNext - (width*4);
      }
      pdds->Unlock(NULL);
      pddsNext->Unlock(NULL);

      pddsNext->Release();

      CreateNextMipMapLevel(pddsNext);
   }
}

BOOL Texture::SetAlpha(const COLORREF rgbTransparent, const int width, const int height)
{
   // Set alpha of each pixel
   DDSURFACEDESC2 ddsd;
   ddsd.dwSize = sizeof(ddsd);

   if ( m_pdsBufferColorKey==NULL )
      return fFalse;

   m_pdsBufferColorKey->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

   BOOL fTransparent = fFalse;	

   const int pitch = ddsd.lPitch;

   const COLORREF rtrans = (rgbTransparent & 0x000000ff);
   const COLORREF gtrans = (rgbTransparent & 0x0000ff00) >> 8;
   const COLORREF btrans = (rgbTransparent & 0x00ff0000) >> 16;

   const COLORREF bgrTransparent = btrans | (gtrans << 8) | (rtrans << 16) | 0xff000000;  // color order different in DirectX texture buffer
   // Assume our 32 bit color structure

   BYTE *pch = (BYTE *)ddsd.lpSurface;
   {
      // check if image has it's own alpha channel
      unsigned int aMax = ((*(COLORREF *)pch) & 0xff000000)>>24;
      unsigned int aMin = ((*(COLORREF *)pch) & 0xff000000)>>24;
      for (int i=0;i<height;i++)
      {
         for (int l=0;l<width;l++)
         {	

            if (((*(COLORREF *)pch) & 0xff000000)>>24 > aMax)
               aMax = ((*(COLORREF *)pch) & 0xff000000)>>24;
            if (((*(COLORREF *)pch) & 0xff000000)>>24 < aMin)
               aMin = ((*(COLORREF *)pch) & 0xff000000)>>24;
            pch += 4;
         }
         pch += pitch-(width*4);
      }
      slintf("amax:%d amin:%d\n",aMax,aMin);
      pch = (BYTE *)ddsd.lpSurface;

      for (int i=0;i<height;i++)
      {
         for (int l=0;l<width;l++)
         {	
            const COLORREF tc = (*(COLORREF *)pch) | 0xff000000; //set to opaque
            if (tc == bgrTransparent )					// reg-blue order reversed
            {
               *(unsigned int *)pch = 0x00000000;		// set transparent colorkey to black	and alpha transparent	
               fTransparent = fTrue;					// colorkey is true
            }
            else 
            { 
               //if (!(tc & MINBLACKMASK)) 
               //	{tc |= MINBLACK;}	// set minimum black

               //to enable alpha uncomment these three lines (does not work with HD-Render)
               if ((aMin == aMax) && (aMin == 255))    // if there is no alpha-channel info in the image, set to opaque
                  *(COLORREF *)pch = tc; 
               else 
                  fTransparent = fTrue;   // does not work. - cupid: i need a real PC to test this.
            }
            pch += 4;
         }
         pch += pitch-(width*4);
      }	


   }
   m_pdsBufferColorKey->Unlock(NULL);

   return fTransparent;
}

void Texture::Lock()
{
   DDSURFACEDESC2 ddsd;
   ddsd.dwSize = sizeof(ddsd);
   m_pdsBufferColorKey->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
   pitch = ddsd.lPitch;
   surfaceData = (BYTE*)ddsd.lpSurface;
}

void Texture::Unlock()
{
   m_pdsBufferColorKey->Unlock(NULL);
}
