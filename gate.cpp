// Gate.cpp : Implementation of CVBATestApp and DLL registration.

#include "stdafx.h"
#include "VBATest.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////
//

Gate::Gate()
	{
	m_phitgate = NULL;
	m_plineseg = NULL;
	}

Gate::~Gate()
	{
	}

HRESULT Gate::Init(PinTable *ptable, float x, float y)
	{
	m_ptable = ptable;

	m_d.m_vCenter.x = x;
	m_d.m_vCenter.y = y;

	SetDefaults();

	return InitVBA(fTrue, 0, NULL);
	}

void Gate::SetDefaults()
	{
	m_d.m_length = 100;
	m_d.m_rotation = -90;
	m_d.m_color = RGB(128,128,128);

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_szSurface[0] = 0;

	m_d.m_elasticity = 0.3f;
	}

void Gate::PreRender(Sur *psur)
	{
	/*psur->SetBorderColor(-1,fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);*/
	}

void Gate::Render(Sur *psur)
	{
	psur->SetLineColor(RGB(0,0,0),fFalse,2);
	psur->SetObject(this);

	float halflength = m_d.m_length * 0.5f;
	float arrowang;

	Vertex rgv[2];

	float radangle = m_d.m_rotation / 360 * PI * 2;
	float sn = (float)sin(radangle);
	float cs = (float)cos(radangle);

	rgv[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv[0].y = m_d.m_vCenter.y + sn*halflength;

	rgv[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv[1].y = m_d.m_vCenter.y - sn*halflength;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);

	// Draw Arrow

	psur->SetLineColor(RGB(0,0,0),fFalse,1);

	halflength *= 0.5;

	rgv[0].x = m_d.m_vCenter.x + sn*halflength;
	rgv[0].y = m_d.m_vCenter.y - cs*halflength;

	rgv[1].x = m_d.m_vCenter.x;
	rgv[1].y = m_d.m_vCenter.y;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);

	halflength *= 0.5;

	arrowang = radangle+0.6f;
	sn = (float)sin(arrowang);
	cs = (float)cos(arrowang);

	rgv[1].x = m_d.m_vCenter.x + sn*halflength;
	rgv[1].y = m_d.m_vCenter.y - cs*halflength;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);

	arrowang = radangle-0.6f;
	sn = (float)sin(arrowang);
	cs = (float)cos(arrowang);

	rgv[1].x = m_d.m_vCenter.x + sn*halflength;
	rgv[1].y = m_d.m_vCenter.y - cs*halflength;

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
	}

void Gate::RenderBlueprint(Sur *psur)
	{
	/*psur->SetFillColor(-1);
	psur->SetBorderColor(RGB(0,0,0), fFalse, 0);
	psur->SetObject(this);

	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);*/
	}

void Gate::GetTimers(Vector<HitTimer> *pvht)
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

