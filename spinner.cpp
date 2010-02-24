// Spinner.cpp : Implementation of CVBATestApp and DLL registration.

#include "stdafx.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////
//

Spinner::Spinner()
	{
	m_phitspinner = NULL;
	}

Spinner::~Spinner()
	{
	}

HRESULT Spinner::Init(PinTable *ptable, float x, float y)
	{
	HRESULT hr = S_OK;

	m_ptable = ptable;

	m_d.m_vCenter.x = x;
	m_d.m_vCenter.y = y;

	SetDefaults();

	InitVBA(fTrue, 0, NULL);

	return hr;
	}

void Spinner::SetDefaults()
	{
	m_d.m_length = 80;
	m_d.m_rotation = 0;
	m_d.m_height = 60;
	m_d.m_overhang = 10;
	m_d.m_color = RGB(50,200,50);
	m_d.m_fCastsShadow = fTrue;			//<<< added by Chris

	// Anti-friction is 1-friction (throughput)
	m_d.m_antifriction = 0.99f;

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_szImageFront[0] = 0;
	m_d.m_szImageBack[0] = 0;
	m_d.m_szSurface[0] = 0;
	
	m_d.m_fSupports = fTrue;
	m_d.m_angleMax = 0;
	m_d.m_angleMin = 0;
	m_d.m_elasticity = 0.3f;
	m_d.m_friction = 0;	//zero uses global value
	m_d.m_scatter = 0;	//zero uses global value

	m_d.m_animations = 0;	// manual selection of the animations frame count
	m_d.m_fVisible = fTrue;


	}

void Spinner::PreRender(Sur *psur)
	{
	/*psur->SetBorderColor(-1,fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);*/
	}

