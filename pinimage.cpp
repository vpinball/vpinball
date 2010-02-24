// PinImage.cpp: implementation of the PinImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAXTEXTURESIZE 4096 // 1024

PinImage::PinImage()
{
	m_pdsBuffer = NULL;
	m_pdsBufferColorKey = NULL;
	m_pdsBufferBackdrop = NULL;
	m_rgbTransparent = RGB(255,255,255);
	m_hbmGDIVersion = NULL;
	m_ppb = NULL;
	m_fUnneededAfterCache = fFalse;
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
	
	bw.WriteBool(FID(CACH), m_fUnneededAfterCache);

	if (!m_ppb)
		{
		DDSURFACEDESC2 ddsd;
		ddsd.dwSize = sizeof(ddsd);

		m_pdsBuffer->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		char *pch = (char *)ddsd.lpSurface;

		bw.WriteTag(FID(BITS));

		// 32-bit picture
		LZWWriter lzwwriter(pstream, (int *)ddsd.lpSurface, m_width*4, m_height, ddsd.lPitch);

		lzwwriter.CompressBits(8+1);

		m_pdsBuffer->Unlock(NULL);
		}
	else // JPEG (or other binary format)
		{
		int linkid = pt->GetImageLink(this);
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
	else if (id == FID(CACH))
		{
		pbr->GetBool(&m_fUnneededAfterCache);
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
		//if (g_pvp->m_fPlayOnly && m_fUnneededAfterCache
		m_pdsBuffer = g_pvp->m_pdd.CreateTextureOffscreen(m_width, m_height);

		if (m_pdsBuffer == NULL)
			{
			return fFalse;
			}

		HRESULT hr;
		DDSURFACEDESC2 ddsd;
		ddsd.dwSize = sizeof(ddsd);

		hr = m_pdsBuffer->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

		char *pch = (char *)ddsd.lpSurface;

		// 32-bit picture
		LZWReader lzwreader(pbr->m_pistream, (int *)ddsd.lpSurface, m_width*4, m_height, ddsd.lPitch);

		lzwreader.Decoder();

		m_pdsBuffer->Unlock(NULL);
		}
	else if (id == FID(JPEG))
		{
		m_ppb = new PinBinary();
		m_ppb->LoadFromStream(pbr->m_pistream, pbr->m_version);
		m_pdsBuffer = g_pvp->m_pdd.DecompressJPEG(this, m_ppb, &m_width, &m_height);

		if (m_pdsBuffer == NULL)
			{
			return fFalse;
			}
		}
	else if (id == FID(LINK))
		{
		int linkid;
		PinTable *pt = (PinTable *)pbr->m_pdata;
		pbr->GetInt(&linkid);
		m_ppb = pt->GetImageLinkBinary(linkid);
		m_pdsBuffer = g_pvp->m_pdd.DecompressJPEG(this, m_ppb, &m_width, &m_height);

		if (m_pdsBuffer == NULL)
			{
			return fFalse;
			}
		}
	return fTrue;
	}

void PinImage::SetTransparentColor(COLORREF color)
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
		m_pdsBufferColorKey->Blt(NULL,m_pdsBuffer,NULL,0,NULL);
		m_fTransparent = g_pvp->m_pdd.SetAlpha(m_pdsBufferColorKey, m_rgbTransparent, m_width, m_height);
		g_pvp->m_pdd.CreateNextMipMapLevel(m_pdsBufferColorKey);
		}
	}

void PinImage::EnsureBackdrop(COLORREF color)
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
		m_pdsBufferBackdrop->Blt(NULL,m_pdsBuffer,NULL,0,NULL);
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
	/*if (m_hbmGDIVersion)
		{
		DeleteObject(m_hbmGDIVersion);
		m_hbmGDIVersion = NULL;
		}*/

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
	m_fHardwareAccel = fFalse;
	m_pDD = NULL;
	}

PinDirectDraw::~PinDirectDraw()
	{
	SAFE_RELEASE(m_pDD);
	}

