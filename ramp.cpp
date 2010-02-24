// Ramp.cpp : Implementation of CVBATestApp and DLL registration.

#include "stdafx.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////
//
Ramp::Ramp()
	{
	m_menuid = IDR_SURFACEMENU;
	}

Ramp::~Ramp()
	{
	}

HRESULT Ramp::Init(PinTable *ptable, float x, float y)
	{
	HRESULT hr = S_OK;

	m_ptable = ptable;

	CComObject<DragPoint> *pdp;
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x, y+100);
		pdp->m_fSmooth = fTrue;
		m_vdpoint.AddElement(pdp);
		}
	CComObject<DragPoint>::CreateInstance(&pdp);
	if (pdp)
		{
		pdp->AddRef();
		pdp->Init(this, x, y-100);
		pdp->m_fSmooth = fTrue;
		m_vdpoint.AddElement(pdp);
		}

	SetDefaults();

	InitVBA(fTrue, 0, NULL);

	return hr;
	}

void Ramp::SetDefaults()
	{
	m_d.m_heightbottom =0;
	m_d.m_heighttop = 100;
	m_d.m_widthbottom = 75;
	m_d.m_widthtop = 60;

	m_d.m_color = RGB(50,200,50);

	m_d.m_type = RampTypeFlat;

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_szImage[0] = 0;
	m_d.m_imagealignment = ImageModeWorld;
	m_d.m_fImageWalls = fTrue;

	m_d.m_fCastsShadow = fTrue;

	m_d.m_leftwallheight = 62;
	m_d.m_rightwallheight = 62;
	m_d.m_leftwallheightvisible = 30;
	m_d.m_rightwallheightvisible = 30;

	m_d.m_elasticity = 0.2f;
	}

void Ramp::PreRender(Sur *psur)
	{
	psur->SetFillColor(RGB(192,192,192));
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetObject(this);

	Vertex *rgv;
	int cvertex;

	rgv = GetRampVertex(&cvertex, NULL, NULL, NULL);

	psur->Polygon(rgv, cvertex*2);

	delete rgv;
	}

void Ramp::Render(Sur *psur)
	{
	int 	i;
	BOOL 	*pfCross;
	BOOL	fDrawDragpoints;		//>>> added by chris

	psur->SetFillColor(-1);
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetLineColor(RGB(0,0,0),fFalse,0);
	psur->SetObject(this);
	psur->SetObject(NULL); // NULL so this won't be hit-tested

	Vertex *rgv;
	int cvertex;

	rgv = GetRampVertex(&cvertex, NULL, &pfCross, NULL);

	psur->Polygon(rgv, cvertex*2);

	for (i=0;i<cvertex;i++)
		{
		if (pfCross[i])
			{
			psur->Line(rgv[i].x, rgv[i].y, rgv[cvertex*2 - i - 1].x, rgv[cvertex*2 - i - 1].y);
			}
		}

	if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
		{
		psur->SetLineColor(RGB(0,0,0),fFalse,3);
		psur->Polyline(rgv, cvertex);
		}

	if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
		{
		psur->SetLineColor(RGB(0,0,0),fFalse,3);
		psur->Polyline(&rgv[cvertex], cvertex);
		}

	delete rgv;
	delete pfCross;

//>>> added by chris
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
//<<<

	if (fDrawDragpoints == fTrue)
		{
		for (i=0;i<m_vdpoint.Size();i++)
			{
			CComObject<DragPoint> *pdp;
			pdp = m_vdpoint.ElementAt(i);
			psur->SetFillColor(-1);
			psur->SetBorderColor(RGB(255,0,0),fFalse,0);
			psur->SetObject(pdp);

			if (pdp->m_fDragging)
				{
				psur->SetBorderColor(RGB(0,255,0),fFalse,0);
				}

			psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
			}
		}
	}

