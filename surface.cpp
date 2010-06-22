// Surface.cpp : Implementation of Surface
#include "StdAfx.h"

//////////////////////////////////////////////////////////////////
// Surface

Surface::Surface()
	{
	m_rgvT = NULL;

	m_menuid = IDR_SURFACEMENU;

	m_phitdrop = NULL;
	m_d.m_fCollidable = fTrue;
	m_d.m_fSlingshotAnimation = fTrue;
	}

Surface::~Surface()
	{
	}

HRESULT Surface::Init(PinTable *ptable, float x, float y)
	{
	m_ptable = ptable;

	CComObject<DragPoint> *pdp;
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x-50.0f, y-50.0f);
		m_vdpoint.AddElement(pdp);
		}
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x-50.0f, y+50.0f);
		m_vdpoint.AddElement(pdp);
		}
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x+50.0f, y+50.0f);
		m_vdpoint.AddElement(pdp);
		}
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x+50.0f, y-50.0f);
		m_vdpoint.AddElement(pdp);
		}

	SetDefaults();

	return InitVBA(fTrue, 0, NULL);
	}

HRESULT Surface::InitTarget(PinTable * const ptable, const float x, const float y)
	{
	m_ptable = ptable;

	CComObject<DragPoint> *pdp;
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x-30.0f, y-6.0f);
		m_vdpoint.AddElement(pdp);
		}
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x-30.0f, y+6.0f);
		pdp->m_fAutoTexture = fFalse;
		pdp->m_texturecoord = 0.0f;
		m_vdpoint.AddElement(pdp);
		}
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x+30.0f, y+6.0f);
		pdp->m_fAutoTexture = fFalse;
		pdp->m_texturecoord = 1.0f;
		m_vdpoint.AddElement(pdp);
		}
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x+30.0f, y-6.0f);
		m_vdpoint.AddElement(pdp);
		}

	SetDefaults();

	m_d.m_fHitEvent = fTrue;

	m_d.m_sidecolor = RGB(127,127,127);
	m_d.m_topcolor = RGB(127,127,127);
	m_d.m_slingshotColor = RGB(242,242,242);

	return InitVBA(fTrue, 0, NULL);
	}

void Surface::SetDefaults()
	{
	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_fHitEvent = fFalse;
	m_d.m_threshold = 1.0f;
	m_d.m_slingshot_threshold = 0.0f;

	m_d.m_fInner = fTrue;

	m_d.m_ia = ImageAlignCenter;
	m_d.m_sidecolor = RGB(255,255,255);
	m_d.m_szImage[0] = 0;
	m_d.m_slingshotColor = RGB(242,242,242);

	m_d.m_topcolor = RGB(63,63,63);

	m_d.m_fDroppable = fFalse;
	m_d.m_fFlipbook = fFalse;
	m_d.m_fFloor = fFalse;
	m_d.m_fCastsShadow = fTrue;

	m_d.m_heightbottom = 0;
	m_d.m_heighttop = 50.0f;

	m_d.m_fDisplayTexture = fFalse;

	m_d.m_slingshotforce = 80.0f;
	
	m_d.m_fSlingshotAnimation = fTrue;

	m_d.m_elasticity = 0.3f;
	m_d.m_friction = 0;	//zero uses global value
	m_d.m_scatter = 0;	//zero uses global value

	m_d.m_fVisible = fTrue;
	m_d.m_fSideVisible = fTrue;
	m_d.m_fCollidable = fTrue;
	}


void Surface::PreRender(Sur *psur)
	{
	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	m_cvertexT = vvertex.Size();
	m_rgvT = new Vertex2D[m_cvertexT + 6]; // Add points so inverted polygons can be drawn

	for (int i=0;i<vvertex.Size();i++)
		{
		m_rgvT[i] = *((Vertex2D *)vvertex.ElementAt(i));
		delete vvertex.ElementAt(i);
		}

	psur->SetFillColor(RGB(192,192,192));

	psur->SetObject(this);

	// Don't want border color to be over-ridden when selected - that will be drawn later
	psur->SetBorderColor(-1,fFalse,0);

	int cvertex;
	if (!m_d.m_fInner)
		{
		m_rgvT[m_cvertexT].x = m_ptable->m_left;
		m_rgvT[m_cvertexT].y = m_ptable->m_top;
		m_rgvT[m_cvertexT+1].x = m_ptable->m_left;
		m_rgvT[m_cvertexT+1].y = m_ptable->m_bottom;
		m_rgvT[m_cvertexT+2].x = m_ptable->m_right;
		m_rgvT[m_cvertexT+2].y = m_ptable->m_bottom;
		m_rgvT[m_cvertexT+3].x = m_ptable->m_right;
		m_rgvT[m_cvertexT+3].y = m_ptable->m_top;
		m_rgvT[m_cvertexT+4].x = m_ptable->m_left;
		m_rgvT[m_cvertexT+4].y = m_ptable->m_top;
		m_rgvT[m_cvertexT+5].x = m_rgvT[m_cvertexT-1].x;
		m_rgvT[m_cvertexT+5].y = m_rgvT[m_cvertexT-1].y;

		cvertex = m_cvertexT + 6;
		}
	else
		{
		cvertex = m_cvertexT;
		}

	PinImage *ppi;
	if (m_d.m_fDisplayTexture && (ppi = m_ptable->GetImage(m_d.m_szImage)))
		{
		ppi->EnsureHBitmap();
		if (ppi->m_hbmGDIVersion)
			{
			psur->PolygonImage(m_rgvT, cvertex, ppi->m_hbmGDIVersion, m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);
			}
		else
			{
			// Do nothing for now to indicate to user that there is a problem
			}
		}
	else
		{
		psur->Polygon(m_rgvT, cvertex);
		}
	}

void Surface::Render(Sur *psur)
	{
	psur->SetFillColor(-1);
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetObject(this); // For selected formatting
	psur->SetObject(NULL);

	// PreRender may not have been called - for export
	if (!m_rgvT)
		{
		Vector<RenderVertex> vvertex;
		GetRgVertex(&vvertex);

		m_cvertexT = vvertex.Size();
		m_rgvT = new Vertex2D[m_cvertexT];

		for (int i=0;i<vvertex.Size();i++)
			{
			m_rgvT[i] = *((Vertex2D *)vvertex.ElementAt(i));
			delete vvertex.ElementAt(i);
			}
		//m_rgvT = GetRgVertex(&m_cvertexT);
		}

	psur->Polygon(m_rgvT, m_cvertexT);
	delete m_rgvT;
	m_rgvT = NULL;

	// if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
	bool fDrawDragpoints;		//>>> added by chris

	if ( (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints) )
		{
		fDrawDragpoints = true;
		}
	else
		{
		// if any of the dragpoints of this object are selected then draw all the dragpoints
		fDrawDragpoints = false;
		for (int i=0;i<m_vdpoint.Size();i++)
			{
			const CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
			if (pdp->m_selectstate != eNotSelected)
				{
				fDrawDragpoints = true;
				break;
				}
			}
		}

	for (int i=0;i<m_vdpoint.Size();i++)
		{
		CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
		psur->SetFillColor(-1);
		psur->SetBorderColor(RGB(255,0,0),fFalse,0);

		if (pdp->m_fDragging)
			{
			//psur->SetFillColor(RGB(0,255,0));
			psur->SetBorderColor(RGB(0,255,0),fFalse,0);
			}

		if (fDrawDragpoints)
			{
			psur->SetObject(pdp);
			psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
			}

		if (pdp->m_fSlingshot)
			{
			psur->SetObject(NULL);
			const CComObject<DragPoint> * const pdp2 = m_vdpoint.ElementAt((i+1) % m_vdpoint.Size());

			psur->SetLineColor(RGB(0,0,0),fFalse,3);
			psur->Line(pdp->m_v.x, pdp->m_v.y, pdp2->m_v.x, pdp2->m_v.y);
			}
		}
	}