HRESULT PinDirectDraw::InitDD()
	{
	HRESULT hr;

	m_DDraw=LoadLibrary("ddraw.dll");
	m_DDCreate=(DDCreateFunction)GetProcAddress(m_DDraw,"DirectDrawCreateEx");

	if (m_DDCreate == NULL)
		{
		if (m_DDraw != NULL)
			{
			FreeLibrary(m_DDraw);
			}

		LocalString ls(IDS_NEED_DD7);
		MessageBox(g_pvp->m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);

		return E_FAIL;
		}

	hr = (*m_DDCreate)(NULL, (VOID **)&m_pDD, IID_IDirectDraw7, NULL);
	if (hr != S_OK)
		{
		ShowError("Could not create Direct Draw.");
		}

	hr = m_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL | DDSCL_FPUPRESERVE);
	if (hr != S_OK)
		{
		ShowError("Could not set direct draw cooperative level.");
		}

	return S_OK;
	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateTextureOffscreen(int width, int height)
	{
	const GUID* pDeviceGUID = &IID_IDirect3DRGBDevice;
	DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	// Texture dimensions must be in powers of 2
	int texwidth, texheight;
	texwidth = 1 << ((int)(log((float)(width-1))/log(2.0f) + 0.001f/*round-off*/)+1);
	texheight = 1 << ((int)(log((float)(height-1))/log(2.0f) + 0.001f/*round-off*/)+1);

	// D3D does not support textures greater than 1024 in either dimension
	if (texwidth > MAXTEXTURESIZE)
		{
		texwidth = MAXTEXTURESIZE;
		}

	if (texheight > MAXTEXTURESIZE)
		{
		texheight = MAXTEXTURESIZE;
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

	LPDIRECTDRAWSURFACE7 pdds;
	HRESULT hr;

	/*DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize          = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags         = DDSD_CAPS|DDSD_MIPMAPCOUNT|DDSD_WIDTH|DDSD_HEIGHT|
                           DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_MIPMAP|DDSCAPS_COMPLEX;
    ddsd.dwMipMapCount   = m_dwMipMapCount;
    ddsd.dwWidth         = bm.bmWidth;
    ddsd.dwHeight        = bm.bmHeight;*/

	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | /*DDSD_CKSRCBLT |*/ DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = 0;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = 0;
    ddsd.dwWidth        = texwidth;
    ddsd.dwHeight       = texheight;
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE|DDSCAPS_MIPMAP|DDSCAPS_COMPLEX;
	ddsd.dwMipMapCount	= 4;

	if (m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE; //ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS ;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	ddsd.ddpfPixelFormat.dwRBitMask = 0xff0000;
	ddsd.ddpfPixelFormat.dwGBitMask = 0x00ff00;
	ddsd.ddpfPixelFormat.dwBBitMask = 0x0000ff;
	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
		{
		ShowError("Could not create texture offscreen surface.");
		return NULL;
		}

	//DWORD l;
	//pdds->GetLOD(&l);

	pdds->SetLOD(4);

	return pdds;
	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateFromFile(char *szfile, int *pwidth, int *pheight)
	{
	HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, szfile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	if (hbm == NULL)
		{
		return NULL;
		}

	return CreateFromHBitmap(hbm, pwidth, pheight);
	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateFromResource(int id, int *pwidth, int *pheight)
	{
	HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hbm == NULL)
		{
		return NULL;
		}

	return CreateFromHBitmap(hbm, pwidth, pheight);
	}

LPDIRECTDRAWSURFACE7 PinDirectDraw::CreateFromHBitmap(HBITMAP hbm, int *pwidth, int *pheight)
	{
	LPDIRECTDRAWSURFACE7 pdds;

	BITMAP bm;
	HBITMAP hbmOld;

	GetObject(hbm, sizeof(bm), &bm);

	if (pwidth)
		{
		*pwidth = bm.bmWidth;
		}

	if (pheight)
		{
		*pheight = bm.bmHeight;
		}
		
	/*D3DDEVICEDESC ddcaps;
	
	ddcaps.dwMaxTextureWidth;
		
	g_pvp->m_pdd.m_pd3d->GetCaps(
	
	d3ddevice->GetCaps()*/

	if (bm.bmWidth > MAXTEXTURESIZE || bm.bmHeight > MAXTEXTURESIZE)
		{
		return NULL; // 1k*1k is the limit for directx textures
		}

	pdds = CreateTextureOffscreen(bm.bmWidth, bm.bmHeight);

	HDC hdc;

	pdds->GetDC(&hdc);

	HDC hdcFoo = CreateCompatibleDC(hdc);

	hbmOld = (HBITMAP)SelectObject(hdcFoo, hbm);

	BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcFoo, 0, 0, SRCCOPY);

	SelectObject(hdcFoo, hbmOld);

	DeleteDC(hdcFoo);

	DeleteObject(hbm);

	pdds->ReleaseDC(hdc);

	//SetAlpha(pdds, RGB(0,0,0));

	return pdds;
	}

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  boolean is_os2;		/* saves the OS2 format request flag */

  jvirt_sarray_ptr whole_image;	/* needed to reverse row order */
  JDIMENSION data_width;	/* JSAMPLEs per row */
  JDIMENSION row_width;		/* physical width of one row in the BMP file */
  int pad_bytes;		/* number of padding bytes needed per row */
  JDIMENSION cur_output_row;	/* next row# to write to virtual array */
  DDSURFACEDESC2 *pddsd;
} bmp_dest_struct;
typedef bmp_dest_struct * bmp_dest_ptr;
METHODDEF(void)
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
/* This version is for writing 24-bit pixels */
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  int pad;

  /* Access next row in virtual array */
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);

  BYTE *prawcolor = (BYTE *)dest->pddsd->lpSurface + (dest->pddsd->lPitch * dest->cur_output_row);

  dest->cur_output_row++;

  /* Transfer data.  Note destination values must be in BGR order
   * (even though Microsoft's own documents say the opposite).
   */
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    //outptr[2] = *inptr++;	/* can omit GETJSAMPLE() safely */
    //outptr[1] = *inptr++;
    //outptr[0] = *inptr++;
    outptr += 3;
	prawcolor[2] = *inptr++;
	prawcolor[1] = *inptr++;
	prawcolor[0] = *inptr++;
	prawcolor[3] = 255;
	prawcolor+=4;
  }

  /* Zero out the pad bytes. */
  pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}