void Ramp::RenderOutline(Sur *psur)
	{
	int i;
	BOOL *pfCross;

	psur->SetFillColor(-1);
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetLineColor(RGB(0,0,0),fFalse,0);
	psur->SetObject(this);
	psur->SetObject(NULL); // NULL so this won't be hit-tested

	Vertex *rgv;
	int cvertex;

	rgv = GetRampVertex(&cvertex, NULL, &pfCross, NULL);

	psur->Polygon(rgv, cvertex*2);

	for (i=0;i<cvertex;i++)
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
	if ( (m_d.m_fCastsShadow != fTrue) || (m_ptable->m_fRenderShadows == fFalse) )
		return;

	psur->SetFillColor(RGB(0,0,0));
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetLineColor(RGB(0,0,0),fFalse,2);
	psur->SetObject(this);

	Vertex *rgv;
	float *rgheight;
	int cvertex;

	rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	/*int i;
	for (i=0;i<cvertex;i++)
		{
		rgv[i].x += rgheight[i];
		rgv[i].y -= rgheight[i];
		rgv[cvertex*2 - i - 1].x += rgheight[i];
		rgv[cvertex*2 - i - 1].y -= rgheight[i];
		}*/

	// Find the range of vertices to draw a shadow for
	int startvertex, stopvertex;
	startvertex = cvertex;
	stopvertex = 0;
	int i;
	for (i=0;i<cvertex;i++)
		{
		if (rgheight[i] >= height)
			{
			startvertex = min(startvertex, i);
			stopvertex = i;
			}
		}

	int range = (stopvertex - startvertex);

	if (range > 0)
		{
		if (m_d.m_type == RampType4Wire || m_d.m_type == RampType2Wire || m_d.m_type == RampType3WireLeft || m_d.m_type == RampType3WireRight)
			{
			int i;
			float *rgheight2 = new float[cvertex];

			for (i=0;i<cvertex;i++)
				{
				rgheight2[i] = rgheight[cvertex - i - 1];
				}

			psur->PolylineSkew(rgv, cvertex, rgheight, 0, 0);
			psur->PolylineSkew(&rgv[cvertex], cvertex, rgheight2, 0, 0);

			for (i=0;i<cvertex;i++)
				{
				rgheight[i] += 44;
				rgheight2[i] += 44;
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
			int i;
			Vertex *rgv2 = new Vertex[cvertex*2];
			float *rgheight2 = new float[cvertex*2];

			for (i=0;i<range;i++)
				{
				rgv2[i] = rgv[i + startvertex];
				rgv2[range*2 - i - 1] = rgv[cvertex*2 - i - 1 - startvertex];
				rgheight2[i] = rgheight[i + startvertex];
				rgheight2[range*2 - i - 1] = rgheight[i + startvertex];
				}

			psur->PolygonSkew(rgv2, range*2, rgheight2, 0, 0, fTrue);

			delete rgv2;
			delete rgheight2;
			}
		}

	delete rgv;
	delete rgheight;
	}

void Ramp::GetBoundingVertices(Vector<Vertex3D> *pvvertex3D)
	{
	Vertex3D *pv;
	int i;
	//float minx, maxx, miny, maxy;

	Vertex *rgv;
	float *rgheight;
	int cvertex;

	rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	for (i=0;i<cvertex;i++)
		{
		pv = new Vertex3D();
		pv->x = rgv[i].x;
		pv->y = rgv[i].y;
		pv->z = rgheight[i]+50; // leave room for ball
		pvvertex3D->AddElement(pv);

		pv = new Vertex3D();
		pv->x = rgv[cvertex*2-i-1].x;
		pv->y = rgv[cvertex*2-i-1].y;
		pv->z = rgheight[i]+50; // leave room for ball
		pvvertex3D->AddElement(pv);
		}

	delete rgv;
	delete rgheight;
	}

Vertex *Ramp::GetRampVertex(int *pcvertex, float **ppheight, BOOL **ppfCross, float **ppratio)
	{
	Vertex vnormal;
	Vertex *rgv;
	int cvertex;
	float totallength;
	float currentlength;
	int i;

	Vector<RenderVertex> vvertex;
	GetRgVertex(&vvertex);

	cvertex = vvertex.Size();
	rgv = new Vertex[cvertex * 2];
	if (ppheight)
		{
		*ppheight = new float[cvertex];
		}
	if (ppfCross)
		{
		*ppfCross = new BOOL[cvertex];
		}
	if (ppratio)
		{
		*ppratio = new float[cvertex];
		}

	Vertex *pv1, *pv2, *pvmiddle;

	totallength = 0;
	currentlength = 0;

	for (i=0;i<(cvertex-1);i++)
		{
		pv1 = (Vertex *)vvertex.ElementAt(i);
		pv2 = (Vertex *)vvertex.ElementAt(i+1);

		float dx,dy;
		dx = pv1->x - pv2->x;
		dy = pv1->y - pv2->y;
		float length = (float)sqrt(dx*dx + dy*dy);

		totallength += length;
		}

	for (i=0;i<cvertex;i++)
		{
		if (i>0)
			{
			pv1 = (Vertex *)vvertex.ElementAt(i-1);
			}
		else
			{
			pv1 = (Vertex *)vvertex.ElementAt(i);
			}

		if (i < (cvertex-1))
			{
			pv2 = (Vertex *)vvertex.ElementAt(i+1);
			}
		else
			{
			pv2 = (Vertex *)vvertex.ElementAt(i);
			}

		pvmiddle = (Vertex *)vvertex.ElementAt(i);

		{
		// Get normal at this point

		Vertex v1,v2;

		v1 = *pv1;
		v2 = *pv2;

		Vertex v1normal, v2normal;

		// Notice that these values equal the ones in the line
		// equation and could probably be substituted by them.
		v1normal.x = (pv1->y - pvmiddle->y);
		v1normal.y = -(pv1->x - pvmiddle->x);
		v2normal.x = (pvmiddle->y - pv2->y);
		v2normal.y = -(pvmiddle->x - pv2->x);

		if (i == (cvertex-1))
			{
			v1normal.Normalize();
			vnormal.x = v1normal.x;
			vnormal.y = v1normal.y;
			}
		else if (i == 0)
			{
			v2normal.Normalize();
			vnormal.x = v2normal.x;
			vnormal.y = v2normal.y;
			}
		else
			{
			v1normal.Normalize();
			v2normal.Normalize();
			if (fabs(v1normal.x-v2normal.x) < 0.0001 && fabs(v1normal.y-v2normal.y) < 0.0001)
				{
				// Two parallel segments
				v1normal.Normalize();
				vnormal.x = v1normal.x;
				vnormal.y = v1normal.y;
				}
			else
				{
				v1normal.Normalize();
				v2normal.Normalize();

				// Find intersection of the two edges meeting this points, but
				// shift those lines outwards along their normals

				PINFLOAT A,B,C,D,E,F;

				// First line
				A = -(pvmiddle->y - pv1->y);
				B = (pvmiddle->x - pv1->x);

				// Shift line along the normal
				v1.x -= v1normal.x;
				v1.y -= v1normal.y;

				C = -(A*v1.x + B*v1.y);

				// Second line
				D = -(pvmiddle->y - pv2->y);
				E = (pvmiddle->x - pv2->x);

				// Shift line along the normal
				v2.x -= v2normal.x;
				v2.y -= v2normal.y;

				F = -(D*v2.x + E*v2.y);

				PINFLOAT det = (A*E) - (B*D);

				PINFLOAT intersectx, intersecty;

				intersectx=(B*F-E*C)/det;

				intersecty=(C*D-A*F)/det;

				//rgv[i].x = (float)intersectx;
				//rgv[i].y = (float)intersecty;

				//Calc2DNormal(pv1, pv2, &vnormal);

				vnormal.x = (float)(-intersectx + pvmiddle->x);
				vnormal.y = (float)(-intersecty + pvmiddle->y);
				}
			}
		}

			{
			Vertex *pvT;
			float dx,dy;
			pvT = (Vertex *)vvertex.ElementAt(i);
			dx = pv1->x - pvT->x;
			dy = pv1->y - pvT->y;
			float length = (float)sqrt(dx*dx + dy*dy);

			currentlength += length;
			}

		float widthcur = ((currentlength/totallength) * (m_d.m_widthtop - m_d.m_widthbottom)) + m_d.m_widthbottom;

		if (ppheight)
			{
			float percentage = 1-(currentlength/totallength);
			float heightcur = ((1 - percentage) * (m_d.m_heighttop - m_d.m_heightbottom)) + m_d.m_heightbottom;
			(*ppheight)[i] = heightcur;
			}

		if (ppratio)
			{
			float percentage = 1-(currentlength/totallength);
			(*ppratio)[i] = percentage;
			}

		rgv[i] = *((Vertex *)vvertex.ElementAt(i));
		rgv[cvertex*2 - i - 1] = *((Vertex *)vvertex.ElementAt(i));

		rgv[i].x += vnormal.x * (widthcur/2);
		rgv[i].y += vnormal.y * (widthcur/2);
		rgv[cvertex*2 - i - 1].x -= vnormal.x * (widthcur/2);
		rgv[cvertex*2 - i - 1].y -= vnormal.y * (widthcur/2);
		}

	if (ppfCross)
		{
		for (i=0;i<cvertex;i++)
			{
			(*ppfCross)[i] = vvertex.ElementAt(i)->fControlPoint;
			}
		}

	//rgv[i] = *((Vertex *)vvertex.ElementAt(i));
	//delete vvertex.ElementAt(i);

	for (i=0;i<cvertex;i++)
		{
		delete vvertex.ElementAt(i);
		}

	*pcvertex = cvertex;
	return rgv;
	}

void Ramp::GetRgVertex(Vector<RenderVertex> *pvv)
	{
	//int cpointCur;
	int i;
	int cpoint;
	RenderVertex rendv1, rendv2;

	cpoint = m_vdpoint.Size();

	//cpointCur = 0;

	for (i=0;i<(cpoint-1);i++)
		{
		BOOL fNoSmooth = fTrue;
		CComObject<DragPoint> *pdp0;
		CComObject<DragPoint> *pdp3;
		CComObject<DragPoint> *pdp1;// = m_vdpoint.ElementAt(i);
		CComObject<DragPoint> *pdp2;// = m_vdpoint.ElementAt((i+1)%cpoint);

		pdp1 = m_vdpoint.ElementAt(i);

		pdp2 = m_vdpoint.ElementAt(i+1);

		if (i>0 && pdp1->m_fSmooth)
			{
			pdp0 = m_vdpoint.ElementAt(i-1);
			}
		else
			{
			pdp0 = m_vdpoint.ElementAt(i);
			}

		if (i<cpoint-2 && pdp2->m_fSmooth)
			{
			pdp3 = m_vdpoint.ElementAt(i+2);
			}
		else
			{
			pdp3 = m_vdpoint.ElementAt(i+1);
			}

		CatmullCurve cc;
		cc.SetCurve(&pdp0->m_v, &pdp1->m_v, &pdp2->m_v, &pdp3->m_v);

		rendv1.x = pdp1->m_v.x;
		rendv1.y = pdp1->m_v.y;
		rendv1.fSmooth = pdp1->m_fSmooth;
		rendv1.fControlPoint = fTrue;

		// Properties of last point don't matter, because it won't be added to the list on this pass (it'll get added as the first point of the next curve)
		rendv2.x = pdp2->m_v.x;
		rendv2.y = pdp2->m_v.y;

		RecurseSmoothLine(&cc, 0, 1, &rendv1, &rendv2, pvv);
		}

	// Add the very last point to the list because nobody else added it
	RenderVertex *pvT = new RenderVertex;
	*pvT = rendv2;
	pvv->AddElement(pvT);
	}

void Ramp::GetTimers(Vector<HitTimer> *pvht)
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

void Ramp::GetHitShapes(Vector<HitObject> *pvho)
	{
	int i;
	Vertex *rgv;
	int cvertex;
	Vertex *pv1, *pv2, *pv3, *pv4;
	float *rgheight;

	rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	float wallheightright;
	float wallheightleft;

	if (m_d.m_type == RampTypeFlat)
		{
		wallheightright = m_d.m_rightwallheight;
		wallheightleft = m_d.m_leftwallheight;
		}
	else
		{
		if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
			{
			wallheightright = 62.0f;
			}
		else
			{
			wallheightright = 6+12.5f;
			}

		if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
			{
			wallheightleft = 62.0f;
			}
		else
			{
			wallheightleft = 6+12.5f;
			}

		if (m_d.m_type == RampType2Wire)
			{
			// backwards compatible physics
			wallheightright = 31.0f;
			wallheightleft = 31.0f;
			}
		}

	if (wallheightright > 0)
		{
		for (i=0;i<(cvertex-1);i++)
			{
			if (i>0)
				{
				pv1 = &rgv[i-1];
				}
			else
				{
				pv1 = NULL;
				}

			//pv1 = &rgv[i];
			pv2 = &rgv[i];
			pv3 = &rgv[i+1];
			//pv4 = &rgv[(i+3) % count];

			if (i<(cvertex-2))
				{
				pv4 = &rgv[i+2];
				}
			else
				{
				pv4 = NULL;
				}

			AddLine(pvho, pv2, pv3, pv1, rgheight[i], rgheight[i+1]+wallheightright);

			AddLine(pvho, pv3, pv2, pv4, rgheight[i], rgheight[i+1]+wallheightright);
			}
		}

	if (wallheightleft > 0)
		{
		for (i=0;i<(cvertex-1);i++)
			{
			if (i>0)
				{
				pv1 = &rgv[cvertex + i - 1];
				}
			else
				{
				pv1 = NULL;
				}

			//pv1 = &rgv[i];
			pv2 = &rgv[cvertex + i];
			pv3 = &rgv[cvertex + i + 1];
			//pv4 = &rgv[(i+3) % count];

			if (i<(cvertex-2))
				{
				pv4 = &rgv[cvertex + i + 2];
				}
			else
				{
				pv4 = NULL;
				}

			AddLine(pvho, pv2, pv3, pv1, rgheight[cvertex - i - 2], rgheight[cvertex - i - 1] + wallheightleft);

			AddLine(pvho, pv3, pv2, pv4, rgheight[cvertex - i - 2], rgheight[cvertex - i - 1] + wallheightleft);
			}
		}

	//Level *plevel;

	Hit3DPoly *ph3dpolyOld = NULL;

	for (i=0;i<(cvertex-1);i++)
		{
		Vertex3D rgv3D[4];

		Hit3DPoly *ph3dpoly;

		pv1 = &rgv[i];
		pv2 = &rgv[cvertex*2 - i - 1];
		pv3 = &rgv[cvertex*2 - i - 2];
		pv4 = &rgv[i+1];

		rgv3D[1].x = pv1->x;
		rgv3D[1].y = pv1->y;
		rgv3D[1].z = rgheight[i];
		rgv3D[0].x = pv2->x;
		rgv3D[0].y = pv2->y;
		rgv3D[0].z = rgheight[i];
		rgv3D[2].x = pv3->x;
		rgv3D[2].y = pv3->y;
		rgv3D[2].z = rgheight[i+1];

		ph3dpoly = new Hit3DPoly(rgv3D,3);
		ph3dpoly->m_elasticity = m_d.m_elasticity;

		if (m_d.m_type == RampTypeFlat)
			{
			ph3dpoly->m_fVisible = fTrue;
			}

		pvho->AddElement(ph3dpoly);

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

		ph3dpoly = new Hit3DPoly(rgv3D,3);
		ph3dpoly->m_elasticity = m_d.m_elasticity;

		if (m_d.m_type == RampTypeFlat)
			{
			ph3dpoly->m_fVisible = fTrue;
			}

		pvho->AddElement(ph3dpoly);

		CheckJoint(pvho, ph3dpolyOld, ph3dpoly);

		ph3dpolyOld = ph3dpoly;
		}

	Vertex3D rgv3D[4];
	rgv3D[2].x = pv1->x;
	rgv3D[2].y = pv1->y;
	rgv3D[2].z = rgheight[i];
	rgv3D[1].x = pv3->x;
	rgv3D[1].y = pv3->y;
	rgv3D[1].z = rgheight[i];
	rgv3D[0].x = pv4->x;
	rgv3D[0].y = pv4->y;
	rgv3D[0].z = rgheight[i];
	ph3dpolyOld = new Hit3DPoly(rgv3D,3);
	CheckJoint(pvho, ph3dpolyOld, ph3dpolyOld);
	delete ph3dpolyOld;

	delete rgheight;
	delete rgv;
	}

void Ramp::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void Ramp::CheckJoint(Vector<HitObject> *pvho, Hit3DPoly *ph3d1, Hit3DPoly *ph3d2)
	{
	Vertex3D vCrossProduct;
	Vertex3D vCrossFace;

	// Calculate vector perpendicular to both normals.
	// Then, calculate the vector perpendicular to normal1 and the previous
	// cross product.  This will give the vector co-planar to hitpoly1 that
	// is perpendicular to its common edge with hitpoly2.
	// The angle between the second cross-product and normal2 can then
	// be measured against 90 degrees.

	vCrossProduct.x = ph3d1->normal.y * ph3d2->normal.z - ph3d1->normal.z * ph3d2->normal.y;
	vCrossProduct.y = ph3d1->normal.z * ph3d2->normal.x - ph3d1->normal.x * ph3d2->normal.z;
	vCrossProduct.z = ph3d1->normal.x * ph3d2->normal.y - ph3d1->normal.y * ph3d2->normal.x;

	vCrossFace.x = ph3d1->normal.y * vCrossProduct.z - ph3d1->normal.z * vCrossProduct.y;
	vCrossFace.y = ph3d1->normal.z * vCrossProduct.x - ph3d1->normal.x * vCrossProduct.z;
	vCrossFace.z = ph3d1->normal.x * vCrossProduct.y - ph3d1->normal.y * vCrossProduct.x;

	float dot;

	dot = ph3d1->normal.x*vCrossFace.x + ph3d1->normal.y*vCrossFace.y + ph3d1->normal.z*vCrossFace.z;

	if (1)//dot > 0) // Triangles are greater than zero degrees apart - need joint
		{
		Vertex3D vjointnormal;
		vjointnormal.x = ph3d1->normal.x + ph3d2->normal.x;
		vjointnormal.y = ph3d1->normal.y + ph3d2->normal.y;
		vjointnormal.z = ph3d1->normal.z + ph3d2->normal.z;

		float length = (float)sqrt(vjointnormal.x * vjointnormal.x + vjointnormal.y * vjointnormal.y + vjointnormal.z * vjointnormal.z);
		vjointnormal.x /= length;
		vjointnormal.y /= length;
		vjointnormal.z /= length;

		// By convention of the calling function, points 1 and 2 of the second polygon will
		// be the common-edge points

		Hit3DCylinder *ph3dc;
		ph3dc = new Hit3DCylinder(&ph3d2->m_rgv[0], &ph3d2->m_rgv[1], &vjointnormal);
		ph3dc->m_elasticity = m_d.m_elasticity;
		pvho->AddElement(ph3dc);
		}
	}

void Ramp::AddLine(Vector<HitObject> *pvho, Vertex *pv1, Vertex *pv2, Vertex *pv3, float height1, float height2)
	{
	LineSeg *plineseg;
	Joint *pjoint;
	float dot;
	Vertex vt1, vt2;
	float length;

	plineseg = new LineSeg();
	plineseg->m_elasticity = m_d.m_elasticity;

	//if (m_d.m_fHitEvent)
		//{
		//plineseg->m_pep = (EventProxyBase *)this;
		//plineseg->m_threshold = m_d.m_threshold;
		//}
	//else
		//{
		plineseg->m_pfe = NULL;
		//}

	plineseg->m_rcHitRect.zlow = height1;//m_d.m_heightbottom;
	plineseg->m_rcHitRect.zhigh = height2;//m_d.m_heighttop;

	plineseg->v1.x = pv1->x;
	plineseg->v1.y = pv1->y;
	plineseg->v2.x = pv2->x;
	plineseg->v2.y = pv2->y;

	pvho->AddElement(plineseg);

	plineseg->CalcNormal();

	vt1.x = pv1->x - pv2->x;
	vt1.y = pv1->y - pv2->y;

	// Set up line normal
	/*length = (float)sqrt((vt1.x * vt1.x) + (vt1.y * vt1.y));
	plineseg->normal.x = vt1.y / length;
	plineseg->normal.y = -vt1.x / length;*/

	if (pv3)
		{
		vt2.x = pv1->x - pv3->x;
		vt2.y = pv1->y - pv3->y;

		dot = vt1.x*vt2.y - vt1.y*vt2.x;

		if (dot < 0) // Inside edges don't need joint hit-testing (dot == 0 continuous segments should mathematically never hit)
			{
			pjoint = new Joint();
			pjoint->m_elasticity = m_d.m_elasticity;

			//if (m_d.m_fHitEvent)
				//{
				//pjoint->m_pep = (EventProxyBase *)this;
				//pjoint->m_threshold = m_d.m_threshold;
				//}
			//else
				//{
				pjoint->m_pfe = NULL;
				//}

			pjoint->m_rcHitRect.zlow = height1;//m_d.m_heightbottom;
			pjoint->m_rcHitRect.zhigh = height2;//m_d.m_heighttop;

			pjoint->center.x = pv1->x;
			pjoint->center.y = pv1->y;
			pvho->AddElement(pjoint);

			Vertex normalT;

			// Set up line normal
			length = (float)sqrt((vt2.x * vt2.x) + (vt2.y * vt2.y));
			normalT.x = -vt2.y / length;
			normalT.y = vt2.x / length;

			pjoint->normal.x = normalT.x + plineseg->normal.x;
			pjoint->normal.y = normalT.y + plineseg->normal.y;

			// Set up line normal
			length = (float)sqrt((pjoint->normal.x * pjoint->normal.x) + (pjoint->normal.y * pjoint->normal.y));
			pjoint->normal.x = pjoint->normal.x / length;
			pjoint->normal.y = pjoint->normal.y / length;
			}
		}

	return;
	}

void Ramp::EndPlay()
	{
	int i;
	IEditable::EndPlay();

	for (i=0;i<m_vlevel.Size();i++)
		{
		delete m_vlevel.ElementAt(i);
		}
	m_vlevel.RemoveAllElements();
	}

int rgicrosssection[][3] = {
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
	12,28,31,
	};

void Ramp::RenderStaticHabitrail(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Vertex3D rgv3D[32];
	WORD rgi[4];
	int i,l;

	pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);

	float r = (m_d.m_color & 255) / 255.0f;
	float g = (m_d.m_color & 65280) / 65280.0f;
	float b = (m_d.m_color & 16711680) / 16711680.0f;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = r;
	mtrl.diffuse.g = mtrl.ambient.g = g;
	mtrl.diffuse.b = mtrl.ambient.b = b;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0;

	mtrl.specular.r = 1;
	mtrl.specular.g = 1;
	mtrl.specular.b = 1;
	mtrl.specular.a = 1;
	mtrl.power = 8;

	pd3dDevice->SetMaterial(&mtrl);

	Vertex *rgv;
	float *rgheight;
	int cvertex;

	rgv = GetRampVertex(&cvertex, &rgheight, NULL, NULL);

	for (i=0;i<4;i++)
		{
		rgi[i]=i;
		}

	for (i=0;i<cvertex;i++)
		{
		rgv3D[0].x = -3;
		rgv3D[0].y = -3;
		rgv3D[0].z = 0;
		rgv3D[0].nx = -1;
		rgv3D[0].ny = -1;
		rgv3D[0].nz = 0;
		rgv3D[0].NormalizeNormal();

		rgv3D[1].x = 3;
		rgv3D[1].y = -3;
		rgv3D[1].z = 0;
		rgv3D[1].nx = 1;
		rgv3D[1].ny = -1;
		rgv3D[1].nz = 0;
		rgv3D[1].NormalizeNormal();

		rgv3D[2].x = 3;
		rgv3D[2].y = 3;
		rgv3D[2].z = 0;
		rgv3D[2].nx = 1;
		rgv3D[2].ny = 1;
		rgv3D[2].nz = 0;
		rgv3D[2].NormalizeNormal();

		rgv3D[3].x = -3;
		rgv3D[3].y = 3;
		rgv3D[3].z = 0;
		rgv3D[3].nx = -1;
		rgv3D[3].ny = 1;
		rgv3D[3].nz = 0;
		rgv3D[3].NormalizeNormal();

		for (l=0;l<4;l++)
			{
			rgv3D[l+4].x = rgv3D[l].x + 44; //44
			rgv3D[l+4].y = rgv3D[l].y - 19; //22
			rgv3D[l+4].z = rgv3D[l].z;
			rgv3D[l+4].nx = rgv3D[l].nx;
			rgv3D[l+4].ny = rgv3D[l].ny;
			rgv3D[l+4].nz = rgv3D[l].nz;
			}

		for (l=0;l<4;l++)
			{
			rgv3D[l+8].x = rgv3D[l].x + 9.5f;
			rgv3D[l+8].y = rgv3D[l].y + 19;
			rgv3D[l+8].z = rgv3D[l].z;
			rgv3D[l+8].nx = rgv3D[l].nx;
			rgv3D[l+8].ny = rgv3D[l].ny;
			rgv3D[l+8].nz = rgv3D[l].nz;
			}

		for (l=0;l<4;l++)
			{
			rgv3D[l+12].x = rgv3D[l].x + 44;
			rgv3D[l+12].y = rgv3D[l].y + 19;
			rgv3D[l+12].z = rgv3D[l].z;
			rgv3D[l+12].nx = rgv3D[l].nx;
			rgv3D[l+12].ny = rgv3D[l].ny;
			rgv3D[l+12].nz = rgv3D[l].nz;
			}

		for (l=0;l<4;l++)
			{
			rgv3D[l].x = rgv3D[l].x + 9.5f;
			rgv3D[l].y = rgv3D[l].y - 19;
			}

		int p1,p2,p3,p4;

		p1 = (i==0) ? 0 : i-1;
		p2 = i;
		p3 = (i==(cvertex-1)) ? cvertex-1 : i+1;
		p4 = cvertex*2 - i -1;

		Vertex3D vacross;
		Vertex3D tangent;
		Vertex3D vnewup;

		vacross.x = rgv[p4].x - rgv[p2].x;
		vacross.y = rgv[p4].y - rgv[p2].y;
		vacross.z = 0;

		// The vacross vector is our local up vector.  Rotate the cross-section
		// later to match this up
		vacross.Normalize();

		// vnewup is the beginning up vector of the cross-section
		vnewup.x = 0;
		vnewup.y = 1;
		vnewup.z = 0;

		tangent.x = (rgv[p3].x - rgv[p1].x);
		tangent.y = (rgv[p3].y - rgv[p1].y);
		tangent.z = (rgheight[p3] - rgheight[p1]);

		// This is the vector describing the tangent to the ramp at this point
		tangent.Normalize();

		Vertex3D up;

		up.x = 0;
		up.y = 0;
		up.z = 1;

		Vertex3D rotationaxis;

		// Get axis of rotation to rotate our cross-section into place
		CrossProduct(&tangent, &up, &rotationaxis);

		float dot, angle;

		dot = tangent.Dot(&up);
		angle = (float)acos(dot);

		RotateAround(&rotationaxis, rgv3D, 16, angle);
		RotateAround(&rotationaxis, &vnewup, 1, angle);

		float dotupcorrection, angleupcorrection;

		// vacross is not out real up vector, but the up vector for the cross-section isn't real either
		//Vertex3D vrampup;
		//CrossProduct(&tangent, &vacross, &vrampup);
		dotupcorrection = vnewup.Dot(&vacross);
		angleupcorrection = (float)acos(dotupcorrection);

		if (vacross.x >= 0)
			{
			angleupcorrection = -angleupcorrection;
			}

		RotateAround(&tangent, rgv3D, 16, -angleupcorrection);

		for (l=0;l<16;l++)
			{
			rgv3D[l].x += (rgv[p2].x + rgv[p4].x)/2;
			rgv3D[l].y += (rgv[p2].y + rgv[p4].y)/2;
			rgv3D[l].z += rgheight[p2];
			}

		//int maxsegments = (m_d.m_type == RampType4Wire) ? 32 : 16;

		if (i != 0)
			{
			RenderPolygons(pd3dDevice, rgv3D, (int *)rgicrosssection, 0, 16);
			/*for (l=0;l<16;l++)
				{
				rgi[0] = rgicrosssection[l][0];
				rgi[1] = rgicrosssection[l][1];
				rgi[2] = rgicrosssection[l][2];

				pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 32,
														  rgi, 3, NULL);
				}*/

			if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
				{
				RenderPolygons(pd3dDevice, rgv3D, (int *)rgicrosssection, 16, 24);
				}

			if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
				{
				RenderPolygons(pd3dDevice, rgv3D, (int *)rgicrosssection, 24, 32);
				}
			}

		memcpy(&rgv3D[16], &rgv3D[0], sizeof(Vertex3D)*16);
		}
		/*rgvnormal[0].x = rgv[p1].x - rgv[p2].x;
		rgvnormal[0].y = rgv[p1].y - rgv[p2].y;
		rgvnormal[0].z = rgheight[p1] - rgheight[p2];

		rgvnormal[1].x = rgv[p1].x - rgv[p4].x;
		rgvnormal[1].y = rgv[p1].y - rgv[p4].y;
		rgvnormal[1].z = rgheight[p1] - rgheight[p2];

		// Get the normal to the ramp in this section
		CrossProduct(&rgvnormal[0], &rgvnormal[1], rgvnormal[2]);*/

	delete rgv;
	delete rgheight;

	pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	}