void Gate::GetHitShapes(Vector<HitObject> *pvho)
	{
	//LineSeg *plineseg;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	float halflength = m_d.m_length * 0.5f;
	//float halfheight = m_d.m_length * 0.5f;

	Vertex rgv[2];

	float radangle = m_d.m_rotation / 360 * PI * 2;
	float sn = (float)sin(radangle);
	float cs = (float)cos(radangle);

	rgv[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv[0].y = m_d.m_vCenter.y + sn*halflength;

	rgv[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv[1].y = m_d.m_vCenter.y - sn*halflength;

	m_plineseg = new LineSeg();

	m_plineseg->m_pfe = NULL;

	m_plineseg->m_rcHitRect.zlow = height;
	m_plineseg->m_rcHitRect.zhigh = height+50;

	m_plineseg->v1.x = rgv[0].x;
	m_plineseg->v1.y = rgv[0].y;

	m_plineseg->v2.x = rgv[1].x;
	m_plineseg->v2.y = rgv[1].y;

	m_plineseg->CalcNormal();

	m_plineseg->m_elasticity = m_d.m_elasticity;

	m_phitgate = new HitGate(this);

	m_phitgate->m_pfe = (IFireEvents *)this;

	m_phitgate->m_rcHitRect.zlow = height;
	m_phitgate->m_rcHitRect.zhigh = height+50;

	m_phitgate->v1.x = rgv[1].x;
	m_phitgate->v1.y = rgv[1].y;

	m_phitgate->v2.x = rgv[0].x;
	m_phitgate->v2.y = rgv[0].y;

	m_phitgate->CalcNormal();

	m_phitgate->m_elasticity = m_d.m_elasticity;

	pvho->AddElement(m_plineseg);
	pvho->AddElement(m_phitgate);

	HitCircle *phitcircle;
	phitcircle = new HitCircle();
	phitcircle->m_pfe = NULL;
	phitcircle->center.x = m_d.m_vCenter.x + cs*halflength;
	phitcircle->center.y = m_d.m_vCenter.y + sn*halflength;
	phitcircle->radius = 0.01f;
	phitcircle->zlow = height;
	phitcircle->zhigh = height+50;
	pvho->AddElement(phitcircle);

	phitcircle = new HitCircle();
	phitcircle->m_pfe = NULL;
	phitcircle->center.x = m_d.m_vCenter.x - cs*halflength;
	phitcircle->center.y = m_d.m_vCenter.y - sn*halflength;
	phitcircle->radius = 0.01f;
	phitcircle->zlow = height;
	phitcircle->zhigh = height+50;
	pvho->AddElement(phitcircle);
	}

void Gate::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void Gate::EndPlay()
	{
	IEditable::EndPlay();

	if (m_phitgate) // Failed Player case
		{
		int i;
		for (i=0;i<m_phitgate->m_gateanim.m_vddsFrame.Size();i++)
			{
			//m_phitflipper->m_vddsFrame.ElementAt(i)->ppds->Release();
			delete m_phitgate->m_gateanim.m_vddsFrame.ElementAt(i);
			}

		m_phitgate = NULL;
		}

	m_plineseg = NULL;
	}

void Gate::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Vertex3D rgv3D[12];
	WORD rgi[8];
	int l;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	//pd3dDevice->SetRenderState(D3DRENDERSTATE_LASTPIXEL, FALSE);

	float halflength = (m_d.m_length * 0.5f);// + m_d.m_overhang;
	float halfthick = 2;

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
	rgv3D[2].z = 50 - halfthick;

	rgv3D[3].x = -halflength - halfthick;
	rgv3D[3].y = 0;
	rgv3D[3].z = 50 + halfthick;

	rgv3D[4].x = halflength - halfthick;
	rgv3D[4].y = 0;
	rgv3D[4].z = 0;

	rgv3D[5].x = halflength + halfthick;
	rgv3D[5].y = 0;
	rgv3D[5].z = 0;

	rgv3D[6].x = halflength - halfthick;
	rgv3D[6].y = 0;
	rgv3D[6].z = 50 - halfthick;

	rgv3D[7].x = halflength + halfthick;
	rgv3D[7].y = 0;
	rgv3D[7].z = 50 + halfthick;

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
	/*float halflength = m_d.m_length * 0.5f;

	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = 1;
	mtrl.diffuse.g = mtrl.ambient.g = 1;
	mtrl.diffuse.b = mtrl.ambient.b = 1;

	pd3dDevice->SetMaterial(&mtrl);

	Vertex3D rgv3D[4];
	WORD rgi[4];

	float radangle = m_d.m_rotation / 360 * PI * 2;
	float sn = (float)sin(radangle);
	float cs = (float)cos(radangle);

	rgv3D[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv3D[0].y = m_d.m_vCenter.y + sn*halflength;
	rgv3D[0].z = 0;

	rgv3D[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv3D[1].y = m_d.m_vCenter.y - sn*halflength;
	rgv3D[1].z = 0;

	rgv3D[2].x = m_d.m_vCenter.x + cs*halflength;
	rgv3D[2].y = m_d.m_vCenter.y + sn*halflength;
	rgv3D[2].z = 50;

	rgv3D[3].x = m_d.m_vCenter.x - cs*halflength;
	rgv3D[3].y = m_d.m_vCenter.y - sn*halflength;
	rgv3D[3].z = 50;

	ppin3d->m_lightproject.CalcCoordinates(&rgv3D[0]);
	ppin3d->m_lightproject.CalcCoordinates(&rgv3D[1]);
	ppin3d->m_lightproject.CalcCoordinates(&rgv3D[2]);
	ppin3d->m_lightproject.CalcCoordinates(&rgv3D[3]);

	// Blocked path
	rgi[0] = 0;
	rgi[1] = 2;
	rgi[2] = 3;
	rgi[3] = 1;

	SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
											  rgv3D, 4,
											  rgi, 4, NULL);

	// Open Path
	rgi[0] = 0;
	rgi[1] = 1;
	rgi[2] = 3;
	rgi[3] = 2;

	SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
											  rgv3D, 4,
											  rgi, 4, NULL);*/
	}
	
void Gate::RenderMoversFromCache(Pin3D *ppin3d)
	{
	ppin3d->ReadAnimObjectFromCacheFile(&m_phitgate->m_gateanim, &m_phitgate->m_gateanim.m_vddsFrame);
	}

void Gate::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Pin3D *ppin3d = &g_pplayer->m_pin3d;
	LPDIRECTDRAWSURFACE7 pdds;
	ObjFrame *pof;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	int cframes = 10;

	float halflength = m_d.m_length * 0.5f;

	float halfwidth = 50;

	int i,l;

	float angle;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );

	float r = (m_d.m_color & 255) / 255.0f;
	float g = (m_d.m_color & 65280) / 65280.0f;
	float b = (m_d.m_color & 16711680) / 16711680.0f;

	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFG_LINEAR);

	pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

	ppin3d->ClearExtents(&m_phitgate->m_gateanim.m_rcBounds, &m_phitgate->m_gateanim.m_znear, &m_phitgate->m_gateanim.m_zfar);

	for (i=0;i<cframes;i++)
		{
		//pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET ,
					   //0x00000000, 1.0f, 0L );

		pof = new ObjFrame();

		angle = (0.5f*PI*i)/(float)(cframes-1);

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
		float miny = -1;
		float maxy = 1;
		float minz = -halfwidth;
		float maxz = 0;

		float temp;

		for (l=0;l<8;l++)
			{
			rgv3D[l].x = (l & 1) ? maxx : minx;
			rgv3D[l].y = (l & 2) ? maxy : miny;
			rgv3D[l].z = (l & 4) ? maxz : minz;
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
			rgv3D[l].z += height + 50;

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
			}

		ppin3d->ClearExtents(&pof->rc, NULL, NULL);
		ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitgate->m_gateanim.m_znear, &m_phitgate->m_gateanim.m_zfar, 8, fFalse);

		// Draw Backside

		//ppin3d->SetTexture(NULL);
		mtrl.diffuse.r = mtrl.ambient.r = r;
		mtrl.diffuse.g = mtrl.ambient.g = g;
		mtrl.diffuse.b = mtrl.ambient.b = b;

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

		//ppin3d->SetTexture(NULL);
		mtrl.diffuse.r = mtrl.ambient.r = r;
		mtrl.diffuse.g = mtrl.ambient.g = g;
		mtrl.diffuse.b = mtrl.ambient.b = b;

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
		//ppin3d->SetTexture(NULL);

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

		pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
		pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);

		pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &pof->rc, 0, NULL);
		//HRESULT hr = pof->pddsZBuffer->Blt(NULL, ppin3d->m_pddsZBuffer, &pof->rc, 0, NULL);
		HRESULT hr = pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY);
		
		//pdds->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbfx);
		m_phitgate->m_gateanim.m_vddsFrame.AddElement(pof);
		pof->pdds = pdds;

		ppin3d->ExpandRectByRect(&m_phitgate->m_gateanim.m_rcBounds, &pof->rc);

		// reset the portion of the z-buffer that we changed
		ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY);
		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,
				&pof->rc, DDBLT_COLORFILL, &ddbltfx);
		}
		
	ppin3d->WriteAnimObjectToCacheFile(&m_phitgate->m_gateanim, &m_phitgate->m_gateanim.m_vddsFrame);
	}

