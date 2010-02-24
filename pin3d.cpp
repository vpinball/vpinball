#include "stdafx.h"
#include "main.h"

Pin3D::Pin3D()
	{
	m_pDD = NULL;
	m_pddsFrontBuffer = NULL;
	m_pddsBackBuffer = NULL;
	m_pddsZBuffer = NULL;
	m_pD3D = NULL;
	m_pd3dDevice = NULL;
	m_pddsStatic = NULL;
	m_pddsStaticZ = NULL;
	m_pddsBallTexture = NULL;
	m_pddsTargetTexture = NULL;
	m_pddsLightTexture = NULL;
	//m_pddsLightProjectTexture = NULL;
	m_pddsLightWhite = NULL;
	//m_pddsPlayfieldTexture = NULL;
	m_pddsShadowTexture = NULL;

	m_fSoftwareOnly = fFalse;

	FILE *foo;
	foo = fopen("c:\\vprender.txt","r");
	if (foo)
		{
		fclose(foo);
		m_fSoftwareOnly = fTrue;
		}
	}

Pin3D::~Pin3D()
	{
	int i;

	m_pDD->RestoreDisplayMode();

	if (m_pddsFrontBuffer)
		m_pddsFrontBuffer->Release();

	if (m_pddsBackBuffer)
		m_pddsBackBuffer->Release();

	if (m_pddsZBuffer)
		m_pddsZBuffer->Release();

	if (m_pddsStatic)
		m_pddsStatic->Release();

	if (m_pddsStaticZ)
		m_pddsStaticZ->Release();

	if (m_pddsBallTexture)
		{
		m_pddsBallTexture->Release();
		}

	SAFE_RELEASE(m_pddsTargetTexture);

	if (m_pddsLightTexture)
		{
		m_pddsLightTexture->Release();
		}

	SAFE_RELEASE(m_pddsShadowTexture);

	for (i=0;i<m_xvShadowMap.AbsoluteSize();i++)
		{
		((LPDIRECTDRAWSURFACE)m_xvShadowMap.AbsoluteElementAt(i))->Release();
		}

	/*if (m_pddsPlayfieldTexture)
		{
		m_pddsPlayfieldTexture->Release();
		}*/

	//SAFE_RELEASE(m_pddsLightProjectTexture);

	SAFE_RELEASE(m_pddsLightWhite);

	if (m_pD3D)
		m_pD3D->Release();

	if (m_pd3dDevice)
		m_pd3dDevice->Release();
        //if (0 < m_pd3dDevice->Release())
			//return;

	//if (m_pDD)
		//{
        //m_pDD->Release();
		//}
	}

static HRESULT WINAPI EnumZBufferFormatsCallback( DDPIXELFORMAT* pddpf,
                                                  VOID* pContext )
{
    DDPIXELFORMAT* pddpfOut = (DDPIXELFORMAT*)pContext;

    if((pddpf->dwRGBBitCount > 0) && pddpfOut->dwRGBBitCount == pddpf->dwRGBBitCount )
    {
        (*pddpfOut) = (*pddpf);
        return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}

void Pin3D::ClipRectToVisibleArea(RECT *prc)
{
	prc->top = max(prc->top, 0);
	prc->left = max(prc->left, 0);
	prc->right = min(prc->right, m_dwRenderWidth);
	prc->bottom = min(prc->bottom, m_dwRenderHeight);
}

void Pin3D::TransformVertices(Vertex3D* rgv, WORD *rgi, int count, Vertex3D *rgvout)
	{
	Vertex3D vT;

	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	D3DVIEWPORT7 vp;
	m_pd3dDevice->GetViewport( &vp );
	float rClipWidth  = vp.dwWidth/2.0f;
	float rClipHeight = vp.dwHeight/2.0f;
	int xoffset = vp.dwX;
	int yoffset = vp.dwY;

	int i,l;

	// Transform each vertex through the current matrix set
	for(i=0; i<count; i++ )
		{
		if (rgi)
			{
			l = rgi[i];
			}
		else
			{
			l = i;
			}

		// Get the untransformed vertex position
		FLOAT x = rgv[l].x;
		FLOAT y = rgv[l].y;
		FLOAT z = rgv[l].z;

		// Transform it through the current matrix set
		FLOAT xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		FLOAT yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		FLOAT wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		vT.x  = ((( 1.0f + (xp/wp) ) * rClipWidth + xoffset));
		vT.y  = ((( 1.0f - (yp/wp) ) * rClipHeight + yoffset));

		FLOAT zp = m_matrixTotal._13*x + m_matrixTotal._23*y + m_matrixTotal._33*z + m_matrixTotal._43;
		rgvout[l].x = vT.x;
		rgvout[l].y	= vT.y;
		rgvout[l].z = zp / wp;
		rgvout[l].nx = wp;
		}

	return;
	}
	
LPDIRECTDRAWSURFACE7 Pin3D::CreateOffscreenWithCustomTransparency(int width, int height, int color)
	{
	//const GUID* pDeviceGUID = &IID_IDirect3DRGBDevice;
	DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);
	
	/*if (width < 1 || height < 1)
	{
		return NULL;
	}*/
	
	if (width < 1)
		{
		// This can happen if an object is completely off screen.  Since that's
		// rare, it's easier just to create a tiny surface to handle it.
		width = 1;
		}
		
	if (height < 1)
		{
		height = 1;
		}

	LPDIRECTDRAWSURFACE7 pdds;
	HRESULT hr;

	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_CKSRCBLT;
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = color;//0xffffff;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = color;//0xffffff;
    ddsd.dwWidth        = width;
    ddsd.dwHeight       = height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;// | DDSCAPS_3DDEVICE;

	if (m_fSoftwareOnly)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
		{
		ShowError("Could not create offscreen surface.");
		return NULL;
		}

	return pdds;
	}

LPDIRECTDRAWSURFACE7 Pin3D::CreateOffscreen(int width, int height)
	{
	return CreateOffscreenWithCustomTransparency(width, height, 0);
	}

/*LPDIRECTDRAWSURFACE7 Pin3D::CreateTextureOffscreen(int width, int height)
	{
	const GUID* pDeviceGUID = &IID_IDirect3DRGBDevice;
	DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	LPDIRECTDRAWSURFACE7 pdds;
	HRESULT hr;

	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_CKSRCBLT | DDSD_PIXELFORMAT;
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = 0;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = 0;
    ddsd.dwWidth        = width;
    ddsd.dwHeight       = height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;

	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	ddsd.ddpfPixelFormat.dwRBitMask = 0xff0000;
	ddsd.ddpfPixelFormat.dwGBitMask = 0x00ff00;
	ddsd.ddpfPixelFormat.dwBBitMask = 0x0000ff;
	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
		{
		return NULL;
		}

	return pdds;
	}*/

LPDIRECTDRAWSURFACE7 Pin3D::CreateZBufferOffscreen(int width, int height)
{
    HRESULT hr;
	const GUID* pDeviceGUID;

	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		pDeviceGUID = &IID_IDirect3DHALDevice;
		}
	else
		{
		pDeviceGUID = &IID_IDirect3DRGBDevice;
		}

	LPDIRECTDRAWSURFACE7 pdds;

    // Get z-buffer dimensions from the render target
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsBackBuffer->GetSurfaceDesc( &ddsd ); // read description out of backbuffer so we get the current pixelformat depth to look for
	/*DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);*/

    // Setup the surface desc for the z-buffer.
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth        = width;
    ddsd.dwHeight       = height;
    ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized

    // Get an appropiate pixel format from enumeration of the formats. On the
    // first pass, we look for a zbuffer dpeth which is equal to the frame
    // buffer depth (as some cards unfornately require this).
    m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                (VOID*)&ddsd.ddpfPixelFormat );
    if( 0 == ddsd.ddpfPixelFormat.dwSize )
    {
        // Try again, just accepting any 16-bit zbuffer
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
        m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                    (VOID*)&ddsd.ddpfPixelFormat );

        if( 0 == ddsd.ddpfPixelFormat.dwSize )
        {
            //DEBUG_MSG( _T("Device doesn't support requested zbuffer format") );
            return NULL;// D3DFWERR_NOZBUFFER;
        }
    }

    // Create and attach a z-buffer
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
    {
		ShowError("Could not create offscreen Z-surface.");
		return NULL;
    }

    return pdds;// S_OK;
}

