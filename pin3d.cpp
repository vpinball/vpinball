#include "StdAfx.h"
#include "RenderDevice.h"

int NumVideoBytes = 0;

Pin3D::Pin3D()
{
	m_pddsBackBuffer = NULL;
	m_pdds3DBackBuffer = NULL;
	m_pddsZBuffer = NULL;
	m_pdds3DZBuffer = NULL;
	m_pd3dDevice = NULL;
	m_pddsStatic = NULL;
	m_pddsStaticZ = NULL;
    ballShadowTexture = NULL;
	m_envRadianceTexture = NULL;
	m_device_envRadianceTexture = NULL;
	backgroundVBuffer = NULL;
    tableVBuffer = NULL;
    tableIBuffer = NULL;
}

Pin3D::~Pin3D()
{
    m_pd3dDevice->SetZBuffer(NULL);

	SAFE_RELEASE(m_pdds3DBackBuffer);
	SAFE_RELEASE(m_pdds3DZBuffer);
	SAFE_RELEASE(m_pddsZBuffer);
	SAFE_RELEASE(m_pddsStatic);
	SAFE_RELEASE(m_pddsStaticZ);

    for (std::map<int,MemTexture*>::iterator it = m_xvShadowMap.begin(); it != m_xvShadowMap.end(); ++it)
        delete it->second;

   delete ballShadowTexture;
   ballTexture.FreeStuff();

   envTexture.FreeStuff();

   if(m_envRadianceTexture)
   {
	  m_pd3dDevice->m_texMan.UnloadTexture(m_envRadianceTexture);
	  delete m_envRadianceTexture;
	  m_envRadianceTexture = NULL;
   }
   m_device_envRadianceTexture = NULL;

   lightTexture[0].FreeStuff();
   lightTexture[1].FreeStuff();

	if(backgroundVBuffer)
		backgroundVBuffer->release();
    if (tableVBuffer)
        tableVBuffer->release();
    if (tableIBuffer)
        tableIBuffer->release();

	delete m_pd3dDevice;
}

void Pin3D::TransformVertices(const Vertex3D_NoTex2 * rgv, const WORD * rgi, int count, Vertex2D * rgvout) const
{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	const float rClipWidth  = vp.Width*0.5f;
	const float rClipHeight = vp.Height*0.5f;
	const int xoffset = vp.X;
	const int yoffset = vp.Y;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_proj.m_matrixTotal._11*x + m_proj.m_matrixTotal._21*y + m_proj.m_matrixTotal._31*z + m_proj.m_matrixTotal._41;
		const float yp = m_proj.m_matrixTotal._12*x + m_proj.m_matrixTotal._22*y + m_proj.m_matrixTotal._32*z + m_proj.m_matrixTotal._42;
		const float wp = m_proj.m_matrixTotal._14*x + m_proj.m_matrixTotal._24*y + m_proj.m_matrixTotal._34*z + m_proj.m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
	}
}

void EnvmapPrecalc(const DWORD* const __restrict envmap, const DWORD env_xres, const DWORD env_yres, DWORD* const __restrict rad_envmap, const DWORD rad_env_xres, const DWORD rad_env_yres)
{
	// brute force sampling over hemisphere for each normal direction of the to-be-(ir)radiance-baked environment
	// not the fastest solution, could do a "cosine convolution" over the picture instead (where also just 1024 or x samples could be used per pixel)
	// but with this implementation one can also have custom maps/LUTs for glossy, etc. later-on
	for(unsigned int y = 0; y < rad_env_yres; ++y)
		for(unsigned int x = 0; x < rad_env_xres; ++x)
		{
			// trafo from envmap to normal direction
			const float phi = (float)x/(float)rad_env_xres * (float)(2.0*M_PI) + M_PI;
			const float theta = (float)y/(float)rad_env_yres * (float)M_PI;
			const Vertex3Ds n(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));

			// draw x samples over hemisphere and collect cosine weighted environment map samples
			float sum[3];
			sum[0] = sum[1] = sum[2] = 0.0f;

			const unsigned int num_samples = 64;
			for(unsigned int s = 0; s < num_samples; ++s)
			{
				//!! could use cos_hemisphere_sample instead and trafo result to normal coord system, but as we do not use importance sampling on the environment, just not being smart -could- be better for high frequency environments anyhow
				Vertex3Ds l = sphere_sample((float)s*(float)(1.0/num_samples), radical_inverse(s)); // QMC hammersley point set
				float NdotL = l.Dot(n);
				if(NdotL < 0.0f) // flip if on backside of hemisphere
				{
					NdotL = -NdotL;
					l = -l;
				}

				// trafo from light direction to envmap
				const float u = atan2f(l.y, l.x) * (float)(0.5/M_PI) + 0.5f;
				const float v = acosf(l.z) * (float)(1.0/M_PI);
				
				const DWORD rgb = envmap[(int)(u*(float)env_xres)+(int)(v*(float)env_yres)*env_xres];
				const float r = powf((float)(rgb & 255) * (float)(1.0/255.0), 2.2f); //!! remove invgamma as soon as HDR
			    const float g = powf((float)(rgb & 65280) * (float)(1.0/65280.0), 2.2f);
				const float b = powf((float)(rgb & 16711680) * (float)(1.0/16711680.0), 2.2f);
				
				sum[0] += r * NdotL;
				sum[1] += g * NdotL;
				sum[2] += b * NdotL;
			}

			// average all samples
			sum[0] *= (float)(1.0/(M_PI*num_samples)); // pre-divides by PI for final radiance/color lookup in shader
			sum[1] *= (float)(1.0/(M_PI*num_samples)); // pre-divides by PI for final radiance/color lookup in shader
			sum[2] *= (float)(1.0/(M_PI*num_samples)); // pre-divides by PI for final radiance/color lookup in shader
			sum[0] = powf(sum[0],(float)(1.0/2.2)); //!! remove gamma as soon as HDR
			sum[1] = powf(sum[1],(float)(1.0/2.2));
			sum[2] = powf(sum[2],(float)(1.0/2.2));
			rad_envmap[y*rad_env_xres+x] = ((int)(sum[0]*255.0f)) | (((int)(sum[1]*255.0f))<<8) | (((int)(sum[2]*255.0f))<<16);
		}
}

