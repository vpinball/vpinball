#include "StdAfx.h"

Pin3D::Pin3D()
	{
	m_scalex = m_scaley = 1.0f;
	m_xlatex = m_xlatey = 0.0f;
	m_pDD = NULL;
	m_pddsFrontBuffer = NULL;
	m_pddsBackBuffer = NULL;
	m_pddsZBuffer = NULL;
	m_pD3D = NULL;
	m_pd3dDevice = NULL;
	m_pddsStatic = NULL;
	m_pddsBackTextureBuffer = NULL;
	m_pddsZTextureBuffer = NULL;
	m_pddsStaticZ = NULL;
	m_pddsBallTexture = NULL;
	m_pddsTargetTexture = NULL;
	m_pddsLightTexture = NULL;
	m_pddsLightWhite = NULL;
	m_pddsShadowTexture = NULL;
	}

Pin3D::~Pin3D()
	{
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

	if (m_pddsBackTextureBuffer)
		m_pddsBackTextureBuffer->Release();

	if (m_pddsZTextureBuffer)
		m_pddsZTextureBuffer->Release();

	if (m_pddsBallTexture)
		m_pddsBallTexture->Release();

	SAFE_RELEASE(m_pddsTargetTexture);

	if (m_pddsLightTexture)
		m_pddsLightTexture->Release();

	SAFE_RELEASE(m_pddsShadowTexture);

	for (int i=0;i<m_xvShadowMap.AbsoluteSize();i++)
		((LPDIRECTDRAWSURFACE)m_xvShadowMap.AbsoluteElementAt(i))->Release();

	SAFE_RELEASE(m_pddsLightWhite);

	if (m_pD3D)
		m_pD3D->Release();

	if (m_pd3dDevice)
		m_pd3dDevice->Release();
	}

static HRESULT WINAPI EnumZBufferFormatsCallback( DDPIXELFORMAT* pddpf,
                                                  VOID* pContext )
{
    DDPIXELFORMAT* pddpfOut = (DDPIXELFORMAT*)pContext;

    if((pddpf->dwRGBBitCount > 0) && (pddpfOut->dwRGBBitCount == pddpf->dwRGBBitCount))
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

void Pin3D::TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout)
	{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	D3DVIEWPORT7 vp;
	m_pd3dDevice->GetViewport( &vp );
	const float rClipWidth  = vp.dwWidth*0.5f;
	const float rClipHeight = vp.dwHeight*0.5f;
	const int xoffset = vp.dwX;
	const int yoffset = vp.dwY;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; i++ )
		{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const FLOAT x = rgv[l].x;
		const FLOAT y = rgv[l].y;
		const FLOAT z = rgv[l].z;

		// Transform it through the current matrix set
		const FLOAT xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		const FLOAT yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		const FLOAT wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const FLOAT inv_wp = 1.0f/wp;
		const FLOAT vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const FLOAT vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		const FLOAT zp = m_matrixTotal._13*x + m_matrixTotal._23*y + m_matrixTotal._33*z + m_matrixTotal._43;
		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		rgvout[l].z = zp * inv_wp;
		rgvout[l].nx = wp;
		}
	}

void Pin3D::TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout)
	{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	D3DVIEWPORT7 vp;
	m_pd3dDevice->GetViewport( &vp );
	const float rClipWidth  = vp.dwWidth*0.5f;
	const float rClipHeight = vp.dwHeight*0.5f;
	const int xoffset = vp.dwX;
	const int yoffset = vp.dwY;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; i++ )
		{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const FLOAT x = rgv[l].x;
		const FLOAT y = rgv[l].y;
		const FLOAT z = rgv[l].z;

		// Transform it through the current matrix set
		const FLOAT xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		const FLOAT yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		const FLOAT wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const FLOAT inv_wp = 1.0f/wp;
		const FLOAT vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const FLOAT vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		}
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
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_CKSRCBLT;
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = color;//0xffffff;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = color;//0xffffff;
    ddsd.dwWidth        = width;
    ddsd.dwHeight       = height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;// | DDSCAPS_3DDEVICE;

	// Check if we are rendering in hardware.
	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

	HRESULT hr;
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
		{
		ShowError("Could not create offscreen surface.");
		return NULL;
		}

	return pdds;
	}
	
LPDIRECTDRAWSURFACE7 Pin3D::CreateOffscreen(int width, int height)
	{
	DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	if (width < 1 || height < 1)
	{
		return NULL;
	}

	LPDIRECTDRAWSURFACE7 pdds;
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_CKSRCBLT;
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = 0;//0xffffff;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = 0;//0xffffff;
    ddsd.dwWidth        = width;
    ddsd.dwHeight       = height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	// Check if we are rendering in hardware.
	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

	HRESULT hr;
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
		{
		ShowError("Could not create offscreen surface.");
		exit(-1400); //rlc exit after show error
		return NULL;
		}

	// Update the count.
	NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

	return pdds;
	}

LPDIRECTDRAWSURFACE7 Pin3D::CreateZBufferOffscreen(int width, int height)
{
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

    // Setup the surface desc for the z-buffer.
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER; 
	ddsd.dwWidth        = width;
    ddsd.dwHeight       = height;
    ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized

	// Check if we are rendering in hardware.
	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

	bool retry = true;
retryall:

    // Find a suitable z buffer format.
    m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback, (VOID*)&ddsd.ddpfPixelFormat );

	// Create the z buffer, loop over possible other modes until one found
	HRESULT hr;
	int count = 0;
	while(( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) ) && (count <= 6))
    {
		switch(count) {
		case 0: {
			ddsd.ddpfPixelFormat.dwZBufferBitDepth = 32;
			ddsd.ddpfPixelFormat.dwStencilBitDepth = 0;
			ddsd.ddpfPixelFormat.dwZBitMask = 0xFFFFFFFF;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
			ddsd.ddpfPixelFormat.dwStencilBitMask = 0;
			break;
				}
		case 1: {
			ddsd.ddpfPixelFormat.dwZBufferBitDepth = 16;
			ddsd.ddpfPixelFormat.dwStencilBitDepth = 0;
			ddsd.ddpfPixelFormat.dwZBitMask = 0xFFFF;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
			ddsd.ddpfPixelFormat.dwStencilBitMask = 0;
			break;
				}
		case 2: {
			ddsd.ddpfPixelFormat.dwZBufferBitDepth = 24;
			ddsd.ddpfPixelFormat.dwStencilBitDepth = 0;
			ddsd.ddpfPixelFormat.dwZBitMask = 0xFFFFFF;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
			ddsd.ddpfPixelFormat.dwStencilBitMask = 0;
			break;
				}
		case 3: {
			ddsd.ddpfPixelFormat.dwZBufferBitDepth = 8;
			ddsd.ddpfPixelFormat.dwStencilBitDepth = 0;
			ddsd.ddpfPixelFormat.dwZBitMask = 0xFF;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
			ddsd.ddpfPixelFormat.dwStencilBitMask = 0;
			break;
				}

		case 4: {
			ddsd.ddpfPixelFormat.dwZBufferBitDepth = 32;
			ddsd.ddpfPixelFormat.dwStencilBitDepth = 8;
			ddsd.ddpfPixelFormat.dwZBitMask = 0xFFFFFFFF;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
			ddsd.ddpfPixelFormat.dwStencilBitMask = 0xFF;
			break;
				}
		case 5: {
			ddsd.ddpfPixelFormat.dwZBufferBitDepth = 24;
			ddsd.ddpfPixelFormat.dwStencilBitDepth = 8;
			ddsd.ddpfPixelFormat.dwZBitMask = 0xFFFFFF;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
			ddsd.ddpfPixelFormat.dwStencilBitMask = 0xFF;
			break;
				}
		case 6: {
			ddsd.ddpfPixelFormat.dwZBufferBitDepth = 16;
			ddsd.ddpfPixelFormat.dwStencilBitDepth = 8;
			ddsd.ddpfPixelFormat.dwZBitMask = 0xFFFF;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
			ddsd.ddpfPixelFormat.dwStencilBitMask = 0xFF;
			break;
				}
		}

        m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback, (VOID*)&ddsd.ddpfPixelFormat );

		count++;
	}

	if(FAILED(hr)) {
		// if all failed try with additional flag
		if(retry) {
			retry = false;
			ddsd.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM; //added BDS - corrects "Could not create offscreen Z-surface.", however renders slower on lower end hardware
			goto retryall;
		}

		if( hr != DDERR_OUTOFVIDEOMEMORY )
		{
			ShowError("Could not create offscreen Z-surface.");
		}
		else
		{
			ShowError("Out of Video Memory for offscreen Z-surface.");
		}
		return NULL;
    }

	// Update the count.
	NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwZBufferBitDepth/8);

    return pdds;// S_OK;
}