HRESULT Pin3D::InitDD(HWND hwnd, BOOL fFullScreen, int screenwidth, int screenheight, int colordepth, int refreshrate)
{
    HRESULT hr;

	m_hwnd = hwnd;

	const GUID* pDeviceGUID;

	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		pDeviceGUID = &IID_IDirect3DHALDevice;
		}
	else
		{
		pDeviceGUID = &IID_IDirect3DRGBDevice;
		}

    // Get the dimensions of the viewport and screen bounds
    GetClientRect( hwnd, &m_rcScreen );
    ClientToScreen( hwnd, (POINT*)&m_rcScreen.left );
    ClientToScreen( hwnd, (POINT*)&m_rcScreen.right );
    m_dwRenderWidth  = m_rcScreen.right  - m_rcScreen.left;
    m_dwRenderHeight = m_rcScreen.bottom - m_rcScreen.top;
	//m_dwRenderWidth = 640;
	//m_dwRenderHeight = 480;

	SetUpdatePos(m_rcScreen.left, m_rcScreen.top);

	m_pDD = g_pvp->m_pdd.m_pDD; // Cache pointer from global direct draw object

	hr = m_pDD->QueryInterface( IID_IDirect3D7, (VOID**)&m_pD3D );
	if (hr != S_OK)
		{
		ShowError("Could not create Direct3D.");
		return hr;
		}

	hr = m_pDD->SetCooperativeLevel(hwnd, DDSCL_FPUPRESERVE);

	if (fFullScreen)
		{
		//hr = m_pDD->SetCooperativeLevel(hwnd, DDSCL_ALLOWREBOOT|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_FPUPRESERVE);
		hr = m_pDD->SetDisplayMode(screenwidth, screenheight, colordepth, refreshrate, 0);
		}
	/*else
		{

		}*/

    // Create the primary surface
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
    {
		ShowError("Could not create front buffer.");
        //DEBUG_MSG( _T("Error: Can't create primary surface") );
        //if( hr != DDERR_OUTOFVIDEOMEMORY )
            //return;// D3DFWERR_NOPRIMARY;
        //DEBUG_MSG( _T("Error: Out of video memory") );
        return hr;// DDERR_OUTOFVIDEOMEMORY;
    }

    // If in windowed-mode, create a clipper object
    LPDIRECTDRAWCLIPPER pcClipper;
    if( FAILED( hr = m_pDD->CreateClipper( 0, &pcClipper, NULL ) ) )
    {
		ShowError("Could not create clipper.");
        //DEBUG_MSG( _T("Error: Couldn't create clipper") );
        return hr;// D3DFWERR_NOCLIPPER;
    }

    // Associate the clipper with the window
    pcClipper->SetHWnd( 0, m_hwnd );
    m_pddsFrontBuffer->SetClipper( pcClipper );
    if (pcClipper)
		{
		pcClipper->Release();
		}
	//SAFE_RELEASE( pcClipper );

    // Create a backbuffer
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth        = m_dwRenderWidth;
    ddsd.dwHeight       = m_dwRenderHeight;
    ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;

	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
		}

	DDSURFACEDESC2 ddsdPrimary; // descriptor for current screen format
	// Check for 8-bit color, and create software 16-bit off-screen
	ddsdPrimary.dwSize = sizeof(DDSURFACEDESC2);
	m_pDD->GetDisplayMode( &ddsdPrimary );
	if( ddsdPrimary.ddpfPixelFormat.dwRGBBitCount <= 8 )
		{
		ddsd.dwFlags |= DDSD_PIXELFORMAT;
		ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		ddsd.ddpfPixelFormat.dwRBitMask = 0x007c00;
		ddsd.ddpfPixelFormat.dwGBitMask = 0x0003e0;
		ddsd.ddpfPixelFormat.dwBBitMask = 0x00001f;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0;

		ShowError("Color depth must be 16-bit or greater.");
		return E_FAIL;
		}
	else
		{
		}

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL ) ) )
    {
		ShowError("Could not create back buffer.");
        //DEBUG_ERR( hr, _T("Error: Couldn't create the backbuffer") );
        //if( hr != DDERR_OUTOFVIDEOMEMORY )
            //return;// D3DFWERR_NOBACKBUFFER;
        //DEBUG_MSG( _T("Error: Out of video memory") );
        return hr;// DDERR_OUTOFVIDEOMEMORY;
    }

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsStatic, NULL ) ) )
    {
		ShowError("Could not create static buffer.");
        //DEBUG_ERR( hr, _T("Error: Couldn't create the backbuffer") );
        //if( hr != DDERR_OUTOFVIDEOMEMORY )
            //return;// D3DFWERR_NOBACKBUFFER;
        //DEBUG_MSG( _T("Error: Out of video memory") );
        return hr;// DDERR_OUTOFVIDEOMEMORY;
    }

	hr = Create3DDevice((GUID*) pDeviceGUID);
	if(FAILED(hr))
		{
		return hr;
		}

	hr = CreateZBuffer((GUID*) pDeviceGUID);
	if(FAILED(hr))
		{
		return hr;
		}

	SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

	/*return m_pddsFrontBuffer->Blt( &m_rcScreenRect, m_pddsBackBuffer,
                                       NULL, DDBLT_WAIT, NULL );*/

    return S_OK;// S_OK;
}

HRESULT Pin3D::Create3DDevice(GUID* pDeviceGUID)
	{
	HRESULT hr;

	if( FAILED( hr = m_pD3D->CreateDevice( *pDeviceGUID, m_pddsBackBuffer,
										  &m_pd3dDevice) ) )
		{
			ShowError("Could not create D3D Device.");
			//DEBUG_MSG( _T("Couldn't create the D3DDevice") );
			return hr;// D3DFWERR_NO3DDEVICE;
		}

	D3DDEVICEDESC7 ddfoo;

	m_pd3dDevice->GetCaps(&ddfoo);

	DWORD caps;
	caps = ddfoo.dpcLineCaps.dwRasterCaps;

	if (caps & D3DPRASTERCAPS_ANTIALIASEDGES)
		{
		int i;

		i = 1;
		}

	// Finally, set the viewport for the newly created device
	D3DVIEWPORT7 vp = { 0, 0, m_dwRenderWidth, m_dwRenderHeight, 0.0f, 1.0f };

	if( FAILED( m_pd3dDevice->SetViewport( &vp ) ) )
		{
			ShowError("Could not set viewport.");
			//DEBUG_MSG( _T("Error: Couldn't set current viewport to device") );
			return hr; //D3DFWERR_NOVIEWPORT;
		}

	return S_OK;
	}

void Pin3D::EnsureDebugTextures()
	{
	int width, height;

	if (!m_pddsTargetTexture)
		{
		m_pddsTargetTexture = g_pvp->m_pdd.CreateFromResource(IDB_TARGET, &width, &height);
		g_pvp->m_pdd.SetAlpha(m_pddsTargetTexture, RGB(0,0,0), width, height);
		g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsTargetTexture);
		}
	}

HRESULT Pin3D::CreateZBuffer( GUID* pDeviceGUID )
{
    HRESULT hr;

    // Check if the device supports z-bufferless hidden surface removal. If so,
    // we don't really need a z-buffer
    //D3DDEVICEDESC7 ddDesc;
    //m_pd3dDevice->GetCaps( &ddDesc );
    //if( ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )
        //return;// S_OK;

    // Get z-buffer dimensions from the render target
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsBackBuffer->GetSurfaceDesc( &ddsd );

    // Setup the surface desc for the z-buffer.
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;// | DDSCAPS_VIDEOMEMORY;//DDSCAPS_SYSTEMMEMORY;
    ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized

	//if (m_fSoftwareOnly)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

	/*if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}*/

    // Get an appropiate pixel format from enumeration of the formats. On the
    // first pass, we look for a zbuffer dpeth which is equal to the frame
    // buffer depth (as some cards unfornately require this).
    m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                (VOID*)&ddsd.ddpfPixelFormat );
    if( 0 == ddsd.ddpfPixelFormat.dwSize )
    {
        // Try again, just accepting any 16-bit zbuffer
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
        m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                    (VOID*)&ddsd.ddpfPixelFormat );

        if( 0 == ddsd.ddpfPixelFormat.dwSize )
        {
			ShowError("Could not find z-buffer format.");
            //DEBUG_MSG( _T("Device doesn't support requested zbuffer format") );
            return E_FAIL;// D3DFWERR_NOZBUFFER;
        }
    }

    // Create and attach a z-buffer
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsZBuffer, NULL ) ) )
    {
		ShowError("Could not create Z Buffer.");
        //DEBUG_MSG( _T("Error: Couldn't create a ZBuffer surface") );
        //if( hr != DDERR_OUTOFVIDEOMEMORY )
            //return; // D3DFWERR_NOZBUFFER;
        //DEBUG_MSG( _T("Error: Out of video memory") );
        return hr; // DDERR_OUTOFVIDEOMEMORY;
    }

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsStaticZ, NULL ) ) )
    {
		ShowError("Could not create static Z Buffer.");
        //DEBUG_MSG( _T("Error: Couldn't create a ZBuffer surface") );
        //if( hr != DDERR_OUTOFVIDEOMEMORY )
            //return; // D3DFWERR_NOZBUFFER;
        //DEBUG_MSG( _T("Error: Out of video memory") );
        return hr; // DDERR_OUTOFVIDEOMEMORY;
    }

	int width, height;

	CreateBallShadow();

	//m_pddsShadowTexture = g_pvp->m_pdd.CreateFromResource(IDB_BALLTEXTURE, &width, &height);
	//g_pvp->m_pdd.SetAlpha(m_pddsShadowTexture, RGB(0,0,0), width, height);

	//m_pddsBallTexture = g_pvp->m_pdd.DecompressJPEG("d:\\gdk\\temp\\foo.jpg",&width, &height);

	m_pddsBallTexture = g_pvp->m_pdd.CreateFromResource(IDB_BALLTEXTURE, &width, &height);
	g_pvp->m_pdd.SetAlpha(m_pddsBallTexture, RGB(0,0,0), width, height);
	g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsBallTexture);

	//m_pddsLightTexture = g_pvp->m_pdd.CreateFromFile("d:\\gdk\\vbatest\\sunburst.bmp");
	m_pddsLightTexture = g_pvp->m_pdd.CreateFromResource(IDB_SUNBURST3, &width, &height);
	g_pvp->m_pdd.SetAlpha(m_pddsLightTexture, RGB(0,0,0), width, height);

	//g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsBallTexture);
	g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsLightTexture);

	//m_pddsLightProjectTexture = g_pvp->m_pdd.CreateFromFile("d:\\gdk\\vbatest\\lightproject.bmp", NULL, NULL);
	//g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsLightProjectTexture);

	m_pddsLightWhite = g_pvp->m_pdd.CreateFromResource(IDB_WHITE, &width, &height);
	g_pvp->m_pdd.SetAlpha(m_pddsLightWhite, RGB(0,0,0), width, height);
	//m_pddsLightWhite = g_pvp->m_pdd.CreateFromFile("d:\\gdk\\vbatest\\white.bmp", NULL, NULL);
	g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsLightWhite);

	//m_pddsPlayfieldTexture = g_pvp->m_pdd.CreateFromFile("d:\\gdk\\vbatest\\playfield.bmp");

    if( FAILED( hr = m_pddsBackBuffer->AddAttachedSurface( m_pddsZBuffer ) ) )
    {
		ShowError("Could not attach Z-Buffer.");
        //DEBUG_MSG( _T("Error: Couldn't attach zbuffer to render surface") );
        return hr; // D3DFWERR_NOZBUFFER;
    }

	if( FAILED( hr = m_pddsStatic->AddAttachedSurface( m_pddsStaticZ ) ) )
    {
		ShowError("Could not attach static Z-Buffer.");
        //DEBUG_MSG( _T("Error: Couldn't attach zbuffer to render surface") );
        return hr; // D3DFWERR_NOZBUFFER;
    }

    return S_OK;
}

