// Light.cpp : Implementation of CVBATestApp and DLL registration.

#include "stdafx.h"
//#include "VBATest.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////
//

LightCenter::LightCenter(Light *plight)
	{
	m_plight = plight;
	}
HRESULT LightCenter::GetTypeName(BSTR *pVal) {return m_plight->GetTypeName(pVal);}
IDispatch *LightCenter::GetDispatch() {return m_plight->GetDispatch();}
//int LightCenter::GetDialogID() {return m_plight->GetDialogID();}
void LightCenter::GetDialogPanes(Vector<PropertyPane> *pvproppane) {m_plight->GetDialogPanes(pvproppane);}
void LightCenter::Delete() {m_plight->Delete();}
void LightCenter::Uncreate() {m_plight->Uncreate();}
IEditable *LightCenter::GetIEditable() {return (IEditable *)m_plight;}
PinTable *LightCenter::GetPTable() {return m_plight->GetPTable();}

void LightCenter::GetCenter(Vertex *pv)
	{
	*pv = m_plight->m_d.m_vCenter;
	}

void LightCenter::PutCenter(Vertex *pv)
	{
	m_plight->m_d.m_vCenter = *pv;
	}

void LightCenter::MoveOffset(float dx, float dy)
	{
	m_plight->m_d.m_vCenter.x += dx;
	m_plight->m_d.m_vCenter.y += dy;

	GetPTable()->SetDirtyDraw();
	}

int LightCenter::GetSelectLevel()
	 {
	 if (m_plight->m_d.m_shape == ShapeCircle)
		{
		return 1;
		}
	else
		{
		return 2; // Don't select light bulb twice if we have drag points
		}
	}

Light::Light() : m_lightcenter(this)
	{
	m_menuid = IDR_SURFACEMENU;
	}

Light::~Light()
	{
	}

HRESULT Light::Init(PinTable *ptable, float x, float y)
	{
	m_ptable = ptable;

	m_pobjframe[0] = NULL;
	m_pobjframe[1] = NULL;

	m_d.m_vCenter.x = x;
	m_d.m_vCenter.y = y;

	SetDefaults();

	m_fLockedByLS = fFalse;			//>>> added by chris
	m_realState	= m_d.m_state;		//>>> added by chris

	return InitVBA(fTrue, 0, NULL);
	}

void Light::SetDefaults()
	{
	m_d.m_radius = 50;
	m_d.m_state = LightStateOff;
	m_d.m_shape = ShapeCircle;

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_color = RGB(255,255,0);

	strcpy(m_rgblinkpattern, "10");
	m_blinkinterval = 125;
	m_d.m_borderwidth = 0;
	m_d.m_bordercolor = RGB(0,0,0);

	m_d.m_szSurface[0] = 0;
	}

void Light::PreRender(Sur *psur)
	{
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetFillColor(m_d.m_color);
	psur->SetObject(this);

	switch (m_d.m_shape)
		{
		case ShapeCircle:
		default:
			if (m_d.m_borderwidth > 0)
				{
				psur->SetBorderColor(m_d.m_bordercolor, fFalse, 0); // For off-by-one GDI outline error
				psur->SetFillColor(m_d.m_bordercolor);
				psur->SetObject(this);
				psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius + m_d.m_borderwidth);
				}
			psur->SetBorderColor(m_d.m_color, fFalse, 0); // For off-by-one GDI outline error
			psur->SetFillColor(m_d.m_color);
			psur->SetObject(m_d.m_borderwidth > 0 ? NULL :this);
			psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
			break;

		case ShapeCustom:
			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			int cvertex;
			Vertex *rgv;

			cvertex = vvertex.Size();
			rgv = new Vertex[cvertex];

			int i;
			for (i=0;i<vvertex.Size();i++)
				{
				rgv[i] = *((Vertex *)vvertex.ElementAt(i));
				delete vvertex.ElementAt(i);
				}

			psur->Polygon(rgv, cvertex);

			delete rgv;
			break;
		}
	}

void Light::Render(Sur *psur)
	{
	int 	i;
	BOOL	fDrawDragpoints;		//>>> added by chris

	// if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
	if ( (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints == fTrue) )
		{
		fDrawDragpoints = fTrue;
		}
	else
		{
		// if any of the dragpoints of this object are selected then draw all the dragpoints
		fDrawDragpoints = fFalse;
		for (i=0;i<m_vdpoint.Size();i++)
			{
			CComObject<DragPoint> *pdp;
			pdp = m_vdpoint.ElementAt(i);
			if (pdp->m_selectstate != eNotSelected)
				{
				fDrawDragpoints = fTrue;
				break;
				}
			}
		}

	RenderOutline(psur);

	if ( (m_d.m_shape == ShapeCustom) && (fDrawDragpoints == fTrue) )	//<<< modified by chris
		{
		for (i=0;i<m_vdpoint.Size();i++)
			{
			CComObject<DragPoint> *pdp;
			pdp = m_vdpoint.ElementAt(i);
			psur->SetFillColor(-1);
			psur->SetBorderColor(RGB(0,0,200),fFalse,0);
			psur->SetObject(pdp);

			if (pdp->m_fDragging)
				{
				psur->SetBorderColor(RGB(0,255,0),fFalse,0);
				}

			psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
			}
		}

	/*else if ( (m_d.m_shape == ShapeCircle) && (g_pvp->m_fAlwaysDrawLightCenters == fTrue) )
	{
		psur->Line(m_d.m_vCenter.x - 3, m_d.m_vCenter.y, m_d.m_vCenter.x + 3, m_d.m_vCenter.y);
		psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 3, m_d.m_vCenter.x, m_d.m_vCenter.y + 3);
	}*/
	}