HRESULT Pin3D::InitDD(HWND hwnd, BOOL fFullScreen, int screenwidth, int screenheight, int colordepth, int refreshrate)
{
    m_hwnd = hwnd;

	const GUID* pDeviceGUID;

	// Check if we are rendering in hardware.
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

	SetUpdatePos(m_rcScreen.left, m_rcScreen.top);

	// Cache pointer from global direct draw object
	m_pDD = g_pvp->m_pdd.m_pDD; 

	HRESULT hr = m_pDD->QueryInterface( IID_IDirect3D7, (VOID**)&m_pD3D );
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

    // Create the primary surface
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
    {
		ShowError("Could not create front buffer.");
        return hr;
    }

	// Update the count.
	NumVideoBytes += m_dwRenderWidth * m_dwRenderHeight * 4;

    // If in windowed-mode, create a clipper object
    LPDIRECTDRAWCLIPPER pcClipper;
    if( FAILED( hr = m_pDD->CreateClipper( 0, &pcClipper, NULL ) ) )
    {
		ShowError("Could not create clipper.");
        return hr;
    }

    // Associate the clipper with the window.
    pcClipper->SetHWnd( 0, m_hwnd );
    m_pddsFrontBuffer->SetClipper( pcClipper );
    if (pcClipper)
		{
		pcClipper->Release();
		}

    // Define a backbuffer.
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth        = m_dwRenderWidth;
    ddsd.dwHeight       = m_dwRenderHeight;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

	// Check if we are rendering in hardware.
	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
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

	// Create the back buffer.  
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL ) ) )
    {
		ShowError("Could not create back buffer.");
        return hr;
    }

	// Update the count.
	NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

	// Create the "static" color buffer.  
	// This is will hold a pre-rendered image of the table and any non-changing elements (ie ramps, decals, etc).
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsStatic, NULL ) ) )
    {
		ShowError("Could not create static buffer.");
        return hr;
    }

	// Update the count.
	NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

	// Create the D3D device.  This device will pre-render everything once...
	// Then it will render only the ball and its shadow in real time.
	hr = Create3DDevice((GUID*) pDeviceGUID);
	if(FAILED(hr))
		{
		return hr;
		}

	// Create the z and "static" z buffers.
	// Also attach the z buffers to their corresponding color buffer.
	hr = CreateZBuffer((GUID*) pDeviceGUID);
	if(FAILED(hr))
		{
		return hr;
		}

	// Check if we are blitting with D3D.
	if (g_pvp->m_pdd.m_fUseD3DBlit)
	{
		// Define the back texture buffer.
		ddsd.dwSize		= sizeof ( DDSURFACEDESC2 );
		ddsd.dwFlags	= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
		ddsd.dwWidth	= m_dwRenderWidth; 
		ddsd.dwHeight	= m_dwRenderHeight; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

		// Find a transparent pixel format with 8 bit alpha.
		m_pd3dDevice->EnumTextureFormats ( Display_EnumurateTransparentTextureFormats, &(ddsd.ddpfPixelFormat) );

		// Create the back texture buffer.
		if( FAILED( hr = m_pDD->CreateSurface ( &ddsd, &m_pddsBackTextureBuffer, NULL ) ) )
		{
			ShowError("Could not create back texture buffer.");
			return hr;
		}

		// Update the count.
		NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

		// Create the z texture buffer.

		// Get z texture buffer dimensions from the render target.
		ddsd.dwSize = sizeof(ddsd);
		m_pddsBackTextureBuffer->GetSurfaceDesc( &ddsd );

		// Setup the surface desc for the z-buffer.
		ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY; 
		ddsd.ddsCaps.dwCaps2 = 0;
		ddsd.ddpfPixelFormat.dwSize = 0;  

		// Find a suitable z buffer format. 
		m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback, (VOID*)&ddsd.ddpfPixelFormat );

		// Check if we failed to find an appropriate z texture buffer 
/*		if( 32 != ddsd.ddpfPixelFormat.dwZBufferBitDepth )
		{
			// We couldn't find a 32-bit z texture buffer.  Print an error.
			ShowError("Could not find 32bpp z texture buffer format.");
		}
*/
		// Create the z texture buffer.
		if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsZTextureBuffer, NULL ) ) )
		{
			ShowError("Could not create Z Buffer.");
			return hr; 
		}

		// Update the count.
		NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

		if( FAILED( hr = m_pddsBackTextureBuffer->AddAttachedSurface( m_pddsZTextureBuffer ) ) )
		{
			ShowError("Could not attach Z texture buffer to texture buffer.");
			return hr; 
		}
	}

	// Direct all renders to the "static" buffer.
	SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

    return S_OK;
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

	DWORD caps = ddfoo.dpcLineCaps.dwRasterCaps;

	if (caps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
		{
			//!! hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
			//   and/or set ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING (if DDSCAPS_3DDEVICE also enabled)
		}

	// Finally, set the viewport for the newly created device
	D3DVIEWPORT7 vp = { 0, 0, m_dwRenderWidth, m_dwRenderHeight, 0.0f, 1.0f };

	if( FAILED(hr = m_pd3dDevice->SetViewport( &vp ) ) )
		{
			ShowError("Could not set viewport.");
			return hr; 
		}

	return S_OK;
	}

void Pin3D::EnsureDebugTextures()
	{
	if (!m_pddsTargetTexture)
		{
		int width, height;
		m_pddsTargetTexture = g_pvp->m_pdd.CreateFromResource(IDB_TARGET, &width, &height);
		g_pvp->m_pdd.SetAlpha(m_pddsTargetTexture, RGB(0,0,0), width, height);
		g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsTargetTexture);
		}
	}