/*LPDIRECTDRAWSURFACE7 Pin3D::CreateFromFile(char *szfile)
	{
	LPDIRECTDRAWSURFACE7 pdds;

	HBITMAP hbm = (HBITMAP)LoadImage(g_hinst,szfile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	BITMAP bm;

	GetObject(hbm, sizeof(bm), &bm);

	pdds = g_pvp->m_pdd.CreateTextureOffscreen(bm.bmWidth, bm.bmHeight);

	HDC hdc;

	pdds->GetDC(&hdc);

	HDC hdcFoo = CreateCompatibleDC(hdc);

	SelectObject(hdcFoo, hbm);

	BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcFoo, 0, 0, SRCCOPY);

	DeleteDC(hdcFoo);

	DeleteObject(hbm);

	pdds->ReleaseDC(hdc);

	return pdds;
	}*/

void Pin3D::SetRenderTarget(LPDIRECTDRAWSURFACE7 pddsSurface, LPDIRECTDRAWSURFACE7 pddsZ)
	{
	HRESULT hr;
	hr = m_pd3dDevice->SetRenderTarget(pddsSurface, 0L);
	hr = m_pd3dDevice->SetRenderTarget(pddsZ, 0L);
	}

void Pin3D::InitRenderState()
	{
	HRESULT hr;
	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP/*WRAP*/);
	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);


	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ALPHAARG2, D3DTA_TEXTURE/*D3DTA_CURRENT*/);
	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_TEXCOORDINDEX, 0);

	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_COLOROP, D3DTOP_MODULATE);
	hr = m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	}

void Pin3D::DrawBackground()
	{
	int r,g,b;
	int d3dcolor;

	PinImage *pin = g_pplayer->m_ptable->GetImage((char *)g_pplayer->m_ptable->m_szImageBackdrop);
	float maxtu,maxtv;

	SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

	if (pin)
		{
		D3DMATERIAL7 mtrl;
		ZeroMemory( &mtrl, sizeof(mtrl) );

		mtrl.diffuse.r = mtrl.ambient.r = 1.0;
		mtrl.diffuse.g = mtrl.ambient.g = 1.0;
		mtrl.diffuse.b = mtrl.ambient.b = 1.0;

		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER,
						   0, 1.0f, 0L );

		m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

		g_pplayer->m_ptable->GetTVTU(pin, &maxtu, &maxtv);

		Vertex3D rgv3D[4];
		WORD rgi[4];

		rgv3D[0].x = 0;
		rgv3D[0].y = 0;
		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;

		rgv3D[1].x = 1000;
		rgv3D[1].y = 0;
		rgv3D[1].tu = maxtu;
		rgv3D[1].tv = 0;

		rgv3D[2].x = 1000;
		rgv3D[2].y = 750;
		rgv3D[2].tu = maxtu;
		rgv3D[2].tv = maxtv;

		rgv3D[3].x = 0;
		rgv3D[3].y = 750;
		rgv3D[3].tu = 0;
		rgv3D[3].tv = maxtv;

		rgi[0] = 0;
		rgi[1] = 1;
		rgi[2] = 2;
		rgi[3] = 3;

		SetTexture(pin->m_pdsBuffer);

		SetHUDVertices(rgv3D, 4);
		SetDiffuseFromMaterial(rgv3D, 4, &mtrl);

		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
												  rgv3D, 4,
												  rgi, 4, NULL);

		m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		}
	else
		{
		r = (g_pplayer->m_ptable->m_colorbackdrop & 0xff0000) >> 16;
		g = (g_pplayer->m_ptable->m_colorbackdrop & 0xff00) >> 8;
		b = (g_pplayer->m_ptable->m_colorbackdrop & 0xff);
		d3dcolor = b<<16 | g<<8 | r;

		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
						   d3dcolor, 1.0f, 0L );
		}
	}

void Pin3D::InitLayout(float left, float top, float right, float bottom, float inclination, float FOV)
	{
	RECT rc;

	rc.left = 0;
	rc.top = 0;
	rc.right = m_width;
	rc.bottom = m_height;

	m_rotation = 0;
	m_inclination = (float)(inclination/360.0*2.0*PI);

	HRESULT hr;

	hr = m_pd3dDevice->SetTexture(ePictureTexture, NULL);

	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	//hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);

    hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

	InitRenderState();

	DrawBackground();

	D3DLIGHT7 light;
	ZeroMemory(&light, sizeof(D3DLIGHT7));
    light.dltType        = D3DLIGHT_DIRECTIONAL;
	//light.dltType        = D3DLIGHT_POINT;
	light.dcvAmbient.r   = 0.1f;
    light.dcvAmbient.g   = 0.1f;
    light.dcvAmbient.b   = 0.1f;
    light.dcvDiffuse.r   = 0.4f;
    light.dcvDiffuse.g   = 0.4f;
    light.dcvDiffuse.b   = 0.4f;
	light.dcvSpecular.r   = 0;
    light.dcvSpecular.g   = 0;
    light.dcvSpecular.b   = 0;
	//light.dvDirection = D3DVECTOR( 5, -20, (float)cos(0.5) );
	//light.dvDirection = D3DVECTOR( -5, 20, -(float)cos(0.5) );
	//light.dvDirection = D3DVECTOR(-5, 20, -5);

	float sn = (float)sin(m_inclination + PI - (PI*3/16));
	float cs = (float)cos(m_inclination + PI - (PI*3/16));

	light.dvDirection = D3DVECTOR(5, sn * 21, cs * -21);
	light.dvRange        = D3DLIGHT_RANGE_MAX;
    light.dvAttenuation0 = 0.0f;
	light.dvAttenuation1 = 0.0f;
	light.dvAttenuation2 = 0.0f;

    // Set the light
    hr = m_pd3dDevice->SetLight( 0, &light );

	//light.dvDirection = D3DVECTOR( -(float)sin(-0.9), 0, -(float)cos(-0.9) );
	//light.dvDirection = D3DVECTOR(8, 10, -4);

	//sn = (float)sin(m_inclination + (PI*2/16));
	//cs = (float)cos(m_inclination + (PI*2/16));

    light.dcvDiffuse.r   = 0.6f;
    light.dcvDiffuse.g   = 0.6f;
    light.dcvDiffuse.b   = 0.6f;
	light.dcvSpecular.r   = 1;
    light.dcvSpecular.g   = 1;
    light.dcvSpecular.b   = 1;

	light.dvDirection = D3DVECTOR(-8, sn * 11, cs * -11);

	hr = m_pd3dDevice->SetLight( 1, &light );

	Vertex3D rgv[8];
	//WORD rgi[4];

	rgv[0].Set(left,top,0);
	rgv[3].Set(left,bottom,0);
	rgv[2].Set(right,bottom,0);
	rgv[1].Set(right,top,0);

	// These next 4 vertices are used just to set the extents
	rgv[4].Set(left,top,50);
	rgv[5].Set(left,bottom,50);
	rgv[6].Set(right,bottom,50);
	rgv[7].Set(right,top,50);

	//hr = m_pddsPlayfieldTexture->IsLost();

	if (g_pplayer->m_ptable->m_fRenderShadows == fTrue)
	{
		EnableLightMap(fTrue, 0);
	}
	else
	{
		EnableLightMap(fFalse, 0);
	}

	//m_pd3dDevice->SetTexture(eLightProject1, m_pddsLightProjectTexture);
    hr = m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
    hr = m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA);

	hr = m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	hr = m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	hr = m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
	hr = m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	hr = m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLORARG2, D3DTA_CURRENT );
	hr = m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	hr = m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_TEXCOORDINDEX, 1 );
	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE );

	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLOROP,   D3DTOP_DISABLE);

	//float lightx, lighty;
	//float newx, newy, newz;
	//lightx = 500;
	//lighty = 1000;

	//CreateShadow(0);

	m_lightproject.m_v.x = g_pplayer->m_ptable->m_right / 2;//500;
	m_lightproject.m_v.y = g_pplayer->m_ptable->m_bottom / 2;
	m_lightproject.inclination = 0;
	m_lightproject.rotation = 0;
	m_lightproject.spin = 0;

	int i;

	Vector<Vertex3D> vvertex3D;

	for (i=0;i<g_pplayer->m_ptable->m_vedit.Size();i++)
		{
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->GetBoundingVertices(&vvertex3D);
		}

	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

	double m_aspect = 4.0/3.0;//((double)m_dwRenderWidth)/m_dwRenderHeight;

	FitCameraToVertices(&vvertex3D/*rgv*/, vvertex3D.Size(), m_aspect, m_rotation, m_inclination, FOV);
	SetFieldOfView(FOV, m_aspect, m_rznear, m_rzfar);
	Translate(-m_vertexcamera.x,-m_vertexcamera.y,-m_vertexcamera.z);
	Rotate(m_inclination,0,m_rotation);
	CacheTransform();

	for (i=0;i<vvertex3D.Size();i++)
		{
		delete vvertex3D.ElementAt(i);
		}

	//hr = m_pd3dDevice->SetLight(0, &light);
    hr = m_pd3dDevice->LightEnable(0, TRUE);
	hr = m_pd3dDevice->LightEnable(1, TRUE);
    hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);

	//EnableLightMap(fFalse, -1);

	InitBackGraphics();
	/*hr = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv, 4,
												  rgi, 4, NULL);

	EnableLightMap(fFalse, -1);

	//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	//pddsLightMap->Release();

	//m_pd3dDevice->SetTexture(ePictureTexture, NULL);
	//m_pd3dDevice->SetTexture(1, NULL);
	SetTexture(NULL);

	rgi[0] = 2;
	rgi[1] = 3;
	rgi[2] = 5;
	rgi[3] = 6;

	SetNormal(rgv, rgi, 4, NULL, NULL, 0);

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv, 8,
												  rgi, 4, NULL);*/
	}