HRESULT Pin3D::InitPin3D(const HWND hwnd, const bool fFullScreen, const int screenwidth, const int screenheight, const int colordepth, int &refreshrate, const int VSync, const bool useAA, const bool stereo3DFXAA)
{
    m_hwnd = hwnd;
    //fullscreen = fFullScreen;

    // Get the dimensions of the viewport and screen bounds //!! meh?!
    RECT rcScreen;
    GetClientRect( hwnd, &rcScreen );
    ClientToScreen( hwnd, (POINT*)&rcScreen.left );
    ClientToScreen( hwnd, (POINT*)&rcScreen.right );
    m_dwRenderWidth  = rcScreen.right  - rcScreen.left;
    m_dwRenderHeight = rcScreen.bottom - rcScreen.top;

    try {
        m_pd3dDevice = new RenderDevice(m_hwnd, m_dwRenderWidth, m_dwRenderHeight, fFullScreen, screenwidth, screenheight, colordepth, refreshrate, VSync, useAA, stereo3DFXAA);
    } catch (...) {
        return E_FAIL;
    }

    const int forceAniso = GetRegIntWithDefault("Player", "ForceAnisotropicFiltering", 0);
    m_pd3dDevice->ForceAnisotropicFiltering(!!forceAniso);

    // set the viewport for the newly created device
    vp.X=0;
    vp.Y=0;
    vp.Width=m_dwRenderWidth;
    vp.Height=m_dwRenderHeight;
    vp.MinZ=0.0f;
    vp.MaxZ=1.0f;
    m_pd3dDevice->SetViewport( &vp );

    m_pddsBackBuffer = m_pd3dDevice->GetBackBuffer();

    // Create the "static" color buffer.  
    // This will hold a pre-rendered image of the table and any non-changing elements (ie ramps, decals, etc).
    m_pddsStatic = m_pd3dDevice->DuplicateRenderTarget(m_pddsBackBuffer);

    m_pddsZBuffer = m_pd3dDevice->AttachZBufferTo(m_pddsBackBuffer);
    m_pddsStaticZ = m_pd3dDevice->AttachZBufferTo(m_pddsStatic);
    if (!m_pddsZBuffer || !m_pddsStatic)
        return E_FAIL;

    CreateBallShadow();

    ballTexture.CreateFromResource(IDB_BALL);
    ballTexture.SetAlpha(RGB(0,0,0));

	envTexture.CreateFromResource(IDB_ENV);
    envTexture.SetAlpha(RGB(0,0,0));

	m_envRadianceTexture = new MemTexture(envTexture.m_pdsBufferColorKey->width(),envTexture.m_pdsBufferColorKey->height());

	EnvmapPrecalc((DWORD*)envTexture.m_pdsBufferColorKey->data(),envTexture.m_pdsBufferColorKey->width(),envTexture.m_pdsBufferColorKey->height(),
				  (DWORD*)m_envRadianceTexture->data(),envTexture.m_pdsBufferColorKey->width(),envTexture.m_pdsBufferColorKey->height());

	
	m_device_envRadianceTexture = m_pd3dDevice->m_texMan.LoadTexture(m_envRadianceTexture);
	m_pd3dDevice->m_texMan.SetDirty(m_envRadianceTexture);

    lightTexture[0].CreateFromResource(IDB_SUNBURST);
    lightTexture[0].SetAlpha(RGB(0,0,0));

    lightTexture[1].CreateFromResource(IDB_SUNBURST2);
    lightTexture[1].SetAlpha(RGB(0,0,0));

    m_pddsLightWhite.CreateFromResource(IDB_WHITE);
    m_pddsLightWhite.SetAlpha(RGB(0,0,0));

    if(stereo3DFXAA) {
		m_pdds3DBackBuffer = m_pd3dDevice->DuplicateTexture(m_pddsBackBuffer);
	    if (!m_pdds3DBackBuffer)
		    return E_FAIL;
		m_pdds3DZBuffer = m_pd3dDevice->DuplicateDepthTexture(m_pddsZBuffer);
	    if (!m_pdds3DZBuffer)
		    return E_FAIL;
    }

	InitRenderState();

    // Direct all renders to the "static" buffer.
    SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

    return S_OK;
}


// Sets the texture filtering state.
void Pin3D::SetTextureFilter(const int TextureNum, const int Mode) const
{
    m_pd3dDevice->SetTextureFilter(TextureNum, Mode);
}