void Surface::RenderBlueprint(Sur *psur)
	{
	// Don't render dragpoints for blueprint

	psur->SetFillColor(-1);
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetObject(this); // For selected formatting
	psur->SetObject(NULL);

	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	m_cvertexT = vvertex.Size();
	m_rgvT = new Vertex2D[m_cvertexT];

	for (int i=0;i<vvertex.Size();i++)
		{
		m_rgvT[i] = *((Vertex2D *)vvertex.ElementAt(i));
		delete vvertex.ElementAt(i);
		}

	psur->Polygon(m_rgvT, m_cvertexT);
	delete m_rgvT;
	m_rgvT = NULL;
	}

void Surface::RenderShadow(ShadowSur *psur, float height)
	{	
	if ( (!m_d.m_fCastsShadow) || (!m_ptable->m_fRenderShadows) )
		return;

	psur->SetFillColor(RGB(0,0,0));
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetObject(this); // For selected formatting
	psur->SetObject(NULL);

	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	m_cvertexT = vvertex.Size();
	m_rgvT = new Vertex2D[m_cvertexT+6];

	for (int i=0;i<vvertex.Size();i++)
		{
		m_rgvT[i] = *((Vertex2D *)vvertex.ElementAt(i));
		delete vvertex.ElementAt(i);
		}

	if (!m_d.m_fInner)
		{
		m_rgvT[m_cvertexT].x = m_ptable->m_left;
		m_rgvT[m_cvertexT].y = m_ptable->m_top;
		m_rgvT[m_cvertexT+1].x = m_ptable->m_left;
		m_rgvT[m_cvertexT+1].y = m_ptable->m_bottom;
		m_rgvT[m_cvertexT+2].x = m_ptable->m_right;
		m_rgvT[m_cvertexT+2].y = m_ptable->m_bottom;
		m_rgvT[m_cvertexT+3].x = m_ptable->m_right;
		m_rgvT[m_cvertexT+3].y = m_ptable->m_top;
		m_rgvT[m_cvertexT+4].x = m_ptable->m_left;
		m_rgvT[m_cvertexT+4].y = m_ptable->m_top;
		m_rgvT[m_cvertexT+5].x = m_rgvT[m_cvertexT-1].x;
		m_rgvT[m_cvertexT+5].y = m_rgvT[m_cvertexT-1].y;

		psur->PolygonSkew(m_rgvT, m_cvertexT+6, NULL, m_d.m_heightbottom, m_d.m_heighttop, false);
		}
	else
		{
		psur->PolygonSkew(m_rgvT, m_cvertexT  , NULL, m_d.m_heightbottom, m_d.m_heighttop, false);
		}

	delete m_rgvT;
	m_rgvT = NULL;
	}

void Surface::GetTimers(Vector<HitTimer> *pvht)
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

void Surface::GetHitShapes(Vector<HitObject> *pvho)
	{
	CurvesToShapes(pvho);

	m_fIsDropped = fFalse;
	m_fDisabled = fFalse;
	}

void Surface::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	if (!m_d.m_fInner)
		{
		Vector<RenderVertex> vvertex;
		GetRgVertex(&vvertex);

		const int cvertex = vvertex.Size();
		Vertex3D * const rgv3d = new Vertex3D[cvertex + 5];

		for (int i=0;i<cvertex;i++)
			{
			rgv3d[i].x = vvertex.ElementAt(i)->x;
			rgv3d[i].y = vvertex.ElementAt(i)->y;
			rgv3d[i].z = m_d.m_heighttop;
			delete vvertex.ElementAt(i);
			}

		rgv3d[cvertex].x = m_ptable->m_left;
		rgv3d[cvertex].y = m_ptable->m_top;
		rgv3d[cvertex].z = m_d.m_heighttop;
		rgv3d[cvertex+1].x = m_ptable->m_left;
		rgv3d[cvertex+1].y = m_ptable->m_bottom;
		rgv3d[cvertex+1].z = m_d.m_heighttop;
		rgv3d[cvertex+2].x = m_ptable->m_right;
		rgv3d[cvertex+2].y = m_ptable->m_bottom;
		rgv3d[cvertex+2].z = m_d.m_heighttop;
		rgv3d[cvertex+3].x = m_ptable->m_right;
		rgv3d[cvertex+3].y = m_ptable->m_top;
		rgv3d[cvertex+3].z = m_d.m_heighttop;
		rgv3d[cvertex+4].x = m_ptable->m_left;
		rgv3d[cvertex+4].y = m_ptable->m_top;
		rgv3d[cvertex+4].z = m_d.m_heighttop;

		Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex+5, true);
		pvho->AddElement(ph3dp);

		m_vhoCollidable.AddElement(ph3dp);
		ph3dp->m_fEnabled = m_d.m_fCollidable;
		}
	}

void Surface::CurvesToShapes(Vector<HitObject> * const pvho)
	{
	//rgv = GetRgRenderVertex(&count);

	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	const int count = vvertex.Size();
	RenderVertex * const rgv = new RenderVertex[count + 6]; // Add points so inverted polygons can be drawn
	Vertex3D * const rgv3D = new Vertex3D[count + 6];

	if (m_d.m_fInner)
		{
		for (int i=0;i<count;i++)
			{
			rgv3D[i].x = vvertex.ElementAt(i)->x;
			rgv3D[i].y = vvertex.ElementAt(i)->y;
			rgv3D[i].z = m_d.m_heighttop;
			}
		}

	for (int i=0;i<count;i++)
		{
		rgv[i] = *vvertex.ElementAt(i);
		delete vvertex.ElementAt(i);
		}

	for (int i=0;i<count;i++)
		{
		const RenderVertex * const pv1 = &rgv[i];
		const RenderVertex * const pv2 = &rgv[(i+1) % count];
		const RenderVertex * const pv3 = &rgv[(i+2) % count];
		const RenderVertex * const pv4 = &rgv[(i+3) % count];

		if (m_d.m_fInner)
			{
			AddLine(pvho, pv2, pv3, pv1, pv2->fSlingshot);
			}
		else
			{
			AddLine(pvho, pv3, pv2, pv4, pv2->fSlingshot);
			}
		}

	if (m_d.m_fInner)
		{
		if (m_d.m_fDroppable)
			{
			// Special hit object that will allow us to animate the surface
			m_phitdrop = new Hit3DPolyDrop(rgv3D,count,true);
			m_phitdrop->m_pfe = (IFireEvents *)this;

			m_phitdrop->m_fVisible = fTrue;

			m_phitdrop->m_polydropanim.m_iframedesire = 0;

			pvho->AddElement(m_phitdrop);			

			m_vhoDrop.AddElement(m_phitdrop);	

			m_vhoCollidable.AddElement(m_phitdrop);
			m_phitdrop->m_fEnabled = m_d.m_fCollidable;
			}
		else
			{
			Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,count,true);
			ph3dpoly->m_pfe = (IFireEvents *)this;

			ph3dpoly->m_fVisible = fTrue;

			pvho->AddElement(ph3dpoly);

			m_vhoCollidable.AddElement(ph3dpoly);
			ph3dpoly->m_fEnabled = m_d.m_fCollidable;
			}
		}
	else
		delete rgv3D;

	delete rgv;
	}

