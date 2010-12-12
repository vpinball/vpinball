// PinImage.cpp: implementation of the PinImage class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "freeimage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PinImage::PinImage()
{
	m_pdsBuffer = NULL;
	m_pdsBufferColorKey = NULL;
	m_pdsBufferBackdrop = NULL;
	m_rgbTransparent = RGB(255,255,255);
	m_hbmGDIVersion = NULL;
	m_ppb = NULL;
}

PinImage::~PinImage()
{
	FreeStuff();
}

HRESULT PinImage::SaveToStream(IStream *pstream, PinTable *pt)
	{
	BiffWriter bw(pstream, NULL, NULL);

	bw.WriteString(FID(NAME), m_szName);

	bw.WriteString(FID(INME), m_szInternalName);

	bw.WriteString(FID(PATH), m_szPath);

	bw.WriteInt(FID(WDTH), m_width);
	bw.WriteInt(FID(HGHT), m_height);

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

HRESULT PinImage::LoadFromStream(IStream *pstream, int version, PinTable *pt)
	{
	BiffReader br(pstream, this, pt, version, NULL, NULL);

	br.Load();

	EnsureMaxTextureCoordinates();

	return ((m_pdsBuffer != NULL) ? S_OK : E_FAIL);
	}

BOOL PinImage::LoadToken(int id, BiffReader *pbr)
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
		}
	else if (id == FID(HGHT))
		{
		pbr->GetInt(&m_height);
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

		/*const HRESULT hr =*/ m_pdsBuffer->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

		// 32-bit picture
		LZWReader lzwreader(pbr->m_pistream, (int *)ddsd.lpSurface, m_width*4, m_height, ddsd.lPitch);

		lzwreader.Decoder();

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

		// check if Textures exeed the maximum texture dimension
		int maxTexDim;
		HRESULT hrMaxTex = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
		if (hrMaxTex != S_OK)
		{
			maxTexDim = 0; // default: Don't resize textures
		}
		int pictureWidth = FreeImage_GetWidth(dib);
		int pictureHeight = FreeImage_GetHeight(dib);
		if (((pictureHeight > maxTexDim) ||  (pictureWidth > maxTexDim)) && (maxTexDim != 0))
		{
			dib = FreeImage_Rescale(dib, maxTexDim, maxTexDim, FILTER_BILINEAR);
			m_width = maxTexDim;
			m_height = maxTexDim;
		}

		HDC hDC = GetDC(NULL);

		HBITMAP hbm = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);

		//slintf("Pixel format: %s",hbm.PixelFormat.ToString());
		//g_pvp->m_pdd.get


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


		// check if Textures exeed the maximum texture dimension
		int maxTexDim;
		HRESULT hrMaxTex = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
		if (hrMaxTex != S_OK)
		{
			maxTexDim = 0; // default: Don't resize textures
		}
		int pictureWidth = FreeImage_GetWidth(dib);
		int pictureHeight = FreeImage_GetHeight(dib);
		if (((pictureHeight > maxTexDim) ||  (pictureWidth > maxTexDim)) && (maxTexDim != 0))
		{
			dib = FreeImage_Rescale(dib, maxTexDim, maxTexDim, FILTER_BILINEAR);
			m_width = maxTexDim;
			m_height = maxTexDim;
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

void PinImage::SetTransparentColor(const COLORREF color)
	{
	m_fTransparent = fFalse;
	if (m_rgbTransparent != color)
		{
		m_rgbTransparent = color;
		SAFE_RELEASE(m_pdsBufferColorKey);
		SAFE_RELEASE(m_pdsBufferBackdrop);
		}
	}

void PinImage::EnsureColorKey()
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

void PinImage::EnsureBackdrop(const COLORREF color)
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

void PinImage::EnsureMaxTextureCoordinates()
	{
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);

	m_pdsBuffer->GetSurfaceDesc(&ddsd);

	m_maxtu = (float)m_width / (float)ddsd.dwWidth;
	m_maxtv = (float)m_height / (float)ddsd.dwHeight;
	}