void Pin3D::SetRenderTarget(RenderTarget* pddsSurface, RenderTarget* pddsZ) const
{
	m_pd3dDevice->SetRenderTarget(pddsSurface);
	m_pd3dDevice->SetZBuffer(pddsZ);
}

void Pin3D::InitRenderState() 
{
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
	m_pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA );

	m_pd3dDevice->SetRenderState(RenderDevice::SPECULARENABLE, FALSE);

	m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
	m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
    m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

	m_pd3dDevice->SetRenderState( RenderDevice::CLIPPING, FALSE );
	m_pd3dDevice->SetRenderState( RenderDevice::CLIPPLANEENABLE, 0 );

	m_pd3dDevice->SetRenderState( RenderDevice::NORMALIZENORMALS, TRUE );

    // initialize first texture stage
    m_pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_CLAMP/*WRAP*/);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR );
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_TEXCOORDINDEX, 0);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pd3dDevice->SetTexture(ePictureTexture, NULL);

    // initialize second texture stage (light map)
	SetTextureFilter( eLightProject1, TEXTURE_MODE_TRILINEAR );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLORARG2, D3DTA_CURRENT );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_TEXCOORDINDEX, 1 );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
}

static const WORD rgiPin3D1[4] = {2,3,5,6};

void Pin3D::DrawBackground()
{
	PinTable * const ptable = g_pplayer->m_ptable;
	Texture * const pin = ptable->GetDecalsEnabled() ? ptable->GetImage((char *)g_pplayer->m_ptable->m_szImageBackdrop) : NULL;
	if (pin)
	{
		Vertex3D_NoTex2 rgv3D[4];
		rgv3D[0].x = 0;
		rgv3D[0].y = 0;
		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;

		rgv3D[1].x = (float)EDITOR_BG_WIDTH;
		rgv3D[1].y = 0;
		rgv3D[1].tu = 1.0f;
		rgv3D[1].tv = 0;

		rgv3D[2].x = (float)EDITOR_BG_WIDTH;
		rgv3D[2].y = (float)EDITOR_BG_HEIGHT;
		rgv3D[2].tu = 1.0f;
		rgv3D[2].tv = 1.0f;

		rgv3D[3].x = 0;
		rgv3D[3].y = (float)EDITOR_BG_HEIGHT;
		rgv3D[3].tu = 0;
		rgv3D[3].tv = 1.0f;

		SetHUDVertices(rgv3D, 4);
		SetDiffuse(rgv3D, 4, 0xFFFFFF);
		Vertex3D_NoTex2 *buf;

		//init background
		if( !backgroundVBuffer )
			m_pd3dDevice->CreateVertexBuffer( 4, 0, MY_D3DTRANSFORMED_NOTEX2_VERTEX, &backgroundVBuffer);
		backgroundVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
		memcpy( buf, rgv3D, sizeof(Vertex3D_NoTex2)*4);
		backgroundVBuffer->unlock();

		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0L );

		m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
        m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);

		SetTexture(pin);
		m_pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, backgroundVBuffer, 0, 4);

		m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
        m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
	}
	else
	{
        const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, d3dcolor, 1.0f, 0L );
	}
}



void Pin3D::InitLights()
{
    m_pd3dDevice->basicShader->Core()->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

    g_pplayer->m_ptable->m_Light[0].pos.x = g_pplayer->m_ptable->m_right*0.5f;
    g_pplayer->m_ptable->m_Light[1].pos.x = g_pplayer->m_ptable->m_right*0.5f;
    g_pplayer->m_ptable->m_Light[0].pos.y = g_pplayer->m_ptable->m_bottom*(float)(1.0/3.0);
    g_pplayer->m_ptable->m_Light[1].pos.y = g_pplayer->m_ptable->m_bottom*(float)(2.0/3.0);
    g_pplayer->m_ptable->m_Light[0].pos.z = g_pplayer->m_ptable->m_lightHeight;
    g_pplayer->m_ptable->m_Light[1].pos.z = g_pplayer->m_ptable->m_lightHeight;
    
	const D3DXVECTOR4 ambient = COLORREF_to_D3DXVECTOR4(g_pplayer->m_ptable->m_lightAmbient);
    D3DCOLORVALUE amb_rgb;
    amb_rgb.r = ambient.z;
    amb_rgb.g = ambient.y;
    amb_rgb.b = ambient.x;
    const D3DXVECTOR4 emission= COLORREF_to_D3DXVECTOR4(g_pplayer->m_ptable->m_Light[0].emission);
    D3DCOLORVALUE emission_rgb;
    emission_rgb.r = emission.z;
    emission_rgb.g = emission.y;
    emission_rgb.b = emission.x;

    char tmp[64];
    sprintf_s(tmp,"lights[0].vPos");
    m_pd3dDevice->basicShader->Core()->SetValue(tmp, (void*)&g_pplayer->m_ptable->m_Light[0].pos, sizeof(float)*3);
    sprintf_s(tmp,"lights[1].vPos");
    m_pd3dDevice->basicShader->Core()->SetValue(tmp, (void*)&g_pplayer->m_ptable->m_Light[1].pos, sizeof(float)*3);
    sprintf_s(tmp,"lights[0].vEmission");
    m_pd3dDevice->basicShader->Core()->SetValue(tmp, (void*)&emission_rgb, sizeof(float)*3);
    sprintf_s(tmp,"lights[1].vEmission");
    m_pd3dDevice->basicShader->Core()->SetValue(tmp, (void*)&emission_rgb, sizeof(float)*3);
    sprintf_s(tmp,"vAmbient");
    m_pd3dDevice->basicShader->Core()->SetValue(tmp, (void*)&amb_rgb, sizeof(float)*3);
    sprintf_s(tmp,"flightRange");
    m_pd3dDevice->basicShader->Core()->SetValue(tmp, (void*)&g_pplayer->m_ptable->m_lightRange, sizeof(float));

	m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
}

