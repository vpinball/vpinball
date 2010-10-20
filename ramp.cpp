// Ramp.cpp : Implementation of CVBATestApp and DLL registration.

#include "StdAfx.h"

/////////////////////////////////////////////////////////////////////////////
//
Ramp::Ramp()
	{
	m_menuid = IDR_SURFACEMENU;
	m_d.m_fCollidable = fTrue;
	m_d.m_IsVisible = fTrue;
	}

Ramp::~Ramp()
	{
	}

HRESULT Ramp::Init(PinTable *ptable, float x, float y)
	{
	m_ptable = ptable;
	m_d.m_IsVisible = fTrue;
	
	HRESULT hr;
	float length = 200.0f, fTmp;

	hr = GetRegStringAsFloat("DefaultProps\\Ramp", "Length", &fTmp);
	if (hr == S_OK)
		length = fTmp/2.0f;

	CComObject<DragPoint> *pdp;
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x, y+length);
		pdp->m_fSmooth = fTrue;
		m_vdpoint.AddElement(pdp);
		}

	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x, y-length);
		pdp->m_fSmooth = fTrue;
		m_vdpoint.AddElement(pdp);
		}

	SetDefaults();

	InitVBA(fTrue, 0, NULL);

	return S_OK;
	}

void Ramp::SetDefaults()
	{
	HRESULT hr;
	float fTmp;
	int iTmp;

	hr = GetRegStringAsFloat("DefaultProps\\Ramp","HeightBottom", &fTmp);
	if (hr == S_OK)
		m_d.m_heightbottom = fTmp;
	else
		m_d.m_heightbottom = 0;

	hr = GetRegStringAsFloat("DefaultProps\\Ramp","HeightTop", &fTmp);
	if (hr == S_OK)
		m_d.m_heighttop = fTmp;
	else
		m_d.m_heighttop = 100.0f;
	
	hr = GetRegStringAsFloat("DefaultProps\\Ramp","WidthBottom", &fTmp);
	if (hr == S_OK)
		m_d.m_widthbottom = fTmp;
	else
		m_d.m_widthbottom = 75.0f;

	hr = GetRegStringAsFloat("DefaultProps\\Ramp","WidthTop", &fTmp);
	if (hr == S_OK)
		m_d.m_widthtop = fTmp;
	else
		m_d.m_widthtop = 60.0f;

	hr = GetRegInt("DefaultProps\\Ramp","Color", &iTmp);
	if (hr == S_OK)
		m_d.m_color = iTmp;
	else
		m_d.m_color = RGB(50,200,50);

	hr = GetRegInt("DefaultProps\\Ramp","RampType", &iTmp);
	if (hr == S_OK)
		m_d.m_type = (enum RampType)iTmp;
	else
		m_d.m_type = RampTypeFlat;

	hr = GetRegInt("DefaultProps\\Ramp","TimerEnabled", &iTmp);
	if (hr == S_OK)
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0? false:true;
	else
		m_d.m_tdr.m_fTimerEnabled = false;
	
	hr = GetRegInt("DefaultProps\\Ramp","TimerInterval", &iTmp);
	if (hr == S_OK)
		m_d.m_tdr.m_TimerInterval = iTmp;
	else
		m_d.m_tdr.m_TimerInterval = 100;

	hr = GetRegString("DefaultProps\\Ramp","Image", m_d.m_szImage, MAXTOKEN);
	if (hr != S_OK)
		m_d.m_szImage[0] = 0;


	hr = GetRegInt("DefaultProps\\Ramp","ImageMode", &iTmp);
	if (hr == S_OK)
		m_d.m_imagealignment = (enum RampImageAlignment)iTmp;
	else
		m_d.m_imagealignment = ImageModeWorld;
	
	hr = GetRegInt("DefaultProps\\Ramp","ImageWalls", &iTmp);
	if (hr == S_OK)
		m_d.m_fImageWalls = iTmp == 0? false : true;
	else
		m_d.m_fImageWalls = fTrue;

	hr = GetRegInt("DefaultProps\\Ramp","CastsShadow", &iTmp);
	if (hr == S_OK)
		m_d.m_fCastsShadow = iTmp == 0? false : true;
	else
		m_d.m_fCastsShadow = fTrue;

	hr = GetRegInt("DefaultProps\\Ramp","Acrylic", &iTmp);
	if (hr == S_OK)
		m_d.m_fAcrylic = iTmp == 0? false : true;
	else
		m_d.m_fAcrylic = fFalse;

	hr = GetRegStringAsFloat("DefaultProps\\Ramp","LeftWallHeight", &fTmp);
	if (hr == S_OK)
		m_d.m_leftwallheight = fTmp;
	else
		m_d.m_leftwallheight = 62.0f;
	
	hr = GetRegStringAsFloat("DefaultProps\\Ramp","RightWallHeight", &fTmp);
	if (hr == S_OK)
		m_d.m_rightwallheight = fTmp;
	else
		m_d.m_rightwallheight = 62.0f;
	
	hr = GetRegStringAsFloat("DefaultProps\\Ramp","LeftWallHeightVisible", &fTmp);
	if (hr == S_OK)
		m_d.m_leftwallheightvisible = fTmp;
	else
		m_d.m_leftwallheightvisible = 30.0f;
	
	hr = GetRegStringAsFloat("DefaultProps\\Ramp","RightWallHeightVisible", &fTmp);
	if (hr == S_OK)
		m_d.m_rightwallheightvisible = fTmp;
	else
		m_d.m_rightwallheightvisible = 30.0f;

	hr = GetRegStringAsFloat("DefaultProps\\Ramp","Elasticity", &fTmp);
	if (hr == S_OK)
		m_d.m_elasticity = fTmp;
	else
		m_d.m_elasticity = 0.3f;
	
	hr = GetRegStringAsFloat("DefaultProps\\Ramp","Friction", &fTmp);
	if (hr == S_OK)
		m_d.m_friction = fTmp;
	else
		m_d.m_friction = 0;	//zero uses global value
	
	hr = GetRegStringAsFloat("DefaultProps\\Ramp","Scatter", &fTmp);
	if (hr == S_OK)
		m_d.m_scatter = fTmp;
	else
		m_d.m_scatter = 0;	//zero uses global value

	hr = GetRegInt("DefaultProps\\Ramp","Collidable", &iTmp);
	if (hr == S_OK)
		m_d.m_fCollidable = iTmp == 0? false : true;
	else
		m_d.m_fCollidable = fTrue;
	}

void Ramp::PreRender(Sur *psur)
	{
	psur->SetFillColor(RGB(192,192,192));
	psur->SetBorderColor(-1,false,0);
	psur->SetObject(this);

	int cvertex;
	Vertex2D * const rgv = GetRampVertex(&cvertex, NULL, NULL, NULL);

	psur->Polygon(rgv, cvertex*2);

	delete rgv;
	}

void Ramp::Render(Sur *psur)
	{
	psur->SetFillColor(-1);
	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetLineColor(RGB(0,0,0),false,0);
	psur->SetObject(this);
	psur->SetObject(NULL); // NULL so this won't be hit-tested

	int cvertex;
	bool *pfCross;
	Vertex2D * const rgv = GetRampVertex(&cvertex, NULL, &pfCross, NULL);

	psur->Polygon(rgv, cvertex*2);

	for (int i=0;i<cvertex;i++)
		{
		if (pfCross[i])
			{
			psur->Line(rgv[i].x, rgv[i].y, rgv[cvertex*2 - i - 1].x, rgv[cvertex*2 - i - 1].y);
			}
		}

	if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
		{
		psur->SetLineColor(RGB(0,0,0),false,3);
		psur->Polyline(rgv, cvertex);
		}

	if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
		{
		psur->SetLineColor(RGB(0,0,0),false,3);
		psur->Polyline(&rgv[cvertex], cvertex);
		}

	delete rgv;
	delete pfCross;

//>>> added by chris
	bool fDrawDragpoints;
	// if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
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
//<<<

	if (fDrawDragpoints)
		{
		for (int i=0;i<m_vdpoint.Size();i++)
			{
			CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
			psur->SetFillColor(-1);
			psur->SetBorderColor(RGB(255,0,0),false,0);
			psur->SetObject(pdp);

			if (pdp->m_fDragging)
				{
				psur->SetBorderColor(RGB(0,255,0),false,0);
				}

			psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
			}
		}
	}

void Ramp::RenderOutline(Sur * const psur)
	{
	psur->SetFillColor(-1);
	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetLineColor(RGB(0,0,0),false,0);
	psur->SetObject(this);
	psur->SetObject(NULL); // NULL so this won't be hit-tested

	int cvertex;
	bool *pfCross;
	Vertex2D * const rgv = GetRampVertex(&cvertex, NULL, &pfCross, NULL);

	psur->Polygon(rgv, cvertex*2);

	for (int i=0;i<cvertex;i++)
		{
		if (pfCross[i])
			{
			psur->Line(rgv[i].x, rgv[i].y, rgv[cvertex*2 - i - 1].x, rgv[cvertex*2 - i - 1].y);
			}
		}

	delete rgv;
	delete pfCross;
	}

void Ramp::RenderBlueprint(Sur *psur)
	{
	RenderOutline(psur);
	}