HRESULT Pin3D::CreateZBuffer( GUID* pDeviceGUID )
{
    HRESULT hr;

    // Get z-buffer dimensions from the render target
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsBackBuffer->GetSurfaceDesc( &ddsd );

    // Setup the surface desc for the z-buffer.
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
    ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized

	// Check if we are rendering in software.
	if (!g_pvp->m_pdd.m_fHardwareAccel)
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}
	else
		{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}

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
    // Create the z buffer.
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsZBuffer, NULL ) ) )
    {
		ShowError("Could not create Z Buffer.");
        return hr; 
    }

	// Update the count.
	NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

	// Create the "static" z buffer.
    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsStaticZ, NULL ) ) )
    {
		ShowError("Could not create static Z Buffer.");
        return hr; 
    }

	// Update the count.
	NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

	int width, height;

	CreateBallShadow();

	m_pddsBallTexture = g_pvp->m_pdd.CreateFromResource(IDB_BALLTEXTURE, &width, &height);
	g_pvp->m_pdd.SetAlpha(m_pddsBallTexture, RGB(0,0,0), width, height);
	g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsBallTexture);

	m_pddsLightTexture = g_pvp->m_pdd.CreateFromResource(IDB_SUNBURST3, &width, &height);
	g_pvp->m_pdd.SetAlpha(m_pddsLightTexture, RGB(0,0,0), width, height);

	g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsLightTexture);

	m_pddsLightWhite = g_pvp->m_pdd.CreateFromResource(IDB_WHITE, &width, &height);
	g_pvp->m_pdd.SetAlpha(m_pddsLightWhite, RGB(0,0,0), width, height);
	g_pvp->m_pdd.CreateNextMipMapLevel(m_pddsLightWhite);

	// Attach the z buffer to the back buffer.
    if( FAILED( hr = m_pddsBackBuffer->AddAttachedSurface( m_pddsZBuffer ) ) )
    {
		ShowError("Could not attach Z-Buffer.");
        return hr; 
    }

	// Attach the "static" z buffer to the "static" buffer.
	if( FAILED( hr = m_pddsStatic->AddAttachedSurface( m_pddsStaticZ ) ) )
    {
		ShowError("Could not attach static Z-Buffer.");
        return hr; 
    }

    return S_OK;
}

// Sets the texture filtering state.
void Pin3D::SetTextureFilter(const int TextureNum, const int Mode) 
{
#if 0
	// Don't filter textures.  Don't filter between mip levels.
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MAGFILTER, D3DTFG_POINT);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MINFILTER, D3DTFN_POINT);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_NONE);		
	// Don't filter textures.  Don't filter between mip levels.
	m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_MAGFILTER, D3DTFG_POINT);
	m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_MINFILTER, D3DTFN_POINT);
	m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_MIPFILTER, D3DTFP_NONE);		
	return;
#endif

	// Set the state. 
	switch ( Mode )
	{
		case TEXTURE_MODE_POINT: 
			// Don't filter textures.  Don't filter between mip levels.
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MAGFILTER, D3DTFG_POINT);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MINFILTER, D3DTFN_POINT);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MIPFILTER, D3DTFP_NONE);		
			break;

		case TEXTURE_MODE_BILINEAR:
			// Filter textures when magnified or reduced (average of 2x2 texels).  Don't filter between mip levels.
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MINFILTER, D3DTFN_LINEAR);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MIPFILTER, D3DTFP_POINT);
			break;

		case TEXTURE_MODE_TRILINEAR:
			// Filter textures when magnified or reduced (average of 2x2 texels).  And filter between the 2 mip levels.
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MINFILTER, D3DTFN_LINEAR);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
			break;

		case TEXTURE_MODE_ANISOTROPIC:
			// Filter textures when magnified or reduced (filter to account for perspective distortion).  And filter between the 2 mip levels.
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
			break;

		default:
			// Don't filter textures.  Don't filter between mip levels.
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MAGFILTER, D3DTFG_POINT);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MINFILTER, D3DTFN_POINT);
			m_pd3dDevice->SetTextureStageState(TextureNum, D3DTSS_MIPFILTER, D3DTFP_NONE);
			break;
	}
}


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
	g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );															
	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_TEXCOORDINDEX, 0);

	hr = m_pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_COLOROP, D3DTOP_MODULATE);
	hr = m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	hr = m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_CLIPPING, FALSE );
	hr = m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_CLIPPLANEENABLE, 0 );
	}


void Pin3D::DrawBackground()
	{
	PinTable * const ptable = g_pplayer->m_ptable;
	PinImage * const pin = ptable->GetDecalsEnabled() ? ptable->GetImage((char *)g_pplayer->m_ptable->m_szImageBackdrop) : NULL;

	// Direct all renders to the "static" buffer.
	SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

	if (pin)
		{
		D3DMATERIAL7 mtrl;
		ZeroMemory( &mtrl, sizeof(mtrl) );

		mtrl.diffuse.r = mtrl.ambient.r = 1.0f;
		mtrl.diffuse.g = mtrl.ambient.g = 1.0f;
		mtrl.diffuse.b = mtrl.ambient.b = 1.0f;

		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER,
						   0, 1.0f, 0L );

		m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

		float maxtu,maxtv;
		g_pplayer->m_ptable->GetTVTU(pin, &maxtu, &maxtv);

		Vertex3D rgv3D[4];
		rgv3D[0].x = 0;
		rgv3D[0].y = 0;
		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;

		rgv3D[1].x = 1000.0f;
		rgv3D[1].y = 0;
		rgv3D[1].tu = maxtu;
		rgv3D[1].tv = 0;

		rgv3D[2].x = 1000.0f;
		rgv3D[2].y = 750.0f;
		rgv3D[2].tu = maxtu;
		rgv3D[2].tv = maxtv;

		rgv3D[3].x = 0;
		rgv3D[3].y = 750.0f;
		rgv3D[3].tu = 0;
		rgv3D[3].tv = maxtv;

		WORD rgi[4] = {0,1,2,3};

		SetTexture(pin->m_pdsBuffer);

		SetHUDVertices(rgv3D, 4);
		SetDiffuseFromMaterial(rgv3D, 4, &mtrl);

		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
												  rgv3D, 4,
												  rgi, 4, 0);

		m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		}
	else
		{
		const int r = (g_pplayer->m_ptable->m_colorbackdrop & 0xff0000) >> 16;
		const int g = (g_pplayer->m_ptable->m_colorbackdrop & 0xff00) >> 8;
		const int b = (g_pplayer->m_ptable->m_colorbackdrop & 0xff);
		const int d3dcolor = b<<16 | g<<8 | r;

		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
						   d3dcolor, 1.0f, 0L );
		}
	}

