// Light.cpp : Implementation of CVBATestApp and DLL registration.

#include "StdAfx.h"


/////////////////////////////////////////////////////////////////////////////
//

LightCenter::LightCenter(Light *plight)
	{
	m_plight = plight;
	}

HRESULT LightCenter::GetTypeName(BSTR *pVal) {return m_plight->GetTypeName(pVal);}
IDispatch *LightCenter::GetDispatch() {return m_plight->GetDispatch();}
void LightCenter::GetDialogPanes(Vector<PropertyPane> *pvproppane) {m_plight->GetDialogPanes(pvproppane);}
void LightCenter::Delete() {m_plight->Delete();}
void LightCenter::Uncreate() {m_plight->Uncreate();}
IEditable *LightCenter::GetIEditable() {return (IEditable *)m_plight;}
PinTable *LightCenter::GetPTable() {return m_plight->GetPTable();}

void LightCenter::GetCenter(Vertex2D *pv)
	{
	*pv = m_plight->m_d.m_vCenter;
	}

void LightCenter::PutCenter(Vertex2D *pv)
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

	m_fLockedByLS = false;			//>>> added by chris
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

	m_d.m_szOffImage[0] = 0;
	m_d.m_szOnImage[0] = 0;

	m_d.m_fDisplayImage = fFalse;

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

		case ShapeCustom: {
			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			const int cvertex = vvertex.Size();
			Vertex2D * const rgv = new Vertex2D[cvertex];

			for (int i=0;i<vvertex.Size();i++)
				{
				rgv[i] = *((Vertex2D *)vvertex.ElementAt(i));
				delete vvertex.ElementAt(i);
				}

			// Check if we should display the image in the editor.
			if ((m_d.m_fDisplayImage) && (m_d.m_szOnImage[0]) && (m_d.m_szOffImage[0])) 
				{
				PinImage *ppi;
				// Get the image.
				switch (m_d.m_state)
					{
					case LightStateOff:	
						ppi = m_ptable->GetImage(m_d.m_szOffImage);
						break;
					case LightStateOn:	
						ppi = m_ptable->GetImage(m_d.m_szOnImage);
						break;
					default:
						ppi = NULL;
						break;
					}

				// Make sure we have an image.
				if (ppi != NULL)
					{
					ppi->EnsureHBitmap();
					if (ppi->m_hbmGDIVersion)
						{
						// Draw the polygon with an image applied.
						psur->PolygonImage(rgv, cvertex, ppi->m_hbmGDIVersion, m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);
						}
					}
				else
					{
					// Error.  Just draw the polygon.
					psur->Polygon(rgv, cvertex);
					}
				}
			else
				{
				// Draw the polygon.
				psur->Polygon(rgv, cvertex);
				}

			delete rgv;
			break;
						  }
		}
	}

void Light::Render(Sur *psur)
	{
	BOOL	fDrawDragpoints;		//>>> added by chris

	// if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
	if ( (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints) )
		{
		fDrawDragpoints = fTrue;
		}
	else
		{
		// if any of the dragpoints of this object are selected then draw all the dragpoints
		fDrawDragpoints = fFalse;
		for (int i=0;i<m_vdpoint.Size();i++)
			{
			const CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
			if (pdp->m_selectstate != eNotSelected)
				{
				fDrawDragpoints = fTrue;
				break;
				}
			}
		}

	RenderOutline(psur);

	if ( (m_d.m_shape == ShapeCustom) && (fDrawDragpoints) )	//<<< modified by chris
		{
		for (int i=0;i<m_vdpoint.Size();i++)
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

	}

void Light::RenderOutline(Sur * const psur)
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

		case ShapeCustom: {
			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			const int cvertex = vvertex.Size();
			Vertex2D * const rgv = new Vertex2D[cvertex];

			for (int i=0;i<vvertex.Size();i++)
				{
				rgv[i] = *((Vertex2D *)vvertex.ElementAt(i));
				delete vvertex.ElementAt(i);
				}

			psur->Polygon(rgv, cvertex);

			delete rgv;

			psur->SetObject((ISelect *)&m_lightcenter);
			break;
						  }
		}

	if (m_d.m_shape == ShapeCustom || g_pvp->m_fAlwaysDrawLightCenters)
		{
		psur->Line(m_d.m_vCenter.x - 10.0f, m_d.m_vCenter.y, m_d.m_vCenter.x + 10.0f, m_d.m_vCenter.y);
		psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10.0f, m_d.m_vCenter.x, m_d.m_vCenter.y + 10.0f);
		}
	}