METHODDEF(void)
start_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  /* no work here */
}
METHODDEF(void)
finish_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  /*bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  register FILE * outfile = dest->pub.output_file;
  JSAMPARRAY image_ptr;
  register JSAMPROW data_ptr;
  JDIMENSION row;
  register JDIMENSION col;
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

  /* Write the header and colormap */
  //if (dest->is_os2)
    //write_os2_header(cinfo, dest);
  //else
    //write_bmp_header(cinfo, dest);

  /* Write the file body from our virtual array */
  /*for (row = cinfo->output_height; row > 0; row--) {
    if (progress != NULL) {
      progress->pub.pass_counter = (long) (cinfo->output_height - row);
      progress->pub.pass_limit = (long) cinfo->output_height;
      (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
    }
    image_ptr = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, dest->whole_image, row-1, (JDIMENSION) 1, FALSE);
    data_ptr = image_ptr[0];
    for (col = dest->row_width; col > 0; col--) {
      putc(GETJSAMPLE(*data_ptr), outfile);
      data_ptr++;
    }
  }
  if (progress != NULL)
    progress->completed_extra_passes++;

  /* Make sure we wrote the output file OK */
  /*fflush(outfile);
  if (ferror(outfile))
    ERREXIT(cinfo, JERR_FILE_WRITE);*/
}
GLOBAL(djpeg_dest_ptr)
jinit_write_bmp (j_decompress_ptr cinfo, boolean is_os2, DDSURFACEDESC2 *pddsd)
{
  bmp_dest_ptr dest;
  JDIMENSION row_width;

  /* Create module interface object, fill in method pointers */
  dest = (bmp_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(bmp_dest_struct));
  dest->pub.start_output = start_output_bmp;
  dest->pub.finish_output = finish_output_bmp;
  dest->is_os2 = is_os2;
  dest->pddsd = pddsd;

  if (cinfo->out_color_space == JCS_GRAYSCALE) {
    //dest->pub.put_pixel_rows = put_gray_rows;
  } else if (cinfo->out_color_space == JCS_RGB) {
    //if (cinfo->quantize_colors)
      //dest->pub.put_pixel_rows = put_gray_rows;
    //else
      dest->pub.put_pixel_rows = put_pixel_rows;
  } else {
    ERREXIT(cinfo, JERR_BMP_COLORSPACE);
  }

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Determine width of rows in the BMP file (padded to 4-byte boundary). */
  row_width = cinfo->output_width * cinfo->output_components;
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
  //JOCTET * buffer;		/* start of buffer */
  //boolean start_of_file;	/* have we gotten any data yet? */
} PinImageSourceManager;

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  PinImageSourceManager *src = (PinImageSourceManager *) cinfo->src;

  src->pub.next_input_byte = (BYTE *)src->m_ppb->m_pdata;
  src->pub.bytes_in_buffer = src->m_ppb->m_cdata;

  return TRUE;
}

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  //my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  //src->start_of_file = TRUE;
}

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  PinImageSourceManager *src = (PinImageSourceManager *) cinfo->src;

  src->pub.next_input_byte += num_bytes;//(BYTE *)src->m_ppb->m_pdata;
  src->pub.bytes_in_buffer -= num_bytes;//src->m_ppb->m_cdata;
  /*my_src_ptr src = (my_src_ptr) cinfo->src;

  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }*/
}

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}