void Light::RenderOutline(Sur *psur)
	{
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetLineColor(RGB(0,0,0),fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);
	psur->SetObject(NULL);

	switch (m_d.m_shape)
		{
		case ShapeCircle:
		default:
			psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius + m_d.m_borderwidth);
			break;

		case ShapeCustom:
			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			int cvertex;
			Vertex *rgv;

			cvertex = vvertex.Size();
			rgv = new Vertex[cvertex];

			int i;
			for (i=0;i<vvertex.Size();i++)
				{
				rgv[i] = *((Vertex *)vvertex.ElementAt(i));
				delete vvertex.ElementAt(i);
				}

			psur->Polygon(rgv, cvertex);

			delete rgv;

			psur->SetObject((ISelect *)&m_lightcenter);
			break;
		}

	if (m_d.m_shape == ShapeCustom || g_pvp->m_fAlwaysDrawLightCenters)
		{
		psur->Line(m_d.m_vCenter.x - 10, m_d.m_vCenter.y, m_d.m_vCenter.x + 10, m_d.m_vCenter.y);
		psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10, m_d.m_vCenter.x, m_d.m_vCenter.y + 10);
		}
	}

void Light::RenderBlueprint(Sur *psur)
	{
	RenderOutline(psur);
	}

void Light::GetTimers(Vector<HitTimer> *pvht)
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

void Light::GetHitShapes(Vector<HitObject> *pvho)
	{
	// HACK - should pass pointer to vector in
	if (m_d.m_state == LightStateBlinking)
		{
		g_pplayer->m_vblink.AddElement((IBlink *)this);
		m_timenextblink = g_pplayer->m_timeCur + m_blinkinterval;
		}
		
	m_iblinkframe = 0;
	}

void Light::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	switch (m_d.m_shape)
		{
		case ShapeCircle:
		default:
			{
			HitObject *pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height, m_d.m_radius, 32);
			pvho->AddElement(pho);
			}
			break;

		case ShapeCustom:
			{
			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			int cvertex;
			Vertex3D *rgv3d;

			cvertex = vvertex.Size();
			rgv3d = new Vertex3D[cvertex];

			int i;
			for (i=0;i<vvertex.Size();i++)
				{
				rgv3d[i].x = vvertex.ElementAt(i)->x;
				rgv3d[i].y = vvertex.ElementAt(i)->y;
				rgv3d[i].z = height;
				delete vvertex.ElementAt(i);
				}

			Hit3DPoly *ph3dp = new Hit3DPoly(rgv3d, cvertex);
			pvho->AddElement(ph3dp);

			delete rgv3d;
			}
			break;
		}
	}

void Light::EndPlay()
	{
	int i;

	for (i=0;i<2;i++)
		{
		//m_pobjframe[i]->ppds->Release();
		delete m_pobjframe[i];
		m_pobjframe[i] = NULL;
		}

	// ensure not locked just incase the player exits during a LS sequence
	m_fLockedByLS = fFalse;			//>>> added by chris

	IEditable::EndPlay();
	}

void Light::ClearForOverwrite()
	{
	ClearPointsForOverwrite();
	}