// currently unused
//void LookAt( Matrix3D &mat, D3DVECTOR eye, D3DVECTOR target, D3DVECTOR up )
//{
//   D3DVECTOR zaxis = Normalize(eye - target);
//   D3DVECTOR xaxis = Normalize(CrossProduct(up,zaxis));
//   D3DVECTOR yaxis = CrossProduct(zaxis,xaxis);
//   mat._11 = xaxis.x; mat._12 = yaxis.x; mat._13 = zaxis.x; mat._14=0.0f;
//   mat._21 = xaxis.y; mat._22 = yaxis.y; mat._23 = zaxis.y; mat._24=0.0f;
//   mat._31 = xaxis.z; mat._32 = yaxis.z; mat._33 = zaxis.z; mat._34=0.0f;
//   mat._41 = 0.0f;    mat._42 = 0.0f;    mat._43 = zaxis.x; mat._44=0.0f;
//   Matrix3D trans;
//   trans.SetIdentity();
//   trans._41 = eye.x; trans._42 = eye.y; trans._43=eye.z;
//   mat.Multiply( trans, mat );
//}

Matrix3D ComputeLaybackTransform(float layback)
{
    // skew the coordinate system from kartesian to non kartesian.
    Matrix3D matTrans;
    matTrans.SetIdentity();
    matTrans._32 = -tanf(0.5f * ANGTORAD(layback));
    return matTrans;
}

void Pin3D::InitLayout()
{
    TRACE_FUNCTION();
	const float rotation = ANGTORAD(g_pplayer->m_ptable->m_rotation);
	const float inclination = ANGTORAD(g_pplayer->m_ptable->m_inclination);
	const float FOV = (g_pplayer->m_ptable->m_FOV < 1.0f) ? 1.0f : g_pplayer->m_ptable->m_FOV;

	Vector<Vertex3Ds> vvertex3D;
	for (int i=0; i<g_pplayer->m_ptable->m_vedit.Size(); ++i)
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->GetBoundingVertices(&vvertex3D);

	const float aspect = (float)(4.0/3.0);//((float)m_dwRenderWidth)/m_dwRenderHeight;

    m_proj.FitCameraToVertices(&vvertex3D, aspect, rotation, inclination, FOV, g_pplayer->m_ptable->m_xlatez, g_pplayer->m_ptable->m_layback);

	m_proj.m_matWorld.SetIdentity();

    m_proj.m_matView.RotateXMatrix((float)M_PI);  // convert Z=out to Z=in (D3D coordinate system)
    m_proj.ScaleView(g_pplayer->m_ptable->m_scalex, g_pplayer->m_ptable->m_scaley, 1.0f);
    m_proj.TranslateView(g_pplayer->m_ptable->m_xlatex-m_proj.m_vertexcamera.x, g_pplayer->m_ptable->m_xlatey-m_proj.m_vertexcamera.y, -m_proj.m_vertexcamera.z);
    m_proj.RotateView(0, 0, rotation);
    m_proj.RotateView(inclination, 0, 0);
    m_proj.MultiplyView(ComputeLaybackTransform(g_pplayer->m_ptable->m_layback));

    // recompute near and far plane (workaround for VP9 FitCameraToVertices bugs)
    m_proj.ComputeNearFarPlane(vvertex3D);
    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(&proj, ANGTORAD(FOV), aspect, m_proj.m_rznear, m_proj.m_rzfar);
    memcpy(m_proj.m_matProj.m, proj.m, sizeof(float)*4*4);

	for (int i=0; i<vvertex3D.Size(); ++i)
		delete vvertex3D.ElementAt(i);

	m_pd3dDevice->SetTransform(TRANSFORMSTATE_PROJECTION, &m_proj.m_matProj);
	m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &m_proj.m_matView);
    m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &m_proj.m_matWorld);

	m_proj.CacheTransform();

    // Compute view vector
    /*Matrix3D temp, viewRot;
    temp = m_proj.m_matView;
    temp.Invert();
    temp.GetRotationPart( viewRot );
    viewRot.MultiplyVector(Vertex3Ds(0, 0, 1), m_viewVec);
    m_viewVec.Normalize();*/

	InitLights();
}

