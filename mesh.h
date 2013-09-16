#pragma once
#include "Material.h"

class Triangle
{
public:
	int a,b,c;
};

class ObjFrame
{
public:
	__forceinline ObjFrame() {
		rc.left = -42; // init with nonsense offscreen values
		rc.top = -42;
		rc.right = -23;
		rc.bottom = -23;

		pdds = NULL;
		pddsZBuffer = NULL;
	}
	
	__forceinline ~ObjFrame() {
		SAFE_RELEASE(pdds);
		SAFE_RELEASE(pddsZBuffer);
	}

	RECT rc;
	BaseTexture* pdds;
	BaseTexture* pddsZBuffer;
	};

class CatmullCurve
{
public:	
	inline void SetCurve(const Vertex2D * const pv0, const Vertex2D * const pv1, const Vertex2D * const pv2, const Vertex2D * const pv3)
	{
		x1=pv0->x;
		x2=pv1->x;
		x3=pv2->x;
		x4=pv3->x;
		y1=pv0->y;
		y2=pv1->y;
		y3=pv2->y;
		y4=pv3->y;
	}

	inline void GetPointAt(const float t, Vertex2D * const pv) const
	{
		const float t2 = t*t;
		const float t3 = t2*t;
		pv->x = 0.5f * ((3.0f*(x2-x3) -x1 + x4)*t3
			+ (x1+x1 -5.0f*x2 + 4.0f*x3 - x4)*t2
			+ (x3 - x1)*t
			+ x2+x2);

		pv->y = 0.5f * ((3.0f*(y2-y3) -y1 + y4)*t3
			+ (y1+y1 -5.0f*y2 + 4.0f*y3 - y4)*t2
			+ (y3-y1)*t
			+ y2+y2);
	}

private:	
	float x1,x2,x3,x4;
	float y1,y2,y3,y4;
};

class LightProjected
{
public:
	Vertex3Ds m_v;

	void CalcCoordinates(Vertex3D * const pv, const float inv_width, const float inv_height) const;
};

class RenderVertex : public Vertex2D
{
public:
	bool fSmooth;
	bool fSlingshot;
	bool fControlPoint; // Whether this point was a control point on the curve
	bool padd; // Useless padding to align to 4bytes, should enhance access speeds
};


void SetHUDVertices(Vertex3D * const rgv, const int count);
void SetHUDVertices(Vertex3D_NoTex2 * const rgv, const int count);
void PolygonToTriangles(const RenderVertex * const rgv, Vector<void> * const pvpoly, Vector<Triangle> * const pvtri);
void PolygonToTriangles(const Vector<RenderVertex> &rgv, Vector<void> * const pvpoly, Vector<Triangle> * const pvtri);
void RecurseSmoothLine(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv);
void RecurseSmoothLineWithAccuracy(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv, const float accuracy);


__forceinline float GetDot(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
	return (pvJoint->x - pvEnd1->x)*(pvJoint->y - pvEnd2->y) - (pvJoint->y - pvEnd1->y)*(pvJoint->x - pvEnd2->x);
}
	
inline bool FLinesIntersect(const Vertex2D * const Start1, const Vertex2D * const Start2, const Vertex2D * const End1, const Vertex2D * const End2)
{
	const float x1 = Start1->x;
	const float y1 = Start1->y;
	const float x2 = Start2->x;
	const float y2 = Start2->y;
	const float x3 = End1->x;
	const float y3 = End1->y;
	const float x4 = End2->x;
	const float y4 = End2->y;

    const float d123 = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

	if (d123 == 0.0f) // p3 lies on the same line as p1 and p2
		return (x3 >= min(x1,x2) && x3 <= max(x2,x1));

	const float d124 = (x2 - x1)*(y4 - y1) - (x4 - x1)*(y2 - y1);

	if (d124 == 0.0f) // p4 lies on the same line as p1 and p2
		return (x4 >= min(x1,x2) && x4 <= max(x2,x1));

	if(d123 * d124 >= 0.0f)
		return false;

    const float d341 = (x3 - x1)*(y4 - y1) - (x4 - x1)*(y3 - y1);

	if (d341 == 0.0f) // p1 lies on the same line as p3 and p4
		return (x1 >= min(x3,x4) && x1 <= max(x3,x4));

    const float d342 = d123 - d124 + d341;

	if (d342 == 0.0f) // p1 lies on the same line as p3 and p4
		return (x2 >= min(x3,x4) && x2 <= max(x3,x4));

	return (d341 * d342 < 0.0f);
}