void Pin3D::InitLayout(const float left, const float top, const float right, const float bottom, const float inclination, const float FOV, const float rotation, const float scalex, const float scaley, const float xlatex, const float xlatey)
	{
	RECT rc;

	rc.left = 0;
	rc.top = 0;
	rc.right = m_width;
	rc.bottom = m_height;

	m_scalex = scalex;
	m_scaley = scaley;

	m_xlatex = xlatex;
	m_xlatey = xlatey;

	m_rotation = rotation*(float)(M_PI/180.0);
	m_inclination = inclination*(float)(M_PI/180.0);

	HRESULT hr;

	hr = m_pd3dDevice->SetTexture(ePictureTexture, NULL);

	//hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
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
	//light.dvDirection = D3DVECTOR( 5.0f, -20.0f, (float)cos(0.5) );
	//light.dvDirection = D3DVECTOR( -5.0f, 20.0f, -(float)cos(0.5) );
	//light.dvDirection = D3DVECTOR(-5.0f, 20.0f, -5.0f);

	float sn = sinf(m_inclination + (float)(M_PI - (M_PI*3.0/16.0)));
	float cs = cosf(m_inclination + (float)(M_PI - (M_PI*3.0/16.0)));

	light.dvDirection = D3DVECTOR(5.0f, sn * 21.0f, cs * -21.0f);
	light.dvRange        = D3DLIGHT_RANGE_MAX;
    light.dvAttenuation0 = 0.0f;
	light.dvAttenuation1 = 0.0f;
	light.dvAttenuation2 = 0.0f;

    // Set the light
    hr = m_pd3dDevice->SetLight( 0, &light );

	//light.dvDirection = D3DVECTOR( -(float)sin(-0.9), 0, -(float)cos(-0.9) );
	//light.dvDirection = D3DVECTOR(8.0f, 10.0f, -4.0f);

	//sn = sinf(m_inclination + (float)(M_PI*2.0/16.0));
	//cs = cosf(m_inclination + (float)(M_PI*2.0/16.0));

    light.dcvDiffuse.r   = 0.6f;
    light.dcvDiffuse.g   = 0.6f;
    light.dcvDiffuse.b   = 0.6f;
	light.dcvSpecular.r  = 1.0f;
    light.dcvSpecular.g  = 1.0f;
    light.dcvSpecular.b  = 1.0f;

	light.dvDirection = D3DVECTOR(-8.0f, sn * 11.0f, cs * -11.0f);

	hr = m_pd3dDevice->SetLight( 1, &light );

	Vertex3D rgv[8];
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

	EnableLightMap(g_pplayer->m_ptable->m_fRenderShadows, 0);

	//m_pd3dDevice->SetTexture(eLightProject1, m_pddsLightProjectTexture);
    hr = m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
    hr = m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA);

	g_pplayer->m_pin3d.SetTextureFilter ( eLightProject1, TEXTURE_MODE_BILINEAR ); 
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

	m_lightproject.m_v.x = g_pplayer->m_ptable->m_right *0.5f;//500;
	m_lightproject.m_v.y = g_pplayer->m_ptable->m_bottom *0.5f;
	m_lightproject.inclination = 0;
	m_lightproject.rotation = 0;
	m_lightproject.spin = 0;

	Vector<Vertex3D> vvertex3D;

	for (int i=0;i<g_pplayer->m_ptable->m_vedit.Size();i++)
		{
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->GetBoundingVertices(&vvertex3D);
		}

	hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

	const GPINFLOAT m_aspect = 4.0/3.0;//((GPINFLOAT)m_dwRenderWidth)/m_dwRenderHeight;

	// Clear the world matrix.
	Identity();
    
	FitCameraToVertices(&vvertex3D/*rgv*/, vvertex3D.Size(), m_aspect, m_rotation, m_inclination, FOV);
	SetFieldOfView(FOV, m_aspect, m_rznear, m_rzfar);
	if( m_rotation != 0.0f )
		{
		Scale( m_scalex, m_scaley, 1.0f );
		Rotate( 0, 0, m_rotation );
		Translate(-m_vertexcamera.x,-m_vertexcamera.y,-m_vertexcamera.z);
		Translate( m_xlatex, m_xlatey, 0.0f );
		Rotate( m_inclination, 0, 0 );
		}
	else
		{
		Translate(-m_vertexcamera.x,-m_vertexcamera.y,-m_vertexcamera.z);
		Rotate(m_inclination,0,m_rotation);
		}
	CacheTransform();

	for (int i=0;i<vvertex3D.Size();i++)
		{
		delete vvertex3D.ElementAt(i);
		}

	//hr = m_pd3dDevice->SetLight(0, &light);
    hr = m_pd3dDevice->LightEnable(0, TRUE);
	hr = m_pd3dDevice->LightEnable(1, TRUE);
    hr = m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);

	//EnableLightMap(fFalse, -1);

	InitBackGraphics();
	}

void Pin3D::InitBackGraphics()
	{
	// Direct all renders to the "static" buffer.
	SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

	EnableLightMap(fTrue, 0);

	Vertex3D rgv[8];
	rgv[0].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_top,0);
	rgv[3].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_bottom,0);
	rgv[2].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_bottom,0);
	rgv[1].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_top,0);

	// These next 4 vertices are used just to set the extents
	rgv[4].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_top,50.0f);
	rgv[5].Set(g_pplayer->m_ptable->m_left,g_pplayer->m_ptable->m_bottom,50.0f);
	rgv[6].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_bottom,50.0f);
	rgv[7].Set(g_pplayer->m_ptable->m_right,g_pplayer->m_ptable->m_top,50.0f);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );

	PinImage * const pin = g_pplayer->m_ptable->GetImage((char *)g_pplayer->m_ptable->m_szImage);

	float maxtu,maxtv;

	if (pin)
		{
		// Calculate texture coordinates.
		g_pplayer->m_ptable->GetTVTU(pin, &maxtu, &maxtv);		

		//m_pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBuffer);
		//SetTexture(pin->m_pdsBuffer);

		//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		//m_pd3dDevice->SetTextureStageState(eLightProject1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		mtrl.diffuse.r = mtrl.ambient.r = 1.0f;
		mtrl.diffuse.g = mtrl.ambient.g = 1.0f;
		mtrl.diffuse.b = mtrl.ambient.b = 1.0f;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

		SetTexture(pin->m_pdsBuffer);
		}
	else // No image by that name
		{
		SetTexture(NULL);

		mtrl.diffuse.r = mtrl.ambient.r = (g_pplayer->m_ptable->m_colorplayfield & 255) * (float)(1.0/255.0);
		mtrl.diffuse.g = mtrl.ambient.g = (g_pplayer->m_ptable->m_colorplayfield & 65280) * (float)(1.0/65280.0);
		mtrl.diffuse.b = mtrl.ambient.b = (g_pplayer->m_ptable->m_colorplayfield & 16711680) * (float)(1.0/16711680.0);
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

		maxtv = maxtu = 1.0f;
		}

	for (int i=0;i<4;i++)
		{
		rgv[i].nx = 0;
		rgv[i].ny = 0;
		rgv[i].nz = -1.0f;

		rgv[i].tv = i&2 ? maxtv : 0;
		rgv[i].tu = (i==1 || i==2) ? maxtu : 0;

		m_lightproject.CalcCoordinates(&rgv[i]);
		}

	m_pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[4] = {0,1,2,3};

	const HRESULT hr = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv, 4,
												  rgi, 4, 0);

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
												  rgi, 4, 0);
	}

void Pin3D::CreateBallShadow()
	{
	DDBLTFX ddbltfx;
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);

	ddbltfx.dwSize = sizeof(DDBLTFX);
	ddbltfx.dwFillColor = 0;
	m_pddsShadowTexture = g_pvp->m_pdd.CreateTextureOffscreen(16, 16);
	m_pddsShadowTexture->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	//int width, height;

	//m_pddsShadowTexture = g_pvp->m_pdd.CreateFromResource(IDB_BALLTEXTURE, &width, &height);

	//g_pvp->m_pdd.SetAlpha(m_pddsShadowTexture, RGB(0,0,0), 64, 64);

	//g_pvp->m_pdd.SetOpaque(m_pddsShadowTexture, 64, 64);

	m_pddsShadowTexture->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	{
	const int pitch = ddsd.lPitch;
	const int width = ddsd.dwWidth;
	const int height = ddsd.dwHeight;
	BYTE *pch = (BYTE *)ddsd.lpSurface;

	for (int y=0;y<height;y++)
		{
		for (int x=0;x<width;x++)
			{
			//pch += 3;
			const int dx = 8-x;
			const int dy = 8-y;
			const int dist = dx*dx + dy*dy;
			*pch = (dist < 25) ? 255 : 0;
			pch += 4;
			}
		pch += pitch - (width*4);
		}
	}

	m_pddsShadowTexture->Unlock(NULL);

	g_pvp->m_pdd.BlurAlpha(m_pddsShadowTexture);

	m_pddsShadowTexture->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	
	const int pitch = ddsd.lPitch;
	const int width = ddsd.dwWidth;
	const int height = ddsd.dwHeight;
	BYTE *pch = (BYTE *)ddsd.lpSurface;

	for (int y=0;y<height;y++)
		{
		for (int x=0;x<width;x++)
			{
			pch[0] = 0;
			pch[1] = 0;
			pch[2] = 0;
			pch += 4;
			}
		pch += pitch - (width*4);
		}

	m_pddsShadowTexture->Unlock(NULL);
	}