void PinImage::FreeStuff()
	{
	SAFE_RELEASE(m_pdsBuffer);
	SAFE_RELEASE(m_pdsBufferColorKey);
	SAFE_RELEASE(m_pdsBufferBackdrop);
	if (m_hbmGDIVersion)
		{
		DeleteObject(m_hbmGDIVersion);
		}
	m_pdsBuffer = NULL;
	m_pdsBufferColorKey = NULL;
	m_pdsBufferBackdrop = NULL;
	m_hbmGDIVersion = NULL;
	if (m_ppb)
		{
		delete m_ppb;
		m_ppb = NULL;
		}
	}

void PinImage::EnsureHBitmap()
	{
	if (!m_hbmGDIVersion)
		{
		CreateGDIVersion();
		}
	}

void PinImage::CreateGDIVersion()
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

PinDirectDraw::PinDirectDraw()
	{
	m_pDD = NULL;

	int tmp = 0;										
	HRESULT hr = GetRegInt("Player", "HardwareRender", &tmp);
	m_fHardwareAccel = (tmp != 0);

	tmp = 0;										
	hr = GetRegInt("Player", "UseD3DBlit", &tmp);
	m_fUseD3DBlit = (tmp != 0);
	}

PinDirectDraw::~PinDirectDraw()
	{
	SAFE_RELEASE(m_pDD);
	}

HRESULT PinDirectDraw::InitDD()
	{
	m_DDraw=LoadLibrary("ddraw.dll");
	m_DDCreate=(DDCreateFunction)GetProcAddress(m_DDraw,"DirectDrawCreateEx");

	if (m_DDCreate == NULL)
		{
		if (m_DDraw != NULL)
			{
			FreeLibrary(m_DDraw);
			}

		LocalString ls(IDS_NEED_DD9);
		MessageBox(g_pvp->m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);

		return E_FAIL;
		}

	HRESULT hr = (*m_DDCreate)(NULL, (VOID **)&m_pDD, IID_IDirectDraw7, NULL);
	if (hr != S_OK)
		{
		ShowError("Could not create Direct Draw.");
		}

	hr = m_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL | DDSCL_FPUPRESERVE);
	if (hr != S_OK)
		{
		ShowError("Could not set Direct Draw cooperative level.");
		}

	return S_OK;
	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateTextureOffscreen(const int width, const int height)
	{
	DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	// Texture dimensions must be in powers of 2
	int texwidth  = 1 << ((int)(logf((float)(width -1))*((float)(1.0/log(2.0))) + 0.001f/*round-off*/)+1);
	int texheight = 1 << ((int)(logf((float)(height-1))*((float)(1.0/log(2.0))) + 0.001f/*round-off*/)+1);

	// D3D does not support textures greater than 4096 in either dimension
	if (texwidth > MAX_TEXTURE_SIZE)
		{
		texwidth = MAX_TEXTURE_SIZE;
		}

	if (texheight > MAX_TEXTURE_SIZE)
		{
		texheight = MAX_TEXTURE_SIZE;
		}

	// Or smaller than 8
	if (texwidth < 8)
		{
		texwidth = 8;
		}
		
	if (texheight < 8)
		{
		texheight = 8;
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

	LPDIRECTDRAWSURFACE7 pdds;
	HRESULT hr;
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
		{
		ShowError("Could not create texture offscreen surface.");
		return NULL;
		}

	// Update the count (including mipmaps).
//	NumVideoBytes += (ddsd.dwWidth * ddsd.dwHeight * 4) * (4.0/3.0);

	pdds->SetLOD(0);

	return pdds;
	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateFromFile(char *szfile, int * const pwidth, int * const pheight)
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

		// check if Textures exeed the maximum texture diemension
		int maxTexDim;
		HRESULT hrMaxTex = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
		if (hrMaxTex != S_OK)
		{
			maxTexDim = 0; // default: Don't resize textures
		}
		int pictureWidth = FreeImage_GetWidth(dib);
		int pictureHeight = FreeImage_GetHeight(dib);
		if (((pictureHeight > maxTexDim) ||  (pictureWidth > maxTexDim)) && (maxTexDim != 0))
		{
			dib = FreeImage_Rescale(dib, maxTexDim, maxTexDim, FILTER_BILINEAR);
		}

		HDC hDC = GetDC(NULL);
		HBITMAP hbm = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
		int bitsPerPixel = FreeImage_GetBPP(dib);
		int dibWidth = FreeImage_GetWidth(dib);
		int dibHeight = FreeImage_GetHeight(dib);
		FreeImage_Unload(dib);
		//HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, szfile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

		LPDIRECTDRAWSURFACE7 mySurface = g_pvp->m_pdd.CreateFromHBitmap(hbm, pwidth, pheight);
		//LPDIRECTDRAWSURFACE7 mySurface = CreateFromHBitmap(hbm, pwidth, pheight);

		if (bitsPerPixel == 24)
			g_pvp->m_pdd.SetOpaque(mySurface, dibWidth, dibHeight);

		if (hbm == NULL)
		{
			return NULL;
		}
		
		return mySurface;
	}
	else
		return NULL;


	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateFromResource(const int id, int * const pwidth, int * const pheight)
	{
	HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hbm == NULL)
		{
		return NULL;
		}

	return CreateFromHBitmap(hbm, pwidth, pheight);
	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight)
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
		return NULL; //rlc MAX_TEXTURE_SIZE is the limit for directx textures
		}

	LPDIRECTDRAWSURFACE7 pdds = CreateTextureOffscreen(bm.bmWidth, bm.bmHeight);

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

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  boolean is_os2;				/* saves the OS2 format request flag */

  jvirt_sarray_ptr whole_image;	/* needed to reverse row order */
  JDIMENSION data_width;		/* JSAMPLEs per row */
  JDIMENSION row_width;			/* physical width of one row in the BMP file */
  int pad_bytes;				/* number of padding bytes needed per row */
  JDIMENSION cur_output_row;	/* next row# to write to virtual array */
  DDSURFACEDESC2 *pddsd;
} bmp_dest_struct;

