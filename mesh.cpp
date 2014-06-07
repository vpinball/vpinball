#include "StdAfx.h"

void RecurseSmoothLine(const CatmullCurve2D * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv)
	{
        RecurseSmoothLineWithAccuracy(pcc, t1, t2, pvt1, pvt2, pvv, 1.0 / (0.5 * 0.5));
	}


void RecurseSmoothLineWithAccuracy(const CatmullCurve2D * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv, const float accuracy)
	{
	const float tMid = (t1+t2)*0.5f;
	RenderVertex vmid;
	pcc->GetPointAt(tMid, &vmid);
	vmid.fSmooth = true; // Generated points must always be smooth, because they are part of the curve
	vmid.fSlingshot = false; // Slingshots can't be along curves
	vmid.fControlPoint = false; // We created this point, so it can't be a control point

	if (FlatWithAccuracy(*pvt1, *pvt2, vmid, accuracy))
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
		RecurseSmoothLineWithAccuracy(pcc, t1, tMid, pvt1, &vmid, pvv, accuracy);
		RecurseSmoothLineWithAccuracy(pcc, tMid, t2, &vmid, pvt2, pvv, accuracy);
		} 
	}