LPDIRECTDRAWSURFACE7 Pin3D::CreateShadow(const float z)
	{
	const float centerx = (g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right)*0.5f;
	const float centery = (g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom)*0.5f;

	int shadwidth;// = 128;
	int shadheight;// = 256;
	if (centerx > centery)
		{
		shadwidth = 256;
		m_maxtu = 1;
		m_maxtv = centery/centerx;
		shadheight = (int)(256.0f*m_maxtv);
		}
	else
		{
		shadheight = 256;
		m_maxtu = centerx/centery;
		m_maxtv = 1;
		shadwidth = (int)(256.0f*m_maxtu);
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
	const float zoom = shadwidth/(centerx*2.0f);
	ShadowSur * const psur = new ShadowSur(hdc2, zoom, centerx, centery, shadwidth, shadheight, z, NULL);

	SelectObject(hdc2, GetStockObject(WHITE_BRUSH));
	PatBlt(hdc2, 0, 0, shadwidth, shadheight, PATCOPY);

	for (int i=0;i<g_pplayer->m_ptable->m_vedit.Size();i++)
		{
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->RenderShadow(psur, z);
		}

	//BitBlt(hdcScreen, 0, 0, 128, 256, hdc2, 0, 0, SRCCOPY);

	LPDIRECTDRAWSURFACE7 pddsProjectTexture = g_pvp->m_pdd.CreateTextureOffscreen(shadwidth, shadheight);
	m_xvShadowMap.AddElement(pddsProjectTexture, (int)z);

	DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    pddsProjectTexture->GetSurfaceDesc(&ddsd);
	m_maxtu = (float)shadwidth/(float)ddsd.dwWidth;
	m_maxtv = (float)shadheight/(float)ddsd.dwHeight;
	//m_pddsLightProjectTexture = g_pvp->m_pdd.CreateTextureOffscreen(128, 256);

	delete psur;

	SelectObject(hdc2, hbmOld);

	DeleteDC(hdc2);
	ReleaseDC(NULL, hdcScreen);

	g_pvp->m_pdd.Blur(pddsProjectTexture, pbits, shadwidth, shadheight);

	DeleteObject(hdib);

	return pddsProjectTexture;
	}

void Pin3D::SetTexture(LPDIRECTDRAWSURFACE7 pddsTexture)
	{
	const HRESULT hr = m_pd3dDevice->SetTexture(ePictureTexture, (pddsTexture == NULL) ? m_pddsLightWhite : pddsTexture);
	}

void Pin3D::EnableLightMap(const BOOL fEnable, const float z)
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

void Pin3D::SetMaterial(const float r, const float g, const float b, const float a)
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
	const HRESULT hr = m_pd3dDevice->DrawIndexedPrimitive(d3dptPrimitiveType, dwVertexTypeDesc, lpvVertices,
														  dwVertexCount, lpwIndices, dwIndexCount, 0);

	return hr;
	}

void Pin3D::SetUpdatePos(const int left, const int top)
	{
	m_rcUpdate.left = left;
	m_rcUpdate.top = top;
	m_rcUpdate.right = left+m_dwRenderWidth;
	m_rcUpdate.bottom = top+m_dwRenderHeight;
	}

void Pin3D::Flip(const int offsetx, const int offsety)
	{
	RECT	rcNew;
	DDBLTFX ddbltfx;

	// Set the region to the entire screen dimensions.
	rcNew.left = m_rcUpdate.left + offsetx;
	rcNew.right = m_rcUpdate.right + offsetx;
	rcNew.top = m_rcUpdate.top + offsety;
	rcNew.bottom = m_rcUpdate.bottom + offsety;

	// Set blt effects to minimize tearing.
    ZeroMemory(&ddbltfx, sizeof(DDBLTFX));
	ddbltfx.dwSize = sizeof(DDBLTFX);
	//ddbltfx.dwDDFX = DDBLTFX_NOTEARING;

	// Check if we are mirrored.
	if ( g_pplayer->m_ptable->m_tblMirrorEnabled )
	{
		// Mirror the table.
		ddbltfx.dwDDFX = (ddbltfx.dwDDFX | DDBLTFX_MIRRORUPDOWN);
	}

	// Copy the back buffer to the front buffer.
	HRESULT hr = m_pddsFrontBuffer->Blt(&rcNew, m_pddsBackBuffer, NULL, DDBLT_DDFX, &ddbltfx);

	if (hr == DDERR_SURFACELOST)
		{
		hr = g_pvp->m_pdd.m_pDD->RestoreAllSurfaces();
		}
	}

void Pin3D::FitCameraToVertices(Vector<Vertex3D> * const pvvertex3D/*Vertex3D *rgv*/, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV)
	{
	// Determine camera distance
	
	const GPINFLOAT rrotsin = sin(-rotation);
	const GPINFLOAT rrotcos = cos(-rotation);
	const GPINFLOAT rincsin = sin(-inclination);
	const GPINFLOAT rinccos = cos(-inclination);

	const GPINFLOAT slopey = tan(FOV*(2.0*0.5*M_PI/360.0)); // *0.5 because slope is half of FOV - FOV includes top and bottom

	// Field of view along the axis = atan(tan(yFOV)*width/height)
	// So the slope of x simply equals slopey*width/height

	const GPINFLOAT slopex = slopey*aspect;// slopey*m_rcHard.width/m_rcHard.height;

	GPINFLOAT maxyintercept = -DBL_MAX;
	GPINFLOAT minyintercept = DBL_MAX;
	GPINFLOAT maxxintercept = -DBL_MAX;
	GPINFLOAT minxintercept = DBL_MAX;

	m_rznear = 0;
	m_rzfar = 0;

	for (int i=0;i<cvert;i++)
		{
		//vertexT = rgv[i];
		
		// Rotate vertex about y axis according to incoming rotation
		const GPINFLOAT temp = (*pvvertex3D->ElementAt(i)).x;
		const GPINFLOAT vertexTx = rrotcos*temp + rrotsin*(*pvvertex3D->ElementAt(i)).z;
			  GPINFLOAT vertexTz = rrotcos*(*pvvertex3D->ElementAt(i)).z - rrotsin*temp;

		// Rotate vertex about x axis according to incoming inclination
		const GPINFLOAT temp2 = (*pvvertex3D->ElementAt(i)).y;
		const GPINFLOAT vertexTy = rinccos*temp2 + rincsin*vertexTz;
						vertexTz = rinccos*vertexTz - rincsin*temp2;

		// Extend z-range if necessary
		m_rznear = min(m_rznear, -vertexTz);
		m_rzfar =  max(m_rzfar,  -vertexTz);

		// Extend slope lines from point to find camera intersection
		maxyintercept = max(maxyintercept, vertexTy + slopey*vertexTz);

		minyintercept = min(minyintercept, vertexTy - slopey*vertexTz);

		maxxintercept = max(maxxintercept, vertexTx + slopex*vertexTz);

		minxintercept = min(minxintercept, vertexTx - slopex*vertexTz);
		}

	// Find camera center in xy plane
	//delta = maxyintercept - minyintercept;// Allow for roundoff error
	//delta = maxxintercept - minxintercept;// Allow for roundoff error

	const GPINFLOAT ydist = (maxyintercept - minyintercept) / (slopey*2.0);
	const GPINFLOAT xdist = (maxxintercept - minxintercept) / (slopex*2.0);
	m_vertexcamera.z = (float)(max(ydist,xdist));
	m_vertexcamera.y = (float)(slopey*ydist + minyintercept);
	m_vertexcamera.x = (float)(slopex*xdist + minxintercept);

	m_rznear += m_vertexcamera.z;
	m_rzfar += m_vertexcamera.z;

	const GPINFLOAT delta = m_rzfar - m_rznear;

#if 1 
	m_rznear -= delta*0.05; // Allow for roundoff error (and tweak the setting too).
	m_rzfar += delta*0.01;
#else
	m_rznear -= delta*0.01; // Allow for roundoff error
	m_rzfar += delta*0.01;
#endif
	}