void Light::RenderCustomStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	int i,t;
	//WORD rgi[3];
	Vertex3D rgv3D[3];

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	RenderVertex *rgv;
	int cvertex;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1.0;

	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	cvertex = vvertex.Size();
	rgv = new RenderVertex[cvertex];

	for (i=0;i<cvertex;i++)
		{
		Vertex v1,v2,vmiddle;
		int p1,p2;
		p1 = (i+cvertex-1) % cvertex;
		p2 = (i+1) % cvertex;

		v1 = *vvertex.ElementAt(p1);
		v2 = *vvertex.ElementAt(p2);
		vmiddle = *vvertex.ElementAt(i);

		Vertex v1normal, v2normal;

		// Notice that these values equal the ones in the line
		// equation and could probably be substituted by them.
		v1normal.x = -(v1.y - vmiddle.y);
		v1normal.y = (v1.x - vmiddle.x);
		v2normal.x = -(vmiddle.y - v2.y);
		v2normal.y = (vmiddle.x - v2.x);

		v1normal.Normalize();
		v2normal.Normalize();

		// Find intersection of the two edges meeting this points, but
		// shift those lines outwards along their normals

		PINFLOAT A,B,C,D,E,F;

		// First line
		A = -(vmiddle.y - v1.y);
		B = (vmiddle.x - v1.x);

		// Shift line along the normal
		v1.x -= v1normal.x*m_d.m_borderwidth;
		v1.y -= v1normal.y*m_d.m_borderwidth;

		C = -(A*v1.x + B*v1.y);

		// Second line
		D = -(vmiddle.y - v2.y);
		E = (vmiddle.x - v2.x);

		// Shift line along the normal
		v2.x -= v2normal.x*m_d.m_borderwidth;
		v2.y -= v2normal.y*m_d.m_borderwidth;

		F = -(D*v2.x + E*v2.y);

		PINFLOAT det = (A*E) - (B*D);

		PINFLOAT intersectx, intersecty;

		intersectx=(B*F-E*C)/det;

		intersecty=(C*D-A*F)/det;

		rgv[i].x = (float)intersectx;
		rgv[i].y = (float)intersecty;

		/*v1.Normalize();
		v2.Normalize();
		vnormal.x = -v1.x - v2.x;
		vnormal.y = -v1.y - v2.y;
		vnormal.Normalize();

		rgv[i] = *vvertex.ElementAt(i);
		rgv[i].x += vnormal.x * m_d.m_borderwidth;
		rgv[i].y += vnormal.y * m_d.m_borderwidth;*/
		}

	for (i=0;i<cvertex;i++)
		{
		delete vvertex.ElementAt(i);
		}

	Pin3D *ppin3d = &g_pplayer->m_pin3d;
	float r = (m_d.m_bordercolor & 255) / 255.0f;
	float g = (m_d.m_bordercolor & 65280) / 65280.0f;
	float b = (m_d.m_bordercolor & 16711680) / 16711680.0f;

	//ppin3d->SetTexture(ppin3d->m_pddsLightTexture);

	Vector<void> vpoly;
	Vector<Triangle> vtri;

	for (i=0;i<cvertex;i++)
		{
		vpoly.AddElement((void *)i);
		}

	PolygonToTriangles(rgv, &vpoly, &vtri);

	mtrl.diffuse.r = mtrl.ambient.r = r;///4;
	mtrl.diffuse.g = mtrl.ambient.g = g;///4;
	mtrl.diffuse.b = mtrl.ambient.b = b;///4;

	pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[3];
	rgi[0] = 0;
	rgi[1] = 1;
	rgi[2] = 2;

	if (!m_fBackglass)
		{
		rgv3D[0].nx = 0;
		rgv3D[0].ny = 0;
		rgv3D[0].nz = -1;
		rgv3D[1].nx = 0;
		rgv3D[1].ny = 0;
		rgv3D[1].nz = -1;
		rgv3D[2].nx = 0;
		rgv3D[2].ny = 0;
		rgv3D[2].nz = -1;
		}
	else
		{
		SetDiffuseFromMaterial(rgv3D, 3, &mtrl);
		}

	for (t=0;t<vtri.Size();t++)
		{
		Triangle *ptri = vtri.ElementAt(t);

		int ip[3];

		ip[0] = ptri->a;
		ip[1] = ptri->c;
		ip[2] = ptri->b;

		RenderVertex *pv0 = &rgv[ip[0]];
		RenderVertex *pv1 = &rgv[ip[1]];
		RenderVertex *pv2 = &rgv[ip[2]];

		rgv3D[0].Set(pv0->x,pv0->y,height + 0.05f);
		rgv3D[1].Set(pv1->x,pv1->y,height + 0.05f);
		rgv3D[2].Set(pv2->x,pv2->y,height + 0.05f);

		if (!m_fBackglass)
			{
			int l;
			for (l=0;l<3;l++)
				{
				ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
				}

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 3,
													  rgi, 3, NULL);
			}
		else
			{
			SetHUDVertices(rgv3D, 3);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
													  rgv3D, 3,
													  rgi, 3, NULL);
			}


		}

	for (i=0;i<vtri.Size();i++)
		{
		delete vtri.ElementAt(i);
		}

	delete rgv;
	}

void Light::RenderStaticCircle(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	int l;
	WORD rgi[32];
	Vertex3D rgv3D[32];

	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	float r = (m_d.m_bordercolor & 255) / 255.0f;
	float g = (m_d.m_bordercolor & 65280) / 65280.0f;
	float b = (m_d.m_bordercolor & 16711680) / 16711680.0f;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = r;
	mtrl.diffuse.g = mtrl.ambient.g = g;
	mtrl.diffuse.b = mtrl.ambient.b = b;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0;
	pd3dDevice->SetMaterial(&mtrl);

	for (l=0;l<32;l++)
		{
		float angle = PI*2;
		angle /= 32;
		angle *= l;
		rgv3D[l].x = (float)sin(angle)*(m_d.m_radius + m_d.m_borderwidth) + m_d.m_vCenter.x;
		rgv3D[l].y = (float)-cos(angle)*(m_d.m_radius + m_d.m_borderwidth) + m_d.m_vCenter.y;
		rgv3D[l].z = height + 0.05f;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		}

	for (l=0;l<32;l++)
		{
		rgi[l] = l;
		}

	if (!m_fBackglass)
		{
		SetNormal(rgv3D, rgi, 32, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 32, NULL);
		}
	else
		{
		SetHUDVertices(rgv3D, 32);
		SetDiffuseFromMaterial(rgv3D, 32, &mtrl);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
												  rgv3D, 32,
												  rgi, 32, NULL);
		}

	/*for (l=1;l<15;l++)
		{
		rgi[0] = 0;
		rgi[1] = l;
		rgi[2] = l+1;

		SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 3, NULL);
		}*/
	}

void Light::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	if (m_d.m_borderwidth > 0)
		{
		Pin3D *ppin3d = &g_pplayer->m_pin3d;

		float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

		ppin3d->EnableLightMap(!m_fBackglass, height);
		if (m_d.m_shape == ShapeCustom)
			{
			RenderCustomStatic(pd3dDevice);
			}
		else
			{
			RenderStaticCircle(pd3dDevice);
			}
		ppin3d->EnableLightMap(fFalse, -1);
		}
	}