void Spinner::Render(Sur *psur)
	{
	psur->SetLineColor(RGB(0,0,0),fFalse,3);
	psur->SetObject(this);

	float halflength = m_d.m_length * 0.5f;

	Vertex rgv[2];

	float radangle = m_d.m_rotation / 360 * PI * 2;
	float sn = (float)sin(radangle);
	float cs = (float)cos(radangle);

	rgv[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv[0].y = m_d.m_vCenter.y + sn*halflength;

	rgv[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv[1].y = m_d.m_vCenter.y - sn*halflength;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);

	psur->SetLineColor(RGB(0,0,0),fFalse,1);
	psur->SetObject(this);

	halflength += m_d.m_overhang;

	rgv[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv[0].y = m_d.m_vCenter.y + sn*halflength;

	rgv[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv[1].y = m_d.m_vCenter.y - sn*halflength;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
	}

void Spinner::RenderShadow(ShadowSur *psur, float height)
	{
	if ( (m_d.m_fCastsShadow != fTrue) || (m_ptable->m_fRenderShadows == fFalse) )
		return;

	psur->SetLineColor(RGB(0,0,0),fFalse,4);
	psur->SetObject(this);

	float halflength = m_d.m_length * 0.5f;

	Vertex rgv[2];

	float radangle = m_d.m_rotation / 360 * PI * 2;
	float sn = (float)sin(radangle);
	float cs = (float)cos(radangle);

	rgv[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv[0].y = m_d.m_vCenter.y + sn*halflength;

	rgv[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv[1].y = m_d.m_vCenter.y - sn*halflength;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);

	psur->SetLineColor(RGB(0,0,0),fFalse,1);
	psur->SetObject(this);

	halflength += m_d.m_overhang;

	rgv[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv[0].y = m_d.m_vCenter.y + sn*halflength;

	rgv[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv[1].y = m_d.m_vCenter.y - sn*halflength;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
	}

/*void Spinner::RenderBlueprint(Sur *psur)
	{
	}*/

void Spinner::GetTimers(Vector<HitTimer> *pvht)
	{
	IEditable::BeginPlay();

	HitTimer *pht;
	pht = new HitTimer();
	pht->m_interval = m_d.m_tdr.m_TimerInterval;
	pht->m_nextfire = pht->m_interval;
	pht->m_pfe = (IFireEvents *)this;

	m_phittimer = pht;

	if (m_d.m_tdr.m_fTimerEnabled)
		{
		pvht->AddElement(pht);
		}
	}

void Spinner::GetHitShapes(Vector<HitObject> *pvho)
	{
	HitSpinner *phitspinner;
	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
	float h = m_d.m_height/2 +(float)20;

	phitspinner = new HitSpinner(this, height);
	m_phitspinner = phitspinner;

	pvho->AddElement(phitspinner);

	float halflength = m_d.m_length * 0.5f;
	float radangle = m_d.m_rotation / 360 * PI * 2;
	float sn = (float)sin(radangle);
	float cs = (float)cos(radangle);

	halflength += m_d.m_overhang;

	HitCircle *phitcircle;
	phitcircle = new HitCircle();
	phitcircle->m_pfe = NULL;
	phitcircle->center.x = m_d.m_vCenter.x + cs*halflength;
	phitcircle->center.y = m_d.m_vCenter.y + sn*halflength;
	phitcircle->radius = 0.01f;
	phitcircle->zlow = height;
	phitcircle->zhigh = height+h;
	pvho->AddElement(phitcircle);

	phitcircle = new HitCircle();
	phitcircle->m_pfe = NULL;
	phitcircle->center.x = m_d.m_vCenter.x - cs*halflength;
	phitcircle->center.y = m_d.m_vCenter.y - sn*halflength;
	phitcircle->radius = 0.01f;
	phitcircle->zlow = height;
	phitcircle->zhigh = height+h;
	pvho->AddElement(phitcircle);
	}

void Spinner::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void Spinner::EndPlay()
	{
	IEditable::EndPlay();

	if (m_phitspinner) // Failed Player case
		{
		int i;
		for (i=0;i<m_phitspinner->m_spinneranim.m_vddsFrame.Size();i++)
			{
			delete m_phitspinner->m_spinneranim.m_vddsFrame.ElementAt(i);
			}

		m_phitspinner = NULL;
		}
	}

void Spinner::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	if (!m_d.m_fSupports)
		{
		return;
		}

	Vertex3D rgv3D[12];
	WORD rgi[8];
	int l;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	//pd3dDevice->SetRenderState(D3DRENDERSTATE_LASTPIXEL, FALSE);

	float halflength = (m_d.m_length * 0.5f) + m_d.m_overhang;
	float halfthick = 2;
	float h = m_d.m_height/2  +(float)20;

	float radangle = m_d.m_rotation / 360 * PI * 2;
	float snY = (float)sin(radangle);
	float csY = (float)cos(radangle);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = 0.6f;
	mtrl.diffuse.g = mtrl.ambient.g = 0.6f;
	mtrl.diffuse.b = mtrl.ambient.b = 0.6f;
	pd3dDevice->SetMaterial(&mtrl);

	rgv3D[0].x = -halflength + halfthick;
	rgv3D[0].y = 0;
	rgv3D[0].z = 0;

	rgv3D[1].x = -halflength - halfthick;
	rgv3D[1].y = 0;
	rgv3D[1].z = 0;

	rgv3D[2].x = -halflength + halfthick;
	rgv3D[2].y = 0;
	rgv3D[2].z = h - halfthick;

	rgv3D[3].x = -halflength - halfthick;
	rgv3D[3].y = 0;
	rgv3D[3].z = h + halfthick;

	rgv3D[4].x = halflength - halfthick;
	rgv3D[4].y = 0;
	rgv3D[4].z = 0;

	rgv3D[5].x = halflength + halfthick;
	rgv3D[5].y = 0;
	rgv3D[5].z = 0;

	rgv3D[6].x = halflength - halfthick;
	rgv3D[6].y = 0;
	rgv3D[6].z = h - halfthick;

	rgv3D[7].x = halflength + halfthick;
	rgv3D[7].y = 0;
	rgv3D[7].z = h + halfthick;

	float temp;
	for (l=0;l<8;l++)
		{
		//temp = rgv3D[l].y;
		//rgv3D[l].y = (float)(csTurn*temp + snTurn*rgv3D[l].z);
		//rgv3D[l].z = (float)(csTurn*rgv3D[l].z - snTurn*temp);

		temp = rgv3D[l].x;
		rgv3D[l].x = (float)(csY*temp - snY*rgv3D[l].y);
		rgv3D[l].y = (float)(csY*rgv3D[l].y + snY*temp);

		rgv3D[l].x += m_d.m_vCenter.x;
		rgv3D[l].y += m_d.m_vCenter.y;
		rgv3D[l].z += height;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		//rgv3D[l].z += 60;
		}

	WORD rgiNormal[3];

	rgiNormal[0] = 0;
	rgiNormal[1] = 1;
	rgiNormal[2] = 3;

	rgi[0] = 0;
	rgi[1] = 1;
	rgi[2] = 2;
	rgi[3] = 3;
	rgi[4] = 6;
	rgi[5] = 7;
	rgi[6] = 4;
	rgi[7] = 5;

	SetNormal(rgv3D, rgiNormal, 3, rgv3D, rgi, 8);

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 8, NULL);
	rgi[0] = 4;
	rgi[1] = 5;
	rgi[2] = 6;
	rgi[3] = 7;
	rgi[4] = 2;
	rgi[5] = 3;
	rgi[6] = 0;
	rgi[7] = 1;

	SetNormal(rgv3D, rgiNormal, 3, rgv3D, rgi, 8);

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 8, NULL);

	//pd3dDevice->SetRenderState(D3DRENDERSTATE_LASTPIXEL, TRUE);
	}
	
void Spinner::RenderMoversFromCache(Pin3D *ppin3d)
	{
	ppin3d->ReadAnimObjectFromCacheFile(&m_phitspinner->m_spinneranim, &m_phitspinner->m_spinneranim.m_vddsFrame);
	}

void Spinner::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Pin3D *ppin3d = &g_pplayer->m_pin3d;
	LPDIRECTDRAWSURFACE7 pdds;
	ObjFrame *pof;

	float maxtuback, maxtvback;
	float maxtufront, maxtvfront;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
	float h = m_d.m_height/2 +(float)20;

	PinImage *pinback = m_ptable->GetImage(m_d.m_szImageBack);
	PinImage *pinfront = m_ptable->GetImage(m_d.m_szImageFront);

	if (pinback)
		{
		m_ptable->GetTVTU(pinback, &maxtuback, &maxtvback);
		}
	else
		{
		maxtuback = maxtvback = 1;
		}

	if (pinfront)
		{
		m_ptable->GetTVTU(pinfront, &maxtufront, &maxtvfront);
		}
	else
		{
		maxtufront = maxtvfront = 1;
		}

	int cframes = 40;

	float halflength = m_d.m_length * 0.5f;

	float halfwidth = m_d.m_height/2;

	int i,l;

	float angle;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );

	float r = (m_d.m_color & 255) / 255.0f;
	float g = (m_d.m_color & 65280) / 65280.0f;
	float b = (m_d.m_color & 16711680) / 16711680.0f;

	//pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
	/*pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
    pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
	pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_NONE);*/

	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x80);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

	// Set texture to mirror, so the alpha state of the texture blends correctly to the outside
	pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_MIRROR);

	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	ppin3d->ClearExtents(&m_phitspinner->m_spinneranim.m_rcBounds, &m_phitspinner->m_spinneranim.m_znear, &m_phitspinner->m_spinneranim.m_zfar);

	for (i=0;i<cframes;i++)
		{
		//pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET ,
					   //0x00000000, 1.0f, 0L );

		pof = new ObjFrame();

		angle = (2*PI*i)/(float)cframes;

		//SetVertices(anglerad + ((anglerad2 - anglerad)*i/cframes), &vendcenter, rgv);

		//ppin3d->m_pddsBackBuffer->Blt(NULL, ppin3d->m_pddsStatic, NULL, 0, NULL);
		//ppin3d->m_pddsZBuffer->Blt(NULL, ppin3d->m_pddsStaticZ, NULL, 0, NULL);

		Vertex3D rgv3D[8];
		WORD rgi[4];

		float radangle = m_d.m_rotation / 360 * PI * 2;
		float snY = (float)sin(radangle);
		float csY = (float)cos(radangle);

		float snTurn = (float)sin(angle);
		float csTurn = (float)cos(angle);

		float minx = -halflength;
		float maxx = halflength;
		float miny = -3;
		float maxy = 3;
		float minz = -halfwidth;
		float maxz = halfwidth;

		float temp;

		for (l=0;l<8;l++)
			{
			rgv3D[l].x = (l & 1) ? maxx : minx;
			rgv3D[l].y = (l & 2) ? maxy : miny;
			rgv3D[l].z = (l & 4) ? maxz : minz;

			if (l & 2)
				{
				rgv3D[l].tu = (l & 1) ? maxtufront : 0;
				rgv3D[l].tv = (l & 4) ? 0 : maxtvfront;
				}
			else
				{
				rgv3D[l].tu = (l & 1) ? maxtuback : 0;
				rgv3D[l].tv = (l & 4) ? maxtvback : 0;
				}
			}

		for (l=0;l<8;l++)
			{
			temp = rgv3D[l].y;
			rgv3D[l].y = (float)(csTurn*temp + snTurn*rgv3D[l].z);
			rgv3D[l].z = (float)(csTurn*rgv3D[l].z - snTurn*temp);

			temp = rgv3D[l].x;
			rgv3D[l].x = (float)(csY*temp - snY*rgv3D[l].y);
			rgv3D[l].y = (float)(csY*rgv3D[l].y + snY*temp);

			rgv3D[l].x += m_d.m_vCenter.x;
			rgv3D[l].y += m_d.m_vCenter.y;
			rgv3D[l].z += h + height;

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
			}

		/*float halfXYwidth = snTurn * halfwidth;

		rgv3D[0].x = m_d.m_vCenter.x + csY*halflength + snY*halfXYwidth;
		rgv3D[0].y = m_d.m_vCenter.y + snY*halflength + csY*halfXYwidth;
		rgv3D[0].z = 30 - csTurn*halfwidth;

		rgv3D[1].x = m_d.m_vCenter.x - csY*halflength - snY*halfXYwidth;
		rgv3D[1].y = m_d.m_vCenter.y - snY*halflength + csY*halfXYwidth;
		rgv3D[1].z = 30 - csTurn*halfwidth;

		rgv3D[2].x = m_d.m_vCenter.x + csY*halflength + snY*halfXYwidth;
		rgv3D[2].y = m_d.m_vCenter.y + snY*halflength - csY*halfXYwidth;
		rgv3D[2].z = 30 + csTurn*halfwidth;

		rgv3D[3].x = m_d.m_vCenter.x - csY*halflength - snY*halfXYwidth;
		rgv3D[3].y = m_d.m_vCenter.y - snY*halflength - csY*halfXYwidth;
		rgv3D[3].z = 30 + csTurn*halfwidth;

		ppin3d->ClearExtents(&pof->rc);
		ppin3d->ExpandExtents(&pof->rc, rgv3D, 4);*/

		ppin3d->ClearExtents(&pof->rc, NULL, NULL);
		ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitspinner->m_spinneranim.m_znear, &m_phitspinner->m_spinneranim.m_zfar, 8, fFalse);

		// Draw Backside

		if (pinback)
			{
			//pd3dDevice->SetTexture(ePictureTexture, pinback->m_pdsBuffer);
			pinback->EnsureColorKey();
			pd3dDevice->SetTexture(ePictureTexture, pinback->m_pdsBufferColorKey);
			//ppin3d->SetTexture(pinback->m_pdsBuffer);
			mtrl.diffuse.r = mtrl.ambient.r = 1;
			mtrl.diffuse.g = mtrl.ambient.g = 1;
			mtrl.diffuse.b = mtrl.ambient.b = 1;
			}
		else // No image by that name
			{
			//pd3dDevice->SetTexture(ePictureTexture, NULL);
			ppin3d->SetTexture(NULL);
			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			}

		pd3dDevice->SetMaterial(&mtrl);

		rgi[0] = 0;
		rgi[1] = 1;
		rgi[2] = 5;
		rgi[3] = 4;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);

		// Draw Frontside

		if (pinfront)
			{
			pinfront->EnsureColorKey();
			pd3dDevice->SetTexture(ePictureTexture, pinfront->m_pdsBufferColorKey);
			//pd3dDevice->SetTexture(ePictureTexture, pinfront->m_pdsBuffer);
			//ppin3d->SetTexture(pinfront->m_pdsBuffer);
			mtrl.diffuse.r = mtrl.ambient.r = 1;
			mtrl.diffuse.g = mtrl.ambient.g = 1;
			mtrl.diffuse.b = mtrl.ambient.b = 1;
			}
		else // No image by that name
			{
			//pd3dDevice->SetTexture(ePictureTexture, NULL);
			ppin3d->SetTexture(NULL);
			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			}

		pd3dDevice->SetMaterial(&mtrl);

		rgi[0] = 2;
		rgi[1] = 6;
		rgi[2] = 7;
		rgi[3] = 3;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);

		mtrl.diffuse.r = mtrl.ambient.r = r;
		mtrl.diffuse.g = mtrl.ambient.g = g;
		mtrl.diffuse.b = mtrl.ambient.b = b;
		pd3dDevice->SetMaterial(&mtrl);
		//pd3dDevice->SetTexture(ePictureTexture, NULL);
		ppin3d->SetTexture(NULL);

		if (m_d.m_color != RGB(0,0,0))
			{
			// Top & Bottom

			rgi[0] = 0;
			rgi[1] = 2;
			rgi[2] = 3;
			rgi[3] = 1;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 8,
														  rgi, 4, NULL);

			rgi[0] = 4;
			rgi[1] = 5;
			rgi[2] = 7;
			rgi[3] = 6;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 8,
														  rgi, 4, NULL);

			// Sides

			rgi[0] = 0;
			rgi[1] = 4;
			rgi[2] = 6;
			rgi[3] = 2;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 8,
														  rgi, 4, NULL);

			rgi[0] = 1;
			rgi[1] = 3;
			rgi[2] = 7;
			rgi[3] = 5;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 8,
														  rgi, 4, NULL);
			}

		pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
		pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);

		pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &pof->rc, 0, NULL);
		//HRESULT hr = pof->pddsZBuffer->Blt(NULL, ppin3d->m_pddsZBuffer, &pof->rc, 0, NULL);
		HRESULT hr = pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY);
		
		//pdds->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbfx);
		m_phitspinner->m_spinneranim.m_vddsFrame.AddElement(pof);
		pof->pdds = pdds;

		ppin3d->ExpandRectByRect(&m_phitspinner->m_spinneranim.m_rcBounds, &pof->rc);

		// reset the portion of the z-buffer that we changed
		ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY);
		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,
				&pof->rc, DDBLT_COLORFILL, &ddbltfx);
		}
		
	ppin3d->WriteAnimObjectToCacheFile(&m_phitspinner->m_spinneranim, &m_phitspinner->m_spinneranim.m_vddsFrame);

	pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	/*pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
    pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_LINEAR);*/

	pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
	}