void Pin3D::RenderPlayfieldGraphics()
{
    TRACE_FUNCTION();

	Vertex3D rgv[7];
	rgv[0].x=g_pplayer->m_ptable->m_left;     rgv[0].y=g_pplayer->m_ptable->m_top;      rgv[0].z=0;
	rgv[1].x=g_pplayer->m_ptable->m_right;    rgv[1].y=g_pplayer->m_ptable->m_top;      rgv[1].z=0;
	rgv[2].x=g_pplayer->m_ptable->m_right;    rgv[2].y=g_pplayer->m_ptable->m_bottom;   rgv[2].z=0;
	rgv[3].x=g_pplayer->m_ptable->m_left;     rgv[3].y=g_pplayer->m_ptable->m_bottom;   rgv[3].z=0;

	// These next 4 vertices are used just to set the extents
	rgv[4].x=g_pplayer->m_ptable->m_left;     rgv[4].y=g_pplayer->m_ptable->m_top;      rgv[4].z=50.0f;
	rgv[5].x=g_pplayer->m_ptable->m_left;     rgv[5].y=g_pplayer->m_ptable->m_bottom;   rgv[5].z=50.0f;
	rgv[6].x=g_pplayer->m_ptable->m_right;    rgv[6].y=g_pplayer->m_ptable->m_bottom;   rgv[6].z=50.0f;
	//rgv[7].x=g_pplayer->m_ptable->m_right;    rgv[7].y=g_pplayer->m_ptable->m_top;      rgv[7].z=50.0f;

	Texture * const pin = g_pplayer->m_ptable->GetImage((char *)g_pplayer->m_ptable->m_szImage);
   m_pd3dDevice->SetVertexDeclaration( m_pd3dDevice->m_pVertexNormalTexelTexelDeclaration );

	EnableLightMap(0);

	for (int i=0; i<4; ++i)
	{
		rgv[i].nx = 0;
		rgv[i].ny = 0;
		rgv[i].nz = 1.0f;

		rgv[i].tv = (i&2) ? 1.0f : 0.f;
		rgv[i].tu = (i==1 || i==2) ? 1.0f : 0.f;
	}
	
	CalcShadowCoordinates(rgv,4);

    assert(tableVBuffer == NULL);
    m_pd3dDevice->CreateVertexBuffer( 4+7, 0, MY_D3DFVF_VERTEX, &tableVBuffer); //+7 verts for second rendering step

    Vertex3D *buffer;
	tableVBuffer->lock(0,0,(void**)&buffer, VertexBuffer::WRITEONLY);

	unsigned int offs = 0;
	for(unsigned int y = 0; y <= 1; ++y)
		for(unsigned int x = 0; x <= 1; ++x,++offs)
		{
			Vertex3D &tmp = buffer[offs];
			tmp.x = (x&1) ? rgv[1].x : rgv[0].x;
			tmp.y = (y&1) ? rgv[2].y : rgv[0].y;
			tmp.z = rgv[0].z;

			tmp.tu = (x&1) ? rgv[1].tu : rgv[0].tu;
			tmp.tv = (y&1) ? rgv[2].tv : rgv[0].tv;

			tmp.nx = rgv[0].nx;
			tmp.ny = rgv[0].ny;
			tmp.nz = rgv[0].nz;
		}

	CalcShadowCoordinates(buffer,4);

	SetNormal(rgv, rgiPin3D1, 4, NULL, NULL, 0);

	memcpy(buffer+4, rgv, 7*sizeof(Vertex3D));

	tableVBuffer->unlock();

	EnableLightMap(0);

   Material *mat = g_pplayer->m_ptable->GetMaterial( g_pplayer->m_ptable->m_szPlayfieldMaterial);
   D3DXVECTOR4 diffuseColor( 0.5f, 0.5f, 0.5f, 1.0f );
   D3DXVECTOR4 glossyColor( 0.04f, 0.04f, 0.04f, 1.0f );
   D3DXVECTOR4 specularColor( 0.04f, 0.04f, 0.04f, 1.0f );
   float diffuseWrap = 0.0f;
   float glossyPower = 0.1f;
   bool  bDiffActive=true;
   bool  bGlossyActive = true;
   bool  bSpecActive = false;
   if( mat )
   {
      diffuseColor = mat->getDiffuseColor();
      glossyColor = mat->getGlossyColor();
      specularColor = mat->getSpecularColor();
      diffuseWrap = mat->m_fDiffuse;
      glossyPower = mat->m_fGlossy;
      bDiffActive = mat->m_bDiffuseActive;
      bGlossyActive = mat->m_bGlossyActive;
      bSpecActive = mat->m_bSpecularActive;
   }

   m_pd3dDevice->basicShader->Core()->SetFloat("fDiffuseWrap",diffuseWrap);
   m_pd3dDevice->basicShader->Core()->SetFloat("fGlossyPower",glossyPower);
   m_pd3dDevice->basicShader->Core()->SetVector("vDiffuseColor",&diffuseColor);
   m_pd3dDevice->basicShader->Core()->SetVector("vGlossyColor",&glossyColor);
   m_pd3dDevice->basicShader->Core()->SetVector("vSpecularColor",&specularColor);
   m_pd3dDevice->basicShader->Core()->SetBool("bDiffuse", bDiffActive);
   m_pd3dDevice->basicShader->Core()->SetBool("bGlossy", bGlossyActive);
   m_pd3dDevice->basicShader->Core()->SetBool("bSpecular", bSpecActive);

	if (pin)
	{
      SetTextureFilter(ePictureTexture, TEXTURE_MODE_ANISOTROPIC);
      m_pd3dDevice->basicShader->SetTexture("Texture0",pin);
      m_pd3dDevice->basicShader->Core()->SetTechnique("basic_with_texture");
	}
	else // No image by that name
	{
      m_pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
	}

   assert(tableIBuffer == NULL);
   const WORD playfieldPolyIndices[6] = {0,1,3,0,3,2};
   tableIBuffer = m_pd3dDevice->CreateAndFillIndexBuffer(6,playfieldPolyIndices);
   m_pd3dDevice->basicShader->Begin(0);
	m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, tableVBuffer, 0, 4, tableIBuffer, 0, 6);
   m_pd3dDevice->basicShader->End();

	DisableLightMap();
	SetTexture(NULL);
    if (pin)
    {
        //m_pd3dDevice->SetTexture(0, NULL);
        m_pd3dDevice->m_texMan.UnloadTexture(pin->m_pdsBuffer);
        SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);
        m_pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
    }

    m_pd3dDevice->basicShader->Begin(0);
    m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLEFAN, tableVBuffer, 4, 7, (LPWORD)rgiPin3D1, 4);
    m_pd3dDevice->basicShader->End();  

    // Apparently, releasing the vertex buffer here immediately can cause rendering glitches in
    // later rendering steps, so we keep it around for now.
}

