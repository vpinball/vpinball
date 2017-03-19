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
	const float sn = sinf(ANGTORAD(g_pplayer->m_ptable->m_inclination) + (float)(M_PI - (M_PI*3.0/16.0)));
	const float cs = cosf(ANGTORAD(g_pplayer->m_ptable->m_inclination) + (float)(M_PI - (M_PI*3.0/16.0)));

	for(unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
    {
        if(g_pplayer->m_ptable->m_Light[i].enabled)
        {
            BaseLight light;
            light.setType((g_pplayer->m_ptable->m_Light[i].type == LIGHT_DIRECTIONAL) ? D3DLIGHT_DIRECTIONAL
                    : ((g_pplayer->m_ptable->m_Light[i].type == LIGHT_POINT) ? D3DLIGHT_POINT : D3DLIGHT_SPOT));
            light.setAmbient( (float)(g_pplayer->m_ptable->m_Light[i].ambient & 255) * (float)(1.0/255.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].ambient & 65280) * (float)(1.0/65280.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].ambient & 16711680) * (float)(1.0/16711680.0));
            light.setDiffuse( (float)(g_pplayer->m_ptable->m_Light[i].diffuse & 255) * (float)(1.0/255.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].diffuse & 65280) * (float)(1.0/65280.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].diffuse & 16711680) * (float)(1.0/16711680.0));
            light.setSpecular((float)(g_pplayer->m_ptable->m_Light[i].specular & 255) * (float)(1.0/255.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].specular & 65280) * (float)(1.0/65280.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].specular & 16711680) * (float)(1.0/16711680.0));
            light.setRange( /*(light.getType() == D3DLIGHT_POINT) ? g_pplayer->m_ptable->m_Light[i].pointRange :*/
                    /* DX9 D3DLIGHT_RANGE_MAX */ 1e6f); //!!  expose?

            if((light.getType() == D3DLIGHT_POINT) || (light.getType() == D3DLIGHT_SPOT))
                light.setAttenuation2(0.0000025f); //!!  expose? //!! real world: light.dvAttenuation2 = 1.0f; but due to low dynamic 255-level-RGB lighting, we have to stick with the old crap

            if(light.getType() == D3DLIGHT_SPOT)
            {
                light.setFalloff( /*g_pplayer->m_ptable->m_Light[i].spotExponent;*/ 1.0f ); //!!  expose?
                light.setPhi    ( /*g_pplayer->m_ptable->m_Light[i].spotMin*/ (float)(60*M_PI/180.0) ); //!!  expose?
                light.setTheta  ( /*g_pplayer->m_ptable->m_Light[i].spotMax*/ (float)(20*M_PI/180.0) ); //!!  expose?
            }

            // transform dir & pos with world trafo to be always aligned with table (so that user trafos don't change the lighting!)
            if((g_pplayer->m_ptable->m_Light[i].dir.x == 0.0f) && (g_pplayer->m_ptable->m_Light[i].dir.y == 0.0f) &&
                    (g_pplayer->m_ptable->m_Light[i].dir.z == 0.0f) && (i < 2) && (light.getType() == D3DLIGHT_DIRECTIONAL))
            {
                // backwards compatibilty
                light.setAmbient(0.1f, 0.1f, 0.1f);
                light.setDiffuse(0.4f, 0.4f, 0.4f);
                light.setSpecular(0, 0, 0);
                light.setAttenuation0(0.0f);
                light.setAttenuation1(0.0f);
                light.setAttenuation2(0.0f);

                if ( i==0 )
                {
                    light.setDirection(5.0f, sn * 21.0f, cs * -21.0f);
                }
                else
                {
                    light.setDirection(-8.0f, sn * 11.0f, cs * -11.0f); 
                    light.setDiffuse(0.6f, 0.6f, 0.6f);
                    light.setSpecular(1.0f, 1.0f, 1.0f);
                }

            }
            else 
            {
                const Vertex3Ds tmp = m_proj.m_matWorld.MultiplyVectorNoTranslate(g_pplayer->m_ptable->m_Light[i].dir);
                light.setDirection(tmp.x, tmp.y, tmp.z);
            }

            const Vertex3Ds tmp = m_proj.m_matWorld.MultiplyVector(g_pplayer->m_ptable->m_Light[i].pos);
            light.setPosition(tmp.x, tmp.y, tmp.z);

            m_pd3dDevice->SetLight(i, &light);
            if (light.getAmbient().r  > 0.0f || light.getAmbient().g  > 0.0f || light.getAmbient().b  > 0.0f ||
                light.getDiffuse().r  > 0.0f || light.getDiffuse().g  > 0.0f || light.getDiffuse().b  > 0.0f ||
                light.getSpecular().r > 0.0f || light.getSpecular().g > 0.0f || light.getSpecular().b > 0.0f)
                m_pd3dDevice->LightEnable(i, TRUE);
            else
                m_pd3dDevice->LightEnable(i, FALSE);
        }
        else
            m_pd3dDevice->LightEnable(i, FALSE);
    }

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