void Spinner::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
	}

void Spinner::MoveOffset(float dx, float dy)
	{
	m_d.m_vCenter.x += dx;
	m_d.m_vCenter.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Spinner::GetCenter(Vertex *pv)
	{
	pv->x = m_d.m_vCenter.x;
	pv->y = m_d.m_vCenter.y;
	}

void Spinner::PutCenter(Vertex *pv)
	{
	m_d.m_vCenter.x = pv->x;
	m_d.m_vCenter.y = pv->y;

	m_ptable->SetDirtyDraw();
	}

HRESULT Spinner::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex));
	bw.WriteFloat(FID(ROTA), m_d.m_rotation);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteFloat(FID(HIGH), m_d.m_height);
	bw.WriteFloat(FID(LGTH), m_d.m_length);
	bw.WriteFloat(FID(AFRC), m_d.m_antifriction);
	bw.WriteFloat(FID(OVRH), m_d.m_overhang);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);	//<<< added by Chris
	bw.WriteString(FID(IMGF), m_d.m_szImageFront);
	bw.WriteString(FID(IMGB), m_d.m_szImageBack);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;

	/*ULONG writ = 0;
	HRESULT hr = S_OK;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_d, sizeof(SpinnerData), &writ)))
		return hr;

	return hr;*/
	}