const int rgfilterwindow[7][7] =
   {1, 4, 8, 10, 8, 4, 1,
	4, 12, 25, 29, 25, 12, 4,
	8, 25, 49, 58, 49, 25, 8,
	10, 29, 58, 67, 58, 29, 10,
	8, 25, 49, 58, 49, 25, 8,
	4, 12, 25, 29, 25, 12, 4,
	1, 4, 8, 10, 8, 4, 1};

void Pin3D::CreateBallShadow()
{
	ballShadowTexture = new MemTexture(16, 16);

	const int pitch = ballShadowTexture->pitch();
	const int width = ballShadowTexture->width();
	const int height = ballShadowTexture->height();
	BYTE * const pc = ballShadowTexture->data();

	// Sharp Shadow
	int offset = 0;
	for (int y=0; y<height; ++y)
	{
		for (int x=0; x<width; ++x)
		{
			const int dx = 8-x;
			const int dy = 8-y;
			const int dist = dx*dx + dy*dy;
			pc[offset+x*4] = (dist <= 32) ? (BYTE)255 : (BYTE)0;
		}
		offset += pitch;
	}

	int totalwindow = 0;
	for (int i=0;i<7;++i)
		for (int l=0;l<7;++l)
			totalwindow += rgfilterwindow[i][l];

	for (int i=0;i<height;i++)
	{
		for (int l=0;l<width;l++)
		{
			int value = 0;
			for (int n=0;n<7;n++)
			{
				const int y = i+n-3;
				if(/*y>=0 &&*/ (unsigned int)y<=15)
				{
					BYTE * const pcy = pc + pitch*y;
					for (int m=0;m<7;m++)
					{
						const int x = l+m-3;
						if (/*x>=0 &&*/ (unsigned int)x<=15)
							value += (int)(*(pcy + 4*x)) * rgfilterwindow[m][n];
					}
				}
			}

			value /= totalwindow;

			value = (value*5)>>3;

			*(pc + pitch*i + l*4 + 3) = (BYTE)value;
		}
	}

	// Black out color channel
	offset = 0;
	for (int y=0; y<height; ++y)
	{
		for (int x=0; x<width*4; x+=4)
		{
			pc[offset+x  ] = 0;
			pc[offset+x+1] = 0;
			pc[offset+x+2] = 0;
		}
		offset += pitch;
	}
}

void Pin3D::CalcShadowCoordinates(Vertex3D * const pv, const unsigned int count) const
{
	const float inv_tu = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
	const float inv_tv = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);
	const float tu_offs = 0.5f - g_pplayer->m_ptable->m_right * 0.5f * inv_tu;
	const float tv_offs = 0.5f - g_pplayer->m_ptable->m_bottom * 0.5f * inv_tv;

	for(unsigned int i = 0; i < count; ++i)
	{
		pv[i].tu2 = pv[i].x*inv_tu + tu_offs;
		pv[i].tv2 = pv[i].y*inv_tv + tv_offs;
	}
}

BaseTexture* Pin3D::CreateShadow(const float z)
{
	int shadwidth;
	int shadheight;
	if ((g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right) > (g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom))
	{
		shadwidth = 256;
		shadheight = (int)(256.0f*(g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom)/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right));
	}
	else
	{
		shadheight = 256;
		shadwidth = (int)(256.0f*(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right)/(g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom));
	}

	// Create Shadow Picture
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 256;//shadwidth;
	bmi.bmiHeader.biHeight = -256;//-shadheight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;

	//WriteFile(hfile, &bmi, sizeof(bmi), &foo, NULL);

	HDC hdcScreen = GetDC(NULL);
	HDC hdc = CreateCompatibleDC(hdcScreen);

	BYTE *pbits;
	HBITMAP hdib = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void **)&pbits, NULL, 0);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdc, hdib);
	const float zoom = (float)shadwidth/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
	ShadowSur * const psur = new ShadowSur(hdc, zoom, (g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right)*0.5f, (g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom)*0.5f, shadwidth, shadheight, z);

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	PatBlt(hdc, 0, 0, shadwidth, shadheight, PATCOPY);

	for (int i=0; i<g_pplayer->m_ptable->m_vedit.Size(); ++i)
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->RenderShadow(psur, z);

	delete psur;

	BaseTexture* pddsProjectTexture = new MemTexture(shadwidth, shadheight);
	m_xvShadowMap[(int)z] = pddsProjectTexture;

	SelectObject(hdc, hbmOld);
	DeleteDC(hdc);
	ReleaseDC(NULL, hdcScreen);

    Texture::Blur(pddsProjectTexture, pbits, shadwidth, shadheight);

	DeleteObject(hdib);

	return pddsProjectTexture;
}

