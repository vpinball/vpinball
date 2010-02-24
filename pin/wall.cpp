#include "stdafx.h"
#include "..\Main.h"

/*Wall::Wall()
	{
	Vertex *pvertex;

	pvertex = new Vertex();
	pvertex->x = 4;
	pvertex->y = 4;
	m_vvertex.AddElement(pvertex);

	pvertex = new Vertex();
	pvertex->x = 4;
	pvertex->y = 20;
	m_vvertex.AddElement(pvertex);

	pvertex = new Vertex();
	pvertex->x = 20;
	pvertex->y = 20;
	m_vvertex.AddElement(pvertex);

	pvertex = new Vertex();
	pvertex->x = 15;
	pvertex->y = 15;
	m_vvertex.AddElement(pvertex);

	pvertex = new Vertex();
	pvertex->x = 20;
	pvertex->y = 4;
	m_vvertex.AddElement(pvertex);
	}

void Wall::CurvesToShapes(Vector<HitObject> *pvho)
	{
	int i;
	int count;
	Vertex *pv1, *pv2, *pv3, *pv4;
	Vector<Vertex> vvertex;

	count = m_vvertex.Size();

	for (i=0;i<count;i++)
		{
		pv1 = m_vvertex.ElementAt(i);
		pv2 = m_vvertex.ElementAt((i+1) % count);
		pv3 = m_vvertex.ElementAt((i+2) % count);
		pv4 = m_vvertex.ElementAt((i+3) % count);
		AddSmoothLine(&vvertex, pv1, pv2, pv3, pv4);
		}

	count = vvertex.Size();

	for (i=0;i<count;i++)
		{
		pv1 = vvertex.ElementAt(i);
		pv2 = vvertex.ElementAt((i+1) % count);
		pv3 = vvertex.ElementAt((i+2) % count);
		pv4 = vvertex.ElementAt((i+3) % count);
		//AddLine(pvlineseg, pvjoint, pv2, pv3, pv1);
		AddLine(pvho, pv3, pv2, pv4);
		}
	}

void Wall::AddSmoothLine(Vector<Vertex> *pvvertex, Vertex *pv1, Vertex *pv2, Vertex *pv3, Vertex *pv4)
	{
	float rstep;
	float t;
	float t2,t3;
	float x,y;
	Vertex *pvertex;

	float x1T,x2T,x3T,x4T;
	float y1T,y2T,y3T,y4T;
	
	x1T=pv1->x;
	x2T=pv2->x;
	x3T=pv3->x;
	x4T=pv4->x;
	y1T=pv1->y;
	y2T=pv2->y;
	y3T=pv3->y;
	y4T=pv4->y;

	rstep = 0.1f;

	for (t=0; t<=1.0; t+=rstep)
		{
		t2 = t*t;
		t3 = t2*t;
		x = (float)(0.5 * ((-x1T + 3*x2T -3*x3T + x4T)*t3
			+ (2*x1T -5*x2T + 4*x3T - x4T)*t2
			+ (-x1T + x3T)*t
			+ 2*x2T));

		y = (float)(0.5 * ((-y1T + 3*y2T -3*y3T + y4T)*t3
			+ (2*y1T -5*y2T + 4*y3T - y4T)*t2
			+ (-y1T+y3T)*t
			+ 2*y2T));

		pvertex = new Vertex();
		pvertex->x = x;
		pvertex->y = y;
		pvvertex->AddElement(pvertex);
		}
	}

void Wall::AddLine(Vector<HitObject> *pvho, Vertex *pv1, Vertex *pv2, Vertex *pv3)
	{
	LineSeg *plineseg;
	Joint *pjoint;
	float dot;
	Vertex vt1, vt2;
	float length;

	plineseg = new LineSeg();
	plineseg->v1.x = pv1->x;
	plineseg->v1.y = pv1->y;
	plineseg->v2.x = pv2->x;
	plineseg->v2.y = pv2->y;
	if (plineseg->v1.x == plineseg->v2.x)
		{
		plineseg->v2.x += 0.0001f;
		}
	if (plineseg->v1.y == plineseg->v2.y)
		{
		plineseg->v2.y += 0.0001f;
		}

	pvho->AddElement(plineseg);

	plineseg->CalcNormal();

	vt1.x = pv1->x - pv2->x;
	vt1.y = pv1->y - pv2->y;

	// Set up line normal
	//length = (float)sqrt((vt1.x * vt1.x) + (vt1.y * vt1.y));
	//plineseg->normal.x = vt1.y / length;
	//plineseg->normal.y = -vt1.x / length;

	vt2.x = pv1->x - pv3->x;
	vt2.y = pv1->y - pv3->y;

	dot = vt1.x*vt2.y - vt1.y*vt2.x;

	if (dot < 0) // Inside edges don't need joint hit-testing (dot == 0 continuous segments should mathematically never hit)
		{
		pjoint = new Joint();
		pjoint->v.x = pv1->x;
		pjoint->v.y = pv1->y;
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

	return;
	}

void Wall::GetHitShapes(Vector<HitObject> *pvho)
	{
	CurvesToShapes(pvho);
	}

void WallCircle::GetHitShapes(Vector<HitObject> *pvho)
	{
	HitCircle *pcircle;

	pcircle = new HitCircle();
	pcircle->center.x = 18;
	pcircle->center.y = 18;
	pcircle->radius = 3;

	pvho->AddElement(pcircle);
	}*/