void Light::RenderCustomMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	int i,t;
	//WORD rgi[3];
	Vertex3D rgv3D[3];

	RenderVertex *rgv;
	int cvertex;

	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1.0;

		{
		Vector<RenderVertex> vvertex;
		GetRgVertex(&vvertex);

		cvertex = vvertex.Size();
		rgv = new RenderVertex[cvertex];

		for (i=0;i<vvertex.Size();i++)
			{
			rgv[i] = *vvertex.ElementAt(i);
			delete vvertex.ElementAt(i);
			}
	}

	Pin3D *ppin3d = &g_pplayer->m_pin3d;
	float r = (m_d.m_color & 255) / 255.0f;
	float g = (m_d.m_color & 65280) / 65280.0f;
	float b = (m_d.m_color & 16711680) / 16711680.0f;

	ppin3d->SetTexture(ppin3d->m_pddsLightTexture);

	Vector<void> vpoly;
	Vector<Triangle> vtri;

	float maxdist = 0;

	for (i=0;i<cvertex;i++)
		{
		vpoly.AddElement((void *)i);

		float dx,dy,dist;
		dx = rgv[i].x - m_d.m_vCenter.x;
		dy = rgv[i].y - m_d.m_vCenter.y;
		dist = (dx*dx + dy*dy);
		if (dist > maxdist)
			{
			maxdist = dist;
			}
		}

	maxdist = (float)sqrt(maxdist);

	PolygonToTriangles(rgv, &vpoly, &vtri);

	for (i=0;i<2;i++)
		{
		//ppin3d->m_pd3dDevice->BeginScene();

		//pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET ,
					   //0x00000000, 1.0f, 0L );

		switch (i)
			{
			case 0:
				ppin3d->EnableLightMap(!m_fBackglass, height);
				mtrl.diffuse.r = mtrl.ambient.r = r/3;
				mtrl.diffuse.g = mtrl.ambient.g = g/3;
				mtrl.diffuse.b = mtrl.ambient.b = b/3;
				mtrl.emissive.r = 0;
				mtrl.emissive.g = 0;
				mtrl.emissive.b = 0;
				break;
			case 1:
				ppin3d->EnableLightMap(fFalse, -1);
				mtrl.diffuse.r = mtrl.ambient.r = 0;//r;
				mtrl.diffuse.g = mtrl.ambient.g = 0;//g;
				mtrl.diffuse.b = mtrl.ambient.b = 0;//b;
				mtrl.emissive.r = r;
				mtrl.emissive.g = g;
				mtrl.emissive.b = b;
				break;
			}

		pd3dDevice->SetMaterial(&mtrl);

		m_pobjframe[i] = new ObjFrame();

		ppin3d->ClearExtents(&m_pobjframe[i]->rc, NULL, NULL);

		WORD rgi[3];
		rgi[0] = 0;
		rgi[1] = 1;
		rgi[2] = 2;
		if (!m_fBackglass)
			{
			rgv3D[0].nx = 0;
			rgv3D[0].ny = 0;
			rgv3D[0].nz = -1;
			rgv3D[1].nx = 0;
			rgv3D[1].ny = 0;
			rgv3D[1].nz = -1;
			rgv3D[2].nx = 0;
			rgv3D[2].ny = 0;
			rgv3D[2].nz = -1;
			}
		else
			{
			SetDiffuseFromMaterial(rgv3D, 3, &mtrl);
			}

		for (t=0;t<vtri.Size();t++)
			{
			Triangle *ptri = vtri.ElementAt(t);

			int ip[3];

			ip[0] = ptri->a;
			ip[1] = ptri->c;
			ip[2] = ptri->b;

			RenderVertex *pv0 = &rgv[ip[0]];
			RenderVertex *pv1 = &rgv[ip[1]];
			RenderVertex *pv2 = &rgv[ip[2]];

			rgv3D[0].Set(pv0->x,pv0->y,height + 0.1f);
			rgv3D[1].Set(pv1->x,pv1->y,height + 0.1f);
			rgv3D[2].Set(pv2->x,pv2->y,height + 0.1f);

			rgv3D[0].tu = 0.5f;
			rgv3D[0].tv = 0.5f;
			rgv3D[1].tu = 0.5f;
			rgv3D[1].tv = 0.5f;
			rgv3D[2].tu = 0.5f;
			rgv3D[2].tv = 0.5f;

			int l;
			for (l=0;l<3;l++)
				{
				if (!m_fBackglass)
					{
					ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
					}

				float dist, ang;
				float dx,dy;
				dx = rgv3D[l].x - m_d.m_vCenter.x;
				dy = rgv3D[l].y - m_d.m_vCenter.y;
				ang = (float)atan2(dy,dx);
				dist = (float)sqrt(dx*dx + dy*dy);
				rgv3D[l].tu = 0.5f + ((float)sin(ang) * 0.5f * (dist/maxdist));
				rgv3D[l].tv = 0.5f + ((float)cos(ang) * 0.5f * (dist/maxdist));
				}

			if (!m_fBackglass)
				{
				pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 3,
													  rgi, 3, NULL);
				}
			else
				{
				SetHUDVertices(rgv3D, 3);

				pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
													  rgv3D, 3,
													  rgi, 3, NULL);
				}

			ppin3d->ExpandExtents(&m_pobjframe[i]->rc, rgv3D, NULL, NULL, 3, m_fBackglass);
			}

		//if (i != 0)
			{
			int iedit;
			for (iedit=0;iedit<m_ptable->m_vedit.Size();iedit++)
				{
				IEditable *pie = m_ptable->m_vedit.ElementAt(iedit);
				if (pie->GetItemType() == eItemDecal)
					{
					if (fIntRectIntersect(((Decal *)pie)->m_rcBounds, m_pobjframe[i]->rc))
						{
						pie->GetIHitable()->RenderStatic(pd3dDevice);
						}
					}
				}
			ppin3d->SetTexture(ppin3d->m_pddsLightTexture);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
			}

		//ppin3d->m_pd3dDevice->EndScene();

		ppin3d->ClipRectToVisibleArea(&m_pobjframe[i]->rc);
		m_pobjframe[i]->pdds = ppin3d->CreateOffscreen(m_pobjframe[i]->rc.right - m_pobjframe[i]->rc.left, m_pobjframe[i]->rc.bottom - m_pobjframe[i]->rc.top);

		if (m_pobjframe[i]->pdds == NULL)
			{
			continue;
			}
			
		m_pobjframe[i]->pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &m_pobjframe[i]->rc, 0, NULL);

		ppin3d->WriteObjFrameToCacheFile(m_pobjframe[i]);

		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&m_pobjframe[i]->rc, NULL,
				&m_pobjframe[i]->rc, DDBLT_COLORFILL, &ddbltfx);
		}

	for (i=0;i<vtri.Size();i++)
		{
		delete vtri.ElementAt(i);
		}

	delete rgv;

	//pd3dDevice->SetTexture(ePictureTexture, NULL);
	ppin3d->SetTexture(NULL);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}

