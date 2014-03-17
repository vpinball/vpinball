#pragma once
#include "Material.h"
#include "Texture.h"

class Triangle
{
public:
	int a,b,c;
};

class ObjFrame
{
public:
	inline ObjFrame() {
		rc.left = -42; // init with nonsense offscreen values
		rc.top = -42;
		rc.right = -23;
		rc.bottom = -23;

		pdds = NULL;
		pddsZBuffer = NULL;
	}
	
	inline ~ObjFrame() {
        delete pdds;
        delete pddsZBuffer;
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

class RenderVertex : public Vertex2D
{
public:
	bool fSmooth;
	bool fSlingshot;
	bool fControlPoint; // Whether this point was a control point on the curve
	bool padd; // Useless padding to align to 4bytes, should enhance access speeds
};


template <class VtxType>
void SetHUDVertices(VtxType * const rgv, const int count)
	{
	const float mult = (float)g_pplayer->m_pin3d.m_dwRenderWidth * (float)(1.0/EDITOR_BG_WIDTH);
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
	}

void RecurseSmoothLine(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv);
void RecurseSmoothLineWithAccuracy(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv, const float accuracy);


inline float GetDot(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
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

// RenderVertexCont is either an array or a Vector<> of RenderVertex
template <class RenderVertexCont>
inline bool AdvancePoint(const RenderVertexCont& rgv, const VectorVoid * const pvpoly, const int a, const int b, const int c, const int pre, const int post)
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

inline float GetCos(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
	const Vertex2D vt1(pvJoint->x - pvEnd1->x, pvJoint->y - pvEnd1->y);
	const Vertex2D vt2(pvJoint->x - pvEnd2->x, pvJoint->y - pvEnd2->y);

	const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

	return dot/sqrtf((vt1.x * vt1.x + vt1.y * vt1.y)*(vt2.x * vt2.x + vt2.y * vt2.y));
}

/*
inline float GetAngle(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
	const float slope1 = (pvJoint->y - pvEnd1->y) / (pvJoint->x - pvEnd1->x);
	const float slope2 = (pvJoint->y - pvEnd2->y) / (pvJoint->x - pvEnd2->x);
	return atan2f((slope2-slope1),(1.0f+slope1*slope2));
}
*/

// Computes the normal for a single, plane polygon described by the indices and applies it either
// to the original vertices or to the vertices indexed by rgiApply.
//
// This functions uses Newell's method to compute the normal. However, this is the version for
// a right-handed coordinate system, and therefore produces wrong results for the VP renderer,
// which is left-handed. However, the VP physics seem to be right-handed (!!), and therefore
// in VP10 we will need two versions of this function, left- and right-handed.
template <class VtxType>
void SetNormal(VtxType * const rgv, const WORD * const rgi, const int count, void * prgvApply=NULL, const WORD * rgiApply=NULL, int applycount=NULL)
{
	// If apply-to array is null, just apply the resulting normal to incoming array
    VtxType * rgvApply = prgvApply ? (VtxType*)prgvApply : rgv;

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

template <class VtxType>
void SetDiffuseFromMaterial(VtxType *rgv, int count, Material *pmtrl) // get rid of this?
{
   D3DCOLORVALUE diffuse = pmtrl->getDiffuse();
   D3DCOLORVALUE emissive = pmtrl->getEmissive();
   unsigned int r = (int)(((diffuse.r + emissive.r) * 255.0f) + 0.5f);
   unsigned int g = (int)(((diffuse.g + emissive.g) * 255.0f) + 0.5f);
   unsigned int b = (int)(((diffuse.b + emissive.b) * 255.0f) + 0.5f);

   unsigned int color = (r<<16) | (g<<8) | b;

   for (int i=0; i<count; ++i)
      rgv[i].color = color;
}

template <class VtxType>
void SetDiffuse(VtxType * const rgv, const int count, const unsigned int color) // get rid of this?
{
	for (int i=0; i<count; ++i)
		rgv[i].color = color;
}

// Calculate if two vectors are flat to each other
// accuracy is a float greater 4 and smaller 4000000 (tested this out)
inline bool FlatWithAccuracy(const Vertex2D * const pvt1, const Vertex2D * const pvt2, const Vertex2D * const pvtMid, const float accuracy)
{
	const float det1 = pvt1->x*pvtMid->y - pvt1->y*pvtMid->x;
	const float det2 = pvtMid->x*pvt2->y - pvtMid->y*pvt2->x;
	const float det3 = pvt2->x*pvt1->y - pvt2->y*pvt1->x;

	const float dblarea = det1+det2+det3;

	return (dblarea*dblarea < accuracy);
}

inline bool Flat(const Vertex2D * const pvt1, const Vertex2D * const pvt2, const Vertex2D * const pvtMid)
{
    return FlatWithAccuracy(pvt1, pvt2, pvtMid, 1.0/(0.5*0.5));
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

		const RenderVertex * const rgvi = rgv.ElementAt(i);
		const RenderVertex * const rgvp2 = rgv.ElementAt(p2);
		const float A = rgvi->y - rgvp2->y;
		const float B = rgvp2->x - rgvi->x;
		const float C = -(A*rgvi->x + B*rgvi->y);

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
			if (intersectx >= (min(rgvi->x, rgvp2->x) - 0.1f) &&
				intersectx <= (max(rgvi->x, rgvp2->x) + 0.1f) &&
				intersecty >= (min(rgvi->y, rgvp2->y) - 0.1f) &&
				intersecty <= (max(rgvi->y, rgvp2->y) + 0.1f))
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

template <class RenderVertexCont>
void PolygonToTriangles(const RenderVertexCont& rgv, VectorVoid * const pvpoly, Vector<Triangle> * const pvtri)
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
				pvpoly->RemoveElementAt((i < s-1) ? (i+1) : 0); // b
				break;
				}
			}
		}
	}