void Surface::AddLine(Vector<HitObject> * const pvho, const RenderVertex * const pv1, const RenderVertex * const pv2, const RenderVertex * const pv3, const bool fSlingshot)
	{
	LineSeg *plineseg;
	
	if (!fSlingshot)
		{
		plineseg = new LineSeg();

		if (m_d.m_fHitEvent)
			{
			plineseg->m_pfe = (IFireEvents *)this;
			plineseg->m_threshold = m_d.m_threshold;
			}
		else
			{
			plineseg->m_pfe = NULL;
			}
		}
	else
		{
		LineSegSlingshot * const plinesling = new LineSegSlingshot();
		plineseg = (LineSeg *)plinesling;

		// Slingshots always have hit events
		plineseg->m_pfe = (IFireEvents *)this;
		plineseg->m_threshold = m_d.m_threshold;

		plinesling->m_force = m_d.m_slingshotforce;
		plinesling->m_psurface = this;

		m_vlinesling.AddElement(plinesling);
		}

	plineseg->m_rcHitRect.zlow = m_d.m_heightbottom;
	plineseg->m_rcHitRect.zhigh = m_d.m_heighttop;

	plineseg->v1.x = pv1->x;
	plineseg->v1.y = pv1->y;
	plineseg->v2.x = pv2->x;
	plineseg->v2.y = pv2->y;

	plineseg->m_elasticity = m_d.m_elasticity;
	plineseg->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
	plineseg->m_scatter = ANGTORAD(m_d.m_scatter);
	
	pvho->AddElement(plineseg);
	if (m_d.m_fDroppable)
		{
		m_vhoDrop.AddElement(plineseg);
		}

	m_vhoCollidable.AddElement(plineseg);
	plineseg->m_fEnabled = m_d.m_fCollidable;

	plineseg->CalcNormal();

	Vertex2D vt1, vt2;
	vt1.x = pv1->x - pv2->x;
	vt1.y = pv1->y - pv2->y;

	vt2.x = pv1->x - pv3->x;
	vt2.y = pv1->y - pv3->y;

	const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

	if (dot < 0) // Inside edges don't need joint hit-testing (dot == 0 continuous segments should mathematically never hit)
		{
		Joint * const pjoint = new Joint();

		if (m_d.m_fHitEvent)
			{
			pjoint->m_pfe = (IFireEvents *)this;
			pjoint->m_threshold = m_d.m_threshold;
			}
		else
			{
			pjoint->m_pfe = NULL;
			}

		pjoint->m_rcHitRect.zlow = m_d.m_heightbottom;
		pjoint->m_rcHitRect.zhigh = m_d.m_heighttop;

		pjoint->m_elasticity = m_d.m_elasticity;
		pjoint->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
		pjoint->m_scatter = ANGTORAD(m_d.m_scatter);

		pjoint->center.x = pv1->x;
		pjoint->center.y = pv1->y;
		pvho->AddElement(pjoint);
		if (m_d.m_fDroppable)
			{
			m_vhoDrop.AddElement(pjoint);
			}

		m_vhoCollidable.AddElement(pjoint);
		pjoint->m_fEnabled = m_d.m_fCollidable;

		Vertex2D normalT;

		// Set up line normal
		{
		const float inv_length = 1.0f/sqrtf((vt2.x * vt2.x) + (vt2.y * vt2.y));
		normalT.x = -vt2.y * inv_length;
		normalT.y = vt2.x * inv_length;
		}

		pjoint->normal.x = normalT.x + plineseg->normal.x;
		pjoint->normal.y = normalT.y + plineseg->normal.y;

		// Set up line normal
		{
		const float inv_length = 1.0f/sqrtf((pjoint->normal.x * pjoint->normal.x) + (pjoint->normal.y * pjoint->normal.y));
		pjoint->normal.x *= inv_length;
		pjoint->normal.y *= inv_length;
		}
		}
	}

void Surface::GetBoundingVertices(Vector<Vertex3D> *pvvertex3D)
	{
	const float top = m_d.m_heighttop;
	const float bottom = m_d.m_heightbottom;

	for (int i=0;i<8;i++)
		{
		Vertex3D * const pv = new Vertex3D();
		pv->x = i&1 ? m_ptable->m_right : m_ptable->m_left;
		pv->y = i&2 ? m_ptable->m_bottom : m_ptable->m_top;
		pv->z = i&4 ? top : bottom;
		pvvertex3D->AddElement(pv);
		}
	}

void Surface::EndPlay()
	{
	IEditable::EndPlay();

	if (m_phitdrop) // Failed Player Case
		{
		if (m_d.m_fDroppable)
			{
			for (int i=0;i<2;i++)
				{
				delete m_phitdrop->m_polydropanim.m_pobjframe[i];
				}
			}

		m_phitdrop = NULL;
		}

	m_vlinesling.RemoveAllElements();
	m_vhoDrop.RemoveAllElements();
	m_vhoCollidable.RemoveAllElements();
	}

void Surface::MoveOffset(float dx, float dy)
	{
	for (int i=0;i<m_vdpoint.Size();i++)
		{
		CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

		pdp->m_v.x += dx;
		pdp->m_v.y += dy;
		}

	m_ptable->SetDirtyDraw();
	}


void Surface::PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}


void Surface::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	if (!m_d.m_fDroppable || !m_d.m_fInner)
		{
		RenderWallsAtHeight(pd3dDevice, fFalse, fFalse);
		}
	}

