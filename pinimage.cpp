// PinImage.cpp: implementation of the PinImage class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "freeimage.h"
#include "Texture.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


PinDirectDraw::PinDirectDraw()
{
   m_pDD = NULL;

   int tmp = 1;
   HRESULT hr = GetRegInt("Player", "HardwareRender", &tmp);
   m_fHardwareAccel = (tmp != 0);
}

PinDirectDraw::~PinDirectDraw()
{
   SAFE_RELEASE(m_pDD);
}

HRESULT PinDirectDraw::InitDD()
{
   m_DDraw=LoadLibrary("ddraw.dll");
   if ( m_DDraw==NULL )
   {
      return E_FAIL;
   }
   m_DDCreate=(DDCreateFunction)GetProcAddress(m_DDraw,"DirectDrawCreateEx");

   if (m_DDCreate == NULL)
   {
      FreeLibrary(m_DDraw);

      LocalString ls(IDS_NEED_DD9);
      MessageBox(g_pvp->m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);

      return E_FAIL;
   }

   HRESULT hr = (*m_DDCreate)(NULL, (VOID **)&m_pDD, IID_IDirectDraw7, NULL);
   if (hr != S_OK)
   {
      ShowError("Could not create Direct Draw.");
   }

   hr = m_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL | DDSCL_FPUSETUP); // was DDSCL_FPUPRESERVE, which in theory adds lots of overhead, but who knows if this is even supported nowadays by the drivers
   if (hr != S_OK)
   {
      ShowError("Could not set Direct Draw cooperative level.");
   }

   return S_OK;
}

BaseTexture* PinDirectDraw::CreateTextureOffscreen(const int width, const int height)
{
   DDSURFACEDESC2 ddsd;
   ZeroMemory( &ddsd, sizeof(ddsd) );
   ddsd.dwSize = sizeof(ddsd);

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

   if (m_fHardwareAccel)
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
   bool retryflag = (m_fHardwareAccel != 0);
retryimage:
   if( FAILED( hr = m_pDD->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&pdds, NULL ) ) )
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

   pdds->SetLOD(0);

   return pdds;
}

BaseTexture* PinDirectDraw::CreateFromFile(char *szfile, int * const pwidth, int * const pheight, int& originalWidth, int& originalHeight)
{
   FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

   // check the file signature and deduce its format
   // (the second argument is currently not used by FreeImage)
   fif = FreeImage_GetFileType(szfile, 0);
   if(fif == FIF_UNKNOWN) {
      // no signature ?
      // try to guess the file format from the file extension
      fif = FreeImage_GetFIFFromFilename(szfile);
   }
   // check that the plugin has reading capabilities ...
   if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
      // ok, let's load the file
      FIBITMAP *dib = FreeImage_Load(fif, szfile, 0);
      // unless a bad file format, we are done !

      // check if Textures exceed the maximum texture diemension
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
      originalWidth = pictureWidth;
      originalHeight = pictureHeight;
      if ((pictureHeight > maxTexDim) || (pictureWidth > maxTexDim))
      {
         dib = FreeImage_Rescale(dib, min(pictureWidth,maxTexDim), min(pictureHeight,maxTexDim), FILTER_BILINEAR);
      }

      HDC hDC = GetDC(NULL);
      HBITMAP hbm = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
      int bitsPerPixel = FreeImage_GetBPP(dib);
      int dibWidth = FreeImage_GetWidth(dib);
      int dibHeight = FreeImage_GetHeight(dib);
      FreeImage_Unload(dib);
      //HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, szfile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

      BaseTexture* mySurface = g_pvp->m_pdd.CreateFromHBitmap(hbm, pwidth, pheight);
      //LPDIRECTDRAWSURFACE7 mySurface = CreateFromHBitmap(hbm, pwidth, pheight);

      if (bitsPerPixel == 24)
         g_pvp->m_pdd.SetOpaque(mySurface, dibWidth, dibHeight);

      return (hbm == NULL) ? NULL : mySurface;
   }
   else
      return NULL;
}

BaseTexture* PinDirectDraw::CreateFromResource(const int id, int * const pwidth, int * const pheight)
{
   HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

   if (hbm == NULL)
   {
      return NULL;
   }

   return CreateFromHBitmap(hbm, pwidth, pheight);
}

BaseTexture* PinDirectDraw::CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight)
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

   BaseTexture* pdds = CreateTextureOffscreen(bm.bmWidth, bm.bmHeight);

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

void PinDirectDraw::SetOpaque(BaseTexture* pdds, const int width, const int height)
{
   DDSURFACEDESC2 ddsd;
   ddsd.dwSize = sizeof(ddsd);	

   pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

   const int pitch = ddsd.lPitch;

   // Assume our 32 bit color structure
   BYTE *pch = (BYTE *)ddsd.lpSurface;

   for (int i=0;i<height;i++)
   {
      for (int l=0;l<width;l++)
      {				
         pch[3] = 0xff;
         pch += 4;
      }
      pch += pitch-(width*4);
   }

   pdds->Unlock(NULL);
}

void PinDirectDraw::SetOpaqueBackdrop(BaseTexture* pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop, const int width, const int height)
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