inline bool AdvancePoint(const RenderVertex * const rgv, const Vector<void> * const pvpoly, const int a, const int b, const int c, const int pre, const int post)
{
	const RenderVertex * const pv1 = &rgv[a];
	const RenderVertex * const pv2 = &rgv[b];
	const RenderVertex * const pv3 = &rgv[c];

	const RenderVertex * const pvPre = &rgv[pre];
	const RenderVertex * const pvPost = &rgv[post];

	if ((GetDot(pv1,pv2,pv3) < 0) ||
		// Make sure angle created by new triangle line falls inside existing angles
		// If the existing angle is a concave angle, then new angle must be smaller,
		// because our triangle can't have angles greater than 180
	   ((GetDot(pvPre, pv1, pv2)  > 0) && (GetDot(pvPre, pv1, pv3)  < 0)) || // convex angle, make sure new angle is smaller than it
	   ((GetDot(pv2, pv3, pvPost) > 0) && (GetDot(pv1, pv3, pvPost) < 0)))
	   return false;
	
	// Now make sure the interior segment of this triangle (line ac) does not
	// intersect the polygon anywhere

	// sort our static line segment

	const float minx = min(pv1->x, pv3->x);
	const float maxx = max(pv1->x, pv3->x);
	const float miny = min(pv1->y, pv3->y);
	const float maxy = max(pv1->y, pv3->y);

	for (int i=0; i<pvpoly->Size(); ++i)
	{		
		const RenderVertex * const pvCross1 = &rgv[(int)pvpoly->ElementAt(i)];
		const RenderVertex * const pvCross2 = &rgv[(int)pvpoly->ElementAt((i < pvpoly->Size()-1) ? (i+1) : 0)];
	
		if ( pvCross1 != pv1 && pvCross2 != pv1 && pvCross1 != pv3 && pvCross2 != pv3 &&
		    (pvCross1->y >= miny || pvCross2->y >= miny) &&
			(pvCross1->y <= maxy || pvCross2->y <= maxy) &&
			(pvCross1->x >= minx || pvCross2->x >= minx) &&
            (pvCross1->x <= maxx || pvCross2->y <= maxx) &&
			FLinesIntersect(pv1, pv3, pvCross1, pvCross2))
			return false;
	}

	return true;
}

//!! copypasted from above
inline bool AdvancePoint(const Vector<RenderVertex> &rgv, const Vector<void> * const pvpoly, const int a, const int b, const int c, const int pre, const int post)
{
	const RenderVertex * const pv1 = rgv.ElementAt(a);
	const RenderVertex * const pv2 = rgv.ElementAt(b);
	const RenderVertex * const pv3 = rgv.ElementAt(c);

	const RenderVertex * const pvPre = rgv.ElementAt(pre);
	const RenderVertex * const pvPost = rgv.ElementAt(post);

	if ((GetDot(pv1,pv2,pv3) < 0) ||
		// Make sure angle created by new triangle line falls inside existing angles
		// If the existing angle is a concave angle, then new angle must be smaller,
		// because our triangle can't have angles greater than 180
	   ((GetDot(pvPre, pv1, pv2)  > 0) && (GetDot(pvPre, pv1, pv3)  < 0)) || // convex angle, make sure new angle is smaller than it
	   ((GetDot(pv2, pv3, pvPost) > 0) && (GetDot(pv1, pv3, pvPost) < 0)))
	   return false;
	
	// Now make sure the interior segment of this triangle (line ac) does not
	// intersect the polygon anywhere

	// sort our static line segment

	const float minx = min(pv1->x, pv3->x);
	const float maxx = max(pv1->x, pv3->x);
	const float miny = min(pv1->y, pv3->y);
	const float maxy = max(pv1->y, pv3->y);

	for (int i=0; i<pvpoly->Size(); ++i)
	{		
		const RenderVertex * const pvCross1 = rgv.ElementAt((int)pvpoly->ElementAt(i));
		const RenderVertex * const pvCross2 = rgv.ElementAt((int)pvpoly->ElementAt((i < pvpoly->Size()-1) ? (i+1) : 0));
	
		if ( pvCross1 != pv1 && pvCross2 != pv1 && pvCross1 != pv3 && pvCross2 != pv3 &&
		    (pvCross1->y >= miny || pvCross2->y >= miny) &&
			(pvCross1->y <= maxy || pvCross2->y <= maxy) &&
			(pvCross1->x >= minx || pvCross2->x >= minx) &&
            (pvCross1->x <= maxx || pvCross2->y <= maxx) &&
			FLinesIntersect(pv1, pv3, pvCross1, pvCross2))
			return false;
	}

	return true;
}