void Ramp::RenderShadow(ShadowSur *psur, float height)
	{
	if (!m_d.m_fCastsShadow || !m_ptable->m_fRenderShadows || !m_d.m_IsVisible) 
		return; //skip render if not visible

	psur->SetFillColor(RGB(0,0,0));
	psur->SetBorderColor(-1,false,0);
	psur->SetLineColor(RGB(0,0,0),false,2);
	psur->SetObject(this);

	float *rgheight;
	int cvertex;
	Vertex2D * const rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	// Find the range of vertices to draw a shadow for
	int startvertex = cvertex;
	int stopvertex = 0;
	for (int i=0;i<cvertex;i++)
		{
		if (rgheight[i] >= height)
			{
			if(i < startvertex)
				startvertex = i;
			stopvertex = i;
			}
		}

	const int range = (stopvertex - startvertex);

	if (range > 0)
		{
		if (m_d.m_type == RampType4Wire 
			|| m_d.m_type == RampType2Wire 
			|| m_d.m_type == RampType3WireLeft 
			|| m_d.m_type == RampType3WireRight)
			{
			float * const rgheight2 = new float[cvertex];

			for (int i=0;i<cvertex;i++)
				{
				rgheight2[i] = rgheight[cvertex - i - 1];
				}

			psur->PolylineSkew(rgv, cvertex, rgheight, 0, 0);
			psur->PolylineSkew(&rgv[cvertex], cvertex, rgheight2, 0, 0);

			for (int i=0;i<cvertex;i++)
				{
				rgheight[i]  += 44.0f;
				rgheight2[i] += 44.0f;
				}

			if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
				{
				psur->PolylineSkew(rgv, cvertex, rgheight, 0, 0);
				}

			if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
				{
				psur->PolylineSkew(&rgv[cvertex], cvertex, rgheight2, 0, 0);
				}

			delete rgheight2;
			}
		else
			{
			Vertex2D * const rgv2 = new Vertex2D[range*2];
			float * const rgheight2 = new float[range*2];

			for (int i=0;i<range;i++)
				{
				rgv2[i] = rgv[i + startvertex];
				rgv2[range*2 - i - 1] = rgv[cvertex*2 - i - 1 - startvertex];
				rgheight2[i] = rgheight[i + startvertex];
				rgheight2[range*2 - i - 1] = rgheight[i + startvertex];
				}

			psur->PolygonSkew(rgv2, range*2, rgheight2, 0, 0, true);

			delete rgv2;
			delete rgheight2;
			}
		}

	delete rgv;
	delete rgheight;
	}

void Ramp::GetBoundingVertices(Vector<Vertex3D> * const pvvertex3D)
	{
	float *rgheight;
	int cvertex;
	const Vertex2D * const rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	for (int i=0;i<cvertex;i++)
		{
			{
		Vertex3D * const pv = new Vertex3D();
		pv->x = rgv[i].x;
		pv->y = rgv[i].y;
		pv->z = rgheight[i]+50.0f; // leave room for ball
		pvvertex3D->AddElement(pv);
			}

		Vertex3D * const pv = new Vertex3D();
		pv->x = rgv[cvertex*2-i-1].x;
		pv->y = rgv[cvertex*2-i-1].y;
		pv->z = rgheight[i]+50.0f; // leave room for ball
		pvvertex3D->AddElement(pv);
		}

	delete rgv;
	delete rgheight;
	}

Vertex2D *Ramp::GetRampVertex(int * const pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio)
	{
	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	const int cvertex = vvertex.Size();
	Vertex2D * const rgv = new Vertex2D[cvertex * 2];
	if (ppheight)
		{
		*ppheight = new float[cvertex];
		}
	if (ppfCross)
		{
		*ppfCross = new bool[cvertex];
		}
	if (ppratio)
		{
		*ppratio = new float[cvertex];
		}

	float totallength = 0;
	for (int i=0;i<(cvertex-1);i++)
		{
		const Vertex2D * const pv1 = (Vertex2D *)vvertex.ElementAt(i);
		const Vertex2D * const pv2 = (Vertex2D *)vvertex.ElementAt(i+1);

		const float dx = pv1->x - pv2->x;
		const float dy = pv1->y - pv2->y;
		const float length = sqrtf(dx*dx + dy*dy);

		totallength += length;
		}

	float currentlength = 0;
	for (int i=0;i<cvertex;i++)
		{
		const Vertex2D * const pv1 = (Vertex2D *)vvertex.ElementAt((i>0) ? i-1 : i);
		const Vertex2D * const pv2 = (Vertex2D *)vvertex.ElementAt((i < (cvertex-1)) ? i+1 : i);
		const Vertex2D * const pvmiddle = (Vertex2D *)vvertex.ElementAt(i);

		Vertex2D vnormal;
		{
		// Get normal at this point
		// Notice that these values equal the ones in the line
		// equation and could probably be substituted by them.
		Vertex2D v1normal(pv1->y - pvmiddle->y, pvmiddle->x - pv1->x);
		Vertex2D v2normal(pvmiddle->y - pv2->y, pv2->x - pvmiddle->x);		

		if (i == (cvertex-1))
			{
			v1normal.Normalize();
			vnormal = v1normal;
			}
		else if (i == 0)
			{
			v2normal.Normalize();
			vnormal = v2normal;
			}
		else
			{
			v1normal.Normalize();
			v2normal.Normalize();
			if (fabsf(v1normal.x-v2normal.x) < 0.0001f && fabsf(v1normal.y-v2normal.y) < 0.0001f)
				{
				// Two parallel segments
				v1normal.Normalize();
				vnormal = v1normal;
				}
			else
				{
				v1normal.Normalize();
				v2normal.Normalize();

				// Find intersection of the two edges meeting this points, but
				// shift those lines outwards along their normals

				// First line
				const float A = pv1->y - pvmiddle->y;
				const float B = pvmiddle->x - pv1->x;

				// Shift line along the normal
				const float C = -(A*(pv1->x-v1normal.x) + B*(pv1->y-v1normal.y));

				// Second line
				const float D = pv2->y - pvmiddle->y;
				const float E = pvmiddle->x - pv2->x;

				// Shift line along the normal
				const float F = -(D*(pv2->x-v2normal.x) + E*(pv2->y-v2normal.y));

				const float inv_det = 1.0f/((A*E) - (B*D));

				const float intersectx = (B*F-E*C)*inv_det;
				const float intersecty = (C*D-A*F)*inv_det;

				//rgv[i].x = intersectx;
				//rgv[i].y = intersecty;

				//vnormal = Calc2DNormal(pv1, pv2);

				vnormal.x = pvmiddle->x - intersectx;
				vnormal.y = pvmiddle->y - intersecty;
				}
			}
		}

		{
		const float dx = pv1->x - pvmiddle->x;
		const float dy = pv1->y - pvmiddle->y;
		const float length = sqrtf(dx*dx + dy*dy);

		currentlength += length;
		}

		const float widthcur = ((currentlength/totallength) * (m_d.m_widthtop - m_d.m_widthbottom)) + m_d.m_widthbottom;

		if (ppheight)
			{
			const float percentage = 1.0f-(currentlength/totallength); //rlc ramps have no ends ... a line joint is needed
			const float heightcur = ((1.0f - percentage) * (m_d.m_heighttop - m_d.m_heightbottom)) + m_d.m_heightbottom;
			(*ppheight)[i] = heightcur;
			}

		if (ppratio)
			{
			const float percentage = 1.0f-(currentlength/totallength);
			(*ppratio)[i] = percentage;
			}

		rgv[i].x = pvmiddle->x + vnormal.x * (widthcur*0.5f);
		rgv[i].y = pvmiddle->y + vnormal.y * (widthcur*0.5f);
		rgv[cvertex*2 - i - 1].x = pvmiddle->x - vnormal.x * (widthcur*0.5f);
		rgv[cvertex*2 - i - 1].y = pvmiddle->y - vnormal.y * (widthcur*0.5f);
		}

	if (ppfCross)
		{
		for (int i=0;i<cvertex;i++)
			{
			(*ppfCross)[i] = vvertex.ElementAt(i)->fControlPoint;
			}
		}

	//rgv[i] = *((Vertex2D *)vvertex.ElementAt(i));
	//delete vvertex.ElementAt(i);

	for (int i=0;i<vvertex.Size();i++)
		{
		delete vvertex.ElementAt(i);
		}

	*pcvertex = cvertex;
	return rgv;
	}