BOOL PinDirectDraw::SetAlpha(BaseTexture* pdds, const COLORREF rgbTransparent, const int width, const int height)
{
   // Set alpha of each pixel


   DDSURFACEDESC2 ddsd;
   ddsd.dwSize = sizeof(ddsd);
   pdds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

   BOOL fTransparent = fFalse;	

   const int pitch = ddsd.lPitch;

   const COLORREF rtrans = (rgbTransparent & 0x000000ff);
   const COLORREF gtrans = (rgbTransparent & 0x0000ff00) >> 8;
   const COLORREF btrans = (rgbTransparent & 0x00ff0000) >> 16;

   const COLORREF bgrTransparent = btrans | (gtrans << 8) | (rtrans << 16) | 0xff000000;  // color order different in DirectX texture buffer
   // Assume our 32 bit color structure

   BYTE *pch = (BYTE *)ddsd.lpSurface;
   if (rgbTransparent == NOTRANSCOLOR)
   {
      /*for (int i=0;i<height;i++)
      {
      for (int l=0;l<width;l++)
      {	
      const COLORREF tc = *(COLORREF *)pch;
      if (!(tc & MINBLACKMASK)) 
      *(COLORREF *)pch = tc | MINBLACK;	//preserve alpha channel

      pch += 4;
      }
      pch += pitch-(width*4);
      }*/
   }
   else  
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
   pdds->Unlock(NULL);

   return fTransparent;
}

const int rgfilterwindow[7][7] = {
   1, 4, 8, 10, 8, 4, 1,
   4, 12, 25, 29, 25, 12, 4,
   8, 25, 49, 58, 49, 25, 8,
   10, 29, 58, 67, 58, 29, 10,
   8, 25, 49, 58, 49, 25, 8,
   4, 12, 25, 29, 25, 12, 4,
   1, 4, 8, 10, 8, 4, 1};

   void PinDirectDraw::Blur(BaseTexture* pdds, const BYTE * const pbits, const int shadwidth, const int shadheight)
   {
      if (!pbits) return;	// found this pointer to be NULL after some graphics errors

      /*int window[7][7]; // custom filter kernel
      for (int i=0;i<4;i++)
      {
      window[0][i] = i+1;
      window[0][6-i] = i+1;
      window[i][0] = i+1;
      window[6-i][0] = i+1;
      }*/

      int totalwindow = 0;
      for (int i=0;i<7;i++)
      {
         for (int l=0;l<7;l++)
         {
            //window[i][l] = window[0][l] * window[i][0];
            totalwindow += rgfilterwindow[i][l];
         }
      }

      // Gaussian Blur the sharp shadows

      DDSURFACEDESC2 ddsd;
      ddsd.dwSize = sizeof(ddsd);

      pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK | DDLOCK_DISCARDCONTENTS | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

      const int pitch = ddsd.lPitch;
      const int pitchSharp = 256*3;
      BYTE *pc = (BYTE *)ddsd.lpSurface;

      for (int i=0;i<shadheight;i++)
      {
         for (int l=0;l<shadwidth;l++)
         {
            int value = 0;
            int totalvalue = totalwindow;

            for (int n=0;n<7;n++)
            {
               const int y = i+n-3;
               if(/*y>=0 &&*/ (unsigned int)y<(unsigned int)shadheight) // unsigned arithmetic trick includes check for >= zero
               {
                  const BYTE *const py = pbits + pitchSharp*y;
                  for (int m=0;m<7;m++)
                  {
                     const int x = l+m-3;					
                     if (/*x>=0 &&*/ (unsigned int)x<(unsigned int)shadwidth) // dto. //!! opt.
                     {						
                        value += (int)(*(py + x*3)) * rgfilterwindow[m][n];
                     }
                     else
                     {
                        totalvalue -= rgfilterwindow[m][n];
                     }
                  }
               }
               else
               {
                  for (int m=0;m<7;m++)
                  {
                     const int x = l+m-3;
                     if (/*x<0 ||*/ (unsigned int)x>=(unsigned int)shadwidth) // dto.
                     {
                        totalvalue -= rgfilterwindow[m][n];
                     }
                  }
               }
            }

            value /= totalvalue; //totalwindow;

            const unsigned int valueu = 127 + (value>>1);
            *((unsigned int*)pc) = valueu | (valueu<<8) | (valueu<<16) | (valueu<<24); // all R,G,B,A get same value
            pc += 4;
         }

         pc += pitch - shadwidth*4;
      }

      pdds->Unlock(NULL);
   }

   void PinDirectDraw::CreateNextMipMapLevel(BaseTexture* pdds)
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
               if (a0) { count++; rtotal+=pbytes1[2]; gtotal+=pbytes1[1]; btotal+=pbytes1[0]; } //!! opt.
               const unsigned int a1 = pbytes1[7];
               if (a1) { count++; rtotal+=pbytes1[6]; gtotal+=pbytes1[5]; btotal+=pbytes1[4]; }
               const unsigned int a2 = pbytes2[3];
               if (a2) { count++; rtotal+=pbytes2[2]; gtotal+=pbytes2[1]; btotal+=pbytes2[0]; }
               const unsigned int a3 = pbytes2[7];
               if (a3) { count++; rtotal+=pbytes2[6]; gtotal+=pbytes2[5]; btotal+=pbytes2[4]; }

               if (count == 0) // all pixels are transparent - do whatever
               {
                  *(unsigned int*)pchNext = 0;
               } else {
                  const unsigned int atotal = ((a0 + a1 + a2 + a3 + 2)>>2)<<24;
                  const unsigned int round = count>>1;
                  btotal += round;
                  gtotal += round;
                  rtotal += round;
                  if(count == 2) {
                     btotal >>= 1;
                     gtotal >>= 1;
                     rtotal >>= 1;
                  } else if(count == 4) {
                     btotal >>= 2;
                     gtotal >>= 2;
                     rtotal >>= 2;
                  } else if(count == 3) {
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