typedef bmp_dest_struct * bmp_dest_ptr;

METHODDEF(void) put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, JDIMENSION rows_supplied)
/* This version is for writing 24-bit pixels */
{
  bmp_dest_ptr const dest = (bmp_dest_ptr) dinfo;

  /* Access next row in virtual array */
  const JSAMPARRAY image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);

  BYTE *prawcolor = (BYTE *)dest->pddsd->lpSurface + (dest->pddsd->lPitch * dest->cur_output_row);

  dest->cur_output_row++;

  /* Transfer data.  Note destination values must be in BGR order
   * (even though Microsoft's own documents say the opposite).
   */
  JSAMPROW inptr = dest->pub.buffer[0];
  JSAMPROW outptr = image_ptr[0];
  for (JDIMENSION col = cinfo->output_width; col > 0; col--) {
    outptr += 3;
	prawcolor[2] = inptr[0];
	prawcolor[1] = inptr[1];
	prawcolor[0] = inptr[2];
	prawcolor[3] = 255;		//alpha set to opaque
	prawcolor += 4;
	inptr += 3;
  }

  /* Zero out the pad bytes. */
  int pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}
METHODDEF(void) start_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  /* no work here */
}
METHODDEF(void) finish_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  
}
GLOBAL(djpeg_dest_ptr)jinit_write_bmp (j_decompress_ptr cinfo, boolean is_os2, DDSURFACEDESC2 *pddsd)
{
  /* Create module interface object, fill in method pointers */
  bmp_dest_ptr const dest = (bmp_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(bmp_dest_struct));
  dest->pub.start_output = start_output_bmp;
  dest->pub.finish_output = finish_output_bmp;
  dest->is_os2 = is_os2;
  dest->pddsd = pddsd;

  if (cinfo->out_color_space == JCS_GRAYSCALE) {

  } else if (cinfo->out_color_space == JCS_RGB) {
    dest->pub.put_pixel_rows = put_pixel_rows;
  } else {
      //SHagendo 18 June 2010: cinfo's errexit pointer is invalid, caller must check for returned NULL pointer
      //ERREXIT(cinfo, JERR_BMP_COLORSPACE);      
      return NULL;
  }

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Determine width of rows in the BMP file (padded to 4-byte boundary). */
  JDIMENSION row_width = cinfo->output_width * cinfo->output_components;
  dest->data_width = row_width;
  while ((row_width & 3) != 0) row_width++;
  dest->row_width = row_width;
  dest->pad_bytes = (int) (row_width - dest->data_width);

  /* Allocate space for inversion array, prepare for write pass */
  dest->whole_image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     row_width, cinfo->output_height, (JDIMENSION) 1);
  dest->cur_output_row = 0;
  if (cinfo->progress != NULL) {
    cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
    progress->total_extra_passes++; /* count file input as separate pass */
  }

  /* Create decompressor output buffer. */
  dest->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, row_width, (JDIMENSION) 1);
  dest->pub.buffer_height = 1;

  return (djpeg_dest_ptr) dest;
}

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */

  PinImage *m_ppi;		/* source stream */
  PinBinary *m_ppb;
} PinImageSourceManager;