void Surface::RenderSlingshots(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Pin3D * const ppin3d = &g_pplayer->m_pin3d;

	const float slingbottom = ((m_d.m_heighttop - m_d.m_heightbottom) * 0.2f) + m_d.m_heightbottom;
	const float slingtop = ((m_d.m_heighttop - m_d.m_heightbottom) * 0.8f) + m_d.m_heightbottom;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );

	for (int i=0;i<m_vlinesling.Size();i++)
		{
		LineSegSlingshot * const plinesling = m_vlinesling.ElementAt(i);
		
		plinesling->m_slingshotanim.m_fAnimations = (m_d.m_fSlingshotAnimation != 0); //rlc

		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET,0x00000000, 1.0f, 0L );

		ppin3d->m_pddsZBuffer->Blt(NULL, ppin3d->m_pddsStaticZ, NULL, DDBLT_WAIT, NULL);

		const float r = (m_d.m_slingshotColor & 255) * (float)(1.0/255.0);
		const float g = (m_d.m_slingshotColor & 65280) * (float)(1.0/65280.0);
		const float b = (m_d.m_slingshotColor & 16711680) * (float)(1.0/16711680.0);

		mtrl.diffuse.r = mtrl.ambient.r = r;
		mtrl.diffuse.g = mtrl.ambient.g = g;
		mtrl.diffuse.b = mtrl.ambient.b = b;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

		pd3dDevice->SetMaterial(&mtrl);

		ObjFrame * const pof = new ObjFrame();
		
		plinesling->m_slingshotanim.m_pobjframe = pof;		

		ppin3d->ClearExtents(&plinesling->m_slingshotanim.m_rcBounds, &plinesling->m_slingshotanim.m_znear, &plinesling->m_slingshotanim.m_zfar);

		Vertex3D rgv3D[12];
		rgv3D[0].x = plinesling->v1.x;
		rgv3D[0].y = plinesling->v1.y;
		rgv3D[0].z = slingbottom;

		rgv3D[1].x = (plinesling->v1.x + plinesling->v2.x)*0.5f + plinesling->normal.x*(m_d.m_slingshotforce * 0.25f);//40;//20;
		rgv3D[1].y = (plinesling->v1.y + plinesling->v2.y)*0.5f + plinesling->normal.y*(m_d.m_slingshotforce * 0.25f);//20;
		rgv3D[1].z = slingbottom;

		rgv3D[2].x = plinesling->v2.x;
		rgv3D[2].y = plinesling->v2.y;
		rgv3D[2].z = slingbottom;

		for (int l=0;l<3;l++)
			{
			rgv3D[l+3].x = rgv3D[l].x;
			rgv3D[l+3].y = rgv3D[l].y;
			rgv3D[l+3].z = slingtop;
			}

		for (int l=0;l<6;l++)
			{
			rgv3D[l+6].x = rgv3D[l].x - plinesling->normal.x*5.0f;
			rgv3D[l+6].y = rgv3D[l].y - plinesling->normal.y*5.0f;
			rgv3D[l+6].z = rgv3D[l].z;
			}

		for (int l=0;l<12;l++)
			{
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
			}

		ppin3d->ClearExtents(&pof->rc, NULL, NULL);
		ppin3d->ExpandExtents(&pof->rc, rgv3D, &plinesling->m_slingshotanim.m_znear, &plinesling->m_slingshotanim.m_zfar, 6, fFalse);

		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{			
			// Clear the texture by copying the color and z values from the "static" buffers.
			Display_ClearTexture ( g_pplayer->m_pin3d.m_pd3dDevice, ppin3d->m_pddsBackTextureBuffer, (char) 0x00 );
			ppin3d->m_pddsZTextureBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
			}

		pof->pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
		pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);

		WORD rgi[4] = {0,1,4,3};

		SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

		Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 12,rgi, 4, 0);

		rgi[0] = 1;
		rgi[1] = 2;
		rgi[2] = 5;
		rgi[3] = 4;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

		Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 12,rgi, 4, 0);

		rgi[0] = 0;
		rgi[3] = 1;
		rgi[2] = 4;
		rgi[1] = 3;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

		Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 12,rgi, 4, 0);

		rgi[0] = 1;
		rgi[3] = 2;
		rgi[2] = 5;
		rgi[1] = 4;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

		Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 12,rgi, 4, 0);

		rgi[0] = 3;
		rgi[1] = 9;
		rgi[2] = 10;
		rgi[3] = 4;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

		Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 12,rgi, 4, 0);

		rgi[0] = 4;
		rgi[1] = 10;
		rgi[2] = 11;
		rgi[3] = 5;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

		Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 12,rgi, 4, 0);

		pof->pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &pof->rc, DDBLT_WAIT, NULL);
		pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
		
		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{
			// Create the D3D texture that we will blit.
			Display_CreateTexture ( g_pplayer->m_pin3d.m_pd3dDevice, g_pplayer->m_pin3d.m_pDD, NULL, (pof->rc.right - pof->rc.left), (pof->rc.bottom - pof->rc.top), &(pof->pTexture), &(pof->u), &(pof->v) );
			Display_CopyTexture ( g_pplayer->m_pin3d.m_pd3dDevice, pof->pTexture, &(pof->rc), ppin3d->m_pddsBackTextureBuffer );
			}

		ppin3d->ExpandRectByRect(&plinesling->m_slingshotanim.m_rcBounds, &pof->rc);

		ppin3d->WriteAnimObjectToCacheFile(&plinesling->m_slingshotanim, &plinesling->m_slingshotanim.m_pobjframe, 1);

		// reset the portion of the z-buffer that we changed
		ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,&pof->rc, DDBLT_COLORFILL | DDBLT_WAIT,&ddbltfx);
		}
	}