LPDIRECTDRAWSURFACE7 PinDirectDraw::DecompressJPEG(PinImage *ppi, PinBinary *ppb, int *pwidth, int *pheight)
	{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;
	//FILE * output_file;
	JDIMENSION num_scanlines;

	PinImageSourceManager* pism;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

  if (cinfo.src == NULL) {	/* first time for this JPEG object? */
    cinfo.src = (struct jpeg_source_mgr *)
      (cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
				  SIZEOF(PinImageSourceManager));
    pism = (PinImageSourceManager *) cinfo.src;
    /*pism->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  INPUT_BUF_SIZE * SIZEOF(JOCTET));*/
  }
  pism = (PinImageSourceManager *) cinfo.src;
  pism->pub.init_source = init_source;
  pism->pub.fill_input_buffer = fill_input_buffer;
  pism->pub.skip_input_data = skip_input_data;
  pism->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  pism->pub.term_source = term_source;
  pism->m_ppi = ppi;
  pism->m_ppb = ppb;
  pism->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  pism->pub.next_input_byte = NULL; /* until buffer loaded */

	//FILE * input_file;
	//input_file = fopen("d:\\gdk\\Data\\Tables\\Test\\policeforcebg.jpg", READ_BINARY);
	//output_file = fopen("d:\\gdk\\temp\\foo.bmp", WRITE_BINARY);

	//jpeg_create_decompress(&cinfo);
  /* Add some application-specific error messages (from cderror.h) */
  /*jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;*/

	//jpeg_stdio_src(&cinfo, input_file);

	(void) jpeg_read_header(&cinfo, TRUE);

	if (pwidth)
		{
		*pwidth = cinfo.image_width;
		}

	if (pheight)
		{
		*pheight = cinfo.image_height;
		}

	if (cinfo.image_width > MAXTEXTURESIZE || cinfo.image_height > MAXTEXTURESIZE)
		{
		jpeg_destroy_decompress(&cinfo);
		ShowErrorID(IDS_IMAGETOOLARGE);
		return NULL; // 1k*1k is the limit for directx textures
		}

	LPDIRECTDRAWSURFACE7 pdds;
	pdds = CreateTextureOffscreen(cinfo.image_width, cinfo.image_height);
	if (pdds == NULL)
		{
		return NULL;
		}

	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

    dest_mgr = jinit_write_bmp(&cinfo, FALSE, &ddsd);

  //dest_mgr->output_file = output_file;

  (void) jpeg_start_decompress(&cinfo);

  (*dest_mgr->start_output) (&cinfo, dest_mgr);

  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }

	(*dest_mgr->finish_output) (&cinfo, dest_mgr);
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	pdds->Unlock(NULL);

    //fclose(input_file);

    //fclose(output_file);

	return pdds;
	}