/*const float realFOV = (ptable->m_FOV < 1.0f) ? 1.0f : ptable->m_FOV; // Can't have a real zero FOV, but this will look the same

	m_pin3d.InitLayout(ptable->m_left, ptable->m_top, ptable->m_right,
					   ptable->m_bottom, ptable->m_inclination, realFOV,
					   ptable->m_rotation, ptable->m_scalex, ptable->m_scaley,
					   ptable->m_xlatex, ptable->m_xlatey, ptable->m_xlatez, ptable->m_layback);*/
//const float left, const float top, const float right, const float bottom, const float inclination, const float FOV, const float rotation, const float scalex, const float scaley, const float xlatex, const float xlatey, const float xlatez, const float layback
void Pin3D::InitLayout()
{
    TRACE_FUNCTION();
	const float rotation = ANGTORAD(g_pplayer->m_ptable->m_rotation);
	const float inclination = ANGTORAD(g_pplayer->m_ptable->m_inclination);
	const float FOV = (g_pplayer->m_ptable->m_FOV < 1.0f) ? 1.0f : g_pplayer->m_ptable->m_FOV;

	Vector<Vertex3Ds> vvertex3D;
	for (int i=0; i<g_pplayer->m_ptable->m_vedit.Size(); ++i)
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->GetBoundingVertices(&vvertex3D);

	const GPINFLOAT aspect = 4.0/3.0;//((GPINFLOAT)m_dwRenderWidth)/m_dwRenderHeight;
	m_proj.FitCameraToVertices(&vvertex3D/*rgv*/, aspect, rotation, inclination, FOV, g_pplayer->m_ptable->m_xlatez);

    m_proj.SetFieldOfView(FOV, aspect, m_proj.m_rznear, m_proj.m_rzfar);

	const float skew = -tanf(0.5f*ANGTORAD(g_pplayer->m_ptable->m_layback));
	// skew the coordinate system from kartesian to non kartesian.
	skewX = -sinf(rotation)*skew;
	skewY =  cosf(rotation)*skew;
	// create skew the z axis to x and y direction.
	const float skewtan = tanf(ANGTORAD((180.0f-FOV)*0.5f))*m_proj.m_vertexcamera.y;
	Matrix3D matTrans;
	matTrans.SetIdentity();
	matTrans._31 = skewX;
	matTrans._32 = skewY;
	matTrans._41 = skewtan*skewX;
	matTrans._42 = skewtan*skewY;
	m_proj.Multiply(matTrans);

    m_proj.Scale( g_pplayer->m_ptable->m_scalex != 0.0f ? g_pplayer->m_ptable->m_scalex : 1.0f, g_pplayer->m_ptable->m_scaley != 0.0f ? g_pplayer->m_ptable->m_scaley : 1.0f, 1.0f );
#ifdef VP10
	m_proj.Translate(g_pplayer->m_ptable->m_xlatex-m_proj.m_vertexcamera.x, g_pplayer->m_ptable->m_xlatey-m_proj.m_vertexcamera.y, -m_proj.m_vertexcamera.z);
	m_proj.Rotate( 0, 0, rotation );
#else
	m_proj.Rotate( 0, 0, rotation );
	m_proj.Translate(g_pplayer->m_ptable->m_xlatex-m_proj.m_vertexcamera.x, g_pplayer->m_ptable->m_xlatey-m_proj.m_vertexcamera.y, -m_proj.m_vertexcamera.z);
#endif
	m_proj.Rotate( inclination, 0, 0 );

    // recompute near and far plane (workaround for VP9 FitCameraToVertices bugs)
    m_proj.ComputeNearFarPlane(vvertex3D);
    m_proj.SetupProjectionMatrix(FOV, aspect, m_proj.m_rznear, m_proj.m_rzfar);

	for (int i=0; i<vvertex3D.Size(); ++i)
		delete vvertex3D.ElementAt(i);

	m_pd3dDevice->SetTransform(TRANSFORMSTATE_PROJECTION, &m_proj.m_matProj);
	m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &m_proj.m_matView);
    m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &m_proj.m_matWorld);

	m_proj.CacheTransform();

    // Compute view vector
    Matrix3D temp, worldViewRot;
    m_proj.m_matView.Multiply( m_proj.m_matWorld, temp );   // TODO: use only view matrix once the camera transform is put there
    temp.Invert();
    temp.GetRotationPart( worldViewRot );
    worldViewRot.MultiplyVector(Vertex3Ds(0, 0, 1), m_viewVec);
    m_viewVec.Normalize();

	InitLights();
}