ObjFrame *Surface::RenderWallsAtHeight(LPDIRECT3DDEVICE7 pd3dDevice, BOOL fMover, BOOL fDrop)
	{
	ObjFrame *pof = NULL;

	Pin3D * const ppin3d = &g_pplayer->m_pin3d;

	if (fMover)
		{
		pof = new ObjFrame();

		ppin3d->ClearExtents(&m_phitdrop->m_polydropanim.m_rcBounds, &m_phitdrop->m_polydropanim.m_znear, &m_phitdrop->m_polydropanim.m_zfar);
		ppin3d->ClearExtents(&pof->rc, NULL, NULL);

		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{			
			// Since we don't know the final dimensions of the 
			// object we're rendering, clear the whole buffer.
			RECT Rect;
			Rect.top = 0;
			Rect.left = 0;
			Rect.bottom = g_pplayer->m_pin3d.m_dwRenderHeight - 1;
			Rect.right = g_pplayer->m_pin3d.m_dwRenderWidth - 1;

			// Clear the texture by copying the color and z values from the "static" buffers.
			Display_ClearTexture ( g_pplayer->m_pin3d.m_pd3dDevice, ppin3d->m_pddsBackTextureBuffer, (char) 0x00 );
			ppin3d->m_pddsZTextureBuffer->BltFast(Rect.left, Rect.top, ppin3d->m_pddsStaticZ, &Rect, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
			}
		}

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );

	PinImage * const pinSide = m_ptable->GetImage(m_d.m_szSideImage);
	float maxtuSide, maxtvSide;
	if (pinSide)
		{
		m_ptable->GetTVTU(pinSide, &maxtuSide, &maxtvSide);		

		//rlc add transparent texture support ... replaced this line with >>>>	
		pinSide->EnsureColorKey();
		if (pinSide->m_fTransparent)
			{				
			if (g_pvp->m_pdd.m_fHardwareAccel)
				{
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 128);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 

				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
				pd3dDevice->SetTexture(ePictureTexture, pinSide->m_pdsBufferColorKey);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				}
			else
				{
				pd3dDevice->SetTexture(ePictureTexture, pinSide->m_pdsBufferColorKey);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				}
			}
		else 
			{	
			pd3dDevice->SetTexture(ePictureTexture, pinSide->m_pdsBufferColorKey);     //rlc  alpha channel support		
			pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE); 	
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
			if (g_pvp->m_pdd.m_fHardwareAccel)
				{
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 128);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
				}
			else
				{
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
				}
			pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA); 			
			}

		pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

		mtrl.diffuse.r = mtrl.ambient.r = 1.0f;
		mtrl.diffuse.g = mtrl.ambient.g = 1.0f;
		mtrl.diffuse.b = mtrl.ambient.b = 1.0f;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;
		}
	else
		{
		const float r = (m_d.m_sidecolor & 255) * (float)(1.0/255.0);
		const float g = (m_d.m_sidecolor & 65280) * (float)(1.0/65280.0);
		const float b = (m_d.m_sidecolor & 16711680) * (float)(1.0/16711680.0);

		mtrl.diffuse.r = mtrl.ambient.r = r;
		mtrl.diffuse.g = mtrl.ambient.g = g;
		mtrl.diffuse.b = mtrl.ambient.b = b;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;
		}

	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	float *rgtexcoord = NULL;
	if (pinSide)
		{
		GetTextureCoords(&vvertex, &rgtexcoord);
		}

	const int cvertex = vvertex.Size();
	RenderVertex * const rgv = new RenderVertex[cvertex + 6]; // Add points so inverted polygons can be drawn

	for (int i=0;i<vvertex.Size();i++)
		{
		rgv[i] = *vvertex.ElementAt(i);
		delete vvertex.ElementAt(i);
		}

	Vertex2D * const rgnormal = new Vertex2D[cvertex];

	pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[4] = {0,1,2,3};

	for (int i=0;i<cvertex;i++)
		{
		const RenderVertex * const pv1 = &rgv[i];
		const RenderVertex * const pv2 = &rgv[(i+1) % cvertex];
		const float dx = pv1->x - pv2->x;
		const float dy = pv1->y - pv2->y;

		const float inv_len = 1.0f/sqrtf(dx*dx + dy*dy);

		rgnormal[i].x = dy*inv_len;
		rgnormal[i].y = dx*inv_len;
		}

	ppin3d->EnableLightMap(fTrue, fDrop ? m_d.m_heightbottom : m_d.m_heighttop);
	
		// Render side
	{
		for (int i=0;i<cvertex;i++)
			{
			//RenderVertex *pv0 = &rgv[(i-1+cvertex) % cvertex];
			const RenderVertex * const pv1 = &rgv[i];
			const RenderVertex * const pv2 = &rgv[(i+1) % cvertex];
			//RenderVertex *pv3 = &rgv[(i+2) % cvertex];

			Vertex3D rgv3D[4];
			rgv3D[0].Set(pv1->x,pv1->y,m_d.m_heightbottom);
			rgv3D[1].Set(pv1->x,pv1->y,m_d.m_heighttop);
			rgv3D[2].Set(pv2->x,pv2->y,m_d.m_heighttop);
			rgv3D[3].Set(pv2->x,pv2->y,m_d.m_heightbottom);

			if (pinSide)
				{
				rgv3D[0].tu = rgtexcoord[i] * maxtuSide;
				rgv3D[0].tv = maxtvSide;

				rgv3D[1].tu = rgtexcoord[i] * maxtuSide;
				rgv3D[1].tv = 0;

				rgv3D[2].tu = rgtexcoord[(i+1) % cvertex] * maxtuSide;
				rgv3D[2].tv = 0;

				rgv3D[3].tu = rgtexcoord[(i+1) % cvertex] * maxtuSide;
				rgv3D[3].tv = maxtvSide;
				}

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[0]);
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[1]);
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[2]);
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[3]);

			const int a = (i-1+cvertex) % cvertex;
			const int b = i;
			const int c = (i+1)%cvertex;

			Vertex2D vnormal[2];
			if (pv1->fSmooth)
				{
				vnormal[0].x = (rgnormal[a].x + rgnormal[b].x)*0.5f;
				vnormal[0].y = (rgnormal[a].y + rgnormal[b].y)*0.5f;
				}
			else
				{
				vnormal[0].x = rgnormal[b].x;
				vnormal[0].y = rgnormal[b].y;
				}

			if (pv2->fSmooth)
				{
				vnormal[1].x = (rgnormal[b].x + rgnormal[c].x)*0.5f;
				vnormal[1].y = (rgnormal[b].y + rgnormal[c].y)*0.5f;
				}
			else
				{
				vnormal[1].x = rgnormal[b].x;
				vnormal[1].y = rgnormal[b].y;
				}

			{
			const float inv_len = 1.0f/sqrtf(vnormal[0].x * vnormal[0].x + vnormal[0].y * vnormal[0].y);
			vnormal[0].x *= inv_len;
			vnormal[0].y *= inv_len;
			}
			{
			const float inv_len = 1.0f/sqrtf(vnormal[1].x * vnormal[1].x + vnormal[1].y * vnormal[1].y);
			vnormal[1].x *= inv_len;
			vnormal[1].y *= inv_len;
			}

			if (m_d.m_fInner)
				{
				rgi[1] = 1;
				rgi[3] = 3;

				for (int l=0;l<2;l++)
					{
					rgv3D[l].nx = -vnormal[0].x;
					rgv3D[l].ny = vnormal[0].y;
					rgv3D[l].nz = 0;

					rgv3D[l+2].nx = -vnormal[1].x;
					rgv3D[l+2].ny = vnormal[1].y;
					rgv3D[l+2].nz = 0;
					}
				}
			else
				{
				rgi[1] = 3;
				rgi[3] = 1;

				for (int l=0;l<2;l++)
					{
					rgv3D[l].nx = vnormal[0].x;
					rgv3D[l].ny = -vnormal[0].y;
					rgv3D[l].nz = 0;

					rgv3D[l+2].nx = vnormal[1].x;
					rgv3D[l+2].ny = -vnormal[1].y;
					rgv3D[l+2].nz = 0;
					}
				}

			if (!fDrop && m_d.m_fSideVisible) // Don't need to render walls if dropped, but we do need to extend the extrema
				{
				// Draw side.
				Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,rgi,4,0);
				}

			if (fMover)
				{
				// Only do two points - each segment has two new points
				ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitdrop->m_polydropanim.m_znear
											, &m_phitdrop->m_polydropanim.m_zfar, 2, fFalse);
				}
			}
	}

	SAFE_DELETE(rgtexcoord);

	if (m_d.m_fVisible)
		{
		Vector<void> vpoly;
		Vector<Triangle> vtri;

		if (!m_d.m_fInner)
			{
			float miny = FLT_MAX;
			int minyindex;

			// Find smallest y point - use it to connect with surrounding border

			for (int i=0;i<cvertex;i++)
				{
				if (rgv[i].y < miny)
					{
					miny = rgv[i].y;
					minyindex = i;
					}
				}

			rgv[cvertex].x = m_ptable->m_left;
			rgv[cvertex].y = m_ptable->m_top;
			rgv[cvertex+3].x = m_ptable->m_left;
			rgv[cvertex+3].y = m_ptable->m_bottom;
			rgv[cvertex+2].x = m_ptable->m_right;
			rgv[cvertex+2].y = m_ptable->m_bottom;
			rgv[cvertex+1].x = m_ptable->m_right;
			rgv[cvertex+1].y = m_ptable->m_top;
			rgv[cvertex+4].x = m_ptable->m_left - 1.0f; // put tiny gap in to avoid errors
			rgv[cvertex+4].y = m_ptable->m_top;
			rgv[cvertex+5].x = rgv[minyindex].x;
			rgv[cvertex+5].y = rgv[minyindex].y - 1.0f; // put tiny gap in to avoid errors

			for (int i=0;i<cvertex;i++)
				{
				vpoly.AddElement((void *)(cvertex-i-1));
				}

			for (int i=0;i<6;i++)
				{
				vpoly.InsertElementAt((void *)(cvertex+i), (cvertex-minyindex-1));
				}
			}
		else
			{
			for (int i=0;i<cvertex;i++)
				{
				vpoly.AddElement((void *)i);
				}
			}

		pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

		if (pinSide)
			{
			ppin3d->EnableLightMap(fTrue, fDrop ? m_d.m_heightbottom : m_d.m_heighttop);

			//rlc add transparent texture support ... replaced this line with >>>>	
			pinSide->EnsureColorKey();
			if (pinSide->m_fTransparent)
				{				
				pd3dDevice->SetTexture(ePictureTexture, pinSide->m_pdsBufferColorKey);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				}
			else // ppin3d->SetTexture(pin->m_pdsBuffer);
				{
				pd3dDevice->SetTexture(ePictureTexture, pinSide->m_pdsBufferColorKey);     //rlc  alpha channel support
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE); 	
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
				if (g_pvp->m_pdd.m_fHardwareAccel)
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 128);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					}
				else
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					}
				pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA); 
				}

			pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
			g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
			}

		PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
		float maxtu, maxtv;

		if (pin)
			{
			m_ptable->GetTVTU(pin, &maxtu, &maxtv);
			
			pin->EnsureColorKey();
			if (pin->m_fTransparent)
				{				
				pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				}
			else 
				{
				pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);     //rlc  alpha channel support
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE); 	
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
				if (g_pvp->m_pdd.m_fHardwareAccel)
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 128);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					}
				else
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					}
				pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA); 
				}

			pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
			g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

			mtrl.diffuse.r = mtrl.ambient.r = 1.0f;
			mtrl.diffuse.g = mtrl.ambient.g = 1.0f;
			mtrl.diffuse.b = mtrl.ambient.b = 1.0f;
			mtrl.diffuse.a = mtrl.ambient.a = 1.0f;//0.5f;
			}
		else
			{
			ppin3d->SetTexture(NULL);
			const float r = (m_d.m_topcolor & 255) * (float)(1.0/255.0);
			const float g = (m_d.m_topcolor & 65280) * (float)(1.0/65280.0);
			const float b = (m_d.m_topcolor & 16711680) * (float)(1.0/16711680.0);

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			mtrl.diffuse.a = mtrl.ambient.a = 1.0f;
			
			maxtv = maxtu = 1.0f;
			}

		PolygonToTriangles(rgv, &vpoly, &vtri);

		if (!m_d.m_fInner)
			{
			// Remove tiny gap
			rgv[cvertex+4].x += 1.0f;
			rgv[cvertex+5].y += 1.0f;
			}

		pd3dDevice->SetMaterial(&mtrl);

		const float height = (!fDrop) ? m_d.m_heighttop : (m_d.m_heightbottom + 0.1f);

		const float inv_tablewidth = maxtu/(m_ptable->m_right - m_ptable->m_left);
		const float inv_tableheight = maxtv/(m_ptable->m_bottom - m_ptable->m_top);

		for (int i=0;i<vtri.Size();i++)
			{
			const Triangle * const ptri = vtri.ElementAt(i);

			const RenderVertex * const pv0 = &rgv[ptri->a];
			const RenderVertex * const pv1 = &rgv[ptri->b];
			const RenderVertex * const pv2 = &rgv[ptri->c];

			Vertex3D rgv3D[3];
			rgv3D[0].Set(pv0->x,pv0->y,height);
			rgv3D[2].Set(pv1->x,pv1->y,height);
			rgv3D[1].Set(pv2->x,pv2->y,height);

			rgv3D[0].tu = rgv3D[0].x *inv_tablewidth;
			rgv3D[0].tv = rgv3D[0].y *inv_tableheight;
			rgv3D[1].tu = rgv3D[1].x *inv_tablewidth;
			rgv3D[1].tv = rgv3D[1].y *inv_tableheight;
			rgv3D[2].tu = rgv3D[2].x *inv_tablewidth;
			rgv3D[2].tv = rgv3D[2].y *inv_tableheight;

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[0]);
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[1]);
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[2]);

			WORD rgi[3] = {0,1,2};
			for (int l=0;l<3;l++)
				{
				rgv3D[l].nx = 0;
				rgv3D[l].ny = 0;
				rgv3D[l].nz = -1.0f;
				}

			// Draw top.
			Display_DrawIndexedPrimitive(pd3dDevice, D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 3,rgi, 3, 0);
			}

		for (int i=0;i<vtri.Size();i++)
			{
			delete vtri.ElementAt(i);
			}
		}

	ppin3d->SetTexture(NULL);

	ppin3d->EnableLightMap(fFalse, -1);

	delete [] rgv;
	delete [] rgnormal;

	if (fMover)
		{
		// Create the color surface.
		pof->pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
		pof->pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &pof->rc, DDBLT_WAIT, NULL);
		
		// Check if we are a floor... in which case we don't want to affect z.
		if (!m_d.m_fFloor)
			{
			// Create the z surface.
			pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
			const HRESULT hr = pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
			}

		// Check if we are blitting with D3D.
		if (g_pvp->m_pdd.m_fUseD3DBlit)
			{
			// Create the D3D texture that we will blit.
			Display_CreateTexture ( g_pplayer->m_pin3d.m_pd3dDevice, g_pplayer->m_pin3d.m_pDD, NULL, (pof->rc.right - pof->rc.left), (pof->rc.bottom - pof->rc.top), &(pof->pTexture), &(pof->u), &(pof->v) );
			Display_CopyTexture ( g_pplayer->m_pin3d.m_pd3dDevice, pof->pTexture, &(pof->rc), ppin3d->m_pddsBackTextureBuffer );
			}

		ppin3d->ExpandRectByRect(&m_phitdrop->m_polydropanim.m_rcBounds, &pof->rc);

		// reset the portion of the z-buffer that we changed
		ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,&pof->rc, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
		}

	return pof;
	}
	