void Pin3D::Identity()
	{
	Matrix3D matTrans;

	matTrans._11 = matTrans._22 = matTrans._33 = matTrans._44 = 1.0f;
	matTrans._12 = matTrans._13 = matTrans._14 = 0.0f;
	matTrans._21 = matTrans._23 = matTrans._24 = 0.0f;
	matTrans._31 = matTrans._32 = matTrans._34 = 0.0f;
	matTrans._41 = matTrans._42 = matTrans._43 = 0.0f;

	m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTrans);
	}

void Pin3D::Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z)
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

void Pin3D::Scale(const float x, const float y, const float z)
{
	Matrix3D matTemp;

	m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);
	matTemp.Scale( x, y, z );
	m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
}

void Pin3D::Translate(const float x, const float y, const float z)
	{
	Matrix3D matTemp, matTrans;

	m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);

	matTrans._11 = matTrans._22 = matTrans._33 = matTrans._44 = 1.0f;
	matTrans._12 = matTrans._13 = matTrans._14 = 0.0f;
	matTrans._21 = matTrans._23 = matTrans._24 = 0.0f;
	matTrans._31 = matTrans._32 = matTrans._34 = 0.0f;

	matTrans._41 = x;
	matTrans._42 = y;
	matTrans._43 = z;
	matTemp.Multiply(matTrans, matTemp);

	m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
	}

void Pin3D::SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar)
	{
	// From the Field Of View and far z clipping plane, determine the front clipping plane size
	const GPINFLOAT yrange = rznear * tan(rFOV * (M_PI/360.0));
	const GPINFLOAT xrange = yrange * raspect; //width/height

	D3DMATRIX mat;
	memset(&mat, 0, sizeof(D3DMATRIX));

    const FLOAT Q = (float)(rzfar / ( rzfar - rznear ));

	mat._11 = (float)(rznear / xrange);
	mat._22 = -(float)(rznear / yrange);
	mat._33 = Q;
	mat._34 = 1.0f;
	mat._43 = -Q*(float)rznear;

	//mat._41 = 200;

	m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &mat);

	mat._11 = mat._22 = mat._44 = 1.0f;
	mat._12 = mat._13 = mat._14 = mat._41 = 0.0f;
	mat._21 = mat._23 = mat._24 = mat._42 = 0.0f;
	mat._31 = mat._32 = mat._34 = mat._43 = 0.0f;
	mat._33 = -1.0f;
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

void Pin3D::ClearExtents(RECT * const prc, float * const pznear, float * const pzfar)
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