HRESULT Spinner::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults();
#ifndef OLDLOAD
	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
#else
	ULONG read = 0;
	HRESULT hr = S_OK;

	m_ptable = ptable;

	DWORD dwID;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_d, sizeof(SpinnerData), &read)))
		return hr;

	*pid = dwID;

	return hr;
#endif
	}

BOOL Spinner::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VCEN))
		{
		pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex));
		}
	else if (id == FID(ROTA))
		{
		pbr->GetFloat(&m_d.m_rotation);
		}
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_d.m_color);
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(HIGH))
		{
		pbr->GetFloat(&m_d.m_height);
		}
	else if (id == FID(LGTH))
		{
		pbr->GetFloat(&m_d.m_length);
		}
	else if (id == FID(AFRC))
		{
		pbr->GetFloat(&m_d.m_antifriction);
		}
	else if (id == FID(OVRH))
		{
		pbr->GetFloat(&m_d.m_overhang);
		}
	else if (id == FID(IMGF))
		{
		pbr->GetString(m_d.m_szImageFront);
		}
	else if (id == FID(IMGB))
		{
		pbr->GetString(m_d.m_szImageBack);
		}
	else if (id == FID(SURF))
		{
		pbr->GetString(m_d.m_szSurface);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(CSHD))
		{
		pbr->GetBool(&m_d.m_fCastsShadow);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Spinner::InitPostLoad()
	{
	return S_OK;
	}

STDMETHODIMP Spinner::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_ISpinner,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP Spinner::get_Length(float *pVal)
{
	*pVal = m_d.m_length;

	return S_OK;
}

STDMETHODIMP Spinner::put_Length(float newVal)
{
	STARTUNDO

	m_d.m_length = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_Rotation(float *pVal)
{
	*pVal = m_d.m_rotation;

	return S_OK;
}

STDMETHODIMP Spinner::put_Rotation(float newVal)
{
	STARTUNDO

	m_d.m_rotation = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_Height(float *pVal)
{
	*pVal = m_d.m_height;

	return S_OK;
}

STDMETHODIMP Spinner::put_Height(float newVal)
{
	STARTUNDO

	m_d.m_height = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_Overhang(float *pVal)
{
	*pVal = m_d.m_overhang;

	return S_OK;
}

STDMETHODIMP Spinner::put_Overhang(float newVal)
{
	STARTUNDO

	m_d.m_overhang = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_Friction(float *pVal)
{
	*pVal = (float)((1-(double)m_d.m_antifriction)*100);

	return S_OK;
}

STDMETHODIMP Spinner::put_Friction(float newVal)
{
	STARTUNDO

	m_d.m_antifriction = 1-(newVal/100);

	if (m_d.m_antifriction < 0)
		{
		m_d.m_antifriction = 0;
		}
	else if (m_d.m_antifriction > 1)
		{
		m_d.m_antifriction = 1;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_Color(OLE_COLOR *pVal)
{
	*pVal = m_d.m_color;

	return S_OK;
}

STDMETHODIMP Spinner::put_Color(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_color = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_ImageFront(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageFront, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Spinner::put_ImageFront(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImageFront, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_ImageBack(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageBack, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Spinner::put_ImageBack(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImageBack, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_X(float *pVal)
{
	*pVal = m_d.m_vCenter.x;

	return S_OK;
}

STDMETHODIMP Spinner::put_X(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_Y(float *pVal)
{
	*pVal = m_d.m_vCenter.y;

	return S_OK;
}

STDMETHODIMP Spinner::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Spinner::get_Surface(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Spinner::put_Surface(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

//>>> added by Chris
STDMETHODIMP Spinner::get_CastsShadow(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fCastsShadow);

	return S_OK;
}

STDMETHODIMP Spinner::put_CastsShadow(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_d.m_fCastsShadow = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}
//<<<



/*HRESULT Spinner::GetTypeName(BSTR *pVal)
	{
	*pVal = SysAllocString(L"Spinner");

	return S_OK;
	}*/

/*int Spinner::GetDialogID()
	{
	return IDD_PROPSPINNER;
	}*/

void Spinner::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPSPINNER_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPSPINNER_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPSPINNER_PHYSICS, IDS_PHYSICS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}