void Pin3D::RenderPlayfieldGraphics()
{
    TRACE_FUNCTION();
#define TRIANGULATE_BACK 100

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

	EnableLightMap(0);

	for (int i=0; i<4; ++i)
	{
		rgv[i].nx = 0;
		rgv[i].ny = 0;
		rgv[i].nz = -1.0f;

		rgv[i].tv = (i&2) ? 1.0f : 0.f;
		rgv[i].tu = (i==1 || i==2) ? 1.0f : 0.f;
	}
	
	CalcShadowCoordinates(rgv,4);

	// triangulate for better vertex based lighting //!! disable/set to 0 as soon as pixel shaders do the lighting

	const DWORD numVerts = (TRIANGULATE_BACK+1)*(TRIANGULATE_BACK+1);
	const DWORD numIndices = TRIANGULATE_BACK*TRIANGULATE_BACK*6;

    assert(tableVBuffer == NULL);
    m_pd3dDevice->CreateVertexBuffer( numVerts+7, 0, MY_D3DFVF_VERTEX, &tableVBuffer); //+7 verts for second rendering step

    Vertex3D *buffer;
	tableVBuffer->lock(0,0,(void**)&buffer, VertexBuffer::WRITEONLY);

	const float inv_tb = (float)(1.0/TRIANGULATE_BACK);
	unsigned int offs = 0;
	for(unsigned int y = 0; y <= TRIANGULATE_BACK; ++y)
	{
		for(unsigned int x = 0; x <= TRIANGULATE_BACK; ++x,++offs) //!! triangulate more in y then in x?
		{
			Vertex3D &tmp = buffer[offs];
			tmp.x = rgv[0].x + (rgv[1].x-rgv[0].x) * ((float)x*inv_tb);
			tmp.y = rgv[0].y + (rgv[2].y-rgv[0].y) * ((float)y*inv_tb);
			tmp.z = rgv[0].z;

			tmp.tu = rgv[0].tu + (rgv[1].tu-rgv[0].tu) * ((float)x*inv_tb);
			tmp.tv = rgv[0].tv + (rgv[2].tv-rgv[0].tv) * ((float)y*inv_tb);

			tmp.nx = rgv[0].nx;
			tmp.ny = rgv[0].ny;
			tmp.nz = rgv[0].nz;
		}
	}

	CalcShadowCoordinates(buffer,numVerts);

    std::vector<WORD> playfieldPolyIndices(numIndices);

	offs = 0;
	for(int y = 0; y < TRIANGULATE_BACK; ++y)
	{
        unsigned int offs2 = y * (TRIANGULATE_BACK+1);
		for(int x = 0; x < TRIANGULATE_BACK; ++x,offs+=6,++offs2)
		{
			WORD *tmp = &playfieldPolyIndices[offs];
			tmp[3] = tmp[0] = offs2;
			tmp[1] = offs2+1;
			tmp[4] = tmp[2] = offs2+1+(TRIANGULATE_BACK+1);
			tmp[5] = offs2+(TRIANGULATE_BACK+1);
		}
	}

	SetNormal(rgv, rgiPin3D1, 4, NULL, NULL, 0);
	memcpy( buffer+numVerts, rgv, 7*sizeof(Vertex3D));

	tableVBuffer->unlock();

	EnableLightMap(0);

	Material mtrl;

	if (pin)
	{
		SetTexture(pin);
        SetTextureFilter(ePictureTexture, TEXTURE_MODE_ANISOTROPIC);
	}
	else // No image by that name
	{
		SetTexture(NULL);
		mtrl.setColor( 1.0f, g_pplayer->m_ptable->m_colorplayfield );
	}
	m_pd3dDevice->SetMaterial(mtrl);

    assert(tableIBuffer == NULL);
    tableIBuffer = m_pd3dDevice->CreateAndFillIndexBuffer(playfieldPolyIndices);
	m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, tableVBuffer, 0, numVerts, tableIBuffer, 0, numIndices);

	DisableLightMap();
	SetTexture(NULL);
    if (pin)
    {
        m_pd3dDevice->SetTexture(0, NULL);
        m_pd3dDevice->m_texMan.UnloadTexture(pin->m_pdsBuffer);
        SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);
    }

	m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLEFAN, tableVBuffer, numVerts, 7, (LPWORD)rgiPin3D1, 4);

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
	HDC hdc2 = CreateCompatibleDC(hdcScreen);

	BYTE *pbits;
	HBITMAP hdib = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void **)&pbits, NULL, 0);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdc2, hdib);
	const float zoom = (float)shadwidth/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
	ShadowSur * const psur = new ShadowSur(hdc2, zoom, (g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right)*0.5f, (g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom)*0.5f, shadwidth, shadheight, z, NULL);

	SelectObject(hdc2, GetStockObject(WHITE_BRUSH));
	PatBlt(hdc2, 0, 0, shadwidth, shadheight, PATCOPY);

	for (int i=0; i<g_pplayer->m_ptable->m_vedit.Size(); ++i)
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->RenderShadow(psur, z);

	delete psur;

	BaseTexture* pddsProjectTexture = new MemTexture(shadwidth, shadheight);
	m_xvShadowMap[(int)z] = pddsProjectTexture;

	SelectObject(hdc2, hbmOld);
	DeleteDC(hdc2);
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

