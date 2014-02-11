#include "StdAfx.h"


void LightProjected::CalcCoordinates(Vertex3D * const pv) const
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