void Light::RenderBlueprint(Sur *psur)
	{
	RenderOutline(psur);
	}

void Light::GetTimers(Vector<HitTimer> *pvht)
	{
	IEditable::BeginPlay();

	HitTimer * const pht = new HitTimer();
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
	const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	switch (m_d.m_shape)
		{
		case ShapeCircle:
		default:
			{
			HitObject * const pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height, m_d.m_radius, 32);
			pvho->AddElement(pho);
			}
			break;

		case ShapeCustom:
			{
			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			const int cvertex = vvertex.Size();
			Vertex3D * const rgv3d = new Vertex3D[cvertex];

			for (int i=0;i<vvertex.Size();i++)
				{
				rgv3d[i].x = vvertex.ElementAt(i)->x;
				rgv3d[i].y = vvertex.ElementAt(i)->y;
				rgv3d[i].z = height;
				delete vvertex.ElementAt(i);
				}

			Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex, true);
			pvho->AddElement(ph3dp);
			}
			break;
		}
	}

void Light::EndPlay()
	{
	for (int i=0;i<2;i++)
		{
		//m_pobjframe[i]->ppds->Release();
		delete m_pobjframe[i];
		m_pobjframe[i] = NULL;
		}

	// ensure not locked just incase the player exits during a LS sequence
	m_fLockedByLS = false;			//>>> added by chris

	IEditable::EndPlay();
	}

void Light::ClearForOverwrite()
	{
	ClearPointsForOverwrite();
	}

void Light::RenderCustomStatic(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	const int cvertex = vvertex.Size();
	RenderVertex *const rgv = new RenderVertex[cvertex];

	for (int i=0;i<cvertex;i++)
		{
		const int p1 = (i+cvertex-1) % cvertex;
		const int p2 = (i+1) % cvertex;

		Vertex2D v1 = *vvertex.ElementAt(p1);
		Vertex2D v2 = *vvertex.ElementAt(p2);
		const Vertex2D vmiddle = *vvertex.ElementAt(i);

		Vertex2D v1normal, v2normal;
		// Notice that these values equal the ones in the line
		// equation and could probably be substituted by them.
		v1normal.x = vmiddle.y - v1.y;
		v1normal.y = v1.x - vmiddle.x;
		v2normal.x = v2.y - vmiddle.y;
		v2normal.y = vmiddle.x - v2.x;

		v1normal.Normalize();
		v2normal.Normalize();

		// Find intersection of the two edges meeting this points, but
		// shift those lines outwards along their normals

		// First line
		const float A = v1.y - vmiddle.y;
		const float B = vmiddle.x - v1.x;

		// Shift line along the normal
		v1.x -= v1normal.x*m_d.m_borderwidth;
		v1.y -= v1normal.y*m_d.m_borderwidth;

		const float C = -(A*v1.x + B*v1.y);

		// Second line
		const float D = v2.y - vmiddle.y;
		const float E = vmiddle.x - v2.x;

		// Shift line along the normal
		v2.x -= v2normal.x*m_d.m_borderwidth;
		v2.y -= v2normal.y*m_d.m_borderwidth;

		const float F = -(D*v2.x + E*v2.y);

		const float inv_det = 1.0f/((A*E) - (B*D));

		const float intersectx=(B*F-E*C)*inv_det;
		const float intersecty=(C*D-A*F)*inv_det;

		rgv[i].x = intersectx;
		rgv[i].y = intersecty;
		}

	for (int i=0;i<cvertex;i++)
		{
		delete vvertex.ElementAt(i);
		}

	const float r = (m_d.m_bordercolor & 255) * (float)(1.0/255.0);
	const float g = (m_d.m_bordercolor & 65280) * (float)(1.0/65280.0);
	const float b = (m_d.m_bordercolor & 16711680) * (float)(1.0/16711680.0);

	Vector<void> vpoly;
	Vector<Triangle> vtri;

	for (int i=0;i<cvertex;i++)
		{
		vpoly.AddElement((void *)i);
		}

	PolygonToTriangles(rgv, &vpoly, &vtri);

	mtrl.diffuse.r = mtrl.ambient.r = r;///4;
	mtrl.diffuse.g = mtrl.ambient.g = g;///4;
	mtrl.diffuse.b = mtrl.ambient.b = b;///4;

	pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[3] = {0,1,2};

	Vertex3D rgv3D[3];
	if (!m_fBackglass)
		{
		rgv3D[0].nx = 0;
		rgv3D[0].ny = 0;
		rgv3D[0].nz = -1.0f;
		rgv3D[1].nx = 0;
		rgv3D[1].ny = 0;
		rgv3D[1].nz = -1.0f;
		rgv3D[2].nx = 0;
		rgv3D[2].ny = 0;
		rgv3D[2].nz = -1.0f;
		}
	else
		{
		SetDiffuseFromMaterial(rgv3D, 3, &mtrl);
		}

	for (int t=0;t<vtri.Size();t++)
		{
		const Triangle * const ptri = vtri.ElementAt(t);

		const int ip[3] = {ptri->a,ptri->c,ptri->b};

		const RenderVertex * const pv0 = &rgv[ip[0]];
		const RenderVertex * const pv1 = &rgv[ip[1]];
		const RenderVertex * const pv2 = &rgv[ip[2]];

		rgv3D[0].Set(pv0->x,pv0->y,height + 0.05f);
		rgv3D[1].Set(pv1->x,pv1->y,height + 0.05f);
		rgv3D[2].Set(pv2->x,pv2->y,height + 0.05f);

		if (!m_fBackglass)
			{
			Pin3D * const ppin3d = &g_pplayer->m_pin3d;
			for (int l=0;l<3;l++)
				ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 3,
													  rgi, 3, 0);
			}
		else
			{
			SetHUDVertices(rgv3D, 3);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
													  rgv3D, 3,
													  rgi, 3, 0);
			}
		}

	for (int i=0;i<vtri.Size();i++)
		delete vtri.ElementAt(i);

	delete rgv;
	}