void Ramp::GetRgVertex(Vector<RenderVertex> * const pvv)
	{
	const int cpoint = m_vdpoint.Size();
	RenderVertex rendv2;
	if(cpoint < 2) {
		const CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(0);
		rendv2.x = pdp->m_v.x;
		rendv2.y = pdp->m_v.y;
	}

	for (int i=0;i<(cpoint-1);i++)
		{
		const CComObject<DragPoint> * const pdp1 = m_vdpoint.ElementAt(i);
		const CComObject<DragPoint> * const pdp2 = m_vdpoint.ElementAt(i+1);
		const CComObject<DragPoint> * const pdp0 = m_vdpoint.ElementAt((i>0 && pdp1->m_fSmooth) ? i-1 : i);
		const CComObject<DragPoint> * const pdp3 = m_vdpoint.ElementAt((i<cpoint-2 && pdp2->m_fSmooth) ? i+2 : (i+1));
		CatmullCurve cc;
		cc.SetCurve(&pdp0->m_v, &pdp1->m_v, &pdp2->m_v, &pdp3->m_v);

		RenderVertex rendv1;
		rendv1.x = pdp1->m_v.x;
		rendv1.y = pdp1->m_v.y;
		rendv1.fSlingshot = (pdp1->m_fSlingshot != 0);
		rendv1.fSmooth = (pdp1->m_fSmooth != 0);
		rendv1.fControlPoint = true;

		// Properties of last point don't matter, because it won't be added to the list on this pass (it'll get added as the first point of the next curve)
		rendv2.x = pdp2->m_v.x;
		rendv2.y = pdp2->m_v.y;

		RecurseSmoothLine(&cc, 0, 1, &rendv1, &rendv2, pvv);
		}

	// Add the very last point to the list because nobody else added it
	rendv2.fSmooth = true;
	rendv2.fSlingshot = false;
	rendv2.fControlPoint = false;	//rlc --- trapped on uninitialized varible (DevPartner)
	RenderVertex * const pvT = new RenderVertex;
	*pvT = rendv2;
	pvv->AddElement(pvT);
	}

void Ramp::GetTimers(Vector<HitTimer> *pvht)
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

void Ramp::GetHitShapes(Vector<HitObject> *pvho)
	{
	int cvertex;
	float *rgheight;
	Vertex2D * const rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	float wallheightright;
	float wallheightleft;

	if (m_d.m_type == RampTypeFlat)
		{
		wallheightright = m_d.m_rightwallheight;
		wallheightleft = m_d.m_leftwallheight;
		}
	else if (m_d.m_type == RampType2Wire)
		{
		// backwards compatible physics
		wallheightright = 31.0f;
		wallheightleft = 31.0f;
		}
	else if (m_d.m_type == RampType4Wire)
		{
		wallheightleft = 62.0f;
		wallheightright = 62.0f;
		}
	else if (m_d.m_type == RampType3WireRight)
		{
		wallheightright = 62.0f;
		wallheightleft = (float)(6+12.5);
		}
	else if (m_d.m_type == RampType3WireLeft)
		{
		wallheightleft = 62.0f;
		wallheightright = (float)(6+12.5);
		}
			
	if (wallheightright > 0)
		{
		for (int i=0;i<(cvertex-1);i++)
			{
			const Vertex2D * const pv1 = (i>0) ? &rgv[i-1] : NULL;
			const Vertex2D * const pv2 = &rgv[i];
			const Vertex2D * const pv3 = &rgv[i+1];
			const Vertex2D * const pv4 = (i<(cvertex-2)) ? &rgv[i+2] : NULL;

#ifndef RAMPTEST
			AddLine(pvho, pv2, pv3, pv1, rgheight[i], rgheight[i+1]+wallheightright);
			AddLine(pvho, pv3, pv2, pv4, rgheight[i], rgheight[i+1]+wallheightright);
#else
			AddSideWall(pvho, pv2, pv3,rgheight[i], rgheight[i+1], wallheightright);
			AddSideWall(pvho, pv3, pv2,rgheight[i+1], rgheight[i], wallheightright);
#endif
			}
		}

	if (wallheightleft > 0)
		{
		for (int i=0;i<(cvertex-1);i++)
			{
			const Vertex2D * const pv1 = (i>0) ? &rgv[cvertex + i - 1] : NULL;
			const Vertex2D * const pv2 = &rgv[cvertex + i];
			const Vertex2D * const pv3 = &rgv[cvertex + i + 1];
			const Vertex2D * const pv4 = (i<(cvertex-2)) ? &rgv[cvertex + i + 2] : NULL;

#ifndef RAMPTEST
			AddLine(pvho, pv2, pv3, pv1, rgheight[cvertex - i - 2], rgheight[cvertex - i - 1] + wallheightleft);
			AddLine(pvho, pv3, pv2, pv4, rgheight[cvertex - i - 2], rgheight[cvertex - i - 1] + wallheightleft);
#else
			AddSideWall(pvho, pv2, pv3, rgheight[cvertex - i - 1], rgheight[cvertex - i - 2], wallheightleft);
			AddSideWall(pvho, pv3, pv2, rgheight[cvertex - i - 2], rgheight[cvertex - i - 1], wallheightleft);
#endif
			}
		}

	//Level *plevel;
#ifndef RAMPTEST
	{
	Hit3DPoly *ph3dpolyOld = NULL;

	const Vertex2D *pv1;
	const Vertex2D *pv2;
	const Vertex2D *pv3;
	const Vertex2D *pv4;

	for (int i=0;i<(cvertex-1);i++)
		{
		pv1 = &rgv[i];
		pv2 = &rgv[cvertex*2 - i - 1];
		pv3 = &rgv[cvertex*2 - i - 2];
		pv4 = &rgv[i+1];

		Vertex3D rgv3D[3];
		rgv3D[1].x = pv1->x;
		rgv3D[1].y = pv1->y;
		rgv3D[1].z = rgheight[i];
		rgv3D[0].x = pv2->x;
		rgv3D[0].y = pv2->y;
		rgv3D[0].z = rgheight[i];
		rgv3D[2].x = pv3->x;
		rgv3D[2].y = pv3->y;
		rgv3D[2].z = rgheight[i+1];

		Hit3DPoly * ph3dpoly = new Hit3DPoly(rgv3D,3,false);
		ph3dpoly->m_elasticity = m_d.m_elasticity;
		ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
		ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

		if (m_d.m_type == RampTypeFlat)
			{
			ph3dpoly->m_fVisible = fTrue;
			}

		pvho->AddElement(ph3dpoly);

		m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
		ph3dpoly->m_fEnabled = m_d.m_fCollidable;

		if (ph3dpolyOld)
			{
			CheckJoint(pvho, ph3dpolyOld, ph3dpoly);
			}

		ph3dpolyOld = ph3dpoly;

		rgv3D[1].x = pv1->x;
		rgv3D[1].y = pv1->y;
		rgv3D[1].z = rgheight[i];
		rgv3D[0].x = pv3->x;
		rgv3D[0].y = pv3->y;
		rgv3D[0].z = rgheight[i+1];
		rgv3D[2].x = pv4->x;
		rgv3D[2].y = pv4->y;
		rgv3D[2].z = rgheight[i+1];

		ph3dpoly = new Hit3DPoly(rgv3D,3,false);
		ph3dpoly->m_elasticity = m_d.m_elasticity;
		ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
		ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

		if (m_d.m_type == RampTypeFlat)
			{
			ph3dpoly->m_fVisible = fTrue;
			}

		pvho->AddElement(ph3dpoly);

		m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
		ph3dpoly->m_fEnabled = m_d.m_fCollidable;

		CheckJoint(pvho, ph3dpolyOld, ph3dpoly);
		ph3dpolyOld = ph3dpoly;
		}

	Vertex3D rgv3D[3];
	rgv3D[2].x = pv1->x;
	rgv3D[2].y = pv1->y;
	rgv3D[2].z = rgheight[cvertex-1];
	rgv3D[1].x = pv3->x;
	rgv3D[1].y = pv3->y;
	rgv3D[1].z = rgheight[cvertex-1];
	rgv3D[0].x = pv4->x;
	rgv3D[0].y = pv4->y;
	rgv3D[0].z = rgheight[cvertex-1];
	ph3dpolyOld = new Hit3DPoly(rgv3D,3,false);
	
	CheckJoint(pvho, ph3dpolyOld, ph3dpolyOld);
	delete ph3dpolyOld;
	ph3dpolyOld = NULL;
	}

	// add outside bottom, 
	// joints at the intersections are not needed since the inner surface has them
	// this surface is identical... except for the direction of the normal face.
	// hence the joints protect both surface edges from haveing a fall through

	for (int i=0;i<(cvertex-1);i++)
		{
		const Vertex2D * const pv1 = &rgv[i];
		const Vertex2D * const pv2 = &rgv[cvertex*2 - i - 1];
		const Vertex2D * const pv3 = &rgv[cvertex*2 - i - 2];
		const Vertex2D * const pv4 = &rgv[i+1];

		Vertex3D rgv3D[3];
		rgv3D[0].x = pv1->x;
		rgv3D[0].y = pv1->y;
		rgv3D[0].z = rgheight[i];
		rgv3D[1].x = pv2->x;
		rgv3D[1].y = pv2->y;
		rgv3D[1].z = rgheight[i];
		rgv3D[2].x = pv3->x;
		rgv3D[2].y = pv3->y;
		rgv3D[2].z = rgheight[i+1];

		Hit3DPoly *ph3dpoly = new Hit3DPoly(rgv3D,3,false);
		ph3dpoly->m_elasticity = m_d.m_elasticity;
		ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
		ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

		pvho->AddElement(ph3dpoly);

		m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
		ph3dpoly->m_fEnabled = m_d.m_fCollidable;

		rgv3D[2].x = pv1->x;
		rgv3D[2].y = pv1->y;
		rgv3D[2].z = rgheight[i];
		rgv3D[0].x = pv3->x;
		rgv3D[0].y = pv3->y;
		rgv3D[0].z = rgheight[i+1];
		rgv3D[1].x = pv4->x;
		rgv3D[1].y = pv4->y;
		rgv3D[1].z = rgheight[i+1];

		ph3dpoly = new Hit3DPoly(rgv3D,3,false);
		ph3dpoly->m_elasticity = m_d.m_elasticity;
		ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
		ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);
		
		pvho->AddElement(ph3dpoly);

		m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
		ph3dpoly->m_fEnabled = m_d.m_fCollidable;
		}