void Pin3D::SetTexture(Texture* pTexture)
{
    BaseTexture* tex = NULL;
    if (pTexture)
    {
        tex = pTexture->m_pdsBufferColorKey;
        if (tex == NULL)
            tex = pTexture->m_pdsBuffer;
    }
    SetBaseTexture(ePictureTexture, tex);
}

void Pin3D::SetBaseTexture(DWORD texUnit, BaseTexture* pddsTexture)
{
    m_pd3dDevice->SetTexture(texUnit,
            m_pd3dDevice->m_texMan.LoadTexture((pddsTexture == NULL) ? m_pddsLightWhite.m_pdsBufferColorKey : pddsTexture));
}

void Pin3D::EnableLightMap(const float z)
{
    BaseTexture* pdds = m_xvShadowMap[(int)z];
    if (!pdds)
        pdds = CreateShadow(z);
    SetBaseTexture(eLightProject1, pdds);
}

void Pin3D::DisableLightMap()
{
    m_pd3dDevice->SetTexture(eLightProject1, NULL);
}

void Pin3D::EnableAlphaTestReference(DWORD alphaRefValue) const
{
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, alphaRefValue);
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE); 
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATEREQUAL);
}

void Pin3D::EnableAlphaBlend( DWORD alphaRefValue, BOOL additiveBlending )
{
	EnableAlphaTestReference( alphaRefValue );
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
	m_pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, additiveBlending ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
	if(additiveBlending)
		m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_TFACTOR); // factor is 1,1,1,1
}

void Pin3D::DisableAlphaBlend()
{
    m_pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
    m_pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);
}

void Pin3D::Flip(bool vsync)
{
    m_pd3dDevice->Flip(vsync);
}

Vertex3Ds Pin3D::Unproject( Vertex3Ds *point)
{
   m_proj.CacheTransform(); // compute m_matrixTotal

   Matrix3D m2 = m_proj.m_matrixTotal; // = world * view * proj
   m2.Invert();
   Vertex3Ds p,p3;

   p.x = 2.0f * (point->x-g_pplayer->m_pin3d.vp.X) / g_pplayer->m_pin3d.vp.Width - 1.0f; 
   p.y = 1.0f - 2.0f * (point->y-g_pplayer->m_pin3d.vp.Y) / g_pplayer->m_pin3d.vp.Height; 
   p.z = (point->z - g_pplayer->m_pin3d.vp.MinZ) / (g_pplayer->m_pin3d.vp.MaxZ-g_pplayer->m_pin3d.vp.MinZ);
   p3 = m2.MultiplyVector( p );
   return p3;
}

Vertex3Ds Pin3D::Get3DPointFrom2D( POINT *p )
{
   Vertex3Ds p1,p2,pNear,pFar;
   pNear.x = (float)p->x; pNear.y = (float)p->y; pNear.z = (float)vp.MinZ;
   pFar.x = (float)p->x; pFar.y = (float)p->y; pFar.z = (float)vp.MaxZ;
   p1 = Unproject( &pNear );
   p2 = Unproject( &pFar);
   float wz = 0.0f;
   float wx = ((wz-p1.z)*(p2.x-p1.x))/(p2.z-p1.z) + p1.x;
   float wy = ((wz-p1.z)*(p2.y-p1.y))/(p2.z-p1.z) + p1.y;
   Vertex3Ds vertex(wx,wy,wz);
   return vertex;
}

void PinProjection::RotateView(float x, float y, float z)
{
	Matrix3D matRotateX, matRotateY, matRotateZ;

	matRotateX.RotateXMatrix(x);
	m_matView.Multiply(matRotateX, m_matView);
	matRotateY.RotateYMatrix(y);
	m_matView.Multiply(matRotateY, m_matView);
	matRotateZ.RotateZMatrix(z);
	m_matView.Multiply(matRotateZ, m_matView);        // matView = rotZ * rotY * rotX * origMatView
}

void PinProjection::TranslateView(const float x, const float y, const float z)
{
	Matrix3D matTrans;
	matTrans.SetTranslation(x, y, z);
	m_matView.Multiply(matTrans, m_matView);
}

void PinProjection::ScaleView(const float x, const float y, const float z)
{
	m_matView.Scale( x, y, z );
}

void PinProjection::MultiplyView(const Matrix3D& mat)
{
	m_matView.Multiply(mat, m_matView);
}