void PinDirectDraw::SetOpaque(LPDIRECTDRAWSURFACE7 pdds, int width, int height)
	{
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	int surfwidth, surfheight;
	int pitch;

	surfwidth = ddsd.dwWidth;
	surfheight = ddsd.dwHeight;
	pitch = ddsd.lPitch;

	// Assume our 32 bit color structure
	int i,l;
	BYTE *pch = (BYTE *)ddsd.lpSurface;
	for (i=0;i<height;i++)
		{
		for (l=0;l<width;l++)
			{
				pch+=3;
				*pch++ = 0xff;
			}
		pch+=(pitch-(width*4));
		}

	pdds->Unlock(NULL);
	}

void PinDirectDraw::SetOpaqueBackdrop(LPDIRECTDRAWSURFACE7 pdds, COLORREF rgbTransparent, COLORREF rgbBackdrop, int width, int height)
	{
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	int surfwidth, surfheight;
	int pitch;

	surfwidth = ddsd.dwWidth;
	surfheight = ddsd.dwHeight;
	pitch = ddsd.lPitch;

	int rback, gback, bback;
	rback = (rgbBackdrop & 0x00ff0000) >> 16;
	gback = (rgbBackdrop & 0x0000ff00) >> 8;
	bback = (rgbBackdrop & 0x000000ff);

	// Assume our 32 bit color structure	
	int i,l;
	BYTE *pch = (BYTE *)ddsd.lpSurface;
	for (i=0;i<height;i++)
		{
		for (l=0;l<width;l++)
			{
			if ((*(unsigned int *)pch & 0xffffff) != rgbTransparent)
				{
				pch+=3;
				*pch++ = 0xff;
				}
			else
				{
				*pch++ = rback;
				*pch++ = gback;
				*pch++ = bback;
				*pch++ = 0xff;
				}
			}
		pch+=(pitch-(width*4));
		}

	pdds->Unlock(NULL);
	}

BOOL PinDirectDraw::SetAlpha(LPDIRECTDRAWSURFACE7 pdds, COLORREF rgbTransparent, int width, int height)
	{
	// Set alpha of each pixel
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	int surfwidth, surfheight;
	int pitch;
	BOOL fTransparent = fFalse;

	surfwidth = ddsd.dwWidth;
	surfheight = ddsd.dwHeight;
	pitch = ddsd.lPitch;

	int rtrans, gtrans, btrans;
	rtrans = (rgbTransparent & 0x00ff0000) >> 16;
	gtrans = (rgbTransparent & 0x0000ff00) >> 8;
	btrans = (rgbTransparent & 0x000000ff);

	// Assume our 32 bit color structure

	int i,l;
	BYTE *pch = (BYTE *)ddsd.lpSurface;
	for (i=0;i<height;i++)
		{
		for (l=0;l<width;l++)
			{
			/*if ((*(unsigned int *)pch & 0xffffff) != rgbTransparent)
				{
				pch+=3;
				//pch++;
				*pch++ = 0xff;
				}
			else
				{
				pch+=3;
				*pch++ = 0;
				}*/
			if ((*(unsigned int *)pch & 0xffffff) != rgbTransparent)
				{
				pch++;
				pch++;
				pch++;
				//pch++;
				*pch++ = 0xff;
				}
			else
				{
				// Do really complex thing where we set the edges of the
				// transparent area to be the color next to them, so when
				// blending occurs we don't get fringes of the transparent
				// color

				fTransparent = fTrue;

				int a,c;
				int r=0;
				int g=0;
				int b=0;
				int count=0;
				for (a=-1;a<2;a++)
					{
					if (i+a >= 0 && i+a < height)
						{
						for (c=-1;c<2;c++)
							{
							if (l+c >= 0 && l+c < width)
								{
								int add = (c*4)+(a*pitch);
								if (add >= 0)
									{
									int newr,newg,newb;
									newr = *(pch+(c*4)+(a*pitch));
									newg = *(pch+(c*4)+(a*pitch)+1);
									newb = *(pch+(c*4)+(a*pitch)+2);

									if (newr != rtrans && newg != gtrans && newb != btrans)
										{
										r += newr;
										g += newg;
										b += newb;
										count++;
										}
									}
								else // color might have already changed above us
									{
									int newa;
									newa = *(pch+(c*4)+(a*pitch)+3);
									if (newa > 0)
										{
										r += *(pch+(c*4)+(a*pitch));
										g += *(pch+(c*4)+(a*pitch)+1);
										b += *(pch+(c*4)+(a*pitch)+2);
										count++;
										}
									}
								}
							}
						}
					}
				if (count > 0)
					{
					int round = count>>1;
					*pch++ = (r+round)/count;
					*pch++ = (g+round)/count;
					*pch++ = (b+round)/count;
					}
				else
					{
					pch+=3;
					}
				*pch++ = 0;
				}
			}
		pch+=(pitch-(width*4));
		}

	pdds->Unlock(NULL);

	return fTransparent;
	}