void Surface::RenderMoversFromCache(Pin3D *ppin3d)
	{
	for (int i=0;i<m_vlinesling.Size();i++)
		{
		LineSegSlingshot * const plinesling = m_vlinesling.ElementAt(i);
		ppin3d->ReadAnimObjectFromCacheFile(&plinesling->m_slingshotanim, &plinesling->m_slingshotanim.m_pobjframe, 1);
		}
	
	if (m_d.m_fDroppable && m_d.m_fInner)
		{
		ppin3d->ReadAnimObjectFromCacheFile(&m_phitdrop->m_polydropanim, m_phitdrop->m_polydropanim.m_pobjframe, 2);
		}
	}

void Surface::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	RenderSlingshots(pd3dDevice);

	if (m_d.m_fDroppable && m_d.m_fInner)
		{
		// Render wall raised.
		ObjFrame * const pof = RenderWallsAtHeight(pd3dDevice, fTrue, fFalse);
		m_phitdrop->m_polydropanim.m_pobjframe[0] = pof;

		// Check if this wall is being 
		// used as a flipbook animation.
		if (m_d.m_fFlipbook)
			{
			// Don't render a dropped wall. 
			m_phitdrop->m_polydropanim.m_pobjframe[1] = NULL;
			}
		else
			{
			// Render wall dropped (smashed to a pancake at bottom height).
			ObjFrame * const pof2 = RenderWallsAtHeight(pd3dDevice, fTrue, fTrue); 
			m_phitdrop->m_polydropanim.m_pobjframe[1] = pof2;
			}

		Pin3D * const ppin3d = &g_pplayer->m_pin3d;
		ppin3d->WriteAnimObjectToCacheFile(&m_phitdrop->m_polydropanim, m_phitdrop->m_polydropanim.m_pobjframe, 2);
		}
	}

void Surface::DoCommand(int icmd, int x, int y)
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

			HitSur * const phs = new HitSur(NULL, m_ptable->m_zoom, m_ptable->m_offsetx, m_ptable->m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

			Vertex2D v;
			phs->ScreenToSurface(x, y, &v.x, &v.y);
			delete phs;

			Vector<RenderVertex> vvertex;
			GetRgVertex(&vvertex);

			m_cvertexT = vvertex.Size();
			m_rgvT = new Vertex2D[m_cvertexT];

			for (int i=0;i<vvertex.Size();i++)
				{
				m_rgvT[i] = *((Vertex2D *)vvertex.ElementAt(i));
				}

			Vertex2D vOut;
			int iSeg;
			ClosestPointOnPolygon(m_rgvT, m_cvertexT, &v, &vOut, &iSeg, fTrue);

			// Go through vertices (including iSeg itself) counting control points until iSeg
			int icp = 0;
			for (int i=0;i<(iSeg+1);i++)
				{
				if (vvertex.ElementAt(i)->fControlPoint)
					{
					icp++;
					}
				}

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

			delete m_rgvT;
			m_rgvT = NULL;

			SetDirtyDraw();

			STOPUNDO
			}
			break;
		}
	}