__forceinline float GetCos(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
	const Vertex2D vt1(pvJoint->x - pvEnd1->x, pvJoint->y - pvEnd1->y);
	const Vertex2D vt2(pvJoint->x - pvEnd2->x, pvJoint->y - pvEnd2->y);

	const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

	return dot/sqrtf((vt1.x * vt1.x + vt1.y * vt1.y)*(vt2.x * vt2.x + vt2.y * vt2.y));
}

/*
__forceinline float GetAngle(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
	const float slope1 = (pvJoint->y - pvEnd1->y) / (pvJoint->x - pvEnd1->x);
	const float slope2 = (pvJoint->y - pvEnd2->y) / (pvJoint->x - pvEnd2->x);
	return atan2f((slope2-slope1),(1.0f+slope1*slope2));
}
*/

inline void SetNormal(Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * rgvApply, const WORD * rgiApply, int applycount)
{
	// If apply-to array is null, just apply the resulting normal to incoming array
	if (rgvApply == NULL)
		rgvApply = rgv;

	if (rgiApply == NULL)
		rgiApply = rgi;

	if (applycount == 0)
		applycount = count;

	Vertex3Ds vnormal(0.0f,0.0f,0.0f);

	for (int i=0; i<count; ++i)
	{
		const int l = rgi[i];
		const int m = rgi[(i < count-1) ? (i+1) : 0];

		vnormal.x += (rgv[l].y - rgv[m].y) * (rgv[l].z + rgv[m].z);
		vnormal.y += (rgv[l].z - rgv[m].z) * (rgv[l].x + rgv[m].x);
		vnormal.z += (rgv[l].x - rgv[m].x) * (rgv[l].y + rgv[m].y);		
	}

	const float len = vnormal.x * vnormal.x + vnormal.y * vnormal.y + vnormal.z * vnormal.z;
	const float inv_len = (len > 0.0f) ? -1.0f/sqrtf(len) : 0.0f; //!! opt.
	vnormal.x *= inv_len;
	vnormal.y *= inv_len;
	vnormal.z *= inv_len;

	for (int i=0; i<applycount; ++i)
	{
		const int l = rgiApply[i];
		rgvApply[l].nx = vnormal.x;
		rgvApply[l].ny = vnormal.y;
		rgvApply[l].nz = vnormal.z;
	}
}

//copy pasted from above
inline void SetNormal(Vertex3D_NoTex2 * const rgv, const WORD * const rgi, const int count, Vertex3D_NoTex2 * rgvApply, const WORD * rgiApply, int applycount)
{
	// If apply-to array is null, just apply the resulting normal to incoming array
	if (rgvApply == NULL)
		rgvApply = rgv;

	if (rgiApply == NULL)
		rgiApply = rgi;

	if (applycount == 0)
		applycount = count;

	Vertex3Ds vnormal(0.0f,0.0f,0.0f);

	for (int i=0; i<count; ++i)
	{
		const int l = rgi[i];
		const int m = rgi[(i < count-1) ? (i+1) : 0];

		vnormal.x += (rgv[l].y - rgv[m].y) * (rgv[l].z + rgv[m].z);
		vnormal.y += (rgv[l].z - rgv[m].z) * (rgv[l].x + rgv[m].x);
		vnormal.z += (rgv[l].x - rgv[m].x) * (rgv[l].y + rgv[m].y);		
	}

	const float len = vnormal.x * vnormal.x + vnormal.y * vnormal.y + vnormal.z * vnormal.z;
	const float inv_len = (len > 0.0f) ? -1.0f/sqrtf(len) : 0.0f; //!! opt.
	vnormal.x *= inv_len;
	vnormal.y *= inv_len;
	vnormal.z *= inv_len;

	for (int i=0; i<applycount; ++i)
	{
		const int l = rgiApply[i];
		rgvApply[l].nx = vnormal.x;
		rgvApply[l].ny = vnormal.y;
		rgvApply[l].nz = vnormal.z;
	}
}

__forceinline void SetDiffuseFromMaterial(Vertex3D * const rgv, const int count, const Material *pmtrl) // get rid of this?
{
   const D3DCOLORVALUE diffuse = pmtrl->getDiffuse();
   const D3DCOLORVALUE emissive = pmtrl->getEmissive();
   const unsigned int r = (int)(((diffuse.r + emissive.r) * 255.0f) + 0.5f);
   const unsigned int g = (int)(((diffuse.g + emissive.g) * 255.0f) + 0.5f);
   const unsigned int b = (int)(((diffuse.b + emissive.b) * 255.0f) + 0.5f);

   const unsigned int color = (r<<16) | (g<<8) | b;

   for (int i=0; i<count; ++i)
      rgv[i].color = color;
}