void PinProjection::Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z)
{
	Matrix3D matRotateX, matRotateY, matRotateZ;

	matRotateX.RotateXMatrix(x);
	m_matWorld.Multiply(matRotateX, m_matWorld);
	matRotateY.RotateYMatrix(y);
	m_matWorld.Multiply(matRotateY, m_matWorld);
	matRotateZ.RotateZMatrix(z);
	m_matWorld.Multiply(matRotateZ, m_matWorld);        // matWorld = rotZ * rotY * rotX * origMatWorld
}

void PinProjection::Translate(const float x, const float y, const float z)
{
	Matrix3D matTrans;
	matTrans.SetTranslation(x, y, z);
	m_matWorld.Multiply(matTrans, m_matWorld);
}

void PinProjection::Scale(const float x, const float y, const float z)
{
	m_matWorld.Scale( x, y, z );
}

void PinProjection::Multiply(const Matrix3D& mat)
{
	m_matWorld.Multiply(mat, m_matWorld);
}

void PinProjection::FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV, const float xlatez)
{
	// Determine camera distance
	const GPINFLOAT rrotsin = sin(rotation);
	const GPINFLOAT rrotcos = cos(rotation);
	const GPINFLOAT rincsin = sin(inclination);
	const GPINFLOAT rinccos = cos(inclination);

	const GPINFLOAT slopey = tan(0.5*ANGTORAD(FOV)); // *0.5 because slope is half of FOV - FOV includes top and bottom

	// Field of view along the axis = atan(tan(yFOV)*width/height)
	// So the slope of x simply equals slopey*width/height

	const GPINFLOAT slopex = slopey*aspect;

	GPINFLOAT maxyintercept = -DBL_MAX;
	GPINFLOAT minyintercept = DBL_MAX;
	GPINFLOAT maxxintercept = -DBL_MAX;
	GPINFLOAT minxintercept = DBL_MAX;

	m_rznear = FLT_MAX;
	m_rzfar = -FLT_MAX;

	for (int i=0; i<pvvertex3D->Size(); ++i)
	{
#ifdef VP10
		GPINFLOAT vertexTx = (*pvvertex3D->ElementAt(i)).x;
		GPINFLOAT vertexTy = (*pvvertex3D->ElementAt(i)).y;
		GPINFLOAT vertexTz = (*pvvertex3D->ElementAt(i)).z;
		GPINFLOAT temp;

		// Rotate vertex about x axis according to incoming inclination
		temp = vertexTy;
		vertexTy = rinccos*temp - rincsin*vertexTz;
		vertexTz = rincsin*temp + rinccos*vertexTz;

		// Rotate vertex about z axis according to incoming rotation
		temp = vertexTx;
		vertexTx =  rrotcos*temp - rrotsin*vertexTy;
		vertexTy =  rrotsin*temp + rrotcos*vertexTy;

        // TODO: handle layback if possible
#else
		GPINFLOAT vertexTy = (*pvvertex3D->ElementAt(i)).y;

		// Rotate vertex about y axis according to incoming rotation
		const GPINFLOAT temp = (*pvvertex3D->ElementAt(i)).x;
		const GPINFLOAT vertexTx =  rrotcos*temp - rrotsin*(*pvvertex3D->ElementAt(i)).z;
		GPINFLOAT       vertexTz =  rrotsin*temp + rrotcos*(*pvvertex3D->ElementAt(i)).z;

		// Rotate vertex about x axis according to incoming inclination
		const GPINFLOAT temp2 = vertexTy;
		vertexTy = rinccos*temp2 - rincsin*vertexTz;
		vertexTz = rincsin*temp2 + rinccos*vertexTz;
#endif

		// Extend z-range if necessary
		m_rznear = min(m_rznear, -vertexTz);
		m_rzfar  = max(m_rzfar,  -vertexTz);

		// Extend slope lines from point to find camera intersection
		maxyintercept = max(maxyintercept, vertexTy + slopey*vertexTz);
		minyintercept = min(minyintercept, vertexTy - slopey*vertexTz);
		maxxintercept = max(maxxintercept, vertexTx + slopex*vertexTz);
		minxintercept = min(minxintercept, vertexTx - slopex*vertexTz);
	}

	slintf ("maxy: %f\n",maxyintercept);
	slintf ("miny: %f\n",minyintercept);
	slintf ("maxx: %f\n",maxxintercept);
	slintf ("minx: %f\n",minxintercept);
	slintf ("m_rznear: %f\n",m_rznear);
	slintf ("m_rzfar : %f\n",m_rzfar);

	// Find camera center in xy plane

	const GPINFLOAT ydist = (maxyintercept - minyintercept) / (slopey*2.0);
	const GPINFLOAT xdist = (maxxintercept - minxintercept) / (slopex*2.0);
	m_vertexcamera.z = (float)(max(ydist,xdist)) + xlatez;
	m_vertexcamera.y = (float)((maxyintercept + minyintercept) * 0.5);
	m_vertexcamera.x = (float)((maxxintercept + minxintercept) * 0.5);

	m_rznear += m_vertexcamera.z;
	m_rzfar += m_vertexcamera.z;

	const GPINFLOAT delta = m_rzfar - m_rznear;

#if 0
	m_rznear -= delta*0.15; // Allow for roundoff error (and tweak the setting too).
	m_rzfar += delta*0.01;
#else
	m_rznear -= delta*0.05; // Allow for roundoff error
	m_rzfar += delta*0.01;
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
        m_rznear = min(m_rznear, (GPINFLOAT)temp.z);
        m_rzfar  = max(m_rzfar,  (GPINFLOAT)temp.z);
    }

    slintf("m_rznear: %f\n", m_rznear);
    slintf("m_rzfar : %f\n", m_rzfar);

    m_rznear *= 0.99;
    m_rzfar *= 1.01;
}