#endif
	delete rgheight;
	delete rgv;
	}

void Ramp::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

#define walltilt 0.5f

void Ramp::AddSideWall(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const float height1, const float height2, const float wallheight)
	{
	Vertex3D rgv3D[4];
	rgv3D[0].x = pv1->x;
	rgv3D[0].y = pv1->y;
	rgv3D[0].z = height1 - (float)PHYS_SKIN;
	rgv3D[1].x = pv2->x;
	rgv3D[1].y = pv2->y;
	rgv3D[1].z = height2 - (float)PHYS_SKIN;

	rgv3D[2].x = pv2->x + walltilt;
	rgv3D[2].y = pv2->y + walltilt;
	rgv3D[2].z = height2 + wallheight;
	rgv3D[3].x = pv1->x + walltilt;
	rgv3D[3].y = pv1->y + walltilt;
	rgv3D[3].z = height1 + wallheight;	

	Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,4,false);	
	ph3dpoly->m_elasticity = m_d.m_elasticity;
	ph3dpoly->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
	ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);
		
	pvho->AddElement(ph3dpoly);

	m_vhoCollidable.AddElement(ph3dpoly);	//remember hit components of ramp
	ph3dpoly->m_fEnabled = m_d.m_fCollidable;
	}


void Ramp::CheckJoint(Vector<HitObject> * const pvho, const Hit3DPoly * const ph3d1, const Hit3DPoly * const ph3d2)
	{
		Vertex3Ds vjointnormal;
		//vjointnormal.x = ph3d1->normal.x + ph3d2->normal.x;
		//vjointnormal.y = ph3d1->normal.y + ph3d2->normal.y;
		//vjointnormal.z = ph3d1->normal.z + ph3d2->normal.z;
		vjointnormal = CrossProduct(ph3d1->normal, ph3d2->normal);

		const float length = sqrtf(vjointnormal.x * vjointnormal.x + vjointnormal.y * vjointnormal.y + vjointnormal.z * vjointnormal.z);
		if (length < 1.0e-4f) return;

		const float inv_length = 1.0f/length;
		vjointnormal.x *= inv_length;
		vjointnormal.y *= inv_length;
		vjointnormal.z *= inv_length;

		// By convention of the calling function, points 1 [0] and 2 [1] of the second polygon will
		// be the common-edge points

		Hit3DCylinder * const ph3dc = new Hit3DCylinder(&ph3d2->m_rgv[0], &ph3d2->m_rgv[1], &vjointnormal);
		ph3dc->m_elasticity = m_d.m_elasticity;
		ph3dc->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
		ph3dc->m_scatter = ANGTORAD(m_d.m_scatter);
		pvho->AddElement(ph3dc);

		m_vhoCollidable.AddElement(ph3dc);	//remember hit components of ramp
		ph3dc->m_fEnabled = m_d.m_fCollidable;
	}


void Ramp::AddLine(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const Vertex2D * const pv3, const float height1, const float height2)
	{
	LineSeg * const plineseg = new LineSeg();
	plineseg->m_elasticity = m_d.m_elasticity;
	plineseg->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
	plineseg->m_scatter = ANGTORAD(m_d.m_scatter);

	plineseg->m_pfe = NULL;
	
	plineseg->m_rcHitRect.zlow = height1;//m_d.m_heightbottom;
	plineseg->m_rcHitRect.zhigh = height2;//m_d.m_heighttop;

	plineseg->v1 = *pv1;
	plineseg->v2 = *pv2;

	pvho->AddElement(plineseg);

	m_vhoCollidable.AddElement(plineseg);	//remember hit components of ramp
	plineseg->m_fEnabled = m_d.m_fCollidable;

	plineseg->CalcNormal();

	const Vertex2D vt1(pv1->x - pv2->x, pv1->y - pv2->y);

	if (pv3)
		{
		const Vertex2D vt2(pv1->x - pv3->x, pv1->y - pv3->y);

		const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

		if (dot < 0) // Inside edges don't need joint hit-testing (dot == 0 continuous segments should mathematically never hit)
			{
			Joint * const pjoint = new Joint();
			pjoint->m_elasticity = m_d.m_elasticity;
			pjoint->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
			pjoint->m_scatter = ANGTORAD(m_d.m_scatter);

			pjoint->m_pfe = NULL;
		
			pjoint->m_rcHitRect.zlow = height1;//m_d.m_heightbottom;
			pjoint->m_rcHitRect.zhigh = height2;//m_d.m_heighttop;

			pjoint->center = *pv1;
			pvho->AddElement(pjoint);

			m_vhoCollidable.AddElement(pjoint);	//remember hit components of ramp
			pjoint->m_fEnabled = m_d.m_fCollidable;

			// Set up line normal
			const float inv_length = 1.0f/sqrtf(vt2.x * vt2.x + vt2.y * vt2.y);
			pjoint->normal.x = plineseg->normal.x - vt2.y *inv_length;
			pjoint->normal.y = vt2.x *inv_length + plineseg->normal.y;

			// Set up line normal
			const float inv_length2 = 1.0f/sqrtf(pjoint->normal.x * pjoint->normal.x + pjoint->normal.y * pjoint->normal.y);
			pjoint->normal.x *= inv_length2;
			pjoint->normal.y *= inv_length2;
			}
		}
	}

void Ramp::EndPlay()
	{
	IEditable::EndPlay();

	for (int i=0;i<m_vlevel.Size();i++)
		{
		delete m_vlevel.ElementAt(i);
		}
	m_vlevel.RemoveAllElements();
	m_vhoCollidable.RemoveAllElements();
	}

WORD rgicrosssection[] = {
	0,1,16,
	1,17,16,
	1,2,17,
	2,18,17,
	2,3,18,
	3,19,18,
	3,0,19,
	0,16,19,

	8,9,24,
	9,25,24,
	9,10,25,
	10,26,25,
	10,11,26,
	11,27,26,
	11,8,27,
	8,24,27,

	4,5,20,
	5,21,20,
	5,6,21,
	6,22,21,
	6,7,22,
	7,23,22,
	7,4,23,
	4,20,23,

	12,13,28,
	13,29,28,
	13,14,29,
	14,30,29,
	14,15,30,
	15,31,30,
	15,12,31,
	12,28,31
	};