//copy pasted from above
__forceinline void SetDiffuseFromMaterial(Vertex3D_NoTex2 * const rgv, const int count, const Material * const pmtrl) // get rid of this?
{
   const D3DCOLORVALUE diffuse = pmtrl->getDiffuse();
   const D3DCOLORVALUE emissive = pmtrl->getEmissive();
   const unsigned int r = (int)(((diffuse.r + emissive.r) * 255.0f) + 0.5f);
   const unsigned int g = (int)(((diffuse.g + emissive.g) * 255.0f) + 0.5f);
   const unsigned int b = (int)(((diffuse.b + emissive.b) * 255.0f) + 0.5f);

   const unsigned int color = (r<<16) | (g<<8) | b;

   for (int i=0; i<count; ++i)
      rgv[i].color = color;
}

__forceinline void SetDiffuse(Vertex3D * const rgv, const int count, const unsigned int color) // get rid of this?
{
	for (int i=0; i<count; ++i)
		rgv[i].color = color;
}

//copy pasted from above
__forceinline void SetDiffuse(Vertex3D_NoTex2 * const rgv, const int count, const unsigned int color) // get rid of this?
{
	for (int i=0; i<count; ++i)
		rgv[i].color = color;
}

__forceinline bool Flat(const Vertex2D * const pvt1, const Vertex2D * const pvt2, const Vertex2D * const pvtMid)
{
	const float det1 = pvt1->x*pvtMid->y - pvt1->y*pvtMid->x;
	const float det2 = pvtMid->x*pvt2->y - pvtMid->y*pvt2->x;
	const float det3 = pvt2->x*pvt1->y - pvt2->y*pvt1->x;

	const float dblarea = det1+det2+det3;

	return (dblarea*dblarea < (float)(1.0/(0.5*0.5)));
}

// Calculate if two vectors are flat to each other
// accuracy is a float greater 4 and smaller 4000000 (tested this out)
__forceinline bool FlatWithAccuracy(const Vertex2D * const pvt1, const Vertex2D * const pvt2, const Vertex2D * const pvtMid, const float accuracy)
{
	const float det1 = pvt1->x*pvtMid->y - pvt1->y*pvtMid->x;
	const float det2 = pvtMid->x*pvt2->y - pvtMid->y*pvt2->x;
	const float det3 = pvt2->x*pvt1->y - pvt2->y*pvt1->x;

	const float dblarea = det1+det2+det3;

	return (dblarea*dblarea < accuracy);
}

inline void ClosestPointOnPolygon(const Vector<RenderVertex> &rgv, const Vertex2D &pvin, Vertex2D * const pvout, int * const piseg, const bool fClosed)
{
	const int count = rgv.Size();

	float mindist = FLT_MAX;
	int seg = -1;
	*piseg = -1; // in case we are not next to the line

	int cloop = count;
	if (!fClosed)
		--cloop; // Don't check segment running from the end point to the beginning point

	// Go through line segment, calculate distance from point to the line
	// then pick the shortest distance
	for (int i=0; i<cloop; ++i)
	{
		const int p2 = (i < count-1) ? (i+1) : 0;

		const float A = rgv.ElementAt(i)->y - rgv.ElementAt(p2)->y;
		const float B = rgv.ElementAt(p2)->x - rgv.ElementAt(i)->x;
		const float C = -(A*rgv.ElementAt(i)->x + B*rgv.ElementAt(i)->y);

		const float dist = fabsf(A*pvin.x + B*pvin.y + C) / sqrtf(A*A + B*B);

		if (dist < mindist)
		{
			// Assuming we got a segment that we are closet to, calculate the intersection
			// of the line with the perpenticular line projected from the point,
			// to find the closest point on the line
			const float D = -B;
			const float F = -(D*pvin.x + A*pvin.y);
			
			const float det = A*A - B*D;
			const float inv_det = (det != 0.0f) ? 1.0f/det : 0.0f;
			const float intersectx = (B*F-A*C)*inv_det;
			const float intersecty = (C*D-A*F)*inv_det;

			// If the intersect point lies on the polygon segment
			// (not out in space), then make this the closest known point
			if (intersectx >= (min(rgv.ElementAt(i)->x, rgv.ElementAt(p2)->x) - 0.1f) &&
				intersectx <= (max(rgv.ElementAt(i)->x, rgv.ElementAt(p2)->x) + 0.1f) &&
				intersecty >= (min(rgv.ElementAt(i)->y, rgv.ElementAt(p2)->y) - 0.1f) &&
				intersecty <= (max(rgv.ElementAt(i)->y, rgv.ElementAt(p2)->y) + 0.1f))
			{
				mindist = dist;
				seg = i;
				pvout->x = intersectx;
				pvout->y = intersecty;
				*piseg = seg;
			}
		}
	}
}