void Light::RenderMoversFromCache(Pin3D *ppin3d)
	{
	int i;
	for (i=0;i<2;i++)
		{
		m_pobjframe[i] = new ObjFrame();
		ppin3d->ReadObjFrameFromCacheFile(m_pobjframe[i]);
		}
	}

void Light::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	int i,l;
	WORD rgi[32];
	Vertex3D rgv3D[32];
	Pin3D *ppin3d = &g_pplayer->m_pin3d;
	float r = (m_d.m_color & 255) / 255.0f;
	float g = (m_d.m_color & 65280) / 65280.0f;
	float b = (m_d.m_color & 16711680) / 16711680.0f;

	if (m_d.m_shape == ShapeCustom)
		{
		RenderCustomMovers(pd3dDevice);

		/*if (m_d.m_state == LightStateBlinking)
			{
			DrawFrame(m_d.m_rgblinkpattern[0] == '1');
			}
		else
			{
			DrawFrame(m_d.m_state != LightStateOff);
			}*/
		return;
		}

	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	ppin3d->SetTexture(ppin3d->m_pddsLightTexture);

	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);

	//pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1.0;

	//ppin3d->m_pddsBackBuffer->Blt(NULL, ppin3d->m_pddsStatic, NULL, 0, NULL);
	//ppin3d->m_pddsZBuffer->Blt(NULL, ppin3d->m_pddsStaticZ, NULL, 0, NULL);

	for (l=0;l<32;l++)
		{
		float angle = PI*2;
		angle /= 32;
		angle *= l;
		rgv3D[l].x = (float)sin(angle)*m_d.m_radius + m_d.m_vCenter.x;
		rgv3D[l].y = (float)-cos(angle)*m_d.m_radius + m_d.m_vCenter.y;
		rgv3D[l].z = height + 0.1f;

		rgv3D[l].tu = 0.5f+(float)(sin(angle)*0.5);
		rgv3D[l].tv = 0.5f+(float)(cos(angle)*0.5);

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		}

	for (l=0;l<32;l++)
		{
		rgi[l] = l;
		}

	if (!m_fBackglass)
		{
		SetNormal(rgv3D, rgi, 32, NULL, NULL, 0);
		}
	else
		{
		SetHUDVertices(rgv3D, 32);
		}

	for (i=0;i<2;i++)
		{
		//ppin3d->m_pd3dDevice->BeginScene();

		//pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET,
					   //0x00000000, 1.0f, 0L );

		switch (i)
			{
			case 0:
				ppin3d->EnableLightMap(!m_fBackglass, height);
				mtrl.diffuse.r = mtrl.ambient.r = r/3;
				mtrl.diffuse.g = mtrl.ambient.g = g/3;
				mtrl.diffuse.b = mtrl.ambient.b = b/3;
				mtrl.emissive.r = 0;
				mtrl.emissive.g = 0;
				mtrl.emissive.b = 0;
				break;
			case 1:
				ppin3d->EnableLightMap(fFalse, -1);
				mtrl.diffuse.r = mtrl.ambient.r = 0;//r;
				mtrl.diffuse.g = mtrl.ambient.g = 0;//g;
				mtrl.diffuse.b = mtrl.ambient.b = 0;//b;
				mtrl.emissive.r = r;
				mtrl.emissive.g = g;
				mtrl.emissive.b = b;
				break;
			}

		if (m_fBackglass)
			{
			SetDiffuseFromMaterial(rgv3D, 32, &mtrl);
			}

		pd3dDevice->SetMaterial(&mtrl);

		m_pobjframe[i] = new ObjFrame();

		ppin3d->ClearExtents(&m_pobjframe[i]->rc, NULL, NULL);
		ppin3d->ExpandExtents(&m_pobjframe[i]->rc, rgv3D, NULL, NULL, 32, m_fBackglass);

		ppin3d->ClipRectToVisibleArea(&m_pobjframe[i]->rc);

		m_pobjframe[i]->pdds = ppin3d->CreateOffscreen(m_pobjframe[i]->rc.right - m_pobjframe[i]->rc.left, m_pobjframe[i]->rc.bottom - m_pobjframe[i]->rc.top);

		if (m_pobjframe[i]->pdds == NULL)
			{
			continue;
			}

		if (!m_fBackglass)
			{
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 32,
														  rgi, 32, NULL);
			}
		else
			{
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
													  rgv3D, 32,
													  rgi, 32, NULL);
			}

		/*for (l=1;l<31;l++)
			{
			rgi[0] = 0;
			rgi[1] = l;
			rgi[2] = l+1;

			SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 32,
													  rgi, 3, NULL);
			}*/

			{
			int iedit;
			for (iedit=0;iedit<m_ptable->m_vedit.Size();iedit++)
				{
				IEditable *pie = m_ptable->m_vedit.ElementAt(iedit);
				if (pie->GetItemType() == eItemDecal)
					{
					if (fIntRectIntersect(((Decal *)pie)->m_rcBounds, m_pobjframe[i]->rc))
						{
						pie->GetIHitable()->RenderStatic(pd3dDevice);
						}
					}
				}
			ppin3d->SetTexture(ppin3d->m_pddsLightTexture);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
			}

		//ppin3d->m_pd3dDevice->EndScene();

		m_pobjframe[i]->pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &m_pobjframe[i]->rc, 0, NULL);

		ppin3d->WriteObjFrameToCacheFile(m_pobjframe[i]);

		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;//0xffff;
		ppin3d->m_pddsBackBuffer->Blt(&m_pobjframe[i]->rc, NULL,
				&m_pobjframe[i]->rc, DDBLT_COLORFILL, &ddbltfx);

		//pd3dDevice->Clear( 1, &m_pobjframe[i]->rc, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L );
		}

	/*int frame;


	if (m_d.m_state == LightStateOff)
		{
		frame = 0;
		}
	else
		{
		frame = 1;
		}*/

	/*if (m_d.m_state == LightStateBlinking)
		{
		DrawFrame(m_d.m_rgblinkpattern[0] == '1');
		}
	else
		{
		DrawFrame(m_d.m_state != LightStateOff);
		}*/

	//ppin3d->m_pddsStatic->Blt(&m_pobjframe[frame]->rc, m_pobjframe[frame]->pdds, NULL, 0, NULL);

	//ppin3d->EnableLightMap(fTrue);
	//pd3dDevice->SetTexture(ePictureTexture, NULL);
	ppin3d->SetTexture(NULL);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}