void Ramp::RenderPolygons(LPDIRECT3DDEVICE7 pd3dDevice, Vertex3D *rgv3D, int *rgicrosssection, int start, int stop)
	{
	int i;
	WORD rgi[3];
	for (i=start;i<stop;i++)
		{
		rgi[0] = rgicrosssection[i*3 + 0];
		rgi[1] = rgicrosssection[i*3 + 1];
		rgi[2] = rgicrosssection[i*3 + 2];

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 3, NULL);
		}
	}

void Ramp::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	if (m_d.m_type == RampType4Wire || m_d.m_type == RampType2Wire || m_d.m_type == RampType3WireLeft || m_d.m_type == RampType3WireRight)
		{
		RenderStaticHabitrail(pd3dDevice);
		}
	else
		{
		Vertex3D rgv3D[4];
		WORD rgi[4];
		int i;

		Pin3D *ppin3d = &g_pplayer->m_pin3d;

		PinImage *pin = m_ptable->GetImage(m_d.m_szImage);
		float maxtu, maxtv;

		//float r = (m_d.m_color & 255) / 255.0f;
		//float g = (m_d.m_color & 65280) / 65280.0f;
		//float b = (m_d.m_color & 16711680) / 16711680.0f;

		D3DMATERIAL7 mtrl;
		ZeroMemory( &mtrl, sizeof(mtrl) );

		if (pin)
			{
			m_ptable->GetTVTU(pin, &maxtu, &maxtv);
			//pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBuffer);
			ppin3d->SetTexture(pin->m_pdsBuffer);

			mtrl.diffuse.r = mtrl.ambient.r = 1;
			mtrl.diffuse.g = mtrl.ambient.g = 1;
			mtrl.diffuse.b = mtrl.ambient.b = 1;
			mtrl.diffuse.a = mtrl.ambient.a = 1.0;//0.5;
			}
		else
			{
			float r = (m_d.m_color & 255) / 255.0f;
			float g = (m_d.m_color & 65280) / 65280.0f;
			float b = (m_d.m_color & 16711680) / 16711680.0f;

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			mtrl.diffuse.a = mtrl.ambient.a = 1;
			}

		pd3dDevice->SetMaterial(&mtrl);

		Vertex *rgv;
		float *rgheight;
		float *rgratio;
		int cvertex;

		rgv = GetRampVertex(&cvertex, &rgheight, NULL, &rgratio);

		for (i=0;i<4;i++)
			{
			rgi[i]=i;
			}

		float tablewidth = m_ptable->m_right - m_ptable->m_left;
		float tableheight = m_ptable->m_bottom - m_ptable->m_top;

		for (i=0;i<(cvertex-1);i++)
			{
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
					rgv3D[0].tu = rgv3D[0].x / tablewidth * maxtu;
					rgv3D[0].tv = rgv3D[0].y / tableheight * maxtv;
					rgv3D[1].tu = rgv3D[1].x / tablewidth * maxtu;
					rgv3D[1].tv = rgv3D[1].y / tableheight* maxtv;
					rgv3D[2].tu = rgv3D[2].x / tablewidth * maxtu;
					rgv3D[2].tv = rgv3D[2].y / tableheight* maxtv;
					rgv3D[3].tu = rgv3D[3].x / tablewidth * maxtu;
					rgv3D[3].tv = rgv3D[3].y / tableheight* maxtv;
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

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 4,
														  rgi, 4, NULL);
			}

		if (pin && !m_d.m_fImageWalls)
			{
			ppin3d->SetTexture(NULL);

			float r = (m_d.m_color & 255) / 255.0f;
			float g = (m_d.m_color & 65280) / 65280.0f;
			float b = (m_d.m_color & 16711680) / 16711680.0f;

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
			mtrl.diffuse.a = mtrl.ambient.a = 1;

			pd3dDevice->SetMaterial(&mtrl);
			}

		for (i=0;i<(cvertex-1);i++)
			{
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
					rgv3D[0].tu = rgv3D[0].x / tablewidth * maxtu;
					rgv3D[0].tv = rgv3D[0].y / tableheight * maxtv;
					rgv3D[2].tu = rgv3D[2].x / tablewidth * maxtu;
					rgv3D[2].tv = rgv3D[2].y / tableheight* maxtv;
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

			//2-Sided polygon

			rgi[0] = 0;
			rgi[1] = 1;
			rgi[2] = 2;
			rgi[3] = 3;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 4,
														  rgi, 4, NULL);
			rgi[0] = 0;
			rgi[1] = 3;
			rgi[2] = 2;
			rgi[3] = 1;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 4,
														  rgi, 4, NULL);
			}

		for (i=0;i<(cvertex-1);i++)
			{
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
					rgv3D[0].tu = rgv3D[0].x / tablewidth * maxtu;
					rgv3D[0].tv = rgv3D[0].y / tableheight * maxtv;
					rgv3D[2].tu = rgv3D[2].x / tablewidth * maxtu;
					rgv3D[2].tv = rgv3D[2].y / tableheight* maxtv;
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

			//2-Sided polygon

			rgi[0] = 0;
			rgi[1] = 1;
			rgi[2] = 2;
			rgi[3] = 3;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 4,
														  rgi, 4, NULL);
			rgi[0] = 0;
			rgi[1] = 3;
			rgi[2] = 2;
			rgi[3] = 1;

			SetNormal(rgv3D, rgi, 4, NULL, NULL, NULL);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 4,
														  rgi, 4, NULL);
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