void Pin3D::InitBackGraphics()
	{
	Vertex3D rgv[8];
	WORD rgi[4];
	HRESULT hr;

	SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

	EnableLightMap(fTrue, 0);

	rgv[0].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_top,0);
	rgv[3].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_bottom,0);
	rgv[2].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_bottom,0);
	rgv[1].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_top,0);

	// These next 4 vertices are used just to set the extents
	rgv[4].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_top,50);
	rgv[5].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_bottom,50);
	rgv[6].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_bottom,50);
	rgv[7].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_top,50);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );

	PinImage *pin = g_pplayer->m_ptable->GetImage((char *)g_pplayer->m_ptable->m_szImage);
	float maxtu,maxtv;

	if (pin)
		{
		g_pplayer->m_ptable->GetTVTU(pin, &maxtu, &maxtv);

		//m_pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBuffer);
		//SetTexture(pin->m_pdsBuffer);


		//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		mtrl.diffuse.r = mtrl.ambient.r = 1.0;
		mtrl.diffuse.g = mtrl.ambient.g = 1.0;
		mtrl.diffuse.b = mtrl.ambient.b = 1.0;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

		SetTexture(pin->m_pdsBuffer);
		}
	else // No image by that name
		{
		SetTexture(NULL);

		mtrl.diffuse.r = mtrl.ambient.r = (g_pplayer->m_ptable->m_colorplayfield & 255) / 255.0f;
		mtrl.diffuse.g = mtrl.ambient.g = (g_pplayer->m_ptable->m_colorplayfield & 65280) / 65280.0f;
		mtrl.diffuse.b = mtrl.ambient.b = (g_pplayer->m_ptable->m_colorplayfield & 16711680) / 16711680.0f;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0;
		
		maxtv = maxtu = 1;
		}

	int i;
	for (i=0;i<4;i++)
		{
		rgv[i].nx = 0;
		rgv[i].ny = 0;
		rgv[i].nz = -1;

		rgv[i].tv = i&2 ? maxtv : 0;
		rgv[i].tu = (i==1 || i==2) ? maxtu : 0;

		m_lightproject.CalcCoordinates(&rgv[i]);
		}

	for (i=0;i<4;i++)
		{
		rgi[i] = i;
		}

	m_pd3dDevice->SetMaterial(&mtrl);

	hr = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv, 4,
												  rgi, 4, NULL);

	EnableLightMap(fFalse, -1);

	//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	//pddsLightMap->Release();

	//m_pd3dDevice->SetTexture(ePictureTexture, NULL);
	//m_pd3dDevice->SetTexture(1, NULL);
	SetTexture(NULL);

	rgi[0] = 2;
	rgi[1] = 3;
	rgi[2] = 5;
	rgi[3] = 6;

	SetNormal(rgv, rgi, 4, NULL, NULL, 0);

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv, 8,
												  rgi, 4, NULL);
	}

void Pin3D::CreateBallShadow()
	{
	DDBLTFX ddbltfx;
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	int pitch;
	int x,y;
	int width, height;

	ddbltfx.dwSize = sizeof(DDBLTFX);
	ddbltfx.dwFillColor = 0;
	m_pddsShadowTexture = g_pvp->m_pdd.CreateTextureOffscreen(16, 16);
	m_pddsShadowTexture->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx);

	//int width, height;

	//m_pddsShadowTexture = g_pvp->m_pdd.CreateFromResource(IDB_BALLTEXTURE, &width, &height);

	//g_pvp->m_pdd.SetAlpha(m_pddsShadowTexture, RGB(0,0,0), 64, 64);

	//g_pvp->m_pdd.SetOpaque(m_pddsShadowTexture, 64, 64);

	m_pddsShadowTexture->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	pitch = ddsd.lPitch;
	width = ddsd.dwWidth;
	height = ddsd.dwHeight;
	BYTE *pch = (BYTE *)ddsd.lpSurface;

	for (y=0;y<height;y++)
		{
		for (x=0;x<width;x++)
			{
			//pch += 3;
			int dx,dy;
			dx = 8-x;
			dy = 8-y;
			int dist = (dx*dx + dy*dy);
			if (dist < 25)
				{
				*pch++ = 255;
				//alpha = 255 * dist / 2048;
				}
			else
				{
				*pch++ = 0;
				//alpha = 0;
				}
			pch += 3;
			//*pch++ = (alpha*5)>>3;//0xff;*/
			}
		pch += pitch - (width*4);
		}

	m_pddsShadowTexture->Unlock(NULL);

	g_pvp->m_pdd.BlurAlpha(m_pddsShadowTexture);

	m_pddsShadowTexture->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	pitch = ddsd.lPitch;
	width = ddsd.dwWidth;
	height = ddsd.dwHeight;
	pch = (BYTE *)ddsd.lpSurface;

	for (y=0;y<height;y++)
		{
		for (x=0;x<width;x++)
			{
			*pch++ = 0;
			*pch++ = 0;
			*pch++ = 0;
			pch++;
			}
		pch += pitch - (width*4);
		}

	m_pddsShadowTexture->Unlock(NULL);
	}

LPDIRECTDRAWSURFACE7 Pin3D::CreateShadow(float z)
	{
	int i;

	float centerx = (g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right) / 2;
	float centery = (g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom) / 2;

	int shadwidth;// = 128;
	int shadheight;// = 256;
	if (centerx > centery)
		{
		shadwidth = 256;
		m_maxtu = 1;
		m_maxtv = centery/centerx;
		shadheight = (int)(256*m_maxtv);
		}
	else
		{
		shadheight = 256;
		m_maxtu = centerx/centery;
		m_maxtv = 1;
		shadwidth = (int)(256*m_maxtu);
		}

	// Create Shadow Picture
	BITMAPINFO bmi;

	memset(&bmi, 0, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 256;//shadwidth;
	bmi.bmiHeader.biHeight = -256;//-shadheight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;

	//WriteFile(hfile, &bmi, sizeof(bmi), &foo, NULL);

	HBITMAP hdib;
	HDC hdcScreen;
	HDC hdc2;
	BYTE *pbits;

	hdcScreen = GetDC(NULL);
	hdc2 = CreateCompatibleDC(hdcScreen);

	hdib = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void **)&pbits, NULL, 0);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdc2, hdib);
	float zoom = shadwidth/(centerx*2);
	ShadowSur *psur;
	psur = new ShadowSur(hdc2, zoom, centerx, centery, shadwidth, shadheight, z, NULL);

	SelectObject(hdc2, GetStockObject(WHITE_BRUSH));
	PatBlt(hdc2, 0, 0, shadwidth, shadheight, PATCOPY);

	for (i=0;i<g_pplayer->m_ptable->m_vedit.Size();i++)
		{
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->RenderShadow(psur, z);
		}

	//BitBlt(hdcScreen, 0, 0, 128, 256, hdc2, 0, 0, SRCCOPY);

	LPDIRECTDRAWSURFACE7 pddsProjectTexture = g_pvp->m_pdd.CreateTextureOffscreen(shadwidth, shadheight);
	m_xvShadowMap.AddElement(pddsProjectTexture, (int)z);

	DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    pddsProjectTexture->GetSurfaceDesc(&ddsd);
	m_maxtu = ((float)shadwidth)/ddsd.dwWidth;
	m_maxtv = ((float)shadheight)/ddsd.dwHeight;
	//m_pddsLightProjectTexture = g_pvp->m_pdd.CreateTextureOffscreen(128, 256);

	delete psur;

	SelectObject(hdc2, hbmOld);

	DeleteDC(hdc2);
	ReleaseDC(NULL, hdcScreen);

	g_pvp->m_pdd.Blur(pddsProjectTexture, pbits, shadwidth, shadheight);

	// Create Guassian window (actually its not really Guassian, but same idea)

	/*int window[7][7];

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
			window[i][l] = 1;
			totalwindow+=window[i][l];
			}
		}*/

	// Guassian Blur the sharp shadows

	//m_pddsLightProjectTexture = g_pvp->m_pdd.CreateTextureOffscreen(128, 256);

	/*DDSURFACEDESC2 ddsd;//, ddsdSharp;
	ddsd.dwSize = sizeof(ddsd);

	pddsProjectTexure->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	//hr = pddsLightMapSharp->Lock(NULL, &ddsdSharp, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

	int width, height;
	int pitch = ddsd.lPitch;
	int pitchSharp = shadwidth*3;//ddsd.lPitch;
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
					if (x>=0 && x<=127 && y>=0 && y<=255)
						{
						//value += *(pbits+x*4 + pitchSharp*y);
						value += (int)(*(pbits+x*3 + pitchSharp*y)) * window[m][n];
						}
					}
				}

			value /= totalwindow;

			value = 127 + (value>>1);

			//char value = *((pcSharp+l*4 + pitch*i) + 1);

			*pc++ = value;
			*pc++ = value;
			*pc++ = value;
			*pc++ = value;
			//*(pi+l) = 0x00ff00ff;
			}
		pc -= ddsd.dwWidth*4;
		pc += pitch;
		//pi = (int *)((char *)ddsd.lpSurface + (pitch*i));
		}

		pddsProjectTexure->Unlock(NULL);*/

	/*HDC hdcFoo;
	HDC hdcds;
	hdcFoo = GetDC(NULL);
	pddsProjectTexture->GetDC(&hdcds);
	BitBlt(hdcFoo, 0, 0, shadwidth, shadheight, hdcds, 0, 0, SRCCOPY);
	pddsProjectTexture->ReleaseDC(hdcds);
	ReleaseDC(NULL, hdcFoo);*/

	DeleteObject(hdib);
	//m_pd3dDevice->SetTexture(eLightProject1, m_pddsLightProjectTexture);

	return pddsProjectTexture;
	}