void Ramp::RenderStaticHabitrail(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);

	const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
	const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
	const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);

	{
	D3DMATERIAL7 mtrl;
	mtrl.emissive.r = mtrl.emissive.g =	mtrl.emissive.b = mtrl.emissive.a = 0.0f;
	mtrl.diffuse.r = mtrl.ambient.r = r;
	mtrl.diffuse.g = mtrl.ambient.g = g;
	mtrl.diffuse.b = mtrl.ambient.b = b;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;
	mtrl.specular.r = mtrl.specular.g = mtrl.specular.b = mtrl.specular.a = 1.0f;
	mtrl.power = 8.0f;
	pd3dDevice->SetMaterial(&mtrl);
	}

	float *rgheight;
	int cvertex;
	const Vertex2D * const rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	Vertex3D rgv3D[32];
	for (int i=0;i<cvertex;i++)
		{
		rgv3D[0].x = -3.0f;
		rgv3D[0].y = -3.0f;
		rgv3D[0].z = 0;
		rgv3D[0].nx = -1.0f;
		rgv3D[0].ny = -1.0f;
		rgv3D[0].nz = 0;
		rgv3D[0].NormalizeNormal();

		rgv3D[1].x = 3.0f;
		rgv3D[1].y = -3.0f;
		rgv3D[1].z = 0;
		rgv3D[1].nx = 1.0f;
		rgv3D[1].ny = -1.0f;
		rgv3D[1].nz = 0;
		rgv3D[1].NormalizeNormal();

		rgv3D[2].x = 3.0f;
		rgv3D[2].y = 3.0f;
		rgv3D[2].z = 0;
		rgv3D[2].nx = 1.0f;
		rgv3D[2].ny = 1.0f;
		rgv3D[2].nz = 0;
		rgv3D[2].NormalizeNormal();

		rgv3D[3].x = -3.0f;
		rgv3D[3].y = 3.0f;
		rgv3D[3].z = 0;
		rgv3D[3].nx = -1.0f;
		rgv3D[3].ny = 1.0f;
		rgv3D[3].nz = 0;
		rgv3D[3].NormalizeNormal();

		for (int l=0;l<4;l++)
			{
			rgv3D[l+ 4].x = rgv3D[l].x + 44.0f; //44.0f
			rgv3D[l+ 4].y = rgv3D[l].y - 19.0f; //22.0f
			rgv3D[l+ 4].z = rgv3D[l].z;
			rgv3D[l+ 8].x = rgv3D[l].x + 9.5f;
			rgv3D[l+ 8].y = rgv3D[l].y + 19.0f;
			rgv3D[l+ 8].z = rgv3D[l].z;
			rgv3D[l+12].x = rgv3D[l].x + 44.0f;
			rgv3D[l+12].y = rgv3D[l].y + 19.0f;
			rgv3D[l+12].z = rgv3D[l].z;

			rgv3D[l+ 4].nx = rgv3D[l].nx;
			rgv3D[l+ 4].ny = rgv3D[l].ny;
			rgv3D[l+ 4].nz = rgv3D[l].nz;
			rgv3D[l+ 8].nx = rgv3D[l].nx;
			rgv3D[l+ 8].ny = rgv3D[l].ny;
			rgv3D[l+ 8].nz = rgv3D[l].nz;
			rgv3D[l+12].nx = rgv3D[l].nx;
			rgv3D[l+12].ny = rgv3D[l].ny;
			rgv3D[l+12].nz = rgv3D[l].nz;
			}

		for (int l=0;l<4;l++)
			{
			rgv3D[l].x = rgv3D[l].x + 9.5f;
			rgv3D[l].y = rgv3D[l].y - 19.0f;
			}

		const int p1 = (i==0) ? 0 : (i-1);
		const int p2 = i;
		const int p3 = (i==(cvertex-1)) ? i : (i+1);
		const int p4 = cvertex*2 - i - 1; //!! ?? *2 valid?

		Vertex3Ds vacross;
		vacross.x = rgv[p4].x - rgv[p2].x;
		vacross.y = rgv[p4].y - rgv[p2].y;
		vacross.z = 0;

		// The vacross vector is our local up vector.  Rotate the cross-section
		// later to match this up
		vacross.Normalize();

		Vertex3Ds tangent;
		tangent.x = rgv[p3].x - rgv[p1].x;
		tangent.y = rgv[p3].y - rgv[p1].y;
		tangent.z = rgheight[p3] - rgheight[p1];

		// This is the vector describing the tangent to the ramp at this point
		tangent.Normalize();

		Vertex3Ds rotationaxis;
		/*
		Vertex3Ds up;
		up.x = 0;
		up.y = 0;
		up.z = 1.0f;		
		// Get axis of rotation to rotate our cross-section into place
		CrossProduct(tangent, up, &rotationaxis);*/

		rotationaxis.x =  tangent.y;
		rotationaxis.y = -tangent.x;
		rotationaxis.z = 0.0f;

		const float dot = tangent.z; //tangent.Dot(&up);
		const float angle = acosf(dot);

		RotateAround(rotationaxis, rgv3D, 16, angle);
		
		// vnewup is the beginning up vector of the cross-section
		const Vertex2D vnewupdef(0.0f,1.0f);
		const Vertex3Ds vnewup = RotateAround(rotationaxis, vnewupdef, angle);

		// vacross is not out real up vector, but the up vector for the cross-section isn't real either
		//Vertex3D vrampup;
		//CrossProduct(&tangent, &vacross, &vrampup);
		const float dotupcorrection = vnewup.Dot(vacross);
		float angleupcorrection = acosf(dotupcorrection);

		if (vacross.x >= 0)
			{
			angleupcorrection = -angleupcorrection;
			}

		RotateAround(tangent, rgv3D, 16, -angleupcorrection);

		for (int l=0;l<16;l++)
			{
			rgv3D[l].x += (rgv[p2].x + rgv[p4].x)*0.5f;
			rgv3D[l].y += (rgv[p2].y + rgv[p4].y)*0.5f;
			rgv3D[l].z += rgheight[p2];
			}

		if (i != 0)
			{
			RenderPolygons(pd3dDevice, rgv3D, rgicrosssection, 0, 16);

			if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
				{
				RenderPolygons(pd3dDevice, rgv3D, rgicrosssection, 16, 24);
				}

			if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
				{
				RenderPolygons(pd3dDevice, rgv3D, rgicrosssection, 24, 32);
				}
			}

		memcpy(&rgv3D[16], rgv3D, sizeof(Vertex3D)*16);
		}

	delete rgv;
	delete rgheight;

	pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	}

void Ramp::RenderPolygons(const LPDIRECT3DDEVICE7 pd3dDevice, Vertex3D * const rgv3D, WORD * const rgicrosssection, const int start, const int stop)
	{
	for (int i=start;i<stop;i++)
		{
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, rgv3D, 32, &rgicrosssection[i*3], 3, 0);
		}
	}