void Light::RenderStaticCircle(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Pin3D * const ppin3d = &g_pplayer->m_pin3d;

	const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	const float r = (m_d.m_bordercolor & 255) * (float)(1.0/255.0);
	const float g = (m_d.m_bordercolor & 65280) * (float)(1.0/65280.0);
	const float b = (m_d.m_bordercolor & 16711680) * (float)(1.0/16711680.0);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = r;
	mtrl.diffuse.g = mtrl.ambient.g = g;
	mtrl.diffuse.b = mtrl.ambient.b = b;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;
	pd3dDevice->SetMaterial(&mtrl);

	Vertex3D rgv3D[32];
	for (int l=0;l<32;l++)
		{
		const float angle = (float)(M_PI*2.0/32.0)*(float)l;
		rgv3D[l].x = m_d.m_vCenter.x + sinf(angle)*(m_d.m_radius + m_d.m_borderwidth);
		rgv3D[l].y = m_d.m_vCenter.y - cosf(angle)*(m_d.m_radius + m_d.m_borderwidth);
		rgv3D[l].z = height + 0.05f;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		}

	WORD rgi[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

	if (!m_fBackglass)
		{
		SetNormal(rgv3D, rgi, 32, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 32, 0);
		}
	else
		{
		SetHUDVertices(rgv3D, 32);
		SetDiffuseFromMaterial(rgv3D, 32, &mtrl);

		if( GetPTable()->GetDecalsEnabled() )
			{
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
													  rgv3D, 32,
													  rgi, 32, 0);
			}
		}
	}

void Light::PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

void Light::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	if (m_d.m_borderwidth > 0)
		{
		Pin3D * const ppin3d = &g_pplayer->m_pin3d;

		const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

		ppin3d->EnableLightMap(!m_fBackglass, height);
		if (m_d.m_shape == ShapeCustom)
			RenderCustomStatic(pd3dDevice);
		else
			RenderStaticCircle(pd3dDevice);
		ppin3d->EnableLightMap(fFalse, -1);
		}
	}