void Light::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
	}

void Light::MoveOffset(float dx, float dy)
	{
	m_d.m_vCenter.x += dx;
	m_d.m_vCenter.y += dy;

	int i;

	for (i=0;i<m_vdpoint.Size();i++)
		{
		CComObject<DragPoint> *pdp;

		pdp = m_vdpoint.ElementAt(i);

		pdp->m_v.x += dx;
		pdp->m_v.y += dy;
		}

	m_ptable->SetDirtyDraw();
	}

HRESULT Light::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	HRESULT hr;

	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex));
	bw.WriteFloat(FID(RADI), m_d.m_radius);
	bw.WriteInt(FID(STAT), m_d.m_state);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteInt(FID(SHAP), m_d.m_shape);
	bw.WriteString(FID(BPAT), m_rgblinkpattern);
	bw.WriteInt(FID(BINT), m_blinkinterval);
	bw.WriteInt(FID(BCOL), m_d.m_bordercolor);
	bw.WriteFloat(FID(BWTH), m_d.m_borderwidth);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

	bw.WriteBool(FID(BGLS), m_fBackglass);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	//bw.WriteTag(FID(PNTS));
	if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
		return hr;

	bw.WriteTag(FID(ENDB));

	return S_OK;

	/*ULONG writ = 0;
	HRESULT hr = S_OK;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = SavePointData(pstm)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_d, sizeof(LightData), &writ)))
		return hr;

	return hr;*/
	}

HRESULT Light::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults();
#ifndef OLDLOAD

	m_d.m_radius = 50;
	m_d.m_state = LightStateOff;
	m_d.m_shape = ShapeCircle;

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_color = RGB(255,255,0);

	strcpy(m_rgblinkpattern, "10");
	m_blinkinterval = 125;
	m_d.m_borderwidth = 0;
	m_d.m_bordercolor = RGB(0,0,0);

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

	if(FAILED(hr = InitPointLoad(pstm)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_d, sizeof(LightData), &read)))
		return hr;

	*pid = dwID;

	return hr;
#endif
	}