void PinProjection::SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar)
{
    SetupProjectionMatrix(rFOV, raspect, rznear, rzfar);

    m_matView.SetIdentity();
	m_matView._33 = -1.0f;

	m_matWorld.SetIdentity();
}

void PinProjection::SetupProjectionMatrix(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar)
{
    ZeroMemory(&m_matProj, sizeof(Matrix3D));

#ifdef VP10
    if (rFOV < 1.0)     // orthographic? -- disabled, not compatible with old tables
    {
        const GPINFLOAT yrange = EDITOR_BG_HEIGHT;
        const GPINFLOAT xrange = yrange * raspect; //width/height
        const GPINFLOAT zdist = rzfar - rznear;

        m_matProj._11 = (float)(1.0 / xrange);
        m_matProj._22 = -(float)(1.0 / yrange);
        m_matProj._33 = (float)(1.0 / zdist);
        m_matProj._43 = (float)(-rznear / zdist);
        m_matProj._44 = 1.0f;
    }
    else
#endif
    {
        // From the Field Of View and far z clipping plane, determine the front clipping plane size
        const GPINFLOAT yrange = tan(ANGTORAD(rFOV*0.5));
        const GPINFLOAT xrange = yrange * raspect; //width/height
        const GPINFLOAT Q = rzfar / ( rzfar - rznear );

        m_matProj._11 = (float)(1.0 / xrange);
        m_matProj._22 = -(float)(1.0 / yrange);
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