int rgfilterwindow[7][7] = {
	1, 2, 3, 4, 3, 2, 1,
	2, 3, 4, 5, 4, 3, 2,
	3, 4, 5, 6, 5, 4, 3,
	4, 5, 6, 7, 6, 5, 4,
	3, 4, 5, 6, 5, 4, 3,
	2, 3, 4, 5, 4, 3, 2,
	1, 2, 3, 4, 3, 2, 1};

void PinDirectDraw::Blur(LPDIRECTDRAWSURFACE7 pdds, BYTE *pbits, int shadwidth, int shadheight)
	{
	int i,l;

	// Create Guassian window (actually its not really Guassian, but same idea)

	int window[7][7];

	for (i=0;i<4;i++)
		{
		window[0][i] = i+1;
		window[0][6-i] = i+1;
		window[i][0] = i+1;
		window[6-i][0] = i+1;
		}

	int totalwindow = 0;

	for (i=0;i<7;i++)
		{
		for (l=0;l<7;l++)
			{
			window[i][l] = window[0][l] * window[i][0];
			window[i][l] = rgfilterwindow[i][l];
			totalwindow+=window[i][l];
			}
		}

	// Guassian Blur the sharp shadows

	//m_pddsLightProjectTexture = g_pvp->m_pdd.CreateTextureOffscreen(128, 256);

	DDSURFACEDESC2 ddsd;//, ddsdSharp;
	ddsd.dwSize = sizeof(ddsd);

	pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	//hr = pddsLightMapSharp->Lock(NULL, &ddsdSharp, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

	int width, height;
	int pitch = ddsd.lPitch;
	int pitchSharp = 256*3;//shadwidth*3;//ddsd.lPitch;
	BYTE *pc = (BYTE *)ddsd.lpSurface;

	width = (int)ddsd.dwWidth;
	height = (int)ddsd.dwHeight;

	for (i=0;i<shadheight;i++)
		{
		for (l=0;l<shadwidth;l++)
			{

			int m,n;
			int value = 0;
			int totalvalue = totalwindow;

			for (m=0;m<7;m++)
				{
				for (n=0;n<7;n++)
					{
					int x,y;
					x = l+m-3;
					y = i+n-3;
					if (x>=0 && x<=(shadwidth-1) && y>=0 && y<=(shadheight- 1))
						{
						//value += *(pbits+x*4 + pitchSharp*y);
						value += (int)(*(pbits+x*3 + pitchSharp*y)) * window[m][n];
						}
					else
						{
						totalvalue -= window[m][n];
						}
					}
				}

			value /= totalvalue;//totalwindow;

			value = 127 + (value>>1);

			//char value = *((pcSharp+l*4 + pitch*i) + 1);

			*pc++ = value;
			*pc++ = value;
			*pc++ = value;
			*pc++ = value;
			//*(pi+l) = 0x00ff00ff;
			}
		pc -= shadwidth*4;
		pc += pitch;
		//pi = (int *)((char *)ddsd.lpSurface + (pitch*i));
		}

	pdds->Unlock(NULL);
	}