BOOL Light::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VCEN))
		{
		pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex));
		}
	else if (id == FID(RADI))
		{
		pbr->GetFloat(&m_d.m_radius);
		}
	else if (id == FID(STAT))
		{
		pbr->GetInt(&m_d.m_state);
		m_realState	= m_d.m_state;		//>>> added by chris
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
	else if (id == FID(SHAP))
		{
		pbr->GetInt(&m_d.m_shape);
		}
	else if (id == FID(BPAT))
		{
		pbr->GetString(m_rgblinkpattern);
		}
	else if (id == FID(BINT))
		{
		pbr->GetInt(&m_blinkinterval);
		}
	else if (id == FID(BCOL))
		{
		pbr->GetInt(&m_d.m_bordercolor);
		}
	else if (id == FID(BWTH))
		{
		pbr->GetFloat(&m_d.m_borderwidth);
		}
	/*else if (id == FID(PNTS))
		{
		InitPointLoad(pbr->m_pistream, NULL);
		}*/
	else if (id == FID(SURF))
		{
		pbr->GetString(m_d.m_szSurface);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(BGLS))
		{
		pbr->GetBool(&m_fBackglass);
		}
	else
		{
		LoadPointToken(id, pbr, pbr->m_version);
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Light::InitPostLoad()
	{
	m_pobjframe[0] = NULL;
	m_pobjframe[1] = NULL;

	return S_OK;
	}

void Light::GetPointCenter(Vertex *pv)
	{
	*pv = m_d.m_vCenter;
	}

void Light::PutPointCenter(Vertex *pv)
	{
	m_d.m_vCenter = *pv;

	SetDirtyDraw();
	}

void Light::EditMenu(HMENU hmenu)
	{
	EnableMenuItem(hmenu, ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
	}

void Light::DoCommand(int icmd, int x, int y)
	{
	ISelect::DoCommand(icmd, x, y);

	switch (icmd)
		{
		case ID_WALLMENU_FLIP:
			{
			Vertex vCenter;
			GetPointCenter(&vCenter);
			FlipPointY(&vCenter);
			}
			break;

		case ID_WALLMENU_MIRROR:
			{
			Vertex vCenter;
			GetPointCenter(&vCenter);
			FlipPointX(&vCenter);
			}
			break;

		case ID_WALLMENU_ROTATE:
			RotateDialog();
			break;

		case ID_WALLMENU_SCALE:
			ScaleDialog();
			break;

		case ID_WALLMENU_TRANSLATE:
			TranslateDialog();
			break;

		case ID_WALLMENU_ADDPOINT:
			{
			STARTUNDO

			HitSur *phs;

			RECT rc;
			GetClientRect(m_ptable->m_hwnd, &rc);
			Vertex v, vOut;
			int iSeg;

			phs = new HitSur(NULL, m_ptable->m_zoom, m_ptable->m_offsetx, m_ptable->m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

			phs->ScreenToSurface(x, y, &v.x, &v.y);
			delete phs;

			int cvertex;
			Vertex *rgv;

			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			cvertex = vvertex.Size();
			rgv = new Vertex[cvertex];

			int i;
			for (i=0;i<vvertex.Size();i++)
				{
				rgv[i] = *((Vertex *)vvertex.ElementAt(i));
				}

			ClosestPointOnPolygon(rgv, cvertex, &v, &vOut, &iSeg, fTrue);

			// Go through vertices (including iSeg itself) counting control points until iSeg
			int icp = 0;
			for (i=0;i<(iSeg+1);i++)
				{
				if (vvertex.ElementAt(i)->fControlPoint)
					{
					icp++;
					}
				}

			//if (icp == 0) // need to add point after the last point
				//icp = m_vdpoint.Size();

			CComObject<DragPoint> *pdp;

			CComObject<DragPoint>::CreateInstance(&pdp);
			if (pdp)
				{
				pdp->AddRef();
				pdp->Init(this, vOut.x, vOut.y);
				m_vdpoint.InsertElementAt(pdp, icp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
				}

			for (i=0;i<vvertex.Size();i++)
				{
				delete vvertex.ElementAt(i);
				}

			delete rgv;

			SetDirtyDraw();

			STOPUNDO
			}
			break;
		}
	}

STDMETHODIMP Light::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_ILight,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP Light::get_Radius(float *pVal)
{
	*pVal = m_d.m_radius;

	return S_OK;
}

STDMETHODIMP Light::put_Radius(float newVal)
{
	if (newVal < 0)
		{
		return E_FAIL;
		}

	STARTUNDO

	m_d.m_radius = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_State(LightState *pVal)
{
	*pVal = m_d.m_state;

	return S_OK;
}

STDMETHODIMP Light::put_State(LightState newVal)
{
	STARTUNDO
//>>> added by Chris
	// if the light is locked by the LS then just change the state and don't change the actual light
	if (m_fLockedByLS != fTrue)
		{
		setLightState(newVal);
		}
	m_d.m_state = newVal;
//<<<
	STOPUNDO

	return S_OK;
}

void Light::DrawFrame(BOOL fOn)
	{
	m_fOn = fOn;
	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	int frame = fOn ? 1 : 0;

	//ppin3d->m_pddsStatic->Blt(&m_pobjframe[frame]->rc, m_pobjframe[frame]->pdds, NULL, 0, NULL);

	// Light might be off the screen and have no image
	if (m_pobjframe[frame]->pdds != NULL)
		{
		// We can use BltFast here because we are drawing to our own offscreen iamge
		HRESULT hr = ppin3d->m_pddsStatic->BltFast(m_pobjframe[frame]->rc.left, m_pobjframe[frame]->rc.top, m_pobjframe[frame]->pdds, NULL, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);

		g_pplayer->InvalidateRect(&m_pobjframe[frame]->rc);
		}
	}

/*STDMETHODIMP Light::get_Name(BSTR *pVal)
{
	CComBSTR bstr;

	GetApcProjectItem()->get_Name(&bstr);

	//unsigned short wz[512];

	//MultiByteToWideChar(CP_ACP, 0, (char *)m_d.sztext, -1, wz, 512);
	*pVal = SysAllocString(bstr);

	return S_OK;
}

STDMETHODIMP Light::put_Name(BSTR newVal)
{
	GetApcProjectItem()->put_Name(newVal);

	return S_OK;
}*/

void Light::FlipY(Vertex *pvCenter)
	{
	IHaveDragPoints::FlipPointY(pvCenter);
	}

void Light::FlipX(Vertex *pvCenter)
	{
	IHaveDragPoints::FlipPointX(pvCenter);
	}

void Light::Rotate(float ang, Vertex *pvCenter)
	{
	IHaveDragPoints::RotatePoints(ang, pvCenter);
	}

void Light::Scale(float scalex, float scaley, Vertex *pvCenter)
	{
	IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
	}

void Light::Translate(Vertex *pvOffset)
	{
	IHaveDragPoints::TranslatePoints(pvOffset);
	}

STDMETHODIMP Light::get_Color(OLE_COLOR *pVal)
{
	*pVal = m_d.m_color;

	return S_OK;
}

STDMETHODIMP Light::put_Color(OLE_COLOR newVal)
{
	STARTUNDO
	/*if (!g_pplayer)
		{
		BeginUndo();
		MarkForUndo();
		}*/

	m_d.m_color = newVal;

	STOPUNDO
	/*if (!g_pplayer)
		{
		EndUndo();
		SetDirtyDraw();
		}*/

	return S_OK;
}

STDMETHODIMP Light::get_X(float *pVal)
{
	*pVal = m_d.m_vCenter.x;

	return S_OK;
}

STDMETHODIMP Light::put_X(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_Y(float *pVal)
{
	*pVal = m_d.m_vCenter.y;

	return S_OK;
}

STDMETHODIMP Light::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_Shape(Shape *pVal)
{
	*pVal = m_d.m_shape;

	return S_OK;
}

STDMETHODIMP Light::put_Shape(Shape newVal)
{
	STARTUNDO

	m_d.m_shape = newVal;

	if (m_d.m_shape == ShapeCustom && m_vdpoint.Size() == 0)
		{
		// First time shape has been set to custom - set up some points
		float x,y;

		x = m_d.m_vCenter.x;
		y = m_d.m_vCenter.y;

		CComObject<DragPoint> *pdp;
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x-30, y-30);
			m_vdpoint.AddElement(pdp);
			}
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x-30, y+30);
			m_vdpoint.AddElement(pdp);
			}
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x+30, y+30);
			m_vdpoint.AddElement(pdp);
			}
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x+30, y-30);
			m_vdpoint.AddElement(pdp);
			}

		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_BlinkPattern(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_rgblinkpattern, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Light::put_BlinkPattern(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_rgblinkpattern, 32, NULL, NULL);

	if (m_rgblinkpattern[0] == '\0')
		{
		m_rgblinkpattern[0] = '0';
		m_rgblinkpattern[1] = '\0';
		}

	if (g_pplayer)
		{
		// Restart the sequence
		// BUG - merge with code in player for light blinking someday
		char cold = m_rgblinkpattern[m_iblinkframe];
		m_iblinkframe = 0;
		char cnew = m_rgblinkpattern[m_iblinkframe];
		if (cold != cnew)
			{
			DrawFrame(cnew == '1');
			}
		m_timenextblink = g_pplayer->m_timeCur + m_blinkinterval;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_BlinkInterval(long *pVal)
{
	*pVal = m_blinkinterval;

	return S_OK;
}

STDMETHODIMP Light::put_BlinkInterval(long newVal)
{
	STARTUNDO

	m_blinkinterval = newVal;

	if (g_pplayer)
		{
		m_timenextblink = g_pplayer->m_timeCur + m_blinkinterval;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_BorderColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_bordercolor;

	return S_OK;
}

STDMETHODIMP Light::put_BorderColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_bordercolor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_BorderWidth(float *pVal)
{
	*pVal = m_d.m_borderwidth;

	return S_OK;
}

STDMETHODIMP Light::put_BorderWidth(float newVal)
{
	STARTUNDO

	m_d.m_borderwidth = max(0, newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_Surface(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Light::put_Surface(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

/*HRESULT Light::GetTypeName(BSTR *pVal)
	{
	*pVal = SysAllocString(L"Light");

	return S_OK;
	}*/

/*int Light::GetDialogID()
	{
	return IDD_PROPLIGHT;
	}*/

void Light::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPLIGHT_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPLIGHT_STATE, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

void Light::lockLight()
	{
		m_fLockedByLS = fTrue;
	}

void Light::unLockLight()
	{
		m_fLockedByLS = fFalse;
	}

void Light::setLightStateBypass(LightState newVal)
	{
		lockLight();
		setLightState(newVal);
	}
void Light::setLightState(LightState newVal)
	{
   	if (newVal != m_realState)
   		{
   		BOOL fWasBlinking = (m_realState == LightStateBlinking);
		m_realState = newVal;

   		if (g_pplayer)
   			{
   			if (fWasBlinking)
   				{
   				// must not be blinking anymore
   				g_pplayer->m_vblink.RemoveElement((IBlink *)this);
   				}
   			else if (m_realState == LightStateBlinking)
   				{
   				// must be blinking now
   				g_pplayer->m_vblink.AddElement((IBlink *)this);
   				m_timenextblink = g_pplayer->m_timeCur; // Start pattern right away // + m_d.m_blinkinterval;
   				}

   			if (m_pobjframe[0])
   				{
   				switch (m_realState)
   					{
   					case LightStateOff:
   						DrawFrame(fFalse);
   						break;

   					case LightStateOn:
   						DrawFrame(fTrue);
   						break;

   					case LightStateBlinking:
   						// We know we can't be on the list already because we make sure our state has changed
   						m_iblinkframe = 0; // reset pattern
   						break;
   					}
   				}
   			}
   		}
   	}
//<<<