void Ramp::RenderStatic(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	
	if (!m_d.m_IsVisible) return;		// return if no Visible

#ifdef RAMP_RENDER8BITALPHA
	// Don't render acrylics.  
	// They are rendered after the ball.
	if (m_d.m_fAcrylic) return;
#endif

	if (m_d.m_type == RampType4Wire 
		|| m_d.m_type == RampType2Wire 
		|| m_d.m_type == RampType3WireLeft 
		|| m_d.m_type == RampType3WireRight)
		{
		RenderStaticHabitrail(pd3dDevice);
		}
	else
		{
		Pin3D *const ppin3d = &g_pplayer->m_pin3d;

		PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
		float maxtu = 0, maxtv = 0;

		D3DMATERIAL7 mtrl;
		mtrl.specular.r = mtrl.specular.g =	mtrl.specular.b = mtrl.specular.a =
		mtrl.emissive.r = mtrl.emissive.g =	mtrl.emissive.b = mtrl.emissive.a =
		mtrl.power = 0;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

		if (pin)
			{
			m_ptable->GetTVTU(pin, &maxtu, &maxtv);

			//ppin3d->SetTexture(pin->m_pdsBuffer);					//rlc replace with .....	

//rlc add transparent texture support ... replaced this line with >>>>	
			pin->EnsureColorKey();
			if (pin->m_fTransparent)
				{				
				pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				}
			else // ppin3d->SetTexture(pin->m_pdsBuffer);
				{	
				pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);     //rlc  alpha channel support
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE); 	
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
				if (g_pvp->m_pdd.m_fHardwareAccel)
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					}
				else
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					}
				pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA); 
				}
			
			pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

			// Check if this is an acrylic.
			if  (m_d.m_fAcrylic)
				{
				// Set a high threshold for writing transparent pixels to the z buffer.  
				// This allows some of the ball's pixels to write when under the ramp... 
				// giving the illusion of transparency (screen door). 
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD) 127);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);

				// Make sure our textures tile.
				pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);

				// Turn off texture filtering.
				g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_POINT );
				}
			else
				{
				g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
				}

			mtrl.diffuse.r = mtrl.ambient.r =
			mtrl.diffuse.g = mtrl.ambient.g =
			mtrl.diffuse.b = mtrl.ambient.b = 1.0f;			
			}
		else
			{
			const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
			const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
			const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			}

		pd3dDevice->SetMaterial(&mtrl);

		float *rgheight;
		float *rgratio;
		int cvertex;
		const Vertex2D * const rgv = GetRampVertex(&cvertex, &rgheight, NULL, &rgratio);

		WORD rgi[4] = {0,1,2,3};

		const float tablewidth = m_ptable->m_right - m_ptable->m_left;
		const float tableheight = m_ptable->m_bottom - m_ptable->m_top;

		const float scalewidth  = (float) g_pplayer->m_pin3d.m_dwRenderWidth  * (float)(1.0/64.055);		// 64.0f is texture width.			
		const float scaleheight = (float) g_pplayer->m_pin3d.m_dwRenderHeight * (float)(1.0/64.055);		// 64.0f is texture height.
		
		const float inv_width = scalewidth / (float)g_pplayer->m_pin3d.m_dwRenderWidth;
		const float inv_height = scaleheight / (float)g_pplayer->m_pin3d.m_dwRenderHeight;

		const float inv_width2 = maxtu / tablewidth;
		const float inv_height2 = maxtv / tableheight;

		for (int i=0;i<(cvertex-1);i++)
			{
			Vertex3D rgv3D[4];
			rgv3D[0].x = rgv[i].x;
			rgv3D[0].y = rgv[i].y;
			rgv3D[0].z = rgheight[i];

			rgv3D[3].x = rgv[i+1].x;
			rgv3D[3].y = rgv[i+1].y;
			rgv3D[3].z = rgheight[i+1];

			rgv3D[2].x = rgv[cvertex*2-i-2].x;
			rgv3D[2].y = rgv[cvertex*2-i-2].y;
			rgv3D[2].z = rgheight[i+1];

			rgv3D[1].x = rgv[cvertex*2-i-1].x;
			rgv3D[1].y = rgv[cvertex*2-i-1].y;
			rgv3D[1].z = rgheight[i];

			if (pin)
				{
				if (m_d.m_imagealignment == ImageModeWorld)
					{
					// Check if this is an acrylic.
					if (m_d.m_fAcrylic)
						{
							Vertex2D rgvOut[4];
							// Transform vertecies into screen coordinates.
							g_pplayer->m_pin3d.TransformVertices(rgv3D, NULL, 4, rgvOut);

							// Calculate texture coordinate for each vertex.
							for (int r=0; r<4; r++)
								{
								// Set texture coordinates so that there is a 1 to 1 correspondence
								// between texels and pixels.  This is the best case for screen door transparency.
								rgv3D[r].tu = rgvOut[r].x * inv_width; 
								rgv3D[r].tv = rgvOut[r].y * inv_height; 
								}
						}
					else
						{
						rgv3D[0].tu = rgv3D[0].x * inv_width2;
						rgv3D[0].tv = rgv3D[0].y * inv_height2;
						rgv3D[1].tu = rgv3D[1].x * inv_width2;
						rgv3D[1].tv = rgv3D[1].y * inv_height2;
						rgv3D[2].tu = rgv3D[2].x * inv_width2;
						rgv3D[2].tv = rgv3D[2].y * inv_height2;
						rgv3D[3].tu = rgv3D[3].x * inv_width2;
						rgv3D[3].tv = rgv3D[3].y * inv_height2;
						}
					}
				else
					{
					rgv3D[0].tu = maxtu;
					rgv3D[0].tv = rgratio[i] * maxtv;
					rgv3D[1].tu = 0;
					rgv3D[1].tv = rgratio[i] * maxtv;
					rgv3D[2].tu = 0;
					rgv3D[2].tv = rgratio[i+1] * maxtv;
					rgv3D[3].tu = maxtu;
					rgv3D[3].tv = rgratio[i+1] * maxtv;
					}
				}

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the floor of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,rgi, 4, 0);
			}

		if (pin && !m_d.m_fImageWalls)
			{
			ppin3d->SetTexture(NULL);

			const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
			const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
			const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			
			pd3dDevice->SetMaterial(&mtrl);
			}

		for (int i=0;i<(cvertex-1);i++)
			{
			Vertex3D rgv3D[4];
			rgv3D[0].x = rgv[i].x;
			rgv3D[0].y = rgv[i].y;
			rgv3D[0].z = rgheight[i];

			rgv3D[3].x = rgv[i+1].x;
			rgv3D[3].y = rgv[i+1].y;
			rgv3D[3].z = rgheight[i+1];

			rgv3D[2].x = rgv[i+1].x;
			rgv3D[2].y = rgv[i+1].y;
			rgv3D[2].z = rgheight[i+1] + m_d.m_rightwallheightvisible;

			rgv3D[1].x = rgv[i].x;
			rgv3D[1].y = rgv[i].y;
			rgv3D[1].z = rgheight[i] + m_d.m_rightwallheightvisible;

			if (pin && m_d.m_fImageWalls)
				{
				if (m_d.m_imagealignment == ImageModeWorld)
					{
					// Check if this is an acrylic.
					if (m_d.m_fAcrylic)
						{
							Vertex2D rgvOut[4];
							// Transform vertecies into screen coordinates.
							g_pplayer->m_pin3d.TransformVertices(rgv3D, NULL, 4, rgvOut);

							// Calculate texture coordinate for each vertex.
							for (int r=0; r<4; r++)
								{
								// Set texture coordinates so that there is a 1 to 1 correspondence
								// between texels and pixels.  This is the best case for screen door transparency.
								rgv3D[r].tu = rgvOut[r].x * inv_width; 
								rgv3D[r].tv = rgvOut[r].y * inv_height; 
								}
						}
					else
						{
						rgv3D[0].tu = rgv3D[0].x * inv_width2;
						rgv3D[0].tv = rgv3D[0].y * inv_height2;
						rgv3D[2].tu = rgv3D[2].x * inv_width2;
						rgv3D[2].tv = rgv3D[2].y * inv_height2;

						rgv3D[1].tu = rgv3D[0].tu;
						rgv3D[1].tv = rgv3D[0].tv;
						rgv3D[3].tu = rgv3D[2].tu;
						rgv3D[3].tv = rgv3D[2].tv;
						}
					}
				else
					{
					rgv3D[0].tu = maxtu;
					rgv3D[0].tv = rgratio[i] * maxtv;
					rgv3D[2].tu = maxtu;
					rgv3D[2].tv = rgratio[i+1] * maxtv;

					rgv3D[1].tu = rgv3D[0].tu;
					rgv3D[1].tv = rgv3D[0].tv;
					rgv3D[3].tu = rgv3D[2].tu;
					rgv3D[3].tv = rgv3D[2].tv;
					}
				}

			// 2-Sided polygon
			rgi[0] = 0;
			rgi[1] = 1;
			rgi[2] = 2;
			rgi[3] = 3;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,rgi, 4, 0);

			rgi[0] = 0;
			rgi[1] = 3;
			rgi[2] = 2;
			rgi[3] = 1;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,rgi, 4, 0);
			}

		for (int i=0;i<(cvertex-1);i++)
			{
			Vertex3D rgv3D[4];
			rgv3D[0].x = rgv[cvertex*2-i-2].x;
			rgv3D[0].y = rgv[cvertex*2-i-2].y;
			rgv3D[0].z = rgheight[i+1];

			rgv3D[3].x = rgv[cvertex*2-i-1].x;
			rgv3D[3].y = rgv[cvertex*2-i-1].y;
			rgv3D[3].z = rgheight[i];

			rgv3D[2].x = rgv[cvertex*2-i-1].x;
			rgv3D[2].y = rgv[cvertex*2-i-1].y;
			rgv3D[2].z = rgheight[i] + m_d.m_leftwallheightvisible;

			rgv3D[1].x = rgv[cvertex*2-i-2].x;
			rgv3D[1].y = rgv[cvertex*2-i-2].y;
			rgv3D[1].z = rgheight[i+1] + m_d.m_leftwallheightvisible;

			if (pin && m_d.m_fImageWalls)
				{
				if (m_d.m_imagealignment == ImageModeWorld)
					{
					// Check if this is an acrylic.
					if (m_d.m_fAcrylic)
						{
							Vertex2D rgvOut[4];
							// Transform vertecies into screen coordinates.
							g_pplayer->m_pin3d.TransformVertices(rgv3D, NULL, 4, rgvOut);

							// Calculate texture coordinate for each vertex.
							for (int r=0; r<4; r++)
								{
								// Set texture coordinates so that there is a 1 to 1 correspondence
								// between texels and pixels.  This is the best case for screen door transparency.
								rgv3D[r].tu = rgvOut[r].x * inv_width; 
								rgv3D[r].tv = rgvOut[r].y * inv_height; 
								}
						}
					else
						{
						rgv3D[0].tu = rgv3D[0].x * inv_width2;
						rgv3D[0].tv = rgv3D[0].y * inv_height2;
						rgv3D[2].tu = rgv3D[2].x * inv_width2;
						rgv3D[2].tv = rgv3D[2].y * inv_height2;

						rgv3D[1].tu = rgv3D[0].tu;
						rgv3D[1].tv = rgv3D[0].tv;
						rgv3D[3].tu = rgv3D[2].tu;
						rgv3D[3].tv = rgv3D[2].tv;
						}
					}
				else
					{
					rgv3D[0].tu = 0;
					rgv3D[0].tv = rgratio[i] * maxtv;
					rgv3D[2].tu = 0;
					rgv3D[2].tv = rgratio[i+1] * maxtv;

					rgv3D[1].tu = rgv3D[0].tu;
					rgv3D[1].tv = rgv3D[0].tv;
					rgv3D[3].tu = rgv3D[2].tu;
					rgv3D[3].tv = rgv3D[2].tv;
					}
				}

			// 2-Sided polygon
			rgi[0] = 0;
			rgi[1] = 1;
			rgi[2] = 2;
			rgi[3] = 3;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4, rgi, 4, 0);
			
			rgi[0] = 0;
			rgi[1] = 3;
			rgi[2] = 2;
			rgi[3] = 1;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, rgv3D, 4,rgi, 4, 0);
			}

		delete rgv;
		delete rgheight;
		delete rgratio;

		ppin3d->SetTexture(NULL);
		}
	}
	
void Ramp::RenderMoversFromCache(Pin3D *ppin3d)
	{
	}


void Ramp::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}


void Ramp::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(0, 0);
	}

void Ramp::MoveOffset(const float dx, const float dy)
	{
	for (int i=0;i<m_vdpoint.Size();i++)
		{
		CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

		pdp->m_v.x += dx;
		pdp->m_v.y += dy;
		}

	m_ptable->SetDirtyDraw();
	}

void Ramp::ClearForOverwrite()
	{
	ClearPointsForOverwrite();
	}

HRESULT Ramp::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteFloat(FID(HTBT), m_d.m_heightbottom);
	bw.WriteFloat(FID(HTTP), m_d.m_heighttop);
	bw.WriteFloat(FID(WDBT), m_d.m_widthbottom);
	bw.WriteFloat(FID(WDTP), m_d.m_widthtop);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteInt(FID(TYPE), m_d.m_type);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteString(FID(IMAG), m_d.m_szImage);
	bw.WriteInt(FID(ALGN), m_d.m_imagealignment);
	bw.WriteBool(FID(IMGW), m_d.m_fImageWalls);
	bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);
	bw.WriteBool(FID(ACRY), m_d.m_fAcrylic);
	bw.WriteFloat(FID(WLHL), m_d.m_leftwallheight);
	bw.WriteFloat(FID(WLHR), m_d.m_rightwallheight);
	bw.WriteFloat(FID(WVHL), m_d.m_leftwallheightvisible);
	bw.WriteFloat(FID(WVHR), m_d.m_rightwallheightvisible);
	bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
	bw.WriteFloat(FID(RFCT), m_d.m_friction);
	bw.WriteFloat(FID(RSCT), m_d.m_scatter);
	bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
	bw.WriteBool(FID(RVIS), m_d.m_IsVisible);	

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(PNTS));
	HRESULT hr;
	if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
		return hr;

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT Ramp::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults();
#ifndef OLDLOAD
	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
#else
	ULONG read = 0;	

	m_ptable = ptable;

	DWORD dwID;
	HRESULT hr;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

	if(FAILED(hr = InitPointLoad(pstm)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_d, sizeof(RampData), &read)))
		return hr;

	*pid = dwID;

	return hr;