void PinDirectDraw::BlurAlpha(LPDIRECTDRAWSURFACE7 pdds)
	{
	int i,l;

	// Create Guassian window (actually its not really Guassian, but same idea)

	int window[7][7];

	for (i=0;i<4;i++)
		{
		window[0][i] = i+1;
		window[0][6-i] = i+1;
		window[i][0] = i+1;
		window[6-i][0] = i+1;
		}

	int totalwindow = 0;

	for (i=0;i<7;i++)
		{
		for (l=0;l<7;l++)
			{
			window[i][l] = window[0][l] * window[i][0];
			//window[i][l] = 1;
			window[i][l] = rgfilterwindow[i][l];
			totalwindow+=window[i][l];
			}
		}

	// Guassian Blur the sharp shadows

	//m_pddsLightProjectTexture = g_pvp->m_pdd.CreateTextureOffscreen(128, 256);

	DDSURFACEDESC2 ddsd;//, ddsdSharp;
	ddsd.dwSize = sizeof(ddsd);

	pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	//hr = pddsLightMapSharp->Lock(NULL, &ddsdSharp, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

	int width, height;
	int pitch = ddsd.lPitch;
	//int pitchSharp = 64*4;//ddsd.lPitch;
	BYTE *pc = (BYTE *)ddsd.lpSurface;

	width = (int)ddsd.dwWidth;
	height = (int)ddsd.dwHeight;

	for (i=0;i<height;i++)
		{
		for (l=0;l<width;l++)
			{

			int m,n;
			int value = 0;

			for (m=0;m<7;m++)
				{
				for (n=0;n<7;n++)
					{
					int x,y;
					x = l+m-3;
					y = i+n-3;
					if (x>=0 && x<=15 && y>=0 && y<=15)
						{
						//value += *(pbits+x*4 + pitchSharp*y);
						value += (int)(*(pc + 4*x + pitch*y)) * window[m][n];
						}
					}
				}

			value /= totalwindow;

			value = /*127 + */(value*5)>>3;

			//char value = *((pcSharp+l*4 + pitch*i) + 1);

			//*pc++ = value;
			//*pc++ = value;
			//*pc++ = value;
			//pc += 3;
			//*pc++ = value;
			//*(pi+l) = 0x00ff00ff;
			*(pc + pitch*i + l*4 + 3) = value;
			}
		//pc -= ddsd.dwWidth*4;
		//pc += pitch;
		//pi = (int *)((char *)ddsd.lpSurface + (pitch*i));
		}

	pdds->Unlock(NULL);
	}

//#pragma optimize("atg", on)