void Surface::FlipY(Vertex2D *pvCenter)
	{
	IHaveDragPoints::FlipPointY(pvCenter);
	}

void Surface::FlipX(Vertex2D *pvCenter)
	{
	IHaveDragPoints::FlipPointX(pvCenter);
	}

void Surface::Rotate(float ang, Vertex2D *pvCenter)
	{
	IHaveDragPoints::RotatePoints(ang, pvCenter);
	}

void Surface::Scale(float scalex, float scaley, Vertex2D *pvCenter)
	{
	IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
	}

void Surface::Translate(Vertex2D *pvOffset)
	{
	IHaveDragPoints::TranslatePoints(pvOffset);
	}

HRESULT Surface::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	
	bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
	bw.WriteBool(FID(DROP), m_d.m_fDroppable);
	bw.WriteBool(FID(FLIP), m_d.m_fFlipbook);
	bw.WriteBool(FID(FLOR), m_d.m_fFloor);
	bw.WriteBool(FID(CLDW), m_d.m_fCollidable);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteFloat(FID(THRS), m_d.m_threshold);
	bw.WriteString(FID(IMAG), m_d.m_szImage);
	bw.WriteString(FID(SIMG), m_d.m_szSideImage);
	bw.WriteInt(FID(COLR), m_d.m_sidecolor);
	bw.WriteInt(FID(TCLR), m_d.m_topcolor);
	bw.WriteInt(FID(SCLR), m_d.m_slingshotColor);
	bw.WriteInt(FID(ALGN), m_d.m_ia);
	bw.WriteFloat(FID(HTBT), m_d.m_heightbottom);
	bw.WriteFloat(FID(HTTP), m_d.m_heighttop);
	bw.WriteBool(FID(INNR), m_d.m_fInner);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteBool(FID(DSPT), m_d.m_fDisplayTexture);
	bw.WriteFloat(FID(SLGF), m_d.m_slingshotforce);
	bw.WriteFloat(FID(SLTH), m_d.m_slingshot_threshold);
	bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
	bw.WriteFloat(FID(WFCT), m_d.m_friction);
	bw.WriteFloat(FID(WSCT), m_d.m_scatter);
	bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);
	bw.WriteBool(FID(VSBL), m_d.m_fVisible);
	bw.WriteBool(FID(SLGA), m_d.m_fSlingshotAnimation);
	bw.WriteBool(FID(SVBL), m_d.m_fSideVisible);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(PNTS));
	HRESULT hr;
	if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
		return hr;

	bw.WriteTag(FID(ENDB));

	return S_OK;	
	}

void Surface::ClearForOverwrite()
	{
	ClearPointsForOverwrite();
	}

HRESULT Surface::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults();
#ifndef OLDLOAD
	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
#else
	m_ptable = ptable;

	ULONG read = 0;
	DWORD dwID;
	HRESULT hr;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

	int temp;
	if(FAILED(hr = pstm->Read(&temp, sizeof(int), &read)))
		return hr;

	for (int i=0;i<temp;i++)
		{
		Vertex2D v;
		BOOL fSmooth;
		BOOL fSlingshot;

		if(FAILED(hr = pstm->Read(&v, sizeof(Vertex2D), &read)))
			return hr;
		if(FAILED(hr = pstm->Read(&fSmooth, sizeof(BOOL), &read)))
			return hr;
		if(FAILED(hr = pstm->Read(&fSlingshot, sizeof(BOOL), &read)))
			return hr;
		
		CComObject<DragPoint> *pdp;
		CComObject<DragPoint>::CreateInstance(&pdp);
		if (pdp)
			{
			pdp->AddRef();
			pdp->Init(this, v.x, v.y);
			pdp->m_fSmooth = fSmooth;
			pdp->m_fSlingshot = fSlingshot;
			m_vdpoint.AddElement(pdp);
			}
		}

	if(FAILED(hr = pstm->Read(&m_d, sizeof(SurfaceData), &read)))
		return hr;

	*pid = dwID;

	return hr;
#endif
	}

BOOL Surface::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(HTEV))
		{
		pbr->GetBool(&m_d.m_fHitEvent);
		}
	else if (id == FID(DROP))
		{
		pbr->GetBool(&m_d.m_fDroppable);
		}
	else if (id == FID(FLIP))
		{
		pbr->GetBool(&m_d.m_fFlipbook);
		}
	else if (id == FID(FLOR))
		{
		pbr->GetBool(&m_d.m_fFloor);
		}
	else if (id == FID(CLDW))
		{
		pbr->GetBool(&m_d.m_fCollidable); 
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(THRS))
		{
		pbr->GetFloat(&m_d.m_threshold);
		}
	else if (id == FID(IMAG))
		{
		pbr->GetString(m_d.m_szImage);
		}
	else if (id == FID(SIMG))
		{
		pbr->GetString(m_d.m_szSideImage);
		}
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_d.m_sidecolor);
		//if (!(m_d.m_sidecolor & MINBLACKMASK)) {m_d.m_sidecolor |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(TCLR))
		{
		pbr->GetInt(&m_d.m_topcolor);
		//if (!(m_d.m_topcolor & MINBLACKMASK)) {m_d.m_topcolor |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(SCLR))
		{
		pbr->GetInt(&m_d.m_slingshotColor);
		}
	else if (id == FID(ALGN))
		{
		pbr->GetInt(&m_d.m_ia);
		}
	else if (id == FID(HTBT))
		{
		pbr->GetFloat(&m_d.m_heightbottom);
		}
	else if (id == FID(HTTP))
		{
		pbr->GetFloat(&m_d.m_heighttop);
		}
	else if (id == FID(INNR))
		{
		pbr->GetBool(&m_d.m_fInner);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(DSPT))
		{
		pbr->GetBool(&m_d.m_fDisplayTexture);
		}
	else if (id == FID(SLGF))
		{
		pbr->GetFloat(&m_d.m_slingshotforce);
		}
	else if (id == FID(SLTH))
		{
		pbr->GetFloat(&m_d.m_slingshot_threshold);
		}
	else if (id == FID(ELAS))
		{
		pbr->GetFloat(&m_d.m_elasticity);
		}
	else if (id == FID(WFCT))
		{
		pbr->GetFloat(&m_d.m_friction);
		}
	else if (id == FID(WSCT))
		{
		pbr->GetFloat(&m_d.m_scatter);
		}
	else if (id == FID(CSHD))
		{
		pbr->GetBool(&m_d.m_fCastsShadow);
		}
	else if (id == FID(VSBL))
		{
		pbr->GetBool(&m_d.m_fVisible);
		}
	else if (id == FID(SLGA))
		{
		pbr->GetBool(&m_d.m_fSlingshotAnimation);
		}
	else if (id == FID(SVBL))
		{
		pbr->GetBool(&m_d.m_fSideVisible);
		}
	else
		{
		LoadPointToken(id, pbr, pbr->m_version);
		ISelect::LoadToken(id, pbr);
		}
	return fTrue;
	}

HRESULT Surface::InitPostLoad()
	{
	return S_OK;
	}

STDMETHODIMP Surface::get_HasHitEvent(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fHitEvent);

	return S_OK;
}