#endif
	}

BOOL Ramp::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(HTBT))
		{
		pbr->GetFloat(&m_d.m_heightbottom);
		}
	else if (id == FID(HTTP))
		{
		pbr->GetFloat(&m_d.m_heighttop);
		}
	else if (id == FID(WDBT))
		{
		pbr->GetFloat(&m_d.m_widthbottom);
		}
	else if (id == FID(WDTP))
		{
		pbr->GetFloat(&m_d.m_widthtop);
		}
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_d.m_color);
	//	if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(TYPE))
		{
		pbr->GetInt(&m_d.m_type);
		}
	else if (id == FID(IMAG))
		{
		pbr->GetString(m_d.m_szImage);
		}
	else if (id == FID(ALGN))
		{
		pbr->GetInt(&m_d.m_imagealignment);
		}
	else if (id == FID(IMGW))
		{
		pbr->GetBool(&m_d.m_fImageWalls);
		}
	else if (id == FID(CSHD))
		{
		pbr->GetBool(&m_d.m_fCastsShadow);
		}
	else if (id == FID(ACRY))
		{
		pbr->GetBool(&m_d.m_fAcrylic);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(WLHL))
		{
		pbr->GetFloat(&m_d.m_leftwallheight);
		}
	else if (id == FID(WLHR))
		{
		pbr->GetFloat(&m_d.m_rightwallheight);
		}
	else if (id == FID(WVHL))
		{
		pbr->GetFloat(&m_d.m_leftwallheightvisible);
		}
	else if (id == FID(WVHR))
		{
		pbr->GetFloat(&m_d.m_rightwallheightvisible);
		}
	else if (id == FID(ELAS))
		{
		pbr->GetFloat(&m_d.m_elasticity);
		}
	else if (id == FID(RFCT))
		{
		pbr->GetFloat(&m_d.m_friction);
		}
	else if (id == FID(RSCT))
		{
		pbr->GetFloat(&m_d.m_scatter);
		}
	else if (id == FID(CLDRP))
		{
		pbr->GetBool(&m_d.m_fCollidable);
		}
	else if (id == FID(RVIS))
		{
		pbr->GetBool(&m_d.m_IsVisible);///////////////////////////
		}
	else
		{
		LoadPointToken(id, pbr, pbr->m_version);
		ISelect::LoadToken(id, pbr);
		}
	return fTrue;
	}

HRESULT Ramp::InitPostLoad()
	{
	return S_OK;
	}

void Ramp::DoCommand(int icmd, int x, int y)
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

			const int cvertex = vvertex.Size();
			Vertex2D * const rgv = new Vertex2D[cvertex];

			for (int i=0;i<cvertex;i++)
				{
				rgv[i] = *((Vertex2D *)vvertex.ElementAt(i));
				}

			Vertex2D vOut;
			int iSeg;
			ClosestPointOnPolygon(rgv, cvertex, v, &vOut, &iSeg, fFalse);

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
				pdp->m_fSmooth = fTrue; // Ramps are usually always smooth
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

void Ramp::FlipY(Vertex2D * const pvCenter)
	{
	IHaveDragPoints::FlipPointY(pvCenter);
	}

void Ramp::FlipX(Vertex2D * const pvCenter)
	{
	IHaveDragPoints::FlipPointX(pvCenter);
	}

void Ramp::Rotate(float ang, Vertex2D *pvCenter)
	{
	IHaveDragPoints::RotatePoints(ang, pvCenter);
	}

void Ramp::Scale(float scalex, float scaley, Vertex2D *pvCenter)
	{
	IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
	}

void Ramp::Translate(Vertex2D *pvOffset)
	{
	IHaveDragPoints::TranslatePoints(pvOffset);
	}

STDMETHODIMP Ramp::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_IRamp,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP Ramp::get_HeightBottom(float *pVal)
{
	*pVal = m_d.m_heightbottom;

	return S_OK;
}