void Gate::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
	}

void Gate::MoveOffset(float dx, float dy)
	{
	m_d.m_vCenter.x += dx;
	m_d.m_vCenter.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Gate::GetCenter(Vertex *pv)
	{
	pv->x = m_d.m_vCenter.x;
	pv->y = m_d.m_vCenter.y;
	}

void Gate::PutCenter(Vertex *pv)
	{
	m_d.m_vCenter.x = pv->x;
	m_d.m_vCenter.y = pv->y;

	m_ptable->SetDirtyDraw();
	}

HRESULT Gate::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex));
	bw.WriteFloat(FID(LGTH), m_d.m_length);
	bw.WriteFloat(FID(ROTA), m_d.m_rotation);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	/*ULONG writ = 0;
	HRESULT hr = S_OK;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_d, sizeof(GateData), &writ)))
		return hr;

	return hr;*/
	}

HRESULT Gate::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
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

	if(FAILED(hr = pstm->Read(&m_d, sizeof(GateData), &read)))
		return hr;

	//ApcProjectItem.Register(ptable->ApcProject, GetDispatch(), dwID);
	*pid = dwID;

	return hr;
#endif
	}

BOOL Gate::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VCEN))
		{
		pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex));
		}
	else if (id == FID(LGTH))
		{
		pbr->GetFloat(&m_d.m_length);
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
	else if (id == FID(SURF))
		{
		pbr->GetString(m_d.m_szSurface);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(ELAS))
		{
		pbr->GetFloat(&m_d.m_elasticity);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Gate::InitPostLoad()
	{
	return S_OK;
	}

STDMETHODIMP Gate::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_IGate,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP Gate::get_Length(float *pVal)
{
	*pVal = m_d.m_length;

	return S_OK;
}

STDMETHODIMP Gate::put_Length(float newVal)
{
	STARTUNDO

	m_d.m_length = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Gate::get_Rotation(float *pVal)
{
	*pVal = m_d.m_rotation;

	return S_OK;
}

STDMETHODIMP Gate::put_Rotation(float newVal)
{
	STARTUNDO

	m_d.m_rotation = newVal;

	STOPUNDO;

	return S_OK;
}

STDMETHODIMP Gate::get_X(float *pVal)
{
	*pVal = m_d.m_vCenter.x;

	return S_OK;
}

STDMETHODIMP Gate::put_X(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Gate::get_Y(float *pVal)
{
	*pVal = m_d.m_vCenter.y;

	return S_OK;
}

STDMETHODIMP Gate::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Gate::get_Surface(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Gate::put_Surface(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

//>>> added by chris
STDMETHODIMP Gate::get_Color(OLE_COLOR *pVal)
{
	*pVal = m_d.m_color;

	return S_OK;
}

STDMETHODIMP Gate::put_Color(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_color = newVal;

	STOPUNDO

	return S_OK;
}
//<<<

/*HRESULT Gate::GetTypeName(BSTR *pVal)
	{
	*pVal = SysAllocString(L"Gate");

	return S_OK;
	}*/

/*int Gate::GetDialogID()
	{
	return IDD_PROPGATE;
	}*/

void Gate::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPGATE_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPGATE_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPGATE_PHYSICS, IDS_PHYSICS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

STDMETHODIMP Gate::get_Open(VARIANT_BOOL *pVal)
{
	if (m_phitgate)
		{
		*pVal = FTOVB(m_phitgate->m_gateanim.m_fOpen);
		}
	else
		{
		*pVal = FTOVB(fFalse);
		}

	return S_OK;
}

STDMETHODIMP Gate::put_Open(VARIANT_BOOL newVal)
{
	if (m_phitgate)
		{
		if (newVal)
			{
			m_phitgate->m_gateanim.m_fOpen = fTrue;
			m_phitgate->m_fEnabled = fFalse;
			m_plineseg->m_fEnabled = fFalse;
			m_phitgate->m_gateanim.m_anglespeed = 0.2f;
			}
		else
			{
			m_phitgate->m_gateanim.m_fOpen = fFalse;
			m_phitgate->m_fEnabled = fTrue;
			m_plineseg->m_fEnabled = fTrue;
			}
		}

	return S_OK;
}

STDMETHODIMP Gate::get_Elasticity(float *pVal)
{
	*pVal = m_d.m_elasticity;

	return S_OK;
}

STDMETHODIMP Gate::put_Elasticity(float newVal)
{
	STARTUNDO

	m_d.m_elasticity = newVal;

	STOPUNDO

	return S_OK;
}
