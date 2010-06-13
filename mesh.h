#pragma once
class Triangle
	{
public:
	int a,b,c;
	};

class ObjFrame
	{
public:
	ObjFrame();
	~ObjFrame();

	RECT rc;
	LPDIRECTDRAWSURFACE7 pdds;
	LPDIRECTDRAWSURFACE7 pddsZBuffer;

	LPDIRECTDRAWSURFACE7 pTexture;
	float				 u, v;
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
	pv->x = 0.5f * ((3.0f*x2 -x1 -3.0f*x3 + x4)*t3
		+ (2.0f*x1 -5.0f*x2 + 4.0f*x3 - x4)*t2
		+ (x3 - x1)*t
		+ 2.0f*x2);

	pv->y = 0.5f * ((3.0f*y2 -y1 -3.0f*y3 + y4)*t3
		+ (2.0f*y1 -5.0f*y2 + 4.0f*y3 - y4)*t2
		+ (y3-y1)*t
		+ 2.0f*y2);
	}
	
	float x1,x2,x3,x4;
	float y1,y2,y3,y4;
	};

class LightProjected
	{
public:
	float inclination;
	float rotation;
	float spin;

	Vertex3Ds m_v;

	void CalcCoordinates(Vertex3D * const pv);
	};

class RenderVertex : public Vertex2D
	{
public:
	BOOL fSmooth;
	BOOL fSlingshot;
	BOOL fControlPoint; // Whether this point was a control point on the curve
	};

void SetHUDVertices(Vertex3D * const rgv, const int count);
void PolygonToTriangles(const RenderVertex * const rgv, Vector<void> * const pvpoly, Vector<Triangle> * const pvtri);
void RecurseSmoothLine(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv);

inline float GetDot(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
	{
	Vertex2D vt1, vt2;

	vt1.x = pvJoint->x - pvEnd1->x;
	vt1.y = pvJoint->y - pvEnd1->y;

	vt2.x = pvJoint->x - pvEnd2->x;
	vt2.y = pvJoint->y - pvEnd2->y;

	return vt1.x*vt2.y - vt1.y*vt2.x;
	}
	
inline BOOL FLinesIntersect(const Vertex2D * const Start1, const Vertex2D * const Start2, const Vertex2D * const End1, const Vertex2D * const End2)
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

	if (d123 == 0) // p3 lies on the same line as p1 and p2
		{
		return (x3 >= min(x1,x2) && x3 <= max(x2,x1));
		}

	const float d124 = (x2 - x1)*(y4 - y1) - (x4 - x1)*(y2 - y1);

	if (d124 == 0) // p4 lies on the same line as p1 and p2
		{
		return (x4 >= min(x1,x2) && x4 <= max(x2,x1));
		}

    const float d341 = (x3 - x1)*(y4 - y1) - (x4 - x1)*(y3 - y1);

	if (d341 == 0) // p1 lies on the same line as p3 and p4
		{
		return (x1 >= min(x3,x4) && x1 <= max(x3,x4));
		}

    const float d342 = d123 - d124 + d341;

	if (d342 == 0) // p1 lies on the same line as p3 and p4
		{
		return (x2 >= min(x3,x4) && x2 <= max(x3,x4));
		}

	return ((d123 * d124 < 0) && (d341 * d342 < 0));
	}

inline BOOL AdvancePoint(const RenderVertex * const rgv, Vector<void> * const pvpoly, const int a, const int b, const int c, const int pre, const int post)
	{
	const RenderVertex * const pv1 = &rgv[a];
	const RenderVertex * const pv2 = &rgv[b];
	const RenderVertex * const pv3 = &rgv[c];

	const RenderVertex * const pvPre = &rgv[pre];
	const RenderVertex * const pvPost = &rgv[post];

	float dot = GetDot(pv1,pv2,pv3);

	if (dot < 0)
		{
		return fFalse;
		}

	// Make sure angle created by new triangle line falls inside existing angles
	// If the existing angle is a concave angle, then new angle must be smaller,
	// because our triangle can't have angles greater than 180

	dot = GetDot(pvPre, pv1, pv2);
	if (dot > 0)
		{
		// convex angle, make sure new angle is smaller than it
		const float dotDelta = GetDot(pvPre, pv1, pv3);

		if (dotDelta < 0)
			{
			return false;
			}
		}

	dot = GetDot(pv2, pv3, pvPost);
	if (dot > 0)
		{
		const float dotDelta = GetDot(pv1, pv3, pvPost);

		if (dotDelta < 0)
			{
			return false;
			}
		}

	// Now make sure the interior segment of this triangle (line ac) does not
	//intersect the polygon anywhere

	// sort our static line segment

	const float minx = min(pv1->x, pv3->x);
	const float maxx = max(pv1->x, pv3->x);
	const float miny = min(pv1->y, pv3->y);
	const float maxy = max(pv1->y, pv3->y);

	for (int i=0;i<pvpoly->Size();i++)
		{		
		const RenderVertex * const pvCross1 = &rgv[(int)pvpoly->ElementAt(i)];
		const RenderVertex * const pvCross2 = &rgv[(int)pvpoly->ElementAt((i+1) % pvpoly->Size())];
	
		if ( pvCross1 == pv1 || pvCross2 == pv1 || pvCross1 == pv3 || pvCross2 == pv3 ||
		    (pvCross1->y < miny && pvCross2->y < miny) ||
			(pvCross1->y > maxy && pvCross2->y > maxy) ||
			(pvCross1->x < minx && pvCross2->x < minx) ||
            (pvCross1->x > maxx && pvCross2->y > maxx))
			{
			}
		else	
		if (FLinesIntersect(pv1, pv3, pvCross1, pvCross2))
			{
			return fFalse;
			}
		}

	return true;
	}

