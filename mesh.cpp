#include "StdAfx.h"

void PolygonToTriangles(const RenderVertex * const rgv, Vector<void> * const pvpoly, Vector<Triangle> * const pvtri)
	{
	// There should be this many convex triangles.
	// If not, the polygon is self-intersecting
	const int tricount = pvpoly->Size() - 2;

	Assert(tricount > 0);

	for (int l=0; l<tricount; ++l)
	//while (pvpoly->Size() > 2)
		{
		for (int i=0; i<pvpoly->Size(); ++i)
			{
			const int s    = pvpoly->Size();
			const int pre  = (int)pvpoly->ElementAt((i == 0) ? (s-1) : (i-1));
			const int a    = (int)pvpoly->ElementAt(i);
			const int b    = (int)pvpoly->ElementAt((i < s-1) ? (i+1) : 0);
			const int c    = (int)pvpoly->ElementAt((i < s-2) ? (i+2) : ((i+2) - s));
			const int post = (int)pvpoly->ElementAt((i < s-3) ? (i+3) : ((i+3) - s));			
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

void LightProjected::CalcCoordinates(Vertex3D * const pv, const float inv_width, const float inv_height) const
	{
	const Vertex2D vOrigin(
	// light is rotated around the light as the origin
	// z doesn't matter because the texture is projected through z
		pv->x - m_v.x,
		pv->y - m_v.y);
	//	pv->z);

	// Rotation
	//Vertex2D vT;
	{
	//const float sn = sinf(rotation);
	//const float cs = cosf(rotation);

	//vT.x = cs * vOrigin.x + sn * vOrigin.y;
	//vT.y = cs * vOrigin.y - sn * vOrigin.x;
	}

	// Inclination
	{
	//const float sn = sinf(inclination);
	//const float cs = cosf(inclination);

	//vT.z = cs * vOrigin.z - sn * vT.y;
	//vT.y = cs * vT.y + sn * vOrigin.z;
	}

	// Put coordinates into vertex
	//vT.x += m_v.x;
	//vT.y += m_v.y;
	//vT.z += 0;

	pv->tu2 = (vOrigin.x /*(vT.x - m_v.x)*/ * inv_width  + 0.5f) * g_pplayer->m_pin3d.m_maxtu;
	pv->tv2 = (vOrigin.y /*(vT.y - m_v.y)*/ * inv_height + 0.5f) * g_pplayer->m_pin3d.m_maxtv;
	}

void SetHUDVertices(Vertex3D * const rgv, const int count)
	{
	const float mult = (float)g_pplayer->m_pin3d.m_dwRenderWidth * (float)(1.0/1000.0);
	const float ymult = mult / (float)g_pplayer->m_pixelaspectratio;

	for (int i=0; i<count; ++i)
		{
		rgv[i].x *= mult;
		rgv[i].y *= ymult;
		rgv[i].x -= 0.5f;
		rgv[i].y -= 0.5f;
		rgv[i].z = 0;//1.0f;
		rgv[i].rhw = 0.1f;
		rgv[i].specular = 0;
		}

	if (g_pplayer->m_frotate)
		{
		for (int i=0; i<count; ++i)
			{
			const float ftemp = rgv[i].x;
			rgv[i].x = rgv[i].y;
			rgv[i].y = g_pplayer->m_pin3d.m_dwViewPortHeight - ftemp;
			}
		}
	}

//copy pasted from above
void SetHUDVertices(Vertex3D_NoTex2 * const rgv, const int count)
	{
	const float mult = (float)g_pplayer->m_pin3d.m_dwRenderWidth * (float)(1.0/1000.0);
	const float ymult = mult / (float)g_pplayer->m_pixelaspectratio;

	for (int i=0; i<count; ++i)
		{
		rgv[i].x *= mult;
		rgv[i].y *= ymult;
		rgv[i].x -= 0.5f;
		rgv[i].y -= 0.5f;
		rgv[i].z = 0;//1.0f;
		rgv[i].rhw = 0.1f;
		rgv[i].specular = 0;
		}

	if (g_pplayer->m_frotate)
		{
		for (int i=0; i<count; ++i)
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
	vmid.fSmooth = true; // Generated points must always be smooth, because they are part of the curve
	vmid.fSlingshot = false; // Slingshots can't be along curves
	vmid.fControlPoint = false; // We created this point, so it can't be a control point

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


void RecurseSmoothLineWithAccuracy(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv, const float accuracy)
	{
	const float tMid = (t1+t2)*0.5f;
	RenderVertex vmid;
	pcc->GetPointAt(tMid, &vmid);
	vmid.fSmooth = true; // Generated points must always be smooth, because they are part of the curve
	vmid.fSlingshot = false; // Slingshots can't be along curves
	vmid.fControlPoint = false; // We created this point, so it can't be a control point

	if (FlatWithAccuracy(pvt1, pvt2, &vmid, accuracy))
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

///<summary>
///Calculate if two vectors are flat to each other
///accuracy is a float greater 4 and smaller 4000000 (tested this out)
///<param name="pvt1">vector1</param>
///<param name="pvt2">vector2</param>
///<param name="pvtMid">Vector middle</param>
///<param name="accuracy">Accuracy value from 4 to 4000000</param>
///</summary>
bool FlatWithAccuracy(const Vertex2D * const pvt1, const Vertex2D * const pvt2, const Vertex2D * const pvtMid, const float accuracy)
{
	const float det1 = pvt1->x*pvtMid->y - pvt1->y*pvtMid->x;
	const float det2 = pvtMid->x*pvt2->y - pvtMid->y*pvt2->x;
	const float det3 = pvt2->x*pvt1->y - pvt2->y*pvt1->x;

	const float dblarea = det1+det2+det3;

	return (dblarea*dblarea < accuracy);
}