void Pin3D::SetTexture(LPDIRECTDRAWSURFACE7 pddsTexture)
	{
	HRESULT hr;
	if (pddsTexture == NULL)
		{
		hr = m_pd3dDevice->SetTexture(ePictureTexture, m_pddsLightWhite);
		}
	else
		{
		hr = m_pd3dDevice->SetTexture(ePictureTexture, pddsTexture);
		}
	}

void Pin3D::EnableLightMap(BOOL fEnable, float z)
	{
	if (fEnable)
		{
		LPDIRECTDRAWSURFACE7 pdds = (LPDIRECTDRAWSURFACE7)m_xvShadowMap.ElementAt((int)z);
		if (!pdds)
			{
			pdds = CreateShadow(z);
			}
		m_pd3dDevice->SetTexture(eLightProject1, pdds);
		}
	else
		{
		m_pd3dDevice->SetTexture(eLightProject1, NULL);
		}
	}
	
void Pin3D::SetMaterial(float r, float g, float b, float a)
	{
	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = r;
	mtrl.diffuse.g = mtrl.ambient.g = g;
	mtrl.diffuse.b = mtrl.ambient.b = b;
	mtrl.diffuse.a = mtrl.ambient.a = a;
	m_pd3dDevice->SetMaterial(&mtrl);
	}
	
void Pin3D::SetColorKeyEnabled(BOOL fColorKey)
	{
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, fColorKey);
	}

void Pin3D::SetAlphaEnabled(BOOL fAlpha)
	{
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, fAlpha);
	}

void Pin3D::SetFiltersLinear()
	{
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
	}

HRESULT Pin3D::DrawIndexedPrimitive(D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc,
												  LPVOID lpvVertices, DWORD dwVertexCount,
												  LPWORD lpwIndices, DWORD dwIndexCount)
	{
	HRESULT hr;

	hr = m_pd3dDevice->DrawIndexedPrimitive(d3dptPrimitiveType, dwVertexTypeDesc, lpvVertices, dwVertexCount,
															lpwIndices, dwIndexCount, NULL);

	return hr;
	}

void Pin3D::SetUpdatePos(int left, int top)
	{
	m_rcUpdate.left = left;
	m_rcUpdate.top = top;
	m_rcUpdate.right = left+m_dwRenderWidth;
	m_rcUpdate.bottom = top+m_dwRenderHeight;
	}

void Pin3D::Flip(int offsetx, int offsety)
	{
	RECT rcNew;
	rcNew.left = m_rcUpdate.left + offsetx;
	rcNew.right = m_rcUpdate.right + offsetx;
	rcNew.top = m_rcUpdate.top + offsety;
	rcNew.bottom = m_rcUpdate.bottom + offsety;
	HRESULT hr = m_pddsFrontBuffer->Blt(&rcNew, m_pddsBackBuffer, NULL, DDBLTFAST_WAIT, NULL);

	if (hr == DDERR_SURFACELOST)
		{
		//hr = m_pDD->SetDisplayMode(800, 600, 16, 0, 0);
		//ShowWindow(g_pplayer->m_hwnd, SW_SHOW);
		hr = g_pvp->m_pdd.m_pDD->RestoreAllSurfaces();
		/*DrawBackground();
		InitBackGraphics();
		g_pplayer->InitStatic(NULL);
		g_pplayer->InitAnimations(NULL);

		int i;
		for (i=0;i<g_pplayer->m_vscreenupdate.Size();i++)
			{
			HitObject *pho = g_pplayer->m_vscreenupdate.ElementAt(i);
			pho->Reset();
			}

		if (hr != S_OK)
			{
			int i;
			i = 9;
			}*/
		}
	//m_pddsFrontBuffer->BltFast(0, 0, m_pddsBackBuffer, NULL, DDBLTFAST_NOCOLORKEY);
	}

void Pin3D::FitCameraToVertices(Vector<Vertex3D> *pvvertex3D/*Vertex3D *rgv*/, int cvert, double aspect, double rotation, double inclination, double FOV)
	{
	// Determine camera distance
	Vertex3D vertexT;

	double maxyintercept, minyintercept;
	double maxxintercept, minxintercept;

	double rrotsin, rrotcos, rincsin, rinccos, temp;
	double slopey, slopex;

	/*rrotsin = sin(-rotation/360.0*2.0*Math::PI());
	rrotcos = cos(-rotation/360.0*2.0*Math::PI());
	rincsin = sin(-inclination/360.0*-2.0*Math::PI());
	rinccos = cos(-inclination/360.0*-2.0*Math::PI());*/

	rrotsin = sin(-rotation);
	rrotcos = cos(-rotation);
	rincsin = sin(-inclination);
	rinccos = cos(-inclination);

	slopey = tan(FOV/360.0*2.0*0.5*PI); // *0.5 because slope is half of FOV - FOV includes top and bottom

	// Field of view along the axis = atan(tan(yFOV)*width/height)
	// So the slope of x simply equals slopey*width/height

	slopex = slopey*aspect;// slopey*m_rcHard.width/m_rcHard.height;

	maxyintercept = -DBL_MAX;
	minyintercept = DBL_MAX;
	maxxintercept = -DBL_MAX;
	minxintercept = DBL_MAX;

	m_rznear = 0;
	m_rzfar = 0;

	int i;
	for (i=0;i<cvert;i++)
		{
		//vertexT = rgv[i];
		vertexT = *(pvvertex3D->ElementAt(i));

		// Rotate vertex
		temp = vertexT.x;
		vertexT.x = (float)(rrotcos*temp + rrotsin*vertexT.z);
		vertexT.z = (float)(rrotcos*vertexT.z - rrotsin*temp);

		temp = vertexT.y;
		vertexT.y = (float)(rinccos*temp + rincsin*vertexT.z);
		vertexT.z = (float)(rinccos*vertexT.z - rincsin*temp);

		// Extend z-range if necessary
		m_rznear = min(m_rznear, -vertexT.z);
		m_rzfar = max(m_rzfar, -vertexT.z);

		// Extend slope lines from point to find camera intersection
		temp = vertexT.y + slopey*vertexT.z;
		maxyintercept = max(maxyintercept, temp);

		temp = vertexT.y - slopey*vertexT.z;
		minyintercept = min(minyintercept, temp);

		temp = vertexT.x + slopex*vertexT.z;
		maxxintercept = max(maxxintercept, temp);

		temp = vertexT.x - slopex*vertexT.z;
		minxintercept = min(minxintercept, temp);
		}

	double delta;

	// Find camera center in xy plane
	delta = maxyintercept - minyintercept;// Allow for roundoff error
	//maxyintercept += delta*0.01;
	//minyintercept -= delta*0.01;

	delta = maxxintercept - minxintercept;// Allow for roundoff error
	//maxxintercept += delta*0.01;
	//minxintercept -= delta*0.01;

	double ydist, xdist;
	ydist = (maxyintercept - minyintercept) / (slopey*2);
	xdist = (maxxintercept - minxintercept) / (slopex*2);
	m_vertexcamera.z = (float)(max(ydist,xdist));
	m_vertexcamera.y = (float)(slopey*ydist + minyintercept);
	m_vertexcamera.x = (float)(slopex*xdist + minxintercept);

	m_rznear += m_vertexcamera.z;
	m_rzfar += m_vertexcamera.z;

	delta = m_rzfar - m_rznear;

	m_rznear -= delta*0.01; // Allow for roundoff error
	m_rzfar += delta*0.01;
	}

void Pin3D::Rotate(double x, double y, double z)
	{
	Matrix3D matTemp, matRotateX, matRotateY, matRotateZ;

	m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);

	matRotateY.RotateYMatrix(y);
	matRotateX.RotateXMatrix(x);
	matRotateZ.RotateZMatrix(z);
	matTemp.Multiply(matRotateX, matTemp);
	matTemp.Multiply(matRotateY, matTemp);
	matTemp.Multiply(matRotateZ, matTemp);

	m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
	}

void Pin3D::Translate(double x, double y, double z)
	{
	Matrix3D matTemp, matTrans;

	m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);

	matTrans._11 = matTrans._22 = matTrans._33 = matTrans._44 = 1.0f;
	matTrans._12 = matTrans._13 = matTrans._14 = 0.0f;
	matTrans._21 = matTrans._23 = matTrans._24 = 0.0f;
	matTrans._31 = matTrans._32 = matTrans._34 = 0.0f;

	matTrans._41 = (float)x;
	matTrans._42 = (float)y;
	matTrans._43 = (float)z;
	matTemp.Multiply(matTrans, matTemp);

	m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
	}

void Pin3D::SetFieldOfView(double rFOV, double raspect, double rznear, double rzfar)
	{
	// From the Field Of View and far z clipping plane, determine the front clipping plane size
	double yrange = rznear * tan(rFOV/2.0 /360.0 *2*PI);
	double xrange = yrange * raspect; //width/height

	D3DMATRIX mat;

	memset(&mat, 0, sizeof(D3DMATRIX));

    FLOAT Q = (float)(rzfar / ( rzfar - rznear ));

	mat._11 = (float)(2*rznear / (xrange*2));
	mat._22 = -(float)(2*rznear / (yrange*2));
	mat._33 = Q;
	mat._34 = 1;
	mat._43 = -Q*(float)rznear;

	//mat._41 = 200;

	m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &mat);

	mat._11 = mat._22 = mat._44 = 1.0f;
	mat._12 = mat._13 = mat._14 = mat._41 = 0.0f;
	mat._21 = mat._23 = mat._24 = mat._42 = 0.0f;
	mat._31 = mat._32 = mat._34 = mat._43 = 0.0f;
	mat._33 = -1;
    m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &mat);
	}