void Ramp::MoveOffset(float dx, float dy)
	{
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

void Ramp::ClearForOverwrite()
	{
	ClearPointsForOverwrite();
	}

HRESULT Ramp::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	HRESULT hr;

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
	bw.WriteFloat(FID(WLHL), m_d.m_leftwallheight);
	bw.WriteFloat(FID(WLHR), m_d.m_rightwallheight);
	bw.WriteFloat(FID(WVHL), m_d.m_leftwallheightvisible);
	bw.WriteFloat(FID(WVHR), m_d.m_rightwallheightvisible);
	bw.WriteFloat(FID(ELAS), m_d.m_elasticity);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(PNTS));
	if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
		return hr;

	bw.WriteTag(FID(ENDB));

	return S_OK;
	/*ULONG writ = 0;
	HRESULT hr = S_OK;
	//int i,temp;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = SavePointData(pstm)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_d, sizeof(RampData), &writ)))
		return hr;

	return hr;*/
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
	HRESULT hr = S_OK;
	//int i,temp;

	m_ptable = ptable;

	DWORD dwID;
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

			ClosestPointOnPolygon(rgv, cvertex, &v, &vOut, &iSeg, fFalse);

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
				pdp->m_fSmooth = fTrue; // Ramps are usually always smooth
				m_vdpoint.InsertElementAt(pdp, icp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
				}

			for (i=0;i<vvertex.Size();i++)
				{
				delete vvertex.ElementAt(i);
				}

			delete rgv;
			rgv = NULL;

			SetDirtyDraw();

			STOPUNDO
			}
			break;
		}
	}

void Ramp::FlipY(Vertex *pvCenter)
	{
	IHaveDragPoints::FlipPointY(pvCenter);
	}

void Ramp::FlipX(Vertex *pvCenter)
	{
	IHaveDragPoints::FlipPointX(pvCenter);
	}

void Ramp::Rotate(float ang, Vertex *pvCenter)
	{
	IHaveDragPoints::RotatePoints(ang, pvCenter);
	}

void Ramp::Scale(float scalex, float scaley, Vertex *pvCenter)
	{
	IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
	}

void Ramp::Translate(Vertex *pvOffset)
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

/*HRESULT Ramp::GetTypeName(BSTR *pVal)
	{
	*pVal = SysAllocString(L"Ramp");

	return S_OK;
	}*/

/*int Ramp::GetDialogID()
	{
	return IDD_PROPRAMP;
	}*/

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
	OLECHAR wz[512];

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
	*pVal = FTOVB(m_d.m_fImageWalls);

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
	*pVal = FTOVB(m_d.m_fCastsShadow);

	return S_OK;
}

STDMETHODIMP Ramp::put_CastsShadow(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_d.m_fCastsShadow = VBTOF(newVal);
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