STDMETHODIMP Ramp::put_HeightBottom(float newVal)
{
	STARTUNDO

	m_d.m_heightbottom = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_HeightTop(float *pVal)
{
	*pVal = m_d.m_heighttop;

	return S_OK;
}

STDMETHODIMP Ramp::put_HeightTop(float newVal)
{
	STARTUNDO

	m_d.m_heighttop = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_WidthBottom(float *pVal)
{
	*pVal = m_d.m_widthbottom;

	return S_OK;
}

STDMETHODIMP Ramp::put_WidthBottom(float newVal)
{
	STARTUNDO

	m_d.m_widthbottom = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_WidthTop(float *pVal)
{
	*pVal = m_d.m_widthtop;

	return S_OK;
}

STDMETHODIMP Ramp::put_WidthTop(float newVal)
{
	STARTUNDO

	m_d.m_widthtop = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_Color(OLE_COLOR *pVal)
{
	*pVal = m_d.m_color;

	return S_OK;
}

STDMETHODIMP Ramp::put_Color(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_color = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_Type(RampType *pVal)
{
	*pVal = m_d.m_type;

	return S_OK;
}

STDMETHODIMP Ramp::put_Type(RampType newVal)
{
	STARTUNDO

	m_d.m_type = newVal;

	STOPUNDO

	return S_OK;
}

void Ramp::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPRAMP_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPRAMP_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPRAMP_PHYSICS, IDS_PHYSICS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}


STDMETHODIMP Ramp::get_Image(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Ramp::put_Image(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_ImageAlignment(RampImageAlignment *pVal)
{
	*pVal = m_d.m_imagealignment;

	return S_OK;
}

STDMETHODIMP Ramp::put_ImageAlignment(RampImageAlignment newVal)
{
	STARTUNDO

	m_d.m_imagealignment = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_HasWallImage(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fImageWalls);

	return S_OK;
}

STDMETHODIMP Ramp::put_HasWallImage(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fImageWalls = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_CastsShadow(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCastsShadow);

	return S_OK;
}

STDMETHODIMP Ramp::put_CastsShadow(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_d.m_fCastsShadow = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_Acrylic(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fAcrylic);

	return S_OK;
}

STDMETHODIMP Ramp::put_Acrylic(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_d.m_fAcrylic = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_LeftWallHeight(float *pVal)
{
	*pVal = m_d.m_leftwallheight;

	return S_OK;
}

STDMETHODIMP Ramp::put_LeftWallHeight(float newVal)
{
	STARTUNDO

	m_d.m_leftwallheight = newVal;

	if (m_d.m_leftwallheight < 0)
		{
		m_d.m_leftwallheight = 0;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_RightWallHeight(float *pVal)
{
	*pVal = m_d.m_rightwallheight;

	return S_OK;
}

STDMETHODIMP Ramp::put_RightWallHeight(float newVal)
{
	STARTUNDO

	m_d.m_rightwallheight = newVal;

	if (m_d.m_rightwallheight < 0)
		{
		m_d.m_rightwallheight = 0;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_VisibleLeftWallHeight(float *pVal)
{
	*pVal = m_d.m_leftwallheightvisible;

	return S_OK;
}

STDMETHODIMP Ramp::put_VisibleLeftWallHeight(float newVal)
{
	STARTUNDO

	m_d.m_leftwallheightvisible = newVal;

	if (m_d.m_leftwallheightvisible < 0)
		{
		m_d.m_leftwallheightvisible = 0;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_VisibleRightWallHeight(float *pVal)
{
	*pVal = m_d.m_rightwallheightvisible;

	return S_OK;
}

STDMETHODIMP Ramp::put_VisibleRightWallHeight(float newVal)
{
	STARTUNDO

	m_d.m_rightwallheightvisible = newVal;

	if (m_d.m_rightwallheightvisible < 0)
		{
		m_d.m_rightwallheightvisible = 0;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_Elasticity(float *pVal)
{
	*pVal = m_d.m_elasticity;

	return S_OK;
}

STDMETHODIMP Ramp::put_Elasticity(float newVal)
{
	STARTUNDO

	m_d.m_elasticity = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Ramp::get_Friction(float *pVal)
{
	*pVal = m_d.m_friction;

	return S_OK;
}

STDMETHODIMP Ramp::put_Friction(float newVal)
{
	STARTUNDO

	if (newVal > 1.0f) newVal = 1.0f;
	else if (newVal < 0) newVal = 0;
	m_d.m_friction = newVal;

	STOPUNDO

	return S_OK;
}


STDMETHODIMP Ramp::get_Scatter(float *pVal)
{
	*pVal = m_d.m_scatter;

	return S_OK;
}

STDMETHODIMP Ramp::put_Scatter(float newVal)
{
	STARTUNDO

	m_d.m_scatter = newVal;

	STOPUNDO

	return S_OK;
}

//////////////////////////////////////////////////////////
STDMETHODIMP Ramp::get_Collidable(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB((!g_pplayer) ? m_d.m_fCollidable : m_vhoCollidable.ElementAt(0)->m_fEnabled);

	return S_OK;
}

STDMETHODIMP Ramp::put_Collidable(VARIANT_BOOL newVal)
{
	BOOL fNewVal = VBTOF(newVal);	
	if (!g_pplayer)
		{	
		STARTUNDO

		m_d.m_fCollidable = fNewVal;		
		
		STOPUNDO
		}
	else for (int i=0;i < m_vhoCollidable.Size();i++)
		{
		m_vhoCollidable.ElementAt(i)->m_fEnabled = fNewVal;	//copy to hit checking on enities composing the object 
		}		

	return S_OK;
}

STDMETHODIMP Ramp::get_IsVisible(VARIANT_BOOL *pVal) //temporary value of object
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_IsVisible);

	return S_OK;
}

STDMETHODIMP Ramp::put_IsVisible(VARIANT_BOOL newVal)
{	
	if (!g_pplayer )
		{
		STARTUNDO
		m_d.m_IsVisible = VBTOF(newVal);			// set visibility
		STOPUNDO
		}

	return S_OK;
}


// Same code as RenderStatic (with the exception of the acrylic test).
// Copied here to order the rendering of transparent and opaque ramps.
void Ramp::PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	
	// Don't render if invisible.
	if((!m_d.m_IsVisible) ||		
	// Don't render non-acrylics. 
	   (!m_d.m_fAcrylic)) return;

	if (m_d.m_type == RampType4Wire 
		|| m_d.m_type == RampType2Wire 
		|| m_d.m_type == RampType3WireLeft 
		|| m_d.m_type == RampType3WireRight)
		{
		RenderStaticHabitrail(pd3dDevice);
		}
	else
		{		
		Pin3D * const ppin3d = &g_pplayer->m_pin3d;

		PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
		float maxtu, maxtv;

		D3DMATERIAL7 mtrl;
		mtrl.specular.r = mtrl.specular.g =	mtrl.specular.b = mtrl.specular.a =
		mtrl.emissive.r = mtrl.emissive.g =	mtrl.emissive.b = mtrl.emissive.a =
		mtrl.power = 0;
		mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

		if (pin)
			{
			m_ptable->GetTVTU(pin, &maxtu, &maxtv);

			//ppin3d->SetTexture(pin->m_pdsBuffer);					//rlc replace with .....	

//rlc add transparent texture support ... replaced this line with >>>>	
			pin->EnsureColorKey();
			if (pin->m_fTransparent)
				{				
				pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				}
			else // ppin3d->SetTexture(pin->m_pdsBuffer);
				{	
				pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);     //rlc  alpha channel support
				pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE); 	
				pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
				if (g_pvp->m_pdd.m_fHardwareAccel)
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					}
				else
					{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					}
				pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA); 
				}
			
			pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

			g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

			mtrl.diffuse.r = mtrl.ambient.r =
			mtrl.diffuse.g = mtrl.ambient.g =
			mtrl.diffuse.b = mtrl.ambient.b = 1.0f;
			}
		else
			{
			const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
			const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
			const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			}

		pd3dDevice->SetMaterial(&mtrl);

		float *rgheight;
		float *rgratio;
		int cvertex;
		const Vertex2D * const rgv = GetRampVertex(&cvertex, &rgheight, NULL, &rgratio);

		WORD rgi[4] = {0,1,2,3};

		const float inv_tablewidth = maxtu/(m_ptable->m_right - m_ptable->m_left);
		const float inv_tableheight = maxtv/(m_ptable->m_bottom - m_ptable->m_top);

		for (int i=0;i<(cvertex-1);i++)
			{
			Vertex3D rgv3D[4];
			rgv3D[0].x = rgv[i].x;
			rgv3D[0].y = rgv[i].y;
			rgv3D[0].z = rgheight[i];

			rgv3D[3].x = rgv[i+1].x;
			rgv3D[3].y = rgv[i+1].y;
			rgv3D[3].z = rgheight[i+1];

			rgv3D[2].x = rgv[cvertex*2-i-2].x;
			rgv3D[2].y = rgv[cvertex*2-i-2].y;
			rgv3D[2].z = rgheight[i+1];

			rgv3D[1].x = rgv[cvertex*2-i-1].x;
			rgv3D[1].y = rgv[cvertex*2-i-1].y;
			rgv3D[1].z = rgheight[i];

			if (pin)
				{
				if (m_d.m_imagealignment == ImageModeWorld)
					{
					rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
					rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
					rgv3D[1].tu = rgv3D[1].x * inv_tablewidth;
					rgv3D[1].tv = rgv3D[1].y * inv_tableheight;
					rgv3D[2].tu = rgv3D[2].x * inv_tablewidth;
					rgv3D[2].tv = rgv3D[2].y * inv_tableheight;
					rgv3D[3].tu = rgv3D[3].x * inv_tablewidth;
					rgv3D[3].tv = rgv3D[3].y * inv_tableheight;
					}
				else
					{
					rgv3D[0].tu = maxtu;
					rgv3D[0].tv = rgratio[i] * maxtv;
					rgv3D[1].tu = 0;
					rgv3D[1].tv = rgratio[i] * maxtv;
					rgv3D[2].tu = 0;
					rgv3D[2].tv = rgratio[i+1] * maxtv;
					rgv3D[3].tu = maxtu;
					rgv3D[3].tv = rgratio[i+1] * maxtv;
					}
				}

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the floor of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,rgi, 4, 0);
			}

		if (pin && !m_d.m_fImageWalls)
			{
			ppin3d->SetTexture(NULL);

			const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
			const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
			const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;

			pd3dDevice->SetMaterial(&mtrl);
			}

		for (int i=0;i<(cvertex-1);i++)
			{
			Vertex3D rgv3D[4];
			rgv3D[0].x = rgv[i].x;
			rgv3D[0].y = rgv[i].y;
			rgv3D[0].z = rgheight[i];

			rgv3D[3].x = rgv[i+1].x;
			rgv3D[3].y = rgv[i+1].y;
			rgv3D[3].z = rgheight[i+1];

			rgv3D[2].x = rgv[i+1].x;
			rgv3D[2].y = rgv[i+1].y;
			rgv3D[2].z = rgheight[i+1] + m_d.m_rightwallheightvisible;

			rgv3D[1].x = rgv[i].x;
			rgv3D[1].y = rgv[i].y;
			rgv3D[1].z = rgheight[i] + m_d.m_rightwallheightvisible;

			if (pin && m_d.m_fImageWalls)
				{
				if (m_d.m_imagealignment == ImageModeWorld)
					{
					rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
					rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
					rgv3D[2].tu = rgv3D[2].x * inv_tablewidth;
					rgv3D[2].tv = rgv3D[2].y * inv_tableheight;
					}
				else
					{
					rgv3D[0].tu = maxtu;
					rgv3D[0].tv = rgratio[i] * maxtv;
					rgv3D[2].tu = maxtu;
					rgv3D[2].tv = rgratio[i+1] * maxtv;
					}

				rgv3D[1].tu = rgv3D[0].tu;
				rgv3D[1].tv = rgv3D[0].tv;
				rgv3D[3].tu = rgv3D[2].tu;
				rgv3D[3].tv = rgv3D[2].tv;
				}

			// 2-Sided polygon
			rgi[0] = 0;
			rgi[1] = 1;
			rgi[2] = 2;
			rgi[3] = 3;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,rgi, 4, 0);

			rgi[0] = 0;
			rgi[1] = 3;
			rgi[2] = 2;
			rgi[3] = 1;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,rgi, 4, 0);
			}

		for (int i=0;i<(cvertex-1);i++)
			{
			Vertex3D rgv3D[4];
			rgv3D[0].x = rgv[cvertex*2-i-2].x;
			rgv3D[0].y = rgv[cvertex*2-i-2].y;
			rgv3D[0].z = rgheight[i+1];

			rgv3D[3].x = rgv[cvertex*2-i-1].x;
			rgv3D[3].y = rgv[cvertex*2-i-1].y;
			rgv3D[3].z = rgheight[i];

			rgv3D[2].x = rgv[cvertex*2-i-1].x;
			rgv3D[2].y = rgv[cvertex*2-i-1].y;
			rgv3D[2].z = rgheight[i] + m_d.m_leftwallheightvisible;

			rgv3D[1].x = rgv[cvertex*2-i-2].x;
			rgv3D[1].y = rgv[cvertex*2-i-2].y;
			rgv3D[1].z = rgheight[i+1] + m_d.m_leftwallheightvisible;

			if (pin && m_d.m_fImageWalls)
				{
				if (m_d.m_imagealignment == ImageModeWorld)
					{
					rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
					rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
					rgv3D[2].tu = rgv3D[2].x * inv_tablewidth;
					rgv3D[2].tv = rgv3D[2].y * inv_tableheight;
					}
				else
					{
					rgv3D[0].tu = 0;
					rgv3D[0].tv = rgratio[i] * maxtv;
					rgv3D[2].tu = 0;
					rgv3D[2].tv = rgratio[i+1] * maxtv;
					}

				rgv3D[1].tu = rgv3D[0].tu;
				rgv3D[1].tv = rgv3D[0].tv;
				rgv3D[3].tu = rgv3D[2].tu;
				rgv3D[3].tv = rgv3D[2].tv;
				}

			// 2-Sided polygon
			rgi[0] = 0;
			rgi[1] = 1;
			rgi[2] = 2;
			rgi[3] = 3;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4, rgi, 4, 0);

			rgi[0] = 0;
			rgi[1] = 3;
			rgi[2] = 2;
			rgi[3] = 1;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			// Draw the wall of the ramp.
			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, rgv3D, 4,rgi, 4, 0);
			}

		delete rgv;
		delete rgheight;
		delete rgratio;

		ppin3d->SetTexture(NULL);
		}
	}