void Pin3D::CacheTransform()
	{
	Matrix3D matWorld, matView, matProj;
	m_pd3dDevice->GetTransform( D3DTRANSFORMSTATE_WORLD,      &matWorld );
	m_pd3dDevice->GetTransform( D3DTRANSFORMSTATE_VIEW,       &matView );
	m_pd3dDevice->GetTransform( D3DTRANSFORMSTATE_PROJECTION, &matProj );
	matProj.Multiply(matView, matView);
	matView.Multiply(matWorld, m_matrixTotal);
	}

#define MAX_INT 0x0fffffff

void Pin3D::ClearExtents(RECT *prc, float *pznear, float *pzfar)
	{
	prc->left = MAX_INT;
	prc->top = MAX_INT;
	prc->right = -MAX_INT;
	prc->bottom = -MAX_INT;

	if (pznear)
		{
		*pznear = FLT_MAX;
		*pzfar = -FLT_MAX;
		}
	}

void Pin3D::ExpandExtents(RECT *prc, Vertex3D* rgv, float *pznear, float *pzfar, int count, BOOL fTransformed)
	{
	Vertex3D *rgvOut;

	if (!fTransformed)
		{
		rgvOut = new Vertex3D[count];
		TransformVertices(rgv, NULL, count, rgvOut);
		}
	else
		{
		rgvOut = rgv;
		}

	int i;

	for (i=0;i<count;i++)
		{
		int x = (int)(rgvOut[i].x + 0.5);
		int y = (int)(rgvOut[i].y + 0.5);
		prc->left = min(prc->left, x - 1);
		prc->top = min(prc->top, y - 1);
		prc->right = max(prc->right, x + 1);
		prc->bottom = max(prc->bottom, y + 1);

		if (pznear)
			{
			*pznear = min(*pznear, rgvOut[i].z);
			*pzfar = max(*pzfar, rgvOut[i].z);
			}
		}

	if (!fTransformed)
		{
		delete rgvOut;
		}
	}

void Pin3D::ExpandRectByRect(RECT *prc, RECT *prcNew)
	{
	prc->left = min(prc->left, prcNew->left);
	prc->top = min(prc->top, prcNew->top);
	prc->right = max(prc->right, prcNew->right);
	prc->bottom = max(prc->bottom, prcNew->bottom);
	}

#define CURRENT_CACHE_VERSION 2