STDMETHODIMP Surface::put_HasHitEvent(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fHitEvent = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_Threshold(float *pVal)
{
	*pVal = m_d.m_threshold;

	return S_OK;
}

STDMETHODIMP Surface::put_Threshold(float newVal)
{
	STARTUNDO

	m_d.m_threshold = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_Image(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Surface::put_Image(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_SideColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_sidecolor;

	return S_OK;
}

STDMETHODIMP Surface::put_SideColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_sidecolor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_SlingshotColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_slingshotColor;

	return S_OK;
}

STDMETHODIMP Surface::put_SlingshotColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_slingshotColor = newVal;

	STOPUNDO

	return S_OK;
}
STDMETHODIMP Surface::get_ImageAlignment(ImageAlignment *pVal)
{
	*pVal = m_d.m_ia;

	return S_OK;
}

STDMETHODIMP Surface::put_ImageAlignment(ImageAlignment newVal)
{
	STARTUNDO

	m_d.m_ia = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_HeightBottom(float *pVal)
{
	*pVal = m_d.m_heightbottom;

	return S_OK;
}

STDMETHODIMP Surface::put_HeightBottom(float newVal)
{
	STARTUNDO

	m_d.m_heightbottom = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_HeightTop(float *pVal)
{
	*pVal = m_d.m_heighttop;

	return S_OK;
}

STDMETHODIMP Surface::put_HeightTop(float newVal)
{
	STARTUNDO

	m_d.m_heighttop = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_FaceColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_topcolor;

	return S_OK;
}

STDMETHODIMP Surface::put_FaceColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_topcolor = newVal;

	STOPUNDO

	return S_OK;
}

void Surface::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPWALL_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPWALL_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPWALL_PHYSICS, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

void Surface::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROPPOINT_VISUALSWTEX, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPPOINT_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);
	}

STDMETHODIMP Surface::get_CanDrop(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fDroppable);

	return S_OK;
}

STDMETHODIMP Surface::put_CanDrop(VARIANT_BOOL newVal)
{
	if(!m_d.m_fInner)
		{
		if(!m_d.m_fDroppable) return S_OK;		//can not drop outer wall
		else 
			{
			newVal = fFalse;						 // always force to false and cause update pending
			return S_FAIL;
			}
		}

	STARTUNDO

	m_d.m_fDroppable = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_FlipbookAnimation(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fFlipbook);

	return S_OK;
}

STDMETHODIMP Surface::put_FlipbookAnimation(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fFlipbook = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_IsDropped(VARIANT_BOOL *pVal)
{
	if (!g_pplayer)
		{
		return E_FAIL;
		}

	*pVal = FTOVB(m_fIsDropped);

	return S_OK;
}

STDMETHODIMP Surface::put_IsDropped(VARIANT_BOOL newVal)
{

	if (!g_pplayer || !m_d.m_fDroppable || !m_d.m_fInner)
		{
		return E_FAIL;
		}

	const BOOL fNewVal = VBTOF(newVal);

	if (m_fIsDropped != fNewVal)
	{
		m_fIsDropped = fNewVal;

		m_phitdrop->m_polydropanim.m_iframedesire = m_fIsDropped ? 1 : 0;

		for (int i=0;i<m_vhoDrop.Size();i++)
		{
			m_vhoDrop.ElementAt(i)->m_fEnabled = !m_fIsDropped && m_d.m_fCollidable; //disable hit on enities composing the object 
		}

		// Check if this surface has a user value.
		const int index = ((int) ((m_d.m_heighttop * 100.0f) - 111.0f + 0.25f));
		if ( (index >= 0) && (index < LIGHTHACK_MAX) )
		{
			// The same light is getting multiple updates per frame.
			// In the case of player lights, the light is on... then immediately turned off.
			// I don't know why this behavior happens; but it's causing problems.
			
			if ( (m_fIsDropped) &&
				 (!g_pplayer->m_LightHackReadyForDrawLightHackFn[index]) )
			{
				// Set the value.
				g_pplayer->m_LightHackCurrentState[index] = m_fIsDropped;
				g_pplayer->m_LightHackReadyForDrawLightHackFn[index] = TRUE;
				g_pplayer->m_LastUpdateTime[index] = msec();
			}
		}
	}

	return S_OK;
}

STDMETHODIMP Surface::get_DisplayTexture(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fDisplayTexture);

	return S_OK;
}

STDMETHODIMP Surface::put_DisplayTexture(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fDisplayTexture = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_SlingshotStrength(float *pVal)
{
	*pVal = m_d.m_slingshotforce*(float)(1.0/10.0);

	// Force value divided by 10 to make it look more like flipper strength value

	return S_OK;
}

STDMETHODIMP Surface::put_SlingshotStrength(float newVal)
{
	STARTUNDO

	m_d.m_slingshotforce = newVal*10.0f;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_Elasticity(float *pVal)
{
	*pVal = m_d.m_elasticity;

	return S_OK;
}

STDMETHODIMP Surface::put_Elasticity(float newVal)
{
	STARTUNDO

	m_d.m_elasticity = newVal;

	STOPUNDO

	return S_OK;
}


STDMETHODIMP Surface::get_Friction(float *pVal)
{
	*pVal = m_d.m_friction;

	return S_OK;
}

STDMETHODIMP Surface::put_Friction(float newVal)
{
	STARTUNDO

	if (newVal > 1.0f) newVal = 1.0f;
	else if (newVal < 0) newVal = 0;

	m_d.m_friction = newVal;

	STOPUNDO

	return S_OK;
}


STDMETHODIMP Surface::get_Scatter(float *pVal)
{
	*pVal = m_d.m_scatter;

	return S_OK;
}

STDMETHODIMP Surface::put_Scatter(float newVal)
{
	STARTUNDO

	m_d.m_scatter = newVal;

	STOPUNDO

	return S_OK;
}
///////////////////////////////////////////////////////////

STDMETHODIMP Surface::get_CastsShadow(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fCastsShadow);

	return S_OK;
}

STDMETHODIMP Surface::put_CastsShadow(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fCastsShadow = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_Visible(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fVisible);

	return S_OK;
}

STDMETHODIMP Surface::put_Visible(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fVisible = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_SideImage(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSideImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Surface::put_SideImage(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSideImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_Disabled(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_fDisabled);

	return S_OK;
}

STDMETHODIMP Surface::put_Disabled(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_fDisabled = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_SideVisible(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fSideVisible);

	return S_OK;
}

STDMETHODIMP Surface::put_SideVisible(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fSideVisible = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_Collidable(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fCollidable);

	return S_OK;
}

STDMETHODIMP Surface::put_Collidable(VARIANT_BOOL newVal)
{
	const BOOL fNewVal = VBTOF(newVal);	

	if(!m_d.m_fInner)	//outer wall must always be colliable
		{
		if(!newVal) return E_FAIL;		//outer wall must be collidable
		}

	STARTUNDO

	m_d.m_fCollidable = fNewVal;

	for (int i=0;i<m_vhoCollidable.Size();i++)
		{
		if (m_d.m_fDroppable) m_vhoCollidable.ElementAt(i)->m_fEnabled = fNewVal && !m_fIsDropped;
		else m_vhoCollidable.ElementAt(i)->m_fEnabled = fNewVal; //copy to hit checking on enities composing the object 
		}	

	STOPUNDO

	return S_OK;
}

/////////////////////////////////////////////////////////////

STDMETHODIMP Surface::get_SlingshotThreshold(float *pVal)
{
	*pVal = m_d.m_slingshot_threshold;

	return S_OK;
}

STDMETHODIMP Surface::put_SlingshotThreshold(float newVal)
{
	STARTUNDO

	m_d.m_slingshot_threshold = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Surface::get_SlingshotAnimation(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fSlingshotAnimation);

	return S_OK;
}

STDMETHODIMP Surface::put_SlingshotAnimation(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fSlingshotAnimation = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}