METHODDEF(boolean) fill_input_buffer (j_decompress_ptr cinfo)
{
  PinImageSourceManager * const src = (PinImageSourceManager *) cinfo->src;

  src->pub.next_input_byte = (BYTE *)src->m_ppb->m_pdata;
  src->pub.bytes_in_buffer = src->m_ppb->m_cdata;

  return TRUE;
}

METHODDEF(void) init_source (j_decompress_ptr cinfo)
{
  //my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  //src->start_of_file = TRUE;
}

METHODDEF(void) skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  PinImageSourceManager * const src = (PinImageSourceManager *) cinfo->src;

  src->pub.next_input_byte += num_bytes;//(BYTE *)src->m_ppb->m_pdata;
  src->pub.bytes_in_buffer -= num_bytes;//src->m_ppb->m_cdata;
  
}

METHODDEF(void) term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}

LPDIRECTDRAWSURFACE7 PinDirectDraw::DecompressJPEG(PinImage * const ppi, PinBinary * const ppb, int * const pwidth, int * const pheight)
	{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;

	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->msg_code = 0;
	jpeg_create_decompress(&cinfo);

  if (cinfo.src == NULL) {	/* first time for this JPEG object? */
    cinfo.src = (struct jpeg_source_mgr *)
      (cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
				  SIZEOF(PinImageSourceManager));
  }
  PinImageSourceManager * const pism = (PinImageSourceManager *) cinfo.src;
  pism->pub.init_source = init_source;
  pism->pub.fill_input_buffer = fill_input_buffer;
  pism->pub.skip_input_data = skip_input_data;
  pism->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  pism->pub.term_source = term_source;
  pism->m_ppi = ppi;
  pism->m_ppb = ppb;
  pism->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  pism->pub.next_input_byte = NULL; /* until buffer loaded */	

	(void)jpeg_read_header(&cinfo, TRUE);

	if (pwidth)
		{
		*pwidth = cinfo.image_width;
		}

	if (pheight)
		{
		*pheight = cinfo.image_height;
		}

	if (cinfo.image_width > MAX_TEXTURE_SIZE || cinfo.image_height > MAX_TEXTURE_SIZE)
		{
		jpeg_destroy_decompress(&cinfo);
		ShowErrorID(IDS_IMAGETOOLARGE);
		return NULL; // 4k*4k is the limit for directx textures
		}

	LPDIRECTDRAWSURFACE7 pdds = CreateTextureOffscreen(cinfo.image_width, cinfo.image_height);
	if (pdds == NULL)
		{
		return NULL;
		}

	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

    dest_mgr = jinit_write_bmp(&cinfo, FALSE, &ddsd);
    //SHagendo 18 June 2010: jinit_write_bmp will return NULL pointer if colorspace is invalid (not RGB or Grayscale)
   if (dest_mgr == NULL)
   {
      MessageBox(g_pvp->m_hwnd, "Import aborted. Image is not in RGB or Grayscale format.", "Visual Pinball", MB_ICONWARNING);
      return NULL;
   }

  jpeg_start_decompress(&cinfo);

  (*dest_mgr->start_output) (&cinfo, dest_mgr);

  while (cinfo.output_scanline < cinfo.output_height) 
	  {
		JDIMENSION num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer, dest_mgr->buffer_height);
		(*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
	 }

	
	(*dest_mgr->finish_output) (&cinfo, dest_mgr);
	
	jpeg_finish_decompress(&cinfo);

	pdds->Unlock(NULL);


	jpeg_destroy_decompress(&cinfo);	
	
	// Alpha channel set to 0xFF in decompressor and BMP copy
	//if (cinfo.num_components != 4 )g_pvp->m_pdd.SetOpaque(pdds, bm.bmWidth, bm.bmHeight);//rlc no alpha JPG (24 bit only)

	return pdds;
	}