BOOL Pin3D::OpenCacheFileForRead()
	{
	m_fReadingFromCache = fFalse;

	char szCacheFileName[_MAX_PATH];
	if (!GetCacheFileName(szCacheFileName))
		{
		return fFalse;
		}
		
	WIN32_FILE_ATTRIBUTE_DATA fileinfo;
	BOOL fGotInfo = GetFileAttributesEx(g_pplayer->m_ptable->m_szFileName,
			GetFileExInfoStandard, &fileinfo);
	
	if (!fGotInfo)
		{
		return fFalse;
		}

	m_hFileCache = CreateFile(szCacheFileName,
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
		
	if (m_hFileCache == INVALID_HANDLE_VALUE)
		{
		return fFalse;
		}
		
	DWORD bytesRead;
	HRESULT hr;
		
	int version;
		
	ReadFile(m_hFileCache, &version, sizeof(int), &bytesRead, NULL);
	if (version != CURRENT_CACHE_VERSION)
		{
		CloseHandle(m_hFileCache);
		return fFalse;
		}
		
	// Compare the timestamps of the table file
	FILETIME cachedate;
	ReadFile(m_hFileCache, &cachedate, sizeof(cachedate), &bytesRead, NULL);
	if (fileinfo.ftLastWriteTime.dwLowDateTime != cachedate.dwLowDateTime ||
			fileinfo.ftLastWriteTime.dwHighDateTime != cachedate.dwHighDateTime)
		{
		CloseHandle(m_hFileCache);
		return fFalse;
		}
	
	int width, height;
	ReadFile(m_hFileCache, &width, sizeof(int), &bytesRead, NULL);
	ReadFile(m_hFileCache, &height, sizeof(int), &bytesRead, NULL);
	
	if (width != m_dwRenderWidth || height != m_dwRenderHeight)
		{
		CloseHandle(m_hFileCache);
		return fFalse;
		}
		
	DDPIXELFORMAT ddpf;
	DDPIXELFORMAT ddpfFile;
	ddpf.dwSize = sizeof(ddpf);
	ReadFile(m_hFileCache, &ddpfFile, sizeof(DDPIXELFORMAT), &bytesRead, NULL);
	hr = m_pddsStatic->GetPixelFormat(&ddpf);
	
	if (memcmp(&ddpf, &ddpfFile, sizeof(DDPIXELFORMAT)) != 0)
		{
		CloseHandle(m_hFileCache);
		return fFalse;
		}

	ReadFile(m_hFileCache, &ddpfFile, sizeof(DDPIXELFORMAT), &bytesRead, NULL);
	hr = m_pddsStaticZ->GetPixelFormat(&ddpf);
	
	if (memcmp(&ddpf, &ddpfFile, sizeof(DDPIXELFORMAT)) != 0)
		{
		CloseHandle(m_hFileCache);
		return fFalse;
		}

	m_fReadingFromCache = fTrue;
	return fTrue;
	}
	
BOOL Pin3D::GetCacheFileName(char *szFileName)
	{
	if (strlen(g_pplayer->m_ptable->m_szFileName) == 0)
		{
		// Don't bother caching tables that haven't even
		// been saved yet.  Only case - play default table?
		return fFalse;
		}
	
	TitleAndPathFromFilename(g_pplayer->m_ptable->m_szFileName, szFileName);
	
	strcat(szFileName, ".vpcache");
	
	return fTrue;
	}
	
void Pin3D::OpenCacheFileForWrite()
	{
	m_fWritingToCache = fFalse;
	
	char szCacheFileName[_MAX_PATH];
	if (!GetCacheFileName(szCacheFileName))
		{
		return;
		}
		
	WIN32_FILE_ATTRIBUTE_DATA fileinfo;
	BOOL fGotInfo = GetFileAttributesEx(g_pplayer->m_ptable->m_szFileName,
			GetFileExInfoStandard, &fileinfo);
	
	if (!fGotInfo)
		{
		return;
		}
	
	m_hFileCache = CreateFile(szCacheFileName,
		GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
		
	if (m_hFileCache == INVALID_HANDLE_VALUE)
		{
		return;
		}
		
	DWORD bytesWritten;
	int ret;
	HRESULT hr;
	
	// Write out version first.  If version doesn't match then we know we have to bail on everything else
	int version = CURRENT_CACHE_VERSION;
	ret = WriteFile(m_hFileCache, &version, sizeof(int), &bytesWritten, NULL);
	
	// Write out time stamp of on-disk table file.  At this point we know we have a valid file.
	ret = WriteFile(m_hFileCache, &fileinfo.ftLastWriteTime, sizeof(fileinfo.ftLastWriteTime), &bytesWritten, NULL);
	
	// Write out relevant information about what rendering this cache is for
	ret = WriteFile(m_hFileCache, &m_dwRenderWidth, sizeof(int), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &m_dwRenderHeight, sizeof(int), &bytesWritten, NULL);
	
	DDPIXELFORMAT ddpf;
	ddpf.dwSize = sizeof(ddpf);
	hr = m_pddsStatic->GetPixelFormat(&ddpf);
	ret = WriteFile(m_hFileCache, &ddpf, sizeof(ddpf), &bytesWritten, NULL);

	hr = m_pddsStaticZ->GetPixelFormat(&ddpf);
	ret = WriteFile(m_hFileCache, &ddpf, sizeof(ddpf), &bytesWritten, NULL);
	
	m_fWritingToCache = fTrue;
	}
	
void Pin3D::ReadSurfaceFromCacheFile(LPDIRECTDRAWSURFACE7 pdds)
	{
	if (!m_fReadingFromCache)
		{
		return;
		}
		
	DWORD bytesRead;
	//int ret;
	DDSURFACEDESC2 ddsd;
	HRESULT hr;
	
	ddsd.dwSize = sizeof(ddsd);
	hr = pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	
	DWORD width;
	DWORD height;
	long pitch;
	
	ReadFile(m_hFileCache, &width, sizeof(width), &bytesRead, NULL);
	ReadFile(m_hFileCache, &height, sizeof(height), &bytesRead, NULL);
	ReadFile(m_hFileCache, &pitch, sizeof(pitch), &bytesRead, NULL);
	ReadFile(m_hFileCache, ddsd.lpSurface, pitch*height, &bytesRead, NULL);
	//ret = WriteFile(m_hFileCache, &ddsd.dwWidth, sizeof(ddsd.dwWidth), &bytesWritten, NULL);
	//ret = WriteFile(m_hFileCache, &ddsd.dwHeight, sizeof(ddsd.dwHeight), &bytesWritten, NULL);
	//ret = WriteFile(m_hFileCache, &ddsd.lPitch, sizeof(ddsd.lPitch), &bytesWritten, NULL);
	//ret = WriteFile(m_hFileCache, ddsd.lpSurface, ddsd.lPitch * ddsd.dwHeight, &bytesWritten, NULL);
	hr = pdds->Unlock(NULL);
	}
	
void Pin3D::ReadObjFrameFromCacheFile(ObjFrame *pobjframe)
	{
	if (!m_fReadingFromCache)
		{
		return;
		}
		
	/*int key;
	ret = ReadFile(m_hFileCache, &key, sizeof(int), &bytesRead, NULL);
	
	if (key == 0) // null obj frame
		{
		pobjframe->pdds = NULL;
		pobjframe->pddsZBuffer = NULL;
		
		}*/
		
	DWORD bytesRead;
	int ret;
	ret = ReadFile(m_hFileCache, &pobjframe->rc, sizeof(RECT), &bytesRead, NULL);
	if (ret == 0)
		{
		DWORD foo = GetLastError();
		}
		
	int format;
	ret = ReadFile(m_hFileCache, &format, sizeof(int), &bytesRead, NULL);

	Assert(format == 1 || format == 2);
	
	DDSURFACEDESC2 ddsd;
	HRESULT hr;
	
	pobjframe->pdds = CreateOffscreen(pobjframe->rc.right - pobjframe->rc.left, pobjframe->rc.bottom - pobjframe->rc.top);
	
	ddsd.dwSize = sizeof(ddsd);
	hr = pobjframe->pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	
	DWORD width;
	DWORD height;
	long pitch;
	
	ReadFile(m_hFileCache, &width, sizeof(width), &bytesRead, NULL);
	ReadFile(m_hFileCache, &height, sizeof(height), &bytesRead, NULL);
	ReadFile(m_hFileCache, &pitch, sizeof(pitch), &bytesRead, NULL);
	ReadFile(m_hFileCache, ddsd.lpSurface, pitch*height, &bytesRead, NULL);
	hr = pobjframe->pdds->Unlock(NULL);
	
	if (format == 2) // there's a z-buffer
		{
		pobjframe->pddsZBuffer = CreateZBufferOffscreen(pobjframe->rc.right - pobjframe->rc.left, pobjframe->rc.bottom - pobjframe->rc.top);
	
		ddsd.dwSize = sizeof(ddsd);
		hr = pobjframe->pddsZBuffer->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		
		DWORD width;
		DWORD height;
		long pitch;
		
		ReadFile(m_hFileCache, &width, sizeof(width), &bytesRead, NULL);
		ReadFile(m_hFileCache, &height, sizeof(height), &bytesRead, NULL);
		ReadFile(m_hFileCache, &pitch, sizeof(pitch), &bytesRead, NULL);
		ReadFile(m_hFileCache, ddsd.lpSurface, pitch*height, &bytesRead, NULL);
		hr = pobjframe->pddsZBuffer->Unlock(NULL);
		}
	}
	
void Pin3D::ReadAnimObjectFromCacheFile(AnimObject *panimobj, ObjFrame **rgpobjframe, int count)
	{
	Vector<ObjFrame> vframe;
	ReadAnimObjectFromCacheFile(panimobj, &vframe);
	
	int i;
	for (i=0;i<count;i++)
		{
		rgpobjframe[i] = vframe.ElementAt(i);
		}
	}

void Pin3D::ReadAnimObjectFromCacheFile(AnimObject *panimobj, Vector<ObjFrame> *pvobjframe)
	{
	if (!m_fReadingFromCache)
		{
		return;
		}
		
	DWORD bytesRead;
	int ret;
	
	ret = ReadFile(m_hFileCache, &panimobj->m_rcBounds, sizeof(RECT), &bytesRead, NULL);
	ret = ReadFile(m_hFileCache, &panimobj->m_znear, sizeof(float), &bytesRead, NULL);
	ret = ReadFile(m_hFileCache, &panimobj->m_zfar, sizeof(float), &bytesRead, NULL);
	
	int framecount;
	ret = ReadFile(m_hFileCache, &framecount, sizeof(int), &bytesRead, NULL);
	
	int i;
	for (i=0;i<framecount;i++)
		{
		ObjFrame *pof = new ObjFrame();
		ReadObjFrameFromCacheFile(pof);
		pvobjframe->AddElement(pof);
		}
	}
	
void Pin3D::WriteSurfaceToCacheFile(LPDIRECTDRAWSURFACE7 pdds)
	{
	if (!m_fWritingToCache)
		{
		return;
		}
		
	DWORD bytesWritten;
	int ret;
	DDSURFACEDESC2 ddsd;
	HRESULT hr;
	
	ddsd.dwSize = sizeof(ddsd);
	hr = pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	ret = WriteFile(m_hFileCache, &ddsd.dwWidth, sizeof(ddsd.dwWidth), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &ddsd.dwHeight, sizeof(ddsd.dwHeight), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &ddsd.lPitch, sizeof(ddsd.lPitch), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, ddsd.lpSurface, ddsd.lPitch * ddsd.dwHeight, &bytesWritten, NULL);
	hr = pdds->Unlock(NULL);
	}
	
void Pin3D::WriteObjFrameToCacheFile(ObjFrame *pobjframe)
	{
	if (!m_fWritingToCache)
		{
		return;
		}
		
	/*int key = 1; // 1 means good obj frame.  0 means no obj frame.
	ret = WriteFile(m_hFileCache, &key, sizeof(int), &bytesWritten, NULL);*/

	DWORD bytesWritten;
	int ret;
	ret = WriteFile(m_hFileCache, &pobjframe->rc, sizeof(RECT), &bytesWritten, NULL);
	if (ret == 0)
		{
		DWORD foo = GetLastError();
		}
	
	DDSURFACEDESC2 ddsd;
	HRESULT hr;
	
	int format = pobjframe->pddsZBuffer ? 2 : 1;
	ret = WriteFile(m_hFileCache, &format, sizeof(int), &bytesWritten, NULL);
	
	ddsd.dwSize = sizeof(ddsd);
	hr = pobjframe->pdds->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	ret = WriteFile(m_hFileCache, &ddsd.dwWidth, sizeof(ddsd.dwWidth), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &ddsd.dwHeight, sizeof(ddsd.dwHeight), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &ddsd.lPitch, sizeof(ddsd.lPitch), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, ddsd.lpSurface, ddsd.lPitch * ddsd.dwHeight, &bytesWritten, NULL);
	hr = pobjframe->pdds->Unlock(NULL);
	
	if (pobjframe->pddsZBuffer)
		{
		hr = pobjframe->pddsZBuffer->Lock(NULL, &ddsd, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		ret = WriteFile(m_hFileCache, &ddsd.dwWidth, sizeof(ddsd.dwWidth), &bytesWritten, NULL);
		ret = WriteFile(m_hFileCache, &ddsd.dwHeight, sizeof(ddsd.dwHeight), &bytesWritten, NULL);
		ret = WriteFile(m_hFileCache, &ddsd.lPitch, sizeof(ddsd.lPitch), &bytesWritten, NULL);
		ret = WriteFile(m_hFileCache, ddsd.lpSurface, ddsd.lPitch * ddsd.dwHeight, &bytesWritten, NULL);
		hr = pobjframe->pddsZBuffer->Unlock(NULL);
		}
	}
	
void Pin3D::WriteAnimObjectToCacheFile(AnimObject *panimobj, ObjFrame **rgobjframe, int count)
	{
	Vector<ObjFrame> vframe;
	int i;
	for (i=0;i<count;i++)
		{
		vframe.AddElement(rgobjframe[i]);
		}
	WriteAnimObjectToCacheFile(panimobj, &vframe);
	}
	
void Pin3D::WriteAnimObjectToCacheFile(AnimObject *panimobj, Vector<ObjFrame> *pvobjframe)
	{
	if (!m_fWritingToCache)
		{
		return;
		}
		
	DWORD bytesWritten;
	int ret;
	
	ret = WriteFile(m_hFileCache, &panimobj->m_rcBounds, sizeof(RECT), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &panimobj->m_znear, sizeof(float), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &panimobj->m_zfar, sizeof(float), &bytesWritten, NULL);
	
	int framecount = pvobjframe->Size();
	ret = WriteFile(m_hFileCache, &framecount, sizeof(int), &bytesWritten, NULL);
	
	int i;
	for (i=0;i<pvobjframe->Size();i++)
		{
		WriteObjFrameToCacheFile(pvobjframe->ElementAt(i));
		}
	}
	
void Pin3D::CloseCacheFile()
	{
	if (m_hFileCache)
		{
		CloseHandle(m_hFileCache);
		m_hFileCache = NULL;
		}
	}

void PinProjection::Rotate(double x, double y, double z)
	{
	Matrix3D matTemp, matRotateX, matRotateY, matRotateZ;

	//m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);

	matRotateY.RotateYMatrix(y);
	matRotateX.RotateXMatrix(x);
	matRotateZ.RotateZMatrix(z);
	m_matWorld.Multiply(matRotateX, m_matWorld);
	m_matWorld.Multiply(matRotateY, m_matWorld);
	m_matWorld.Multiply(matRotateZ, m_matWorld);

	//m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
	}

void PinProjection::Translate(double x, double y, double z)
	{
	Matrix3D matTrans;

	//m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);

	matTrans._11 = matTrans._22 = matTrans._33 = matTrans._44 = 1.0f;
	matTrans._12 = matTrans._13 = matTrans._14 = 0.0f;
	matTrans._21 = matTrans._23 = matTrans._24 = 0.0f;
	matTrans._31 = matTrans._32 = matTrans._34 = 0.0f;

	matTrans._41 = (float)x;
	matTrans._42 = (float)y;
	matTrans._43 = (float)z;
	m_matWorld.Multiply(matTrans, m_matWorld);

	//m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
	}

void PinProjection::FitCameraToVertices(Vector<Vertex3D> *pvvertex3D, int cvert, double aspect, double rotation, double inclination, double FOV)
	{
	// Determine camera distance
	Vertex3D vertexT;

	double maxyintercept, minyintercept;
	double maxxintercept, minxintercept;

	double rrotsin, rrotcos, rincsin, rinccos, temp;
	double slopey, slopex;

	rrotsin = sin(-rotation);
	rrotcos = cos(-rotation);
	rincsin = sin(-inclination);
	rinccos = cos(-inclination);

	slopey = tan(FOV/360.0*2.0*0.5*PI); // *0.5 because slope is half of FOV - FOV includes top and bottom

	// Field of view along the axis = atan(tan(yFOV)*width/height)
	// So the slope of x simply equals slopey*width/height

	slopex = slopey*aspect;// slopey*m_rcHard.width/m_rcHard.height;

	maxyintercept = -DBL_MAX;
	minyintercept = DBL_MAX;
	maxxintercept = -DBL_MAX;
	minxintercept = DBL_MAX;

	m_rznear = 0;
	m_rzfar = 0;

	int i;
	for (i=0;i<cvert;i++)
		{
		//vertexT = rgv[i];
		vertexT = *(pvvertex3D->ElementAt(i));

		// Rotate vertex
		temp = vertexT.x;
		vertexT.x = (float)(rrotcos*temp + rrotsin*vertexT.z);
		vertexT.z = (float)(rrotcos*vertexT.z - rrotsin*temp);

		temp = vertexT.y;
		vertexT.y = (float)(rinccos*temp + rincsin*vertexT.z);
		vertexT.z = (float)(rinccos*vertexT.z - rincsin*temp);

		// Extend z-range if necessary
		m_rznear = min(m_rznear, -vertexT.z);
		m_rzfar = max(m_rzfar, -vertexT.z);

		// Extend slope lines from point to find camera intersection
		temp = vertexT.y + slopey*vertexT.z;
		maxyintercept = max(maxyintercept, temp);

		temp = vertexT.y - slopey*vertexT.z;
		minyintercept = min(minyintercept, temp);

		temp = vertexT.x + slopex*vertexT.z;
		maxxintercept = max(maxxintercept, temp);

		temp = vertexT.x - slopex*vertexT.z;
		minxintercept = min(minxintercept, temp);
		}

	double delta;

	// Find camera center in xy plane
	delta = maxyintercept - minyintercept;// Allow for roundoff error
	//maxyintercept += delta*0.01;
	//minyintercept -= delta*0.01;

	delta = maxxintercept - minxintercept;// Allow for roundoff error
	//maxxintercept += delta*0.01;
	//minxintercept -= delta*0.01;

	double ydist, xdist;
	ydist = (maxyintercept - minyintercept) / (slopey*2);
	xdist = (maxxintercept - minxintercept) / (slopex*2);
	m_vertexcamera.z = (float)(max(ydist,xdist));
	m_vertexcamera.y = (float)(slopey*ydist + minyintercept);
	m_vertexcamera.x = (float)(slopex*xdist + minxintercept);

	m_rznear += m_vertexcamera.z;
	m_rzfar += m_vertexcamera.z;

	delta = m_rzfar - m_rznear;

	m_rznear -= delta*0.01; // Allow for roundoff error
	m_rzfar += delta*0.01;
	}

void PinProjection::SetFieldOfView(double rFOV, double raspect, double rznear, double rzfar)
	{
// From the Field Of View and far z clipping plane, determine the front clipping plane size
	double yrange = rznear * tan(rFOV/2.0 /360.0 *2*PI);
	double xrange = yrange * raspect; //width/height

	//D3DMATRIX mat;

	memset(&m_matProj, 0, sizeof(D3DMATRIX));

    FLOAT Q = (float)(rzfar / ( rzfar - rznear ));

	m_matProj._11 = (float)(2*rznear / (xrange*2));
	m_matProj._22 = -(float)(2*rznear / (yrange*2));
	m_matProj._33 = Q;
	m_matProj._34 = 1;
	m_matProj._43 = -Q*(float)rznear;

	//m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &mat);

	m_matView._11 = m_matView._22 = m_matView._44 = 1.0f;
	m_matView._12 = m_matView._13 = m_matView._14 = m_matView._41 = 0.0f;
	m_matView._21 = m_matView._23 = m_matView._24 = m_matView._42 = 0.0f;
	m_matView._31 = m_matView._32 = m_matView._34 = m_matView._43 = 0.0f;
	m_matView._33 = -1;
    //m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &mat);

	m_matWorld.SetIdentity();
	}

void PinProjection::CacheTransform()
	{
	//Matrix3D matWorld, matView, matProj;
	Matrix3D matT;
	//m_pd3dDevice->GetTransform( D3DTRANSFORMSTATE_WORLD,      &matWorld );
	//m_pd3dDevice->GetTransform( D3DTRANSFORMSTATE_VIEW,       &matView );
	//m_pd3dDevice->GetTransform( D3DTRANSFORMSTATE_PROJECTION, &matProj );
	m_matProj.Multiply(m_matView, matT);
	matT.Multiply(m_matWorld, m_matrixTotal);
	}

void PinProjection::TransformVertices(Vertex3D* rgv, WORD *rgi, int count, Vertex3D *rgvout)
	{
	Vertex3D vT;

	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	//D3DVIEWPORT7 vp;
	//m_pd3dDevice->GetViewport( &vp );
	float rClipWidth  = (m_rcviewport.right - m_rcviewport.left)/2.0f;
	float rClipHeight = (m_rcviewport.bottom - m_rcviewport.top)/2.0f;
	int xoffset = m_rcviewport.left;
	int yoffset = m_rcviewport.top;

	int i,l;

	// Transform each vertex through the current matrix set
	for(i=0; i<count; i++ )
		{
		if (rgi)
			{
			l = rgi[i];
			}
		else
			{
			l = i;
			}

		// Get the untransformed vertex position
		FLOAT x = rgv[l].x;
		FLOAT y = rgv[l].y;
		FLOAT z = rgv[l].z;

		// Transform it through the current matrix set
		FLOAT xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		FLOAT yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		FLOAT wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		vT.x  = ((( 1.0f + (xp/wp) ) * rClipWidth + xoffset));
		vT.y  = ((( 1.0f - (yp/wp) ) * rClipHeight + yoffset));

		FLOAT zp = m_matrixTotal._13*x + m_matrixTotal._23*y + m_matrixTotal._33*z + m_matrixTotal._43;
		rgvout[l].x = vT.x;
		rgvout[l].y	= vT.y;
		rgvout[l].z = zp / wp;
		rgvout[l].nx = wp;
		}

	return;
	}

void Matrix3D::Multiply(Matrix3D &mult, Matrix3D &result)
	{
	int i,l;
	Matrix3D matrixT;
	for (i=0;i<4;i++)
		{
		for (l=0;l<4;l++)
			{
			matrixT.m[i][l] = (m[0][l] * mult.m[i][0]) + (m[1][l] * mult.m[i][1]) +
						(m[2][l] * mult.m[i][2]) + (m[3][l] * mult.m[i][3]);
			}
		}
	result = matrixT;
	}

void Matrix3D::RotateXMatrix(double x)
	{
	SetIdentity();
	_22 = _33 = (float)cos(x);
	_23 = (float)sin(x);
	_32 = -_23;
	}

void Matrix3D::RotateYMatrix(double y)
	{
	SetIdentity();
	_11 = _33 = (float)cos(y);
	_31 = (float)sin(y);
	_13 = -_31;
	}

void Matrix3D::RotateZMatrix(double z)
	{
	SetIdentity();
	_11 = _22 = (float)cos(z);
	_12 = (float)sin(z);
	_21 = -_12;
	}

void Matrix3D::SetIdentity()
	{
	_11 = _22 = _33 = _44 = 1.0f;
	_12 = _13 = _14 = _41 = 0.0f;
	_21 = _23 = _24 = _42 = 0.0f;
	_31 = _32 = _34 = _43 = 0.0f;
	}

#define MATRIXSIZE 4

void Matrix3D::Invert()
//void Gauss (RK8 ** a, RK8 ** b, int n)
	{
	float d, temp = 0, c;
	int i, j, k, l, nn;

	int ipvt[MATRIXSIZE];

	nn = MATRIXSIZE;
	for (i = 0; i < nn; i++)
		{
		ipvt[i] = i;
		}

	for (k = 0; k < nn; k++)
		{
		temp = 0.;
		l = k;
		for (i = k; i < nn; i++)
			{
			d = m[k][i];
			if (fabs (d) > temp)
				{
				temp = (float)fabs (d);
				l = i;
				}
			}
		if (l != k)
			{
			j = ipvt[k];
			ipvt[k] = ipvt[l];
			ipvt[l] = j;
			for (j = 0; j < nn; j++)
				{
				temp = m[j][k];
				m[j][k] = m[j][l];
				m[j][l] = temp;
				}
			}
		d = 1 / m[k][k];
		for (j = 0; j < k; j++)
			{
			c = m[j][k] * d;
			for (i = 0; i < nn; i++)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
			}
		for (j = k + 1; j < nn; j++)
			{
			c = m[j][k] * d;
			for (i = 0; i < nn; i++)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
			}
		for (i = 0; i < nn; i++)
			m[k][i] = -m[k][i] * d;
		m[k][k] = d;
		}

	Matrix3D mat3D;
	
	for (i = 0; i < nn; i++)
		{
		memcpy (mat3D.m[ipvt[i]], m[i], sizeof(float) * nn);
		}

	for (i = 0; i < nn; i++)
		{
		memcpy (m[i], mat3D.m[i], sizeof(float) * nn);
		}
	}

void Matrix3D::MultiplyVector(float x, float y, float z, Vertex3D *pv3DOut)
	{
	//Vertex3D vT;

	//int i,l;

	// Get the untransformed vertex position
	/*FLOAT x = rgv[l].x;
	FLOAT y = rgv[l].y;
	FLOAT z = rgv[l].z;*/

	// Transform it through the current matrix set
	FLOAT xp = _11*x + _21*y + _31*z + _41;
	FLOAT yp = _12*x + _22*y + _32*z + _42;
	FLOAT wp = _14*x + _24*y + _34*z + _44;

	FLOAT zp = _13*x + _23*y + _33*z + _43;

	pv3DOut->x = xp/wp;
	pv3DOut->y = yp/wp;
	pv3DOut->z = zp/wp;

	// Finally, scale the vertices to screen coords. This step first
	// "flattens" the coordinates from 3D space to 2D device coordinates,
	// by dividing each coordinate by the wp value. Then, the x- and
	// y-components are transformed from device coords to screen coords.
	// Note 1: device coords range from -1 to +1 in the viewport.
	/*vT.x  = ((( 1.0f + (xp/wp) ) * rClipWidth + xoffset));
	vT.y  = ((( 1.0f - (yp/wp) ) * rClipHeight + yoffset));

	FLOAT zp = m_matrixTotal._13*x + m_matrixTotal._23*y + m_matrixTotal._33*z + m_matrixTotal._43;
	rgvout[l].x = vT.x;
	rgvout[l].y	= vT.y;
	rgvout[l].z = zp / wp;
	rgvout[l].nx = wp;*/

	return;
	}