void Light::RenderCustomMovers(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	PinImage* pin = NULL;

	RenderVertex* rgv;
	int cvertex;

	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

	const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

	{
		Vector<RenderVertex> vvertex;
		GetRgVertex(&vvertex);

		cvertex = vvertex.Size();
		rgv = new RenderVertex[cvertex];

		for (int i=0;i<vvertex.Size();i++)
			{
			rgv[i] = *vvertex.ElementAt(i);
			delete vvertex.ElementAt(i);
			}
	}

	Pin3D * const ppin3d = &g_pplayer->m_pin3d;
	const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
	const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
	const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);

	Vector<void> vpoly;
	float maxdist = 0;

	for (int i=0;i<cvertex;i++)
		{
		vpoly.AddElement((void *)i);

		const float dx = rgv[i].x - m_d.m_vCenter.x;
		const float dy = rgv[i].y - m_d.m_vCenter.y;
		const float dist = dx*dx + dy*dy;
		if (dist > maxdist)
			maxdist = dist;
		}

	maxdist = sqrtf(maxdist);

	Vector<Triangle> vtri;
	PolygonToTriangles(rgv, &vpoly, &vtri);

	for (int i=0;i<2;i++)
		{
		switch (i)
			{
			case LightStateOff:
				// Check if the light has an "off" texture.
				if (m_d.m_szOffImage[0] != 0 && (pin = m_ptable->GetImage(m_d.m_szOffImage)) != NULL)
					{
					// Set the texture to the one defined in the editor.

					ppin3d->SetTexture(pin->m_pdsBuffer);
					ppin3d->EnableLightMap(fFalse, -1);
					mtrl.diffuse.r = mtrl.ambient.r = 1.0f;
					mtrl.diffuse.g = mtrl.ambient.g = 1.0f;
					mtrl.diffuse.b = mtrl.ambient.b = 1.0f;
					mtrl.emissive.r = 0.0f;
					mtrl.emissive.g = 0.0f;
					mtrl.emissive.b = 0.0f;
					}
				else
					{
					// Set the texture to a default.
					ppin3d->SetTexture(ppin3d->m_pddsLightTexture);					
					ppin3d->EnableLightMap(!m_fBackglass, height);
					mtrl.diffuse.r = mtrl.ambient.r = r*(float)(1.0/3.0);
					mtrl.diffuse.g = mtrl.ambient.g = g*(float)(1.0/3.0);
					mtrl.diffuse.b = mtrl.ambient.b = b*(float)(1.0/3.0);
					mtrl.emissive.r = 0;
					mtrl.emissive.g = 0;
					mtrl.emissive.b = 0;
					}
				break;
			case LightStateOn:
				// Check if the light has an "on" texture.
				if (m_d.m_szOnImage[0] != 0 && (pin = m_ptable->GetImage(m_d.m_szOnImage)) != NULL)
					{
					// Set the texture to the one defined in the editor.
					ppin3d->SetTexture(pin->m_pdsBuffer);
					ppin3d->EnableLightMap(fFalse, -1);
					mtrl.diffuse.r = mtrl.ambient.r = 1.0f;
					mtrl.diffuse.g = mtrl.ambient.g = 1.0f;
					mtrl.diffuse.b = mtrl.ambient.b = 1.0f;
					mtrl.emissive.r = 0.0f;
					mtrl.emissive.g = 0.0f;
					mtrl.emissive.b = 0.0f;
					}
				else
					{
					// Set the texture to a default.
					ppin3d->SetTexture(ppin3d->m_pddsLightTexture);
					ppin3d->EnableLightMap(fFalse, -1);
					mtrl.diffuse.r = mtrl.ambient.r = 0;//r;
					mtrl.diffuse.g = mtrl.ambient.g = 0;//g;
					mtrl.diffuse.b = mtrl.ambient.b = 0;//b;
					mtrl.emissive.r = r;
					mtrl.emissive.g = g;
					mtrl.emissive.b = b;
					}
				break;
			}

		pd3dDevice->SetMaterial(&mtrl);

		m_pobjframe[i] = new ObjFrame();

		ppin3d->ClearExtents(&m_pobjframe[i]->rc, NULL, NULL);

		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{
			RECT	Rect;
			// Since we don't know the final dimensions of the 
			// object we're rendering, clear the whole buffer.
			Rect.top = 0;
			Rect.left = 0;
			Rect.bottom = g_pplayer->m_pin3d.m_dwRenderHeight - 1;
			Rect.right = g_pplayer->m_pin3d.m_dwRenderWidth - 1;

			// Clear the texture by copying the color and z values from the "static" buffers.
			Display_ClearTexture ( g_pplayer->m_pin3d.m_pd3dDevice, ppin3d->m_pddsBackTextureBuffer, (char) 0x00 );
			ppin3d->m_pddsZTextureBuffer->BltFast(Rect.left, Rect.top, ppin3d->m_pddsStaticZ, &Rect, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
			}

		WORD rgi[3] = {0,1,2};

		Vertex3D rgv3D[3];
		if (!m_fBackglass)
			{
			rgv3D[0].nx = 0;
			rgv3D[0].ny = 0;
			rgv3D[0].nz = -1.0f;
			rgv3D[1].nx = 0;
			rgv3D[1].ny = 0;
			rgv3D[1].nz = -1.0f;
			rgv3D[2].nx = 0;
			rgv3D[2].ny = 0;
			rgv3D[2].nz = -1.0f;
			}
		else
			{
			SetDiffuseFromMaterial(rgv3D, 3, &mtrl);
			}

		for (int t=0;t<vtri.Size();t++)
			{
			const Triangle * const ptri = vtri.ElementAt(t);

			const int ip[3] = {ptri->a,ptri->c,ptri->b};

			const RenderVertex * const pv0 = &rgv[ip[0]];
			const RenderVertex * const pv1 = &rgv[ip[1]];
			const RenderVertex * const pv2 = &rgv[ip[2]];

			rgv3D[0].Set(pv0->x,pv0->y,height + 0.1f);
			rgv3D[1].Set(pv1->x,pv1->y,height + 0.1f);
			rgv3D[2].Set(pv2->x,pv2->y,height + 0.1f);

			rgv3D[0].tu = 0.5f;
			rgv3D[0].tv = 0.5f;
			rgv3D[1].tu = 0.5f;
			rgv3D[1].tv = 0.5f;
			rgv3D[2].tu = 0.5f;
			rgv3D[2].tv = 0.5f;

			for (int l=0;l<3;l++)
				{
				if (!m_fBackglass)
					{
					ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
					}

				// Check if we are using a custom texture.
				if (pin != NULL)
					{
					const float tablewidth = m_ptable->m_right - m_ptable->m_left;
					const float tableheight = m_ptable->m_bottom - m_ptable->m_top;

					float maxtu, maxtv;
					m_ptable->GetTVTU(pin, &maxtu, &maxtv);

					// Set texture coordinates for custom texture (world mode).
					rgv3D[l].tu = rgv3D[l].x / tablewidth * maxtu;
					rgv3D[l].tv = rgv3D[l].y / tableheight * maxtv;
					}
				else
					{
					// Set texture coordinates for default light.
					const float dx = rgv3D[l].x - m_d.m_vCenter.x;
					const float dy = rgv3D[l].y - m_d.m_vCenter.y;
					const float ang = atan2f(dy,dx);
					const float dist = sqrtf(dx*dx + dy*dy);
					rgv3D[l].tu = 0.5f + sinf(ang) * 0.5f * dist/maxdist;
					rgv3D[l].tv = 0.5f + cosf(ang) * 0.5f * dist/maxdist;
					}
				}

			if (!m_fBackglass)
				{
				Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 3,
													  rgi, 3, 0);
				}
			else
				{
				SetHUDVertices(rgv3D, 3);

				if( GetPTable()->GetDecalsEnabled() )
					{
					Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
														  rgv3D, 3,
														  rgi, 3, 0);
					}
				}

			ppin3d->ExpandExtents(&m_pobjframe[i]->rc, rgv3D, NULL, NULL, 3, m_fBackglass);
			}

			{
			for (int iedit=0;iedit<m_ptable->m_vedit.Size();iedit++)
				{
				IEditable * const pie = m_ptable->m_vedit.ElementAt(iedit);
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

		ppin3d->ClipRectToVisibleArea(&m_pobjframe[i]->rc);
		m_pobjframe[i]->pdds = ppin3d->CreateOffscreen(m_pobjframe[i]->rc.right - m_pobjframe[i]->rc.left, m_pobjframe[i]->rc.bottom - m_pobjframe[i]->rc.top);

		if (m_pobjframe[i]->pdds == NULL)
			{
			ppin3d->WriteObjFrameToCacheFile(NULL);
			continue;
			}
			
		m_pobjframe[i]->pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &m_pobjframe[i]->rc, DDBLT_WAIT, NULL);

		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{
			// Create the D3D texture that we will blit.
			Display_CreateTexture ( g_pplayer->m_pin3d.m_pd3dDevice, g_pplayer->m_pin3d.m_pDD, NULL, (m_pobjframe[i]->rc.right - m_pobjframe[i]->rc.left), (m_pobjframe[i]->rc.bottom - m_pobjframe[i]->rc.top), &(m_pobjframe[i]->pTexture), &(m_pobjframe[i]->u), &(m_pobjframe[i]->v) );
			Display_CopyTexture ( g_pplayer->m_pin3d.m_pd3dDevice, m_pobjframe[i]->pTexture, &(m_pobjframe[i]->rc), ppin3d->m_pddsBackTextureBuffer );
			}

		ppin3d->WriteObjFrameToCacheFile(m_pobjframe[i]);

		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&m_pobjframe[i]->rc, NULL,
				&m_pobjframe[i]->rc, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
		}

	for (int i=0;i<vtri.Size();i++)
		{
		delete vtri.ElementAt(i);
		}

	delete rgv;

	ppin3d->SetTexture(NULL);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}