void PinProjection::FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback)
{
	// Determine camera distance
	const float rrotsin = sinf(rotation);
	const float rrotcos = cosf(rotation);
	const float rincsin = sinf(inclination);
	const float rinccos = cosf(inclination);

	const float slopey = tanf(0.5f*ANGTORAD(FOV)); // *0.5 because slope is half of FOV - FOV includes top and bottom

	// Field of view along the axis = atan(tan(yFOV)*width/height)
	// So the slope of x simply equals slopey*width/height

	const float slopex = slopey*aspect;

	float maxyintercept = -FLT_MAX;
	float minyintercept = FLT_MAX;
	float maxxintercept = -FLT_MAX;
	float minxintercept = FLT_MAX;

	m_rznear = FLT_MAX;
	m_rzfar = -FLT_MAX;

    Matrix3D laybackTrans = ComputeLaybackTransform(layback);

	for (int i=0; i<pvvertex3D->Size(); ++i)
	{
        Vertex3Ds v = *pvvertex3D->ElementAt(i);
		float temp;

        v = laybackTrans.MultiplyVector(v);

		// Rotate vertex about x axis according to incoming inclination
		temp = v.y;
		v.y = rinccos*temp - rincsin*v.z;
		v.z = rincsin*temp + rinccos*v.z;

		// Rotate vertex about z axis according to incoming rotation
		temp = v.x;
		v.x =  rrotcos*temp - rrotsin*v.y;
		v.y =  rrotsin*temp + rrotcos*v.y;

		// Extend z-range if necessary
		m_rznear = min(m_rznear, -v.z);
		m_rzfar  = max(m_rzfar,  -v.z);

		// Extend slope lines from point to find camera intersection
		maxyintercept = max(maxyintercept, v.y + slopey*v.z);
		minyintercept = min(minyintercept, v.y - slopey*v.z);
		maxxintercept = max(maxxintercept, v.x + slopex*v.z);
		minxintercept = min(minxintercept, v.x - slopex*v.z);
	}

	slintf ("maxy: %f\n",maxyintercept);
	slintf ("miny: %f\n",minyintercept);
	slintf ("maxx: %f\n",maxxintercept);
	slintf ("minx: %f\n",minxintercept);
	slintf ("m_rznear: %f\n",m_rznear);
	slintf ("m_rzfar : %f\n",m_rzfar);

	// Find camera center in xy plane

	const float ydist = (maxyintercept - minyintercept) / (slopey*2.0f);
	const float xdist = (maxxintercept - minxintercept) / (slopex*2.0f);
	m_vertexcamera.z = (float)(max(ydist,xdist)) + xlatez;
	m_vertexcamera.y = (float)((maxyintercept + minyintercept) * 0.5f);
	m_vertexcamera.x = (float)((maxxintercept + minxintercept) * 0.5f);

	m_rznear += m_vertexcamera.z;
	m_rzfar += m_vertexcamera.z;

	const float delta = m_rzfar - m_rznear;

#if 0
	m_rznear -= delta*0.15; // Allow for roundoff error (and tweak the setting too).
	m_rzfar += delta*0.01;
#else
	m_rznear -= delta*0.05f; // Allow for roundoff error
	m_rzfar += delta*0.01f;
#endif
}

void PinProjection::ComputeNearFarPlane(const Vector<Vertex3Ds>& verts)
{
    m_rznear = FLT_MAX;
    m_rzfar = -FLT_MAX;

    Matrix3D matWorldView;
    m_matView.Multiply( m_matWorld, matWorldView );

    for (int i = 0; i < verts.Size(); ++i)
    {
        Vertex3Ds temp = matWorldView.MultiplyVector(verts[i]);

        // Extend z-range if necessary
        m_rznear = min(m_rznear, temp.z);
        m_rzfar  = max(m_rzfar,  temp.z);
    }

    slintf("m_rznear: %f\n", m_rznear);
    slintf("m_rzfar : %f\n", m_rzfar);

    m_rznear *= 0.99f;
    m_rzfar *= 1.01f;
}


void PinProjection::SetupProjectionMatrix(float rFOV, float raspect, float rznear, float rzfar)
{
    m_matProj.SetIdentity();

#ifdef VP10
    if (rFOV < 1.0)     // orthographic? -- disabled, not compatible with old tables
    {
        const float yrange = EDITOR_BG_HEIGHT;
        const float xrange = yrange * raspect; //width/height
        const float zdist = rzfar - rznear;

        m_matProj._11 = (float)(1.0 / xrange);
        m_matProj._22 = (float)(1.0 / yrange);
        m_matProj._33 = (float)(1.0 / zdist);
        m_matProj._43 = (float)(-rznear / zdist);
        m_matProj._44 = 1.0f;
    }
    else
#endif
    {
        // From the Field Of View and far z clipping plane, determine the front clipping plane size
        const float yrange = tan(ANGTORAD(rFOV*0.5f));
        const float xrange = yrange * raspect; //width/height
        const float Q = rzfar / ( rzfar - rznear );

        m_matProj._11 = (float)(1.0 / xrange);
        m_matProj._22 = (float)(1.0 / yrange);
        m_matProj._33 = (float)Q;
        m_matProj._43 = (float)(-Q*rznear);
        m_matProj._34 = 1.0f;
    }
}

void PinProjection::CacheTransform()
{
	Matrix3D matT;
	m_matProj.Multiply(m_matView, matT);        // matT = matView * matProj
	matT.Multiply(m_matWorld, m_matrixTotal);   // total = matWorld * matView * matProj
}

void PinProjection::TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const
{
	const float rClipWidth  = (m_rcviewport.right - m_rcviewport.left)*0.5f;
	const float rClipHeight = (m_rcviewport.bottom - m_rcviewport.top)*0.5f;
	const int xoffset = m_rcviewport.left;
	const int yoffset = m_rcviewport.top;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		const float yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		const float wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
	}
}