void PinDirectDraw::CreateNextMipMapLevel(LPDIRECTDRAWSURFACE7 pdds)
	{
	DDSURFACEDESC2 ddsd, ddsdNext;
	ddsd.dwSize = sizeof(ddsd);
	ddsdNext.dwSize = sizeof(ddsd);
	HRESULT hr;
	DDSCAPS2 ddsCaps;
	int pitch, pitchNext;
	int x,y;
	int i;
	int width, height;
	LPDIRECTDRAWSURFACE7 pddsNext;

	ddsCaps.dwCaps2 = 0;
	ddsCaps.dwCaps3 = 0;
	ddsCaps.dwCaps4 = 0;
	ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

	hr = pdds->GetAttachedSurface(&ddsCaps, &pddsNext);

	if (hr == S_OK)
		{
		hr = pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		hr = pddsNext->Lock(NULL, &ddsdNext, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

		pitch = ddsd.lPitch;
		pitchNext = ddsdNext.lPitch;
		width = ddsdNext.dwWidth;
		height = ddsdNext.dwHeight;
		BYTE *pch = (BYTE *)ddsd.lpSurface;
		BYTE *pchNext = (BYTE *)ddsdNext.lpSurface;

		BYTE *pbytes1, *pbytes2;

		pbytes1 = pch;
		pbytes2 = pch + pitch;

		int addtoouterpitch = (pitch*2 - (width*2*4));

		for (y=0;y<height;y++)
			{
			for (x=0;x<width;x++)
				{
				int rtotal = 0;
				int gtotal = 0;
				int btotal = 0;
				int atotal;
				int count = 0;

				int r[4];
				int g[4];
				int b[4];
				int a[4];

				//r += *(pch+(x*pixels_per_mip*bytes_per_pixel) + bytes_per_pixel + r_in_rgb + pitch

				b[0] = *pbytes1++;
				g[0] = *pbytes1++;
				r[0] = *pbytes1++;
				a[0] = *pbytes1++;

				b[1] = *pbytes1++;
				g[1] = *pbytes1++;
				r[1] = *pbytes1++;
				a[1] = *pbytes1++;

				b[2] = *pbytes2++;
				g[2] = *pbytes2++;
				r[2] = *pbytes2++;
				a[2] = *pbytes2++;

				b[3] = *pbytes2++;
				g[3] = *pbytes2++;
				r[3] = *pbytes2++;
				a[3] = *pbytes2++;

				/*r[0] = *(pch+(x*2*4) + 2);
				r[1] = *(pch+(x*2*4) + 4 + 2);
				r[2] = *(pch+(x*2*4) + 2 + pitch);
				r[3] = *(pch+(x*2*4) + 4 + 2 + pitch);

				g[0] = *(pch+(x*2*4) + 1);
				g[1] = *(pch+(x*2*4) + 4 + 1);
				g[2] = *(pch+(x*2*4) + 1 + pitch);
				g[3] = *(pch+(x*2*4) + 4 + 1 + pitch);

				b[0] = *(pch+(x*2*4) + 0);
				b[1] = *(pch+(x*2*4) + 4 + 0);
				b[2] = *(pch+(x*2*4) + 0 + pitch);
				b[3] = *(pch+(x*2*4) + 4 + 0 + pitch);

				a[0] = *(pch+(x*2*4) + 3);
				a[1] = *(pch+(x*2*4) + 4 + 3);
				a[2] = *(pch+(x*2*4) + 3 + pitch);
				a[3] = *(pch+(x*2*4) + 4 + 3 + pitch);*/

				// For transparent pixels, do not average them in to the color total
				for (i=0;i<4;i++)
					{
					if (a[i]) //r[i] != 0 || g[i] != 0 || b[i] != 0)
						{
						count++;
						rtotal+=r[i];
						gtotal+=g[i];
						btotal+=b[i];
						}
					}

				//count++;

				//rtotal = r[0]+r[1]+r[2]+r[3];
				//gtotal = g[0]+g[1]+g[2]+g[3];
				//btotal = b[0]+b[1]+b[2]+b[3];
				atotal = a[0]+a[1]+a[2]+a[3];

				if (!count) // all pixels are transparent - do whatever
					{
					//rtotal = 255;
					//gtotal = 0;
					//btotal = 0;
					count = 1;
					}

				int round = count>>1;

				*pchNext++ = (btotal+round)/count;
				*pchNext++ = (gtotal+round)/count;
				*pchNext++ = (rtotal+round)/count;
				*pchNext++ = (atotal+2)/4;

				/**(pchNext + (x*4)) = 0xff;//(btotal+round)/count;
				*(pchNext + (x*4) + 1) = 0xff;//(gtotal+round)/count;
				*(pchNext + (x*4) + 2) = 0xff;//(rtotal+round)/count;
				*(pchNext + (x*4) + 3) = 0xff;//(atotal+2)/4;//0xff;*/

				/*if (ddsdNext.dwMipMapCount == 3)
					{
					*(pchNext + (x*4)) = 255;
					*(pchNext + (x*4) + 1) = 255;
					*(pchNext + (x*4) + 2) = 0;
					*(pchNext + (x*4) + 3) = 255;
					}
				else if (ddsdNext.dwMipMapCount == 2)
					{
					*(pchNext + (x*4)) = 255;
					*(pchNext + (x*4) + 1) = 255;
					*(pchNext + (x*4) + 2) = 255;
					*(pchNext + (x*4) + 3) = 255;
					}
				else
					{
					*(pchNext + (x*4)) = 255;
					*(pchNext + (x*4) + 1) = 0;
					*(pchNext + (x*4) + 2) = 255;
					*(pchNext + (x*4) + 3) = 255;
					}*/
				}
			//pch+=pitch*2;
			//pchNext+=pitchNext;
			pbytes1 += addtoouterpitch;
			pbytes2 += addtoouterpitch;
			pchNext += (pitchNext - (width*4));
			}

		pdds->Unlock(NULL);
		pddsNext->Unlock(NULL);

		pddsNext->Release();

		CreateNextMipMapLevel(pddsNext);
		}

	}