void Light::RenderMoversFromCache(Pin3D *ppin3d)
	{
	for (int i=0;i<2;i++)
		{
		m_pobjframe[i] = new ObjFrame();
		ppin3d->ReadObjFrameFromCacheFile(m_pobjframe[i]);
		}
	}

void Light::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Pin3D * const ppin3d = &g_pplayer->m_pin3d;
	const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
	const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
	const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);

	if (m_d.m_shape == ShapeCustom)
		{
		RenderCustomMovers(pd3dDevice);
		return;
		}

	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

	const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	ppin3d->SetTexture(ppin3d->m_pddsLightTexture);

	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);

	//pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

	Vertex3D rgv3D[32];
	for (int l=0;l<32;l++)
		{
		const float angle = (float)(M_PI*2.0/32.0)*(float)l;
		rgv3D[l].x = m_d.m_vCenter.x + sinf(angle)*m_d.m_radius;
		rgv3D[l].y = m_d.m_vCenter.y - cosf(angle)*m_d.m_radius;
		rgv3D[l].z = height + 0.1f;

		rgv3D[l].tu = 0.5f + sinf(angle)*0.5f;
		rgv3D[l].tv = 0.5f + cosf(angle)*0.5f;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		}

	WORD rgi[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

	if (!m_fBackglass)
		{
		SetNormal(rgv3D, rgi, 32, NULL, NULL, 0);
		}
	else
		{
		SetHUDVertices(rgv3D, 32);
		}

	for (int i=0;i<2;i++)
		{
		switch (i)
			{
			case 0:
				ppin3d->EnableLightMap(!m_fBackglass, height);
				mtrl.diffuse.r = mtrl.ambient.r = r*(float)(1.0/3.0);
				mtrl.diffuse.g = mtrl.ambient.g = g*(float)(1.0/3.0);
				mtrl.diffuse.b = mtrl.ambient.b = b*(float)(1.0/3.0);
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

		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{			
			// Clear the texture by copying the color and z values from the "static" buffers.
			Display_ClearTexture ( g_pplayer->m_pin3d.m_pd3dDevice, ppin3d->m_pddsBackTextureBuffer, (char) 0x00 );
			ppin3d->m_pddsZTextureBuffer->BltFast(m_pobjframe[i]->rc.left, m_pobjframe[i]->rc.top, ppin3d->m_pddsStaticZ, &(m_pobjframe[i]->rc), DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
			}

		ppin3d->ClipRectToVisibleArea(&m_pobjframe[i]->rc);

		m_pobjframe[i]->pdds = ppin3d->CreateOffscreen(m_pobjframe[i]->rc.right - m_pobjframe[i]->rc.left, m_pobjframe[i]->rc.bottom - m_pobjframe[i]->rc.top);

		if (m_pobjframe[i]->pdds == NULL)
			{
			ppin3d->WriteObjFrameToCacheFile(NULL);
			continue;
			}

		if (!m_fBackglass)
			{
			Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 32,
														  rgi, 32, 0);
			}
		else 
			if( GetPTable()->GetDecalsEnabled() )
				{
				Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
														  rgv3D, 32,
														  rgi, 32, 0);
				}

			{
			for (int iedit=0;iedit<m_ptable->m_vedit.Size();iedit++)
				{
				IEditable * const pie = m_ptable->m_vedit.ElementAt(iedit);
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

		m_pobjframe[i]->pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &m_pobjframe[i]->rc, DDBLT_WAIT, NULL);

		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{
			// Create the D3D texture that we will blit.
			Display_CreateTexture ( g_pplayer->m_pin3d.m_pd3dDevice, g_pplayer->m_pin3d.m_pDD, NULL, (m_pobjframe[i]->rc.right - m_pobjframe[i]->rc.left), (m_pobjframe[i]->rc.bottom - m_pobjframe[i]->rc.top), &(m_pobjframe[i]->pTexture), &(m_pobjframe[i]->u), &(m_pobjframe[i]->v) );
			Display_CopyTexture ( g_pplayer->m_pin3d.m_pd3dDevice, m_pobjframe[i]->pTexture, &(m_pobjframe[i]->rc), ppin3d->m_pddsBackTextureBuffer );
			}

		ppin3d->WriteObjFrameToCacheFile(m_pobjframe[i]);

		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;//0xffff;
		ppin3d->m_pddsBackBuffer->Blt(&m_pobjframe[i]->rc, NULL,
				&m_pobjframe[i]->rc, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
		}

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

	for (int i=0;i<m_vdpoint.Size();i++)
		{
		CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

		pdp->m_v.x += dx;
		pdp->m_v.y += dy;
		}

	m_ptable->SetDirtyDraw();
	}

HRESULT Light::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
	bw.WriteFloat(FID(RADI), m_d.m_radius);
	bw.WriteInt(FID(STAT), m_d.m_state);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteBool(FID(DISP), m_d.m_fDisplayImage);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteInt(FID(SHAP), m_d.m_shape);
	bw.WriteString(FID(BPAT), m_rgblinkpattern);
	bw.WriteString(FID(IMG1), m_d.m_szOffImage);
	bw.WriteString(FID(IMG2), m_d.m_szOnImage);
	bw.WriteInt(FID(BINT), m_blinkinterval);
	bw.WriteInt(FID(BCOL), m_d.m_bordercolor);
	bw.WriteFloat(FID(BWTH), m_d.m_borderwidth);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

	bw.WriteBool(FID(BGLS), m_fBackglass);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	//bw.WriteTag(FID(PNTS));
	HRESULT hr;
	if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
		return hr;

	bw.WriteTag(FID(ENDB));

	return S_OK;
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

	m_fLockedByLS = false;			//>>> added by chris
	m_realState	= m_d.m_state;		//>>> added by chris

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
		pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D));
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
//		if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(IMG1))
		{
		pbr->GetString(m_d.m_szOffImage);
		}
	else if (id == FID(IMG2))
		{
		pbr->GetString(m_d.m_szOnImage);
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(DISP))
		{
		pbr->GetBool(&m_d.m_fDisplayImage);
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
//		if (!(m_d.m_bordercolor & MINBLACKMASK)) {m_d.m_bordercolor |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(BWTH))
		{
		pbr->GetFloat(&m_d.m_borderwidth);
		}
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

void Light::GetPointCenter(Vertex2D *pv)
	{
	*pv = m_d.m_vCenter;
	}

void Light::PutPointCenter(Vertex2D *pv)
	{
	m_d.m_vCenter = *pv;

	SetDirtyDraw();
	}

void Light::EditMenu(HMENU hmenu)
	{
	EnableMenuItem(hmenu, ID_WALLMENU_FLIP, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
	EnableMenuItem(hmenu, ID_WALLMENU_MIRROR, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
	EnableMenuItem(hmenu, ID_WALLMENU_ROTATE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
	EnableMenuItem(hmenu, ID_WALLMENU_SCALE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
	EnableMenuItem(hmenu, ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
	}

void Light::DoCommand(int icmd, int x, int y)
	{
	ISelect::DoCommand(icmd, x, y);

	switch (icmd)
		{
		case ID_WALLMENU_FLIP:
			{
			Vertex2D vCenter;
			GetPointCenter(&vCenter);
			FlipPointY(&vCenter);
			}
			break;

		case ID_WALLMENU_MIRROR:
			{
			Vertex2D vCenter;
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

			RECT rc;
			GetClientRect(m_ptable->m_hwnd, &rc);
			Vertex2D v, vOut;
			int iSeg;

			HitSur * const phs = new HitSur(NULL, m_ptable->m_zoom, m_ptable->m_offsetx, m_ptable->m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

			phs->ScreenToSurface(x, y, &v.x, &v.y);
			delete phs;

			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			const int cvertex = vvertex.Size();
			Vertex2D * const rgv = new Vertex2D[cvertex];

			for (int i=0;i<vvertex.Size();i++)
				{
				rgv[i] = *((Vertex2D *)vvertex.ElementAt(i));
				}

			ClosestPointOnPolygon(rgv, cvertex, &v, &vOut, &iSeg, fTrue);

			// Go through vertices (including iSeg itself) counting control points until iSeg
			int icp = 0;
			for (int i=0;i<(iSeg+1);i++)
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

			for (int i=0;i<vvertex.Size();i++)
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
	// if the light is locked by the LS then just change the state and don't change the actual light
	if (!m_fLockedByLS)
		{
		setLightState(newVal);
		}
	m_d.m_state = newVal;

	STOPUNDO

	return S_OK;
}

void Light::DrawFrame(BOOL fOn)
	{
	Pin3D * const ppin3d = &g_pplayer->m_pin3d;

	const int frame = fOn;

	// Light might be off the screen and have no image
	// Check if we are blitting with D3D.

#if 0 // Switching render targets mid-frame kills performance.   - JEP

	if (g_pvp->m_pdd.m_fUseD3DBlit)		
	{
		// Make sure we have a D3D texture.
		if (m_pobjframe[frame]->pTexture)
		{
			// Direct all renders to the "static" buffer.
			g_pplayer->m_pin3d.SetRenderTarget(g_pplayer->m_pin3d.m_pddsStatic);

			// Blit with D3D.
			Display_DrawSprite(g_pplayer->m_pin3d.m_pd3dDevice, 
							(float) m_pobjframe[frame]->rc.left, (float) m_pobjframe[frame]->rc.top, 
							(float) (m_pobjframe[frame]->rc.right - m_pobjframe[frame]->rc.left), (float) (m_pobjframe[frame]->rc.bottom - m_pobjframe[frame]->rc.top), 
							1.0f, 1.0f, 1.0f, 1.0f, 
							0.0f, 
							m_pobjframe[frame]->pTexture, m_pobjframe[frame]->u, m_pobjframe[frame]->v, 
							DISPLAY_TEXTURESTATE_NOFILTER, DISPLAY_RENDERSTATE_TRANSPARENT);

			g_pplayer->InvalidateRect(&m_pobjframe[frame]->rc);

			// Direct all renders to the back buffer.
			g_pplayer->m_pin3d.SetRenderTarget(g_pplayer->m_pin3d.m_pddsBackBuffer);
		}
	}
	else
#endif
	{
		// Make sure we have a DDraw surface. 
		if (m_pobjframe[frame]->pdds != NULL)
		{
			// NOTE: They are drawing to their own static buffer below in the BltFast... NOT the back buffer!
			// We can use BltFast here because we are drawing to our own offscreen iamge
			const HRESULT hr = ppin3d->m_pddsStatic->BltFast(m_pobjframe[frame]->rc.left, m_pobjframe[frame]->rc.top, m_pobjframe[frame]->pdds, NULL, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
			
			g_pplayer->InvalidateRect(&m_pobjframe[frame]->rc);
		}
	}
}

void Light::FlipY(Vertex2D *pvCenter)
	{
	IHaveDragPoints::FlipPointY(pvCenter);
	}

void Light::FlipX(Vertex2D *pvCenter)
	{
	IHaveDragPoints::FlipPointX(pvCenter);
	}

void Light::Rotate(float ang, Vertex2D *pvCenter)
	{
	IHaveDragPoints::RotatePoints(ang, pvCenter);
	}

void Light::Scale(float scalex, float scaley, Vertex2D *pvCenter)
	{
	IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
	}

void Light::Translate(Vertex2D *pvOffset)
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

	m_d.m_color = newVal;

	STOPUNDO

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
		const float x = m_d.m_vCenter.x;
		const float y = m_d.m_vCenter.y;

		CComObject<DragPoint> *pdp;
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x-30.0f, y-30.0f);
			m_vdpoint.AddElement(pdp);
			}
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x-30.0f, y+30.0f);
			m_vdpoint.AddElement(pdp);
			}
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x+30.0f, y+30.0f);
			m_vdpoint.AddElement(pdp);
			}
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, x+30.0f, y-30.0f);
			m_vdpoint.AddElement(pdp);
			}

		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Light::get_BlinkPattern(BSTR *pVal)
{
	WCHAR wz[512];

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
		const char cold = m_rgblinkpattern[m_iblinkframe];
		m_iblinkframe = 0;
		const char cnew = m_rgblinkpattern[m_iblinkframe];
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
	WCHAR wz[512];

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


STDMETHODIMP Light::get_OffImage(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szOffImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Light::put_OffImage(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szOffImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}


STDMETHODIMP Light::get_OnImage(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szOnImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Light::put_OnImage(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szOnImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}


STDMETHODIMP Light::get_DisplayImage(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fDisplayImage);

	return S_OK;
}

STDMETHODIMP Light::put_DisplayImage(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fDisplayImage = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

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
		m_fLockedByLS = true;
	}

void Light::unLockLight()
	{
		m_fLockedByLS = false;
	}

void Light::setLightStateBypass(const LightState newVal)
	{
		lockLight();
		setLightState(newVal);
	}

void Light::setLightState(const LightState newVal)
	{
   	if (newVal != m_realState)//state changed???
   		{
   		const LightState lastState = m_realState;		//rlc make a bit more obvious
		m_realState = newVal;

   		if (g_pplayer)
   			{
   			if (lastState == LightStateBlinking)
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