void PinDirectDraw::SetOpaque(LPDIRECTDRAWSURFACE7 pdds, const int width, const int height)
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

void PinDirectDraw::SetOpaqueBackdrop(LPDIRECTDRAWSURFACE7 pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop, const int width, const int height)
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
				*(unsigned int *)pch = rgbBd;  //rlc optimized
				}
			pch += 4;
			}
		pch += pitch-(width*4);
		}

	pdds->Unlock(NULL);
	}


BOOL PinDirectDraw::SetAlpha(LPDIRECTDRAWSURFACE7 pdds, const COLORREF rgbTransparent, const int width, const int height)
	{
	// Set alpha of each pixel

	
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	pdds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	
	BOOL fTransparent = fFalse;	

	const int pitch = ddsd.lPitch;

	const COLORREF rtrans = (rgbTransparent & 0x000000ff);			//rlc fixed directx texture red-blue color reversal
	const COLORREF gtrans = (rgbTransparent & 0x0000ff00) >> 8;
	const COLORREF btrans = (rgbTransparent & 0x00ff0000) >> 16;

	const COLORREF bgrTransparent = btrans | (gtrans << 8) | (rtrans << 16) | 0xff000000;  //rlc color order different in DirectX texture buffer
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
		//slintf("amax:%d amin:%d\n",aMax,aMin);
		pch = (BYTE *)ddsd.lpSurface;


		for (int i=0;i<height;i++)
			{
			for (int l=0;l<width;l++)
				{	
				const COLORREF tc = (*(COLORREF *)pch) | 0xff000000; //set to opaque
				if (tc == bgrTransparent )					//rlc reg-blue order reversed
					{
					*(unsigned int *)pch = 0x00000000;		//rlc set transparent colorkey to black	and alpha transparent	
					fTransparent = fTrue;					//colorkey is true
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
	1, 2, 3, 4, 3, 2, 1,
	2, 3, 4, 5, 4, 3, 2,
	3, 4, 5, 6, 5, 4, 3,
	4, 5, 6, 7, 6, 5, 4,
	3, 4, 5, 6, 5, 4, 3,
	2, 3, 4, 5, 4, 3, 2,
	1, 2, 3, 4, 3, 2, 1};

void PinDirectDraw::Blur(LPDIRECTDRAWSURFACE7 pdds, const BYTE * const pbits, const int shadwidth, const int shadheight)
	{
	if (!pbits) return;	//rlc  found this pointer to be NULL after some graphics errors

	// Create Gaussian window (actually its not really Gaussian, but same idea)

	int window[7][7];
	for (int i=0;i<4;i++)
		{
		window[0][i] = i+1;
		window[0][6-i] = i+1;
		window[i][0] = i+1;
		window[6-i][0] = i+1;
		}

	int totalwindow = 0;
	for (int i=0;i<7;i++)
		{
		for (int l=0;l<7;l++)
			{
			window[i][l] = window[0][l] * window[i][0];
			window[i][l] = rgfilterwindow[i][l];
			totalwindow += window[i][l];
			}
		}

	// Gaussian Blur the sharp shadows

	DDSURFACEDESC2 ddsd;//, ddsdSharp;
	ddsd.dwSize = sizeof(ddsd);

	pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	
	const int pitch = ddsd.lPitch;
	const int pitchSharp = 256*3;
	BYTE *pc = (BYTE *)ddsd.lpSurface;

	for (int i=0;i<shadheight;i++)
		{
		for (int l=0;l<shadwidth;l++)
			{
			int value = 0;
			int totalvalue = totalwindow;

			for (int m=0;m<7;m++)
				{
				for (int n=0;n<7;n++)
					{
					const int x = l+m-3;
					const int y = i+n-3;
					if (x>=0 && x<=(shadwidth-1) && y>=0 && y<=(shadheight-1)) //!! opt.
						{						
						value += (int)(*(pbits + x*3 + pitchSharp*y)) * window[m][n];
						}
					else
						{
						totalvalue -= window[m][n];
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

void PinDirectDraw::BlurAlpha(LPDIRECTDRAWSURFACE7 pdds)
	{
	// Create Gaussian window (actually its not really Gaussian, but same idea)

	int window[7][7];
	for (int i=0;i<4;++i)
		{
		window[0][i] = i+1;
		window[0][6-i] = i+1;
		window[i][0] = i+1;
		window[6-i][0] = i+1;
		}

	int totalwindow = 0;
	for (int i=0;i<7;++i)
		{
		for (int l=0;l<7;++l)
			{
			window[i][l] = window[0][l] * window[i][0];
			window[i][l] = rgfilterwindow[i][l];
			totalwindow += window[i][l];
			}
		}

	// Gaussian Blur the sharp shadows

	DDSURFACEDESC2 ddsd;//, ddsdSharp;
	ddsd.dwSize = sizeof(ddsd);

	pdds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	
	const int pitch = ddsd.lPitch;
	
	BYTE * const pc = (BYTE *)ddsd.lpSurface;

	const int width = (int)ddsd.dwWidth;
	const int height = (int)ddsd.dwHeight;

	for (int i=0;i<height;i++)
		{
		for (int l=0;l<width;l++)
			{
			int value = 0;
			for (int m=0;m<7;m++)
				{
				for (int n=0;n<7;n++)
					{
					const int x = l+m-3;
					const int y = i+n-3;
					if (x>=0 && x<=15 && y>=0 && y<=15) //!! opt.
						{
						value += (int)(*(pc + 4*x + pitch*y)) * window[m][n];
						}
					}
				}

			value /= totalwindow;

			value = /*127 + */(value*5)>>3;

			*(pc + pitch*i + l*4 + 3) = (BYTE)value; //!! potential bug: blurring within the same buffer leads to artifacts
			}
		}

	pdds->Unlock(NULL);
	}

//#pragma optimize("atg", on)

void PinDirectDraw::CreateNextMipMapLevel(LPDIRECTDRAWSURFACE7 pdds)
	{
	DDSURFACEDESC2 ddsd, ddsdNext;
	ddsd.dwSize = sizeof(ddsd);
	ddsdNext.dwSize = sizeof(ddsd);
	DDSCAPS2 ddsCaps;
	ddsCaps.dwCaps2 = 0;
	ddsCaps.dwCaps3 = 0;
	ddsCaps.dwCaps4 = 0;
	ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;	

	LPDIRECTDRAWSURFACE7 pddsNext;
	HRESULT hr = pdds->GetAttachedSurface(&ddsCaps, &pddsNext);

	if (hr == S_OK)
		{
		hr = pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		hr = pddsNext->Lock(NULL, &ddsdNext, DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

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
				if (a0) { count++; rtotal+=pbytes1[2]; gtotal+=pbytes1[1]; btotal+=pbytes1[0]; } //rlc faster code
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