void Pin3D::ExpandExtents(RECT * const prc, Vertex3D* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed)
	{
	Vertex3D * const rgvOut = (!fTransformed) ? new Vertex3D[count] : rgv;

	if (!fTransformed)
		TransformVertices(rgv, NULL, count, rgvOut);

	for (int i=0;i<count;i++)
		{
		const int x = (int)(rgvOut[i].x + 0.5f);
		const int y = (int)(rgvOut[i].y + 0.5f);
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

#define CURRENT_CACHE_VERSION 1

BOOL Pin3D::OpenCacheFileForRead()
	{
	m_fReadingFromCache = fFalse;

	if(!g_pplayer || !g_pplayer->m_ptable->m_TableCaching)return fFalse;
	
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
	HRESULT hr = m_pddsStatic->GetPixelFormat(&ddpf);

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

void Pin3D::DeleteCacheFileName()
	{
	char szCacheFileName[_MAX_PATH];	
	
	if (m_hFileCache)
		{
		CloseHandle(m_hFileCache);
		m_hFileCache = NULL;
		}

	m_fWritingToCache = fFalse;

	if (!GetCacheFileName(szCacheFileName))
		return;

	remove(szCacheFileName); //remove stale or damaged cache file
	}

void Pin3D::OpenCacheFileForWrite()
	{
	m_fWritingToCache = fFalse;
	
	if(!g_pplayer || !g_pplayer->m_ptable->m_TableCaching)
		return;

	char szCacheFileName[_MAX_PATH];
	if (!GetCacheFileName(szCacheFileName))
		return;

	WIN32_FILE_ATTRIBUTE_DATA fileinfo;
	BOOL fGotInfo = GetFileAttributesEx(g_pplayer->m_ptable->m_szFileName,
			GetFileExInfoStandard, &fileinfo);

	if (!fGotInfo)
		return;

	m_hFileCache = CreateFile(szCacheFileName,
		GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hFileCache == INVALID_HANDLE_VALUE)
		return;

	DWORD bytesWritten;

	// Write out version first.  If version doesn't match then we know we have to bail on everything else
	int version = CURRENT_CACHE_VERSION;
	int ret = WriteFile(m_hFileCache, &version, sizeof(int), &bytesWritten, NULL);

	// Write out time stamp of on-disk table file.  At this point we know we have a valid file.
	ret = WriteFile(m_hFileCache, &fileinfo.ftLastWriteTime, sizeof(fileinfo.ftLastWriteTime), &bytesWritten, NULL);

	// Write out relevant information about what rendering this cache is for
	ret = WriteFile(m_hFileCache, &m_dwRenderWidth, sizeof(int), &bytesWritten, NULL);
	ret = WriteFile(m_hFileCache, &m_dwRenderHeight, sizeof(int), &bytesWritten, NULL);

	DDPIXELFORMAT ddpf;
	ddpf.dwSize = sizeof(ddpf);
	HRESULT hr = m_pddsStatic->GetPixelFormat(&ddpf);
	ret = WriteFile(m_hFileCache, &ddpf, sizeof(ddpf), &bytesWritten, NULL);

	hr = m_pddsStaticZ->GetPixelFormat(&ddpf);
	ret = WriteFile(m_hFileCache, &ddpf, sizeof(ddpf), &bytesWritten, NULL);

	m_fWritingToCache = fTrue;
	}

void Pin3D::ReadSurfaceFromCacheFile(LPDIRECTDRAWSURFACE7 pdds)
	{
	if (!m_fReadingFromCache)
		return;

	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	HRESULT hr = pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

	DWORD width;
	DWORD height;
	long pitch;

	DWORD bytesRead;
	ReadFile(m_hFileCache, &width, sizeof(width), &bytesRead, NULL);
	ReadFile(m_hFileCache, &height, sizeof(height), &bytesRead, NULL);
	ReadFile(m_hFileCache, &pitch, sizeof(pitch), &bytesRead, NULL);
	ReadFile(m_hFileCache, ddsd.lpSurface, pitch*height, &bytesRead, NULL);
	hr = pdds->Unlock(NULL);
	}

void Pin3D::ReadObjFrameFromCacheFile(ObjFrame *pobjframe)
	{
	//Assert( pobjframe );
	if (!pobjframe) return; // can't return value to null struct
	pobjframe->pdds = NULL;	// set to NULL in case of error

	if (!m_fReadingFromCache) return;	//rlc cache is disabled 
	
	DWORD bytesRead;
	int ret = ReadFile(m_hFileCache, &pobjframe->rc, sizeof(RECT), &bytesRead, NULL);

	if (ret == 0) { const DWORD foo = GetLastError(); }

	if (pobjframe->rc.right == 0 && pobjframe->rc.left == 0)
		{
		// dummy case
		pobjframe->pdds = NULL;
		return;
		}

	int format;
	ret = ReadFile(m_hFileCache, &format, sizeof(int), &bytesRead, NULL);

	//Assert(format == 1 || format == 2);

	if(format != 1 && format != 2)	// give up cache file corrupted
		{
		//pobjframe->pdds = NULL;	
		DeleteCacheFileName();	// remove damaged or improper cache file
		exit(-1220);			// error is recoverable on restart executable
		return;
		}

	DDSURFACEDESC2 ddsd;
	pobjframe->pdds = CreateOffscreen(pobjframe->rc.right - pobjframe->rc.left, pobjframe->rc.bottom - pobjframe->rc.top);
	
	ddsd.dwSize = sizeof(ddsd);

	//Assert( pobjframe->pdds );
	if (!pobjframe->pdds ) 
		{
		DeleteCacheFileName();	//remove bad cache file
		exit(-1221);			// recoverable restart 
		return;
		}

	HRESULT hr = pobjframe->pdds->Lock(NULL, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

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

	for (int i=0;i<count;i++)
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

	for (int i=0;i<framecount;i++)
		{
		ObjFrame *pof = new ObjFrame();
		Assert( pof );//out of memory
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
		return;

	DWORD bytesWritten;
	int ret;

	if (pobjframe == NULL)
		{
		// special case - need to write out dummy spacer
		RECT rc;
		rc.left = 0;
		rc.right = 0;
		rc.top = 0;
		rc.bottom = 0;
		ret = WriteFile(m_hFileCache, &rc, sizeof(RECT), &bytesWritten, NULL);
		return;
		}

	ret = WriteFile(m_hFileCache, &pobjframe->rc, sizeof(RECT), &bytesWritten, NULL);
	if (ret == 0)
		{
		const DWORD foo = GetLastError();
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
	for (int i=0;i<count;i++)
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

	for (int i=0;i<pvobjframe->Size();i++)
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

void PinProjection::Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z)
	{
	Matrix3D /*matTemp,*/ matRotateX, matRotateY, matRotateZ;

	//m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);

	matRotateY.RotateYMatrix(y);
	matRotateX.RotateXMatrix(x);
	matRotateZ.RotateZMatrix(z);
	m_matWorld.Multiply(matRotateX, m_matWorld);
	m_matWorld.Multiply(matRotateY, m_matWorld);
	m_matWorld.Multiply(matRotateZ, m_matWorld);

	//m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
	}

void PinProjection::Translate(const float x, const float y, const float z)
	{
	Matrix3D matTrans;

	//m_pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matTemp);

	matTrans._11 = matTrans._22 = matTrans._33 = matTrans._44 = 1.0f;
	matTrans._12 = matTrans._13 = matTrans._14 = 0.0f;
	matTrans._21 = matTrans._23 = matTrans._24 = 0.0f;
	matTrans._31 = matTrans._32 = matTrans._34 = 0.0f;

	matTrans._41 = x;
	matTrans._42 = y;
	matTrans._43 = z;
	m_matWorld.Multiply(matTrans, m_matWorld);

	//m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matTemp);
	}

void PinProjection::FitCameraToVertices(Vector<Vertex3D> * const pvvertex3D, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV)
	{
	// Determine camera distance
	const GPINFLOAT rrotsin = sin(-rotation);
	const GPINFLOAT rrotcos = cos(-rotation);
	const GPINFLOAT rincsin = sin(-inclination);
	const GPINFLOAT rinccos = cos(-inclination);

	const GPINFLOAT slopey = tan(FOV*(2.0*0.5*M_PI/360.0)); // *0.5 because slope is half of FOV - FOV includes top and bottom

	// Field of view along the axis = atan(tan(yFOV)*width/height)
	// So the slope of x simply equals slopey*width/height

	const GPINFLOAT slopex = slopey*aspect;// slopey*m_rcHard.width/m_rcHard.height;

	GPINFLOAT maxyintercept = -DBL_MAX;
	GPINFLOAT minyintercept = DBL_MAX;
	GPINFLOAT maxxintercept = -DBL_MAX;
	GPINFLOAT minxintercept = DBL_MAX;

	m_rznear = 0;
	m_rzfar = 0;

	for (int i=0;i<cvert;i++)
		{
		//vertexT = rgv[i];
		
		// Rotate vertex
		const GPINFLOAT temp = (*pvvertex3D->ElementAt(i)).x;
		const GPINFLOAT vertexTx = rrotcos*temp + rrotsin*(*pvvertex3D->ElementAt(i)).z;
		      GPINFLOAT vertexTz = rrotcos*(*pvvertex3D->ElementAt(i)).z - rrotsin*temp;

		const GPINFLOAT temp2 = (*pvvertex3D->ElementAt(i)).y;
		const GPINFLOAT vertexTy = rinccos*temp2 + rincsin*vertexTz;
					    vertexTz = rinccos*vertexTz - rincsin*temp2;

		// Extend z-range if necessary
		m_rznear = min(m_rznear, -vertexTz);
		m_rzfar  = max(m_rzfar,  -vertexTz);

		// Extend slope lines from point to find camera intersection
		maxyintercept = max(maxyintercept, vertexTy + slopey*vertexTz);

		minyintercept = min(minyintercept, vertexTy - slopey*vertexTz);

		maxxintercept = max(maxxintercept, vertexTx + slopex*vertexTz);

		minxintercept = min(minxintercept, vertexTx - slopex*vertexTz);
		}

	//GPINFLOAT delta;

	// Find camera center in xy plane
	//delta = maxyintercept - minyintercept;// Allow for roundoff error
	//delta = maxxintercept - minxintercept;// Allow for roundoff error

	const GPINFLOAT ydist = (maxyintercept - minyintercept) / (slopey*2.0);
	const GPINFLOAT xdist = (maxxintercept - minxintercept) / (slopex*2.0);
	m_vertexcamera.z = (float)(max(ydist,xdist));
	m_vertexcamera.y = (float)(slopey*ydist + minyintercept);
	m_vertexcamera.x = (float)(slopex*xdist + minxintercept);

	m_rznear += m_vertexcamera.z;
	m_rzfar += m_vertexcamera.z;

	const GPINFLOAT delta = m_rzfar - m_rznear;

	m_rznear -= delta*0.01; // Allow for roundoff error
	m_rzfar += delta*0.01;
	}

void PinProjection::SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar)
	{
// From the Field Of View and far z clipping plane, determine the front clipping plane size
	const GPINFLOAT yrange = rznear * tan(ANGTORAD(rFOV*0.5));
	const GPINFLOAT xrange = yrange * raspect; //width/height

	//D3DMATRIX mat;

	memset(&m_matProj, 0, sizeof(D3DMATRIX));

    const FLOAT Q = (float)(rzfar / ( rzfar - rznear ));

	m_matProj._11 = (float)(rznear / xrange);
	m_matProj._22 = -(float)(rznear / yrange);
	m_matProj._33 = Q;
	m_matProj._34 = 1.0f;
	m_matProj._43 = -Q*(float)rznear;

	//m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &mat);

	m_matView._11 = m_matView._22 = m_matView._44 = 1.0f;
	m_matView._12 = m_matView._13 = m_matView._14 = m_matView._41 = 0.0f;
	m_matView._21 = m_matView._23 = m_matView._24 = m_matView._42 = 0.0f;
	m_matView._31 = m_matView._32 = m_matView._34 = m_matView._43 = 0.0f;
	m_matView._33 = -1.0f;
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

void PinProjection::TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout) const
	{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	//D3DVIEWPORT7 vp;
	//m_pd3dDevice->GetViewport( &vp );
	const float rClipWidth  = (m_rcviewport.right - m_rcviewport.left)*0.5f;
	const float rClipHeight = (m_rcviewport.bottom - m_rcviewport.top)*0.5f;
	const int xoffset = m_rcviewport.left;
	const int yoffset = m_rcviewport.top;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; i++ )
		{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const FLOAT x = rgv[l].x;
		const FLOAT y = rgv[l].y;
		const FLOAT z = rgv[l].z;

		// Transform it through the current matrix set
		const FLOAT xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		const FLOAT yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		const FLOAT wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const FLOAT inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		const FLOAT zp = m_matrixTotal._13*x + m_matrixTotal._23*y + m_matrixTotal._33*z + m_matrixTotal._43;
		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		rgvout[l].z = zp * inv_wp;
		rgvout[l].nx = wp;
		}
	}

void PinProjection::TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const
	{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	//D3DVIEWPORT7 vp;
	//m_pd3dDevice->GetViewport( &vp );
	const float rClipWidth  = (m_rcviewport.right - m_rcviewport.left)*0.5f;
	const float rClipHeight = (m_rcviewport.bottom - m_rcviewport.top)*0.5f;
	const int xoffset = m_rcviewport.left;
	const int yoffset = m_rcviewport.top;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; i++ )
		{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const FLOAT x = rgv[l].x;
		const FLOAT y = rgv[l].y;
		const FLOAT z = rgv[l].z;

		// Transform it through the current matrix set
		const FLOAT xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		const FLOAT yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		const FLOAT wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const FLOAT inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		}
	}

void Matrix3D::Multiply(const Matrix3D &mult, Matrix3D &result) const
	{
	Matrix3D matrixT;
	for (int i=0;i<4;i++)
		{
		for (int l=0;l<4;l++)
			{
			matrixT.m[i][l] = (m[0][l] * mult.m[i][0]) + (m[1][l] * mult.m[i][1]) +
						      (m[2][l] * mult.m[i][2]) + (m[3][l] * mult.m[i][3]);
			}
		}
	result = matrixT;
	}

void Matrix3D::RotateXMatrix(const GPINFLOAT x)
	{
	SetIdentity();
	_22 = _33 = (float)cos(x);
	_23 = (float)sin(x);
	_32 = -_23;
	}

void Matrix3D::RotateYMatrix(const GPINFLOAT y)
	{
	SetIdentity();
	_11 = _33 = (float)cos(y);
	_31 = (float)sin(y);
	_13 = -_31;
	}

void Matrix3D::RotateZMatrix(const GPINFLOAT z)
	{
	SetIdentity();
	_11 = _22 = (float)cos(z);
	_12 = (float)sin(z);
	_21 = -_12;
	}

void Matrix3D::Scale(const float x, const float y, const float z )
	{
	_11 *= x;
	_12 *= x;
	_13 *= x;
	_21 *= y;
	_22 *= y;
	_23 *= y;
	_31 *= z;
	_32 *= z;
	_33 *= z;
	}

void Matrix3D::SetIdentity()
	{
	_11 = _22 = _33 = _44 = 1.0f;
	_12 = _13 = _14 = _41 =
	_21 = _23 = _24 = _42 =
	_31 = _32 = _34 = _43 = 0.0f;
	}

#define MATRIXSIZE 4

void Matrix3D::Invert()
//void Gauss (RK8 ** a, RK8 ** b, int n)
	{
	int ipvt[MATRIXSIZE];
	const int nn = MATRIXSIZE;
	for (int i = 0; i < nn; i++)
		{
		ipvt[i] = i;
		}

	for (int k = 0; k < nn; k++)
		{
		float temp = 0.f;
		int l = k;
		for (int i = k; i < nn; i++)
			{
			const float d = fabsf(m[k][i]);
			if (d > temp)
				{
				temp = d;
				l = i;
				}
			}
		if (l != k)
			{
			const int tmp = ipvt[k];
			ipvt[k] = ipvt[l];
			ipvt[l] = tmp;
			for (int j = 0; j < nn; j++)
				{
				temp = m[j][k];
				m[j][k] = m[j][l];
				m[j][l] = temp;
				}
			}
		const float d = 1.0f / m[k][k];
		for (int j = 0; j < k; j++)
			{
			const float c = m[j][k] * d;
			for (int i = 0; i < nn; i++)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
			}
		for (int j = k + 1; j < nn; j++)
			{
			const float c = m[j][k] * d;
			for (int i = 0; i < nn; i++)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
			}
		for (int i = 0; i < nn; i++)
			m[k][i] = -m[k][i] * d;
		m[k][k] = d;
		}

	Matrix3D mat3D;

	for (int i = 0; i < nn; i++)
		{
		memcpy (mat3D.m[ipvt[i]], m[i], sizeof(float) * nn);
		}

	for (int i = 0; i < nn; i++)
		{
		memcpy (m[i], mat3D.m[i], sizeof(float) * nn);
		}
	}

void Matrix3D::MultiplyVector(const float x, const float y, const float z, Vertex3D * const pv3DOut) const
	{
	// Transform it through the current matrix set
	const FLOAT xp = _11*x + _21*y + _31*z + _41;
	const FLOAT yp = _12*x + _22*y + _32*z + _42;
	const FLOAT wp = _14*x + _24*y + _34*z + _44;

	const FLOAT zp = _13*x + _23*y + _33*z + _43;

	const FLOAT inv_wp = 1.0f/wp;
	pv3DOut->x = xp*inv_wp;
	pv3DOut->y = yp*inv_wp;
	pv3DOut->z = zp*inv_wp;

	return;
	}

void Matrix3D::MultiplyVector(const float x, const float y, const float z, Vertex3Ds * const pv3DOut) const
	{
	// Transform it through the current matrix set
	const FLOAT xp = _11*x + _21*y + _31*z + _41;
	const FLOAT yp = _12*x + _22*y + _32*z + _42;
	const FLOAT wp = _14*x + _24*y + _34*z + _44;

	const FLOAT zp = _13*x + _23*y + _33*z + _43;

	const FLOAT inv_wp = 1.0f/wp;
	pv3DOut->x = xp*inv_wp;
	pv3DOut->y = yp*inv_wp;
	pv3DOut->z = zp*inv_wp;

	return;
	}