inline float GetCos(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
	{
	Vertex2D vt1, vt2;

	vt1.x = pvJoint->x - pvEnd1->x;
	vt1.y = pvJoint->y - pvEnd1->y;

	vt2.x = pvJoint->x - pvEnd2->x;
	vt2.y = pvJoint->y - pvEnd2->y;

	const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

	const float len1 = sqrtf((vt1.x * vt1.x) + (vt1.y * vt1.y));
	const float len2 = sqrtf((vt2.x * vt2.x) + (vt2.y * vt2.y));

	return dot/(len1*len2);
	}

inline float GetAngle(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
	{
	const float slope1 = (pvJoint->y - pvEnd1->y) / (pvJoint->x - pvEnd1->x);
	const float slope2 = (pvJoint->y - pvEnd2->y) / (pvJoint->x - pvEnd2->x);
	return atan2f((slope2-slope1),(1.0f+slope1*slope2));
	}

inline void SetNormal(Vertex3D * rgv, const WORD * rgi, const int count, Vertex3D * rgvApply, const WORD * rgiApply, int applycount)
	{
	// If apply-to array is null, just apply the resulting normal to incoming array
	if (rgvApply == NULL)
		{
		rgvApply = rgv;
		}

	if (rgiApply == NULL)
		{
		rgiApply = rgi;
		}

	if (applycount == 0)
		{
		applycount = count;
		}

	Vertex3Ds vnormal;
	vnormal.x = 0;
	vnormal.y = 0;
	vnormal.z = 0;

	for (int i=0;i<count;i++)
		{
		const int l = rgi[i];
		const int m = rgi[(i+1) % count];

		vnormal.x += (rgv[l].y - rgv[m].y) * (rgv[l].z + rgv[m].z);
		vnormal.y += (rgv[l].z - rgv[m].z) * (rgv[l].x + rgv[m].x);
		vnormal.z += (rgv[l].x - rgv[m].x) * (rgv[l].y + rgv[m].y);		
		}

	const float inv_len = 1.0f/sqrtf((vnormal.x * vnormal.x) + (vnormal.y * vnormal.y) + (vnormal.z * vnormal.z));
	vnormal.x *= inv_len;
	vnormal.y *= inv_len;
	vnormal.z *= inv_len;

	for (int i=0;i<applycount;i++)
		{
		const int l = rgiApply[i];
		rgvApply[l].nx = -vnormal.x;
		rgvApply[l].ny = -vnormal.y;
		rgvApply[l].nz = -vnormal.z;
		}
	}

inline void SetDiffuseFromMaterial(Vertex3D * const rgv, const int count, const D3DMATERIAL7 * const pmtrl)
	{
	const int r = (int)(((pmtrl->diffuse.r + pmtrl->emissive.r) * 255.0f) + 0.5f);
	const int g = (int)(((pmtrl->diffuse.g + pmtrl->emissive.g) * 255.0f) + 0.5f);
	const int b = (int)(((pmtrl->diffuse.b + pmtrl->emissive.b) * 255.0f) + 0.5f);

	const int color = (r<<16) | (g<<8) | b;

	for (int i=0;i<count;i++)
		{
		rgv[i].color = color;
		}
	}

inline BOOL Flat(const Vertex2D * const pvt1, const Vertex2D * const pvt2, const Vertex2D * const pvtMid)
	{
	const float det1 = pvt1->x*pvtMid->y - pvt1->y*pvtMid->x;
	const float det2 = pvtMid->x*pvt2->y - pvtMid->y*pvt2->x;
	const float det3 = pvt2->x*pvt1->y - pvt2->y*pvt1->x;

	const float dblarea = det1+det2+det3;

	return (dblarea*dblarea < (float)(1.0/(0.5*0.5)));
	}
