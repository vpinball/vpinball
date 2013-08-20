#include "stdafx.h"
#include "Texture.h"
#include "freeimage.h"
#include "RenderDevice.h"

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
   if ( m_pdsBufferBackdrop )
   {
      renderDevice->SetTexture( textureChannel, (LPDIRECTDRAWSURFACE7)m_pdsBufferBackdrop);
   }
   else
   {
      renderDevice->SetTexture(textureChannel, NULL);
   }
}

void Texture::Set( DWORD textureChannel )
{
   if ( m_pdsBufferColorKey )
   {
      renderDevice->SetTexture( textureChannel, (LPDIRECTDRAWSURFACE7)m_pdsBufferColorKey);
   }
   else
   {
      renderDevice->SetTexture(textureChannel, NULL);
   }
}

void Texture::Unset( DWORD textureChannel )
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

      //slintf("Pixel format: %s",hbm.PixelFormat.ToString());

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
      g_pvp->m_pdd.CreateNextMipMapLevel(m_pdsBufferColorKey);
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
      g_pvp->m_pdd.SetOpaqueBackdrop(m_pdsBufferBackdrop, m_rgbTransparent, color, ddsd.dwWidth, ddsd.dwHeight);
      g_pvp->m_pdd.CreateNextMipMapLevel(m_pdsBufferBackdrop);

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
