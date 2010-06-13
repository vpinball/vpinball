#include "StdAfx.h"

//#include "math.h"

ObjFrame::ObjFrame()
	{
	rc.left = 0;
	rc.top = 0;
	rc.right = 1000;
	rc.bottom = 750;

	pdds = NULL;
	pddsZBuffer = NULL;

	pTexture = NULL;
	u = 0.0f;
	v = 0.0f;
	}

ObjFrame::~ObjFrame()
	{
	SAFE_RELEASE(pdds);
	SAFE_RELEASE(pddsZBuffer);

	SAFE_RELEASE(pTexture);
	}

void PolygonToTriangles(const RenderVertex * const rgv, Vector<void> * const pvpoly, Vector<Triangle> * const pvtri)
	{
	// There should be this many convex triangles.
	// If not, the polygon is self-intersecting
	const int tricount = pvpoly->Size() - 2;

	Assert(tricount > 0);

	for (int l = 0; l<tricount; l++)
	//while (pvpoly->Size() > 2)
		{
		for (int i=0;i<pvpoly->Size();i++)
			{
			const int a = (int)pvpoly->ElementAt(i);
			const int b = (int)pvpoly->ElementAt((i+1) % pvpoly->Size());
			const int c = (int)pvpoly->ElementAt((i+2) % pvpoly->Size());
			const int pre = (int)pvpoly->ElementAt((i-1+pvpoly->Size()) % pvpoly->Size());
			const int post = (int)pvpoly->ElementAt((i+3) % pvpoly->Size());
			if (AdvancePoint(rgv, pvpoly, a, b, c, pre, post))
				{
				Triangle * const ptri = new Triangle();
				ptri->a = a;
				ptri->b = b;
				ptri->c = c;
				pvtri->AddElement(ptri);
				pvpoly->RemoveElement((void *)b);
				break;
				}
			}
		}
	}

void LightProjected::CalcCoordinates(Vertex3D * const pv)
	{
	Vertex3Ds vOrigin, vT;

	// light is rotated around the light as the origin
	// z doesn't matter because the texture is projected through z
	vOrigin.x = pv->x - m_v.x;
	vOrigin.y = pv->y - m_v.y;
	vOrigin.z = pv->z - 0;

	// Rotation

	{
	const float sn = (float)sin(0.0);
	const float cs = (float)cos(0.0);

	vT.x = cs * vOrigin.x + sn * vOrigin.y;
	vT.y = cs * vOrigin.y - sn * vOrigin.x;
	}

	// Inclination

	{
	const float sn = (float)sin(0.0);
	const float cs = (float)cos(0.0);

	vT.z = cs * vOrigin.z - sn * vT.y;
	vT.y = cs * vT.y + sn * vOrigin.z;
	}

	// Put coordinates into vertex

	vT.x += m_v.x;
	vT.y += m_v.y;
	//vT.z += 0;

	const float width  = g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right;
	const float height = g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom;

	pv->tu2 = (((vT.x - m_v.x) / width)  + 0.5f) * g_pplayer->m_pin3d.m_maxtu;
	pv->tv2 = (((vT.y - m_v.y) / height) + 0.5f) * g_pplayer->m_pin3d.m_maxtv;
	}

void SetHUDVertices(Vertex3D * const rgv, const int count)
	{
	const float mult = (float)g_pplayer->m_pin3d.m_dwRenderWidth * (float)(1.0/1000.0);
	const float ymult = mult / (float)g_pplayer->m_pixelaspectratio;

	for (int i=0;i<count;i++)
		{
		rgv[i].x *= mult;
		rgv[i].y *= ymult;
		rgv[i].x -= 0.5f;
		rgv[i].y -= 0.5f;
		rgv[i].z = 0;//1.0f;//0;
		rgv[i].rhw = 0.1f;
		rgv[i].specular = 0;
		}

	if (g_pplayer->m_frotate)
		{
		for (int i=0;i<count;i++)
			{
			const float ftemp = rgv[i].x;
			rgv[i].x = rgv[i].y;
			rgv[i].y = g_pplayer->m_pin3d.m_dwViewPortHeight - ftemp;
			}
		}
	}

void RecurseSmoothLine(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv)
	{
	const float tMid = (t1+t2)*0.5f;
	RenderVertex vmid;
	pcc->GetPointAt(tMid, &vmid);
	vmid.fSmooth = fTrue; // Generated points must always be smooth, because they are part of the curve
	vmid.fSlingshot = fFalse; // Slingshots can't be along curves
	vmid.fControlPoint = fFalse; // We created this point, so it can't be a control point

	if (Flat(pvt1, pvt2, &vmid))
		{
		// Add first segment point to array.
		// Last point never gets added by this recursive loop,
		// but that's where it wraps around to the next curve.
		RenderVertex * const pvT = new RenderVertex;
		*pvT = *pvt1;
		pvv->AddElement(pvT);
		}
	else
		{
		RecurseSmoothLine(pcc, t1, tMid, pvt1, &vmid, pvv);
		RecurseSmoothLine(pcc, tMid, t2, &vmid, pvt2, pvv);
		} 
	}
