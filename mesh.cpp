#include "stdafx.h"
#include "main.h"

//#include "math.h"

ObjFrame::ObjFrame()
	{
	pdds = NULL;
	pddsZBuffer = NULL;
	}

ObjFrame::~ObjFrame()
	{
	SAFE_RELEASE(pdds);
	SAFE_RELEASE(pddsZBuffer);
	}

void CatmullCurve::SetCurve(Vertex *pv0, Vertex *pv1, Vertex *pv2, Vertex *pv3)
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

void CatmullCurve::GetPointAt(float t, Vertex *pv)
	{
	float t2,t3;

	t2 = t*t;
	t3 = t2*t;
	pv->x = (float)(0.5 * ((-x1 + 3*x2 -3*x3 + x4)*t3
		+ (2*x1 -5*x2 + 4*x3 - x4)*t2
		+ (-x1 + x3)*t
		+ 2*x2));

	pv->y = (float)(0.5 * ((-y1 + 3*y2 -3*y3 + y4)*t3
		+ (2*y1 -5*y2 + 4*y3 - y4)*t2
		+ (-y1+y3)*t
		+ 2*y2));
	}

void PolygonToTriangles(RenderVertex *rgv, Vector<void> *pvpoly, Vector<Triangle> *pvtri)
	{
	int i;
	int l;

	// There should be this many convex triangles.
	// If not, the polygon is self-intersecting
	int tricount = pvpoly->Size() - 2;

	Assert(tricount > 0);

	for (l = 0; l<tricount; l++)
	//while (pvpoly->Size() > 2)
		{

		for (i=0;i<pvpoly->Size();i++)
			{
			int a,b,c;
			int pre,post;
			a = (int)pvpoly->ElementAt(i);
			b = (int)pvpoly->ElementAt((i+1) % pvpoly->Size());
			c = (int)pvpoly->ElementAt((i+2) % pvpoly->Size());
			pre = (int)pvpoly->ElementAt((i-1+pvpoly->Size()) % pvpoly->Size());
			post = (int)pvpoly->ElementAt((i+3) % pvpoly->Size());
			if (AdvancePoint(rgv, pvpoly, a, b, c, pre, post))
				{
				Triangle *ptri = new Triangle();
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

float GetDot(Vertex *pvEnd1, Vertex *pvJoint, Vertex *pvEnd2)
	{
	Vertex vt1, vt2;

	vt1.x = pvJoint->x - pvEnd1->x;
	vt1.y = pvJoint->y - pvEnd1->y;

	vt2.x = pvJoint->x - pvEnd2->x;
	vt2.y = pvJoint->y - pvEnd2->y;

	return vt1.x*vt2.y - vt1.y*vt2.x;
	}
	
BOOL FLinesIntersect(Vertex *Start1, Vertex *Start2, Vertex *End1, Vertex *End2)
	{
	double d123, d124, d341, d342;
	
	double x1 = Start1->x;
	double y1 = Start1->y;
	double x2 = Start2->x;
	double y2 = Start2->y;
	double x3 = End1->x;
	double y3 = End1->y;
	double x4 = End2->x;
	double y4 = End2->y;

    d123 = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

	if (d123 == 0) // p3 lies on the same line as p1 and p2
		{
		return (x3 >= min(x1,x2) && x3 <= max(x2,x1));
		}

	d124 = (x2 - x1)*(y4 - y1) - (x4 - x1)*(y2 - y1);

	if (d124 == 0) // p4 lies on the same line as p1 and p2
		{
		return (x4 >= min(x1,x2) && x4 <= max(x2,x1));
		}

    d341 = (x3 - x1)*(y4 - y1) - (x4 - x1)*(y3 - y1);

	if (d341 == 0) // p1 lies on the same line as p3 and p4
		{
		return (x1 >= min(x3,x4) && x1 <= max(x3,x4));
		}

    d342 = d123 - d124 + d341;

	if (d342 == 0) // p1 lies on the same line as p3 and p4
		{
		return (x2 >= min(x3,x4) && x2 <= max(x3,x4));
		}

	return ((d123 * d124 < 0) && (d341 * d342 < 0));
	}

BOOL AdvancePoint(RenderVertex *rgv, Vector<void> *pvpoly, int a, int b, int c, int pre, int post)
	{
	RenderVertex *pv1 = &rgv[a];
	RenderVertex *pv2 = &rgv[b];
	RenderVertex *pv3 = &rgv[c];

	RenderVertex *pvPre = &rgv[pre];
	RenderVertex *pvPost = &rgv[post];

	float dot, dotDelta;

	dot = GetDot(pv1,pv2,pv3);

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
		dotDelta = GetDot(pvPre, pv1, pv3);

		if (dotDelta < 0)
			{
			return false;
			}
		}

	dot = GetDot(pv2, pv3, pvPost);
	if (dot > 0)
		{
		dotDelta = GetDot(pv1, pv3, pvPost);

		if (dotDelta < 0)
			{
			return false;
			}
		}

	// Now make sure the interior segment of this triangle (line ac) does not
	//intersect the polygon anywhere

	// sort our static line segment

	float minx;
	float maxx;
	float miny;
	float maxy;

	minx = min(pv1->x, pv3->x);
	maxx = max(pv1->x, pv3->x);
	miny = min(pv1->y, pv3->y);
	maxy = max(pv1->y, pv3->y);

	int i;
	for (i=0;i<pvpoly->Size();i++)
		{
		
		RenderVertex *pvCross1 = &rgv[(int)pvpoly->ElementAt(i)];
		RenderVertex *pvCross2 = &rgv[(int)pvpoly->ElementAt((i+1) % pvpoly->Size())];
	
		if (pvCross1 == pv1 || pvCross2 == pv1 || pvCross1 == pv3 || pvCross2 == pv3)
			{
			continue;
			}

		if (pvCross1->y < miny && pvCross2->y < miny)
			{
			continue; // Line segments don't interact
			}

		if (pvCross1->y > maxy && pvCross2->y > maxy)
			{
			continue; // Line segments don't interact
			}

		if (pvCross1->x < minx && pvCross2->x < minx)
			{
			continue; // Line segments don't interact
			}

		if (pvCross1->x > maxx && pvCross2->y > maxx)
			{
			continue; // Line segments don't interact
			}

		// check intersection point
		/*double slope1, slope2;
		double intersect1, intersect2;

		slope1 = (pv3->v.y - pv1->v.y)/(pv3->v.x - pv1->v.x);
		slope2 = (pvCross2->v.y - pvCross1->v.y)/(pvCross2->v.x - pvCross1->v.x);

		intersect1 = pv3->v.y - (slope1 * pv3->v.x);
		intersect2 = pvCross2->v.y - (slope2 * pvCross2->v.x);

		double hitx;

		hitx = (intersect2 - intersect1) / (slope1-slope2);

		if (hitx >= minx && hitx <= maxx)
			{
			double minCrossx = min(pvCross2->v.x, pvCross1->v.x);
			double maxCrossx = max(pvCross2->v.x, pvCross1->v.x);
			if (hitx >= minCrossx && hitx < maxCrossx)
				{
				return fFalse; // Lines intersect
				}
			}*/
		
		if (FLinesIntersect(pv1, pv3, pvCross1, pvCross2))
			{
			return fFalse;
			}
		}

	return true;
	}

float GetCos(Vertex *pvEnd1, Vertex *pvJoint, Vertex *pvEnd2)
	{
	Vertex vt1, vt2;
	float dot, len1, len2;

	vt1.x = pvJoint->x - pvEnd1->x;
	vt1.y = pvJoint->y - pvEnd1->y;

	vt2.x = pvJoint->x - pvEnd2->x;
	vt2.y = pvJoint->y - pvEnd2->y;

	dot = vt1.x*vt2.y - vt1.y*vt2.x;

	len1 = (float)sqrt((vt1.x * vt1.x) + (vt1.y * vt1.y));
	len2 = (float)sqrt((vt2.x * vt2.x) + (vt2.y * vt2.y));

	return dot/(len1*len2);
	}

float GetAngle(Vertex *pvEnd1, Vertex *pvJoint, Vertex *pvEnd2)
	{
	double slope1, slope2;

	slope1 = (pvJoint->y - pvEnd1->y) / (pvJoint->x - pvEnd1->x);
	slope2 = (pvJoint->y - pvEnd2->y) / (pvJoint->x - pvEnd2->x);
	return (float)atan2((slope2-slope1),(1+slope1*slope2));
	}

void SetNormal(Vertex3D *rgv, WORD *rgi, int count, Vertex3D *rgvApply, WORD *rgiApply, int applycount)
	{
	int i;
	int l,m;
	Vertex3D vnormal;
	float len;

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

	vnormal.nx = 0;
	vnormal.ny = 0;
	vnormal.nz = 0;

	for (i=0;i<count;i++)
		{
		l = rgi[i];
		m = rgi[(i+1) % count];

		vnormal.nx += (rgv[l].y - rgv[m].y) * (rgv[l].z + rgv[m].z);
		vnormal.ny += (rgv[l].z - rgv[m].z) * (rgv[l].x + rgv[m].x);
		vnormal.nz += (rgv[l].x - rgv[m].x) * (rgv[l].y + rgv[m].y);		
		}

	len = (float)sqrt((vnormal.nx * vnormal.nx) + (vnormal.ny * vnormal.ny) + (vnormal.nz * vnormal.nz));
	
	vnormal.nx /= len;
	vnormal.ny /= len;
	vnormal.nz /= len;

	for (i=0;i<applycount;i++)
		{
		l = rgiApply[i];
		rgvApply[l].nx = -vnormal.nx;
		rgvApply[l].ny = -vnormal.ny;
		rgvApply[l].nz = -vnormal.nz;
		}
	}

void SetHUDVertices(Vertex3D *rgv, int count)
	{
	int i;

	float mult = (float)g_pplayer->m_pin3d.m_dwRenderWidth / (float)1000;
	float ymult = mult / (float)g_pplayer->m_pixelaspectratio;

	for (i=0;i<count;i++)
		{
		rgv[i].x *= mult;
		rgv[i].y *= ymult;
		rgv[i].x -= 0.5;
		rgv[i].y -= 0.5;
		rgv[i].z = 0;//1.0f;//0;
		rgv[i].rhw = 0.1f;
		rgv[i].specular = 0;
		}

	if (g_pplayer->m_frotate)
		{
		float ftemp;
		for (i=0;i<count;i++)
			{
			ftemp = rgv[i].x;
			rgv[i].x = rgv[i].y;
			rgv[i].y = g_pplayer->m_pin3d.m_dwViewPortHeight - ftemp;
			}
		}
	}

void SetDiffuseFromMaterial(Vertex3D *rgv, int count, D3DMATERIAL7 *pmtrl)
	{
	int i;
	int color,r,g,b;

	r = (int)(((pmtrl->diffuse.r + pmtrl->emissive.r) * 255) + 0.5f);
	g = (int)(((pmtrl->diffuse.g + pmtrl->emissive.g) * 255) + 0.5f);
	b = (int)(((pmtrl->diffuse.b + pmtrl->emissive.b) * 255) + 0.5f);

	color = (r<<16) | (g<<8) | b;

	for (i=0;i<count;i++)
		{
		rgv[i].color = color;
		}
	}

void LightProjected::CalcCoordinates(Vertex3D *pv)
	{
	Vertex3D vOrigin, vT;

	// light is rotated around the light as the origin
	// z doesn't matter because the texture is projected through z
	vOrigin.x = pv->x - m_v.x;
	vOrigin.y = pv->y - m_v.y;
	vOrigin.z = pv->z - 0;

	// Rotation

	float sn = (float)sin(0.0f);
	float cs = (float)cos(0.0f);

	vT.x = cs * vOrigin.x + sn * vOrigin.y;
	vT.y = cs * vOrigin.y - sn * vOrigin.x;

	// Inclination

	sn = (float)sin(0.0f);
	cs = (float)cos(0.0f);

	vT.z = cs * vOrigin.z - sn * vT.y;
	vT.y = cs * vT.y + sn * vOrigin.z;

	// Put coordinates into vertex

	vT.x += m_v.x;
	vT.y += m_v.y;
	vT.z += 0;

	float width = (g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
	float height = (g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom);

	pv->tu2 = (((vT.x - m_v.x) / width) + 0.5f) * g_pplayer->m_pin3d.m_maxtu;
	pv->tv2 = (((vT.y - m_v.y) / height) + 0.5f) * g_pplayer->m_pin3d.m_maxtv;
	}

BOOL Flat(Vertex *pvt1, Vertex *pvt2, Vertex *pvtMid)
	{
	float det1, det2, det3;

	det1 = pvt1->x*pvtMid->y - pvt1->y*pvtMid->x;
	det2 = pvtMid->x*pvt2->y - pvtMid->y*pvt2->x;
	det3 = pvt2->x*pvt1->y - pvt2->y*pvt1->x;

	float area = 0.5f*(det1+det2+det3);

	return ((area*area) < 1);
	}

void RecurseSmoothLine(CatmullCurve *pcc, float t1, float t2, RenderVertex *pvt1, RenderVertex *pvt2, Vector<RenderVertex> *pvv)
	{
	float tMid;
	RenderVertex vmid;

	tMid = (t1+t2)/2;
	pcc->GetPointAt(tMid, &vmid);
	vmid.fSmooth = fTrue; // Generated points must always be smooth, because they are part of the curve
	vmid.fSlingshot = fFalse; // Slingshots can't be along curves
	vmid.fControlPoint = fFalse; // We created this point, so it can't be a control point

	if (Flat(pvt1, pvt2, &vmid))
		{
		// Add first segment point to array.
		// Last point never gets added by this recursive loop,
		// but that's where it wraps around to the next curve.
		RenderVertex *pvT = new RenderVertex;
		*pvT = *pvt1;
		pvv->AddElement(pvT);
		}
	else
		{
		RecurseSmoothLine(pcc, t1, tMid, pvt1, &vmid, pvv);
		RecurseSmoothLine(pcc, tMid, t2, &vmid, pvt2, pvv);
		} 
	}

void Calc2DNormal(Vertex *pv1, Vertex *pv2, Vertex *pnormal)
	{
	Vertex vT;
	float length;

	vT.x = pv1->x - pv2->x;
	vT.y = pv1->y - pv2->y;

	// Set up line normal
	length = (float)sqrt((vT.x * vT.x) + (vT.y * vT.y));
	pnormal->x = vT.y / length;
	pnormal->y = -vT.x / length;
	}

void ClosestPointOnPolygon(Vertex *rgv, int count, Vertex *pvin, Vertex *pvout, int *piseg, BOOL fClosed)
	{
	int i;
	float dist;
	float mindist = FLT_MAX;
	int seg = -1;
	*piseg = -1; // in case we are not next to the line

	int cloop = count;
	if (!fClosed)
		{
		cloop--; // Don't check segment running from the end point to the beginning point
		}

	// Go through line segment, calculate distance from point to the line
	// then pick the shortest distance
	for (i=0;i<cloop;i++)
		{
		float A,B,C;
		int p1 = i;
		int p2 = (i+1)%count;

		A = -(rgv[p2].y - rgv[p1].y);
		B = rgv[p2].x - rgv[p1].x;
		C = -(A*rgv[p1].x + B*rgv[p1].y);

		dist = (float)fabs((A*pvin->x + B*pvin->y + C) / (sqrt(A*A + B*B)));

		if (dist < mindist)
			{
			// Assuming we got a segment that we are closet to, calculate the intersection
			// of the line with the perpenticular line projected from the point,
			// to find the closest point on the line
			float A,B,C,D,E,F;
			int p1 = i;
			int p2 = (i+1)%count;

			A = -(rgv[p2].y - rgv[p1].y);
			B = rgv[p2].x - rgv[p1].x;
			C = -(A*rgv[p1].x + B*rgv[p1].y);

			D = -B;
			E = A;
			F = -(D*pvin->x + E*pvin->y);
			
			float det = (A*E) - (B*D);
			float intersectx = (B*F-E*C)/det;
			float intersecty = (C*D-A*F)/det;

			// If the intersect point lies on the polygon segment
			// (not out in space), then make this the closest known point
			if (intersectx >= (min(rgv[p1].x, rgv[p2].x) - 0.1) &&
				intersectx <= (max(rgv[p1].x, rgv[p2].x) + 0.1) &&
				intersecty >= (min(rgv[p1].y, rgv[p2].y) - 0.1) &&
				intersecty <= (max(rgv[p1].y, rgv[p2].y) + 0.1))
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

void RotateAround(Vertex3D *pvAxis, Vertex3D *pvPoint, int count, float angle)
	{
	int i;

	PINFLOAT rsin, rcos;

	PINFLOAT x = pvAxis->x;
	PINFLOAT y = pvAxis->y;
	PINFLOAT z = pvAxis->z;

	rsin = (PINFLOAT)sin(angle);
	rcos = (PINFLOAT)cos(angle);

	PINFLOAT matrix[3][3];

	PINFLOAT result[3];

	// Matrix for rotating around an arbitrary vector

	matrix[0][0] = x*x + rcos*(1-x*x);
	matrix[1][0] = x*y*(1-rcos) - z*rsin;
	matrix[2][0] = z*x*(1-rcos) + y*rsin;

	matrix[0][1] = x*y*(1-rcos) + z*rsin;
	matrix[1][1] = y*y + rcos*(1-y*y);
	matrix[2][1] = y*z*(1-rcos) - x*rsin;

	matrix[0][2] = z*x*(1-rcos) - y*rsin;
	matrix[1][2] = y*z*(1-rcos) + x*rsin;
	matrix[2][2] = z*z + rcos*(1-z*z);

	for (i=0;i<count;i++)
		{
		result[0] = matrix[0][0]*pvPoint[i].x + matrix[0][1]*pvPoint[i].y + matrix[0][2]*pvPoint[i].z;
		result[1] = matrix[1][0]*pvPoint[i].x + matrix[1][1]*pvPoint[i].y + matrix[1][2]*pvPoint[i].z;
		result[2] = matrix[2][0]*pvPoint[i].x + matrix[2][1]*pvPoint[i].y + matrix[2][2]*pvPoint[i].z;

		pvPoint[i].x = (float)result[0];
		pvPoint[i].y = (float)result[1];
		pvPoint[i].z = (float)result[2];

		result[0] = matrix[0][0]*pvPoint[i].nx + matrix[0][1]*pvPoint[i].ny + matrix[0][2]*pvPoint[i].nz;
		result[1] = matrix[1][0]*pvPoint[i].nx + matrix[1][1]*pvPoint[i].ny + matrix[1][2]*pvPoint[i].nz;
		result[2] = matrix[2][0]*pvPoint[i].nx + matrix[2][1]*pvPoint[i].ny + matrix[2][2]*pvPoint[i].nz;

		pvPoint[i].nx = (float)result[0];
		pvPoint[i].ny = (float)result[1];
		pvPoint[i].nz = (float)result[2];
		}
	}

void CrossProduct(Vertex3D *pv1, Vertex3D *pv2, Vertex3D *pvCross)
	{
	pvCross->x = pv1->y * pv2->z - pv1->z * pv2->y;
	pvCross->y = pv1->z * pv2->x - pv1->x * pv2->z;
	pvCross->z = pv1->x * pv2->y - pv1->y * pv2->x;
	}