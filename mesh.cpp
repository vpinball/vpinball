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

void PolygonToTriangles(RenderVertex *rgv, Vector<void> *pvpoly, Vector<Triangle> *pvtri)
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

BOOL AdvancePoint(RenderVertex *rgv, Vector<void> *pvpoly, int a, int b, int c, int pre, int post)
	{
	RenderVertex *pv1 = &rgv[a];
	RenderVertex *pv2 = &rgv[b];
	RenderVertex *pv3 = &rgv[c];

	RenderVertex *pvPre = &rgv[pre];
	RenderVertex *pvPost = &rgv[post];

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

	vt1.x = pvJoint->x - pvEnd1->x;
	vt1.y = pvJoint->y - pvEnd1->y;

	vt2.x = pvJoint->x - pvEnd2->x;
	vt2.y = pvJoint->y - pvEnd2->y;

	const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

	const float len1 = sqrtf((vt1.x * vt1.x) + (vt1.y * vt1.y));
	const float len2 = sqrtf((vt2.x * vt2.x) + (vt2.y * vt2.y));

	return dot/(len1*len2);
	}

float GetAngle(Vertex *pvEnd1, Vertex *pvJoint, Vertex *pvEnd2)
	{
	const float slope1 = (pvJoint->y - pvEnd1->y) / (pvJoint->x - pvEnd1->x);
	const float slope2 = (pvJoint->y - pvEnd2->y) / (pvJoint->x - pvEnd2->x);
	return atan2f((slope2-slope1),(1.0f+slope1*slope2));
	}

void SetNormal(Vertex3D *rgv, WORD *rgi, int count, Vertex3D *rgvApply, WORD *rgiApply, int applycount)
	{
	Vertex3D vnormal;
	
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

	for (int i=0;i<count;i++)
		{
		const int l = rgi[i];
		const int m = rgi[(i+1) % count];

		vnormal.nx += (rgv[l].y - rgv[m].y) * (rgv[l].z + rgv[m].z);
		vnormal.ny += (rgv[l].z - rgv[m].z) * (rgv[l].x + rgv[m].x);
		vnormal.nz += (rgv[l].x - rgv[m].x) * (rgv[l].y + rgv[m].y);		
		}

	const float inv_len = 1.0f/sqrtf((vnormal.nx * vnormal.nx) + (vnormal.ny * vnormal.ny) + (vnormal.nz * vnormal.nz));
	
	vnormal.nx *= inv_len;
	vnormal.ny *= inv_len;
	vnormal.nz *= inv_len;

	for (int i=0;i<applycount;i++)
		{
		const int l = rgiApply[i];
		rgvApply[l].nx = -vnormal.nx;
		rgvApply[l].ny = -vnormal.ny;
		rgvApply[l].nz = -vnormal.nz;
		}
	}

void SetHUDVertices(Vertex3D *rgv, int count)
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

void SetDiffuseFromMaterial(Vertex3D *rgv, int count, D3DMATERIAL7 *pmtrl)
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

void LightProjected::CalcCoordinates(Vertex3D *pv)
	{
	Vertex3D vOrigin, vT;

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

	const float width  = (g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
	const float height = (g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

	pv->tu2 = (((vT.x - m_v.x) / width)  + 0.5f) * g_pplayer->m_pin3d.m_maxtu;
	pv->tv2 = (((vT.y - m_v.y) / height) + 0.5f) * g_pplayer->m_pin3d.m_maxtv;
	}

BOOL Flat(Vertex *pvt1, Vertex *pvt2, Vertex *pvtMid)
	{
	const float det1 = pvt1->x*pvtMid->y - pvt1->y*pvtMid->x;
	const float det2 = pvtMid->x*pvt2->y - pvtMid->y*pvt2->x;
	const float det3 = pvt2->x*pvt1->y - pvt2->y*pvt1->x;

	const float area = 0.5f*(det1+det2+det3);

	return (area*area < 1.0f);
	}

void RecurseSmoothLine(CatmullCurve *pcc, float t1, float t2, RenderVertex *pvt1, RenderVertex *pvt2, Vector<RenderVertex> *pvv)
	{
	RenderVertex vmid;

	const float tMid = (t1+t2)*0.5f;
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
	vT.x = pv1->x - pv2->x;
	vT.y = pv1->y - pv2->y;

	// Set up line normal
	const float inv_length = 1.0f/sqrtf((vT.x * vT.x) + (vT.y * vT.y));
	pnormal->x =  vT.y * inv_length;
	pnormal->y = -vT.x * inv_length;
	}

void ClosestPointOnPolygon(Vertex *rgv, int count, Vertex *pvin, Vertex *pvout, int *piseg, BOOL fClosed)
	{
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
	for (int i=0;i<cloop;i++)
		{
		const int p1 = i;
		const int p2 = (i+1)%count;

		const float A = -(rgv[p2].y - rgv[p1].y);
		const float B =   rgv[p2].x - rgv[p1].x;
		const float C = -(A*rgv[p1].x + B*rgv[p1].y);

		const float dist = fabsf((A*pvin->x + B*pvin->y + C) / sqrtf(A*A + B*B));

		if (dist < mindist)
			{
			// Assuming we got a segment that we are closet to, calculate the intersection
			// of the line with the perpenticular line projected from the point,
			// to find the closest point on the line
			const int p1 = i;
			const int p2 = (i+1)%count;

			const float A = -(rgv[p2].y - rgv[p1].y);
			const float B = rgv[p2].x - rgv[p1].x;
			const float C = -(A*rgv[p1].x + B*rgv[p1].y);

			const float D = -B;
			const float E = A;
			const float F = -(D*pvin->x + E*pvin->y);
			
			const float inv_det = 1.0f/((A*E) - (B*D));
			const float intersectx = (B*F-E*C)*inv_det;
			const float intersecty = (C*D-A*F)*inv_det;

			// If the intersect point lies on the polygon segment
			// (not out in space), then make this the closest known point
			if (intersectx >= (min(rgv[p1].x, rgv[p2].x) - 0.1f) &&
				intersectx <= (max(rgv[p1].x, rgv[p2].x) + 0.1f) &&
				intersecty >= (min(rgv[p1].y, rgv[p2].y) - 0.1f) &&
				intersecty <= (max(rgv[p1].y, rgv[p2].y) + 0.1f))
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
	const PINFLOAT x = pvAxis->x;
	const PINFLOAT y = pvAxis->y;
	const PINFLOAT z = pvAxis->z;

	const PINFLOAT rsin = (PINFLOAT)sinf(angle);
	const PINFLOAT rcos = (PINFLOAT)cosf(angle);

	PINFLOAT matrix[3][3];

	// Matrix for rotating around an arbitrary vector

	matrix[0][0] = x*x + rcos*(1.0f-x*x);
	matrix[1][0] = x*y*(1.0f-rcos) - z*rsin;
	matrix[2][0] = z*x*(1.0f-rcos) + y*rsin;

	matrix[0][1] = x*y*(1.0f-rcos) + z*rsin;
	matrix[1][1] = y*y + rcos*(1.0f-y*y);
	matrix[2][1] = y*z*(1.0f-rcos) - x*rsin;

	matrix[0][2] = z*x*(1.0f-rcos) - y*rsin;
	matrix[1][2] = y*z*(1.0f-rcos) + x*rsin;
	matrix[2][2] = z*z + rcos*(1.0f-z*z);

	for (int i=0;i<count;i++)
		{
		PINFLOAT result[3];
		result[0] = matrix[0][0]*pvPoint[i].x + matrix[0][1]*pvPoint[i].y + matrix[0][2]*pvPoint[i].z;
		result[1] = matrix[1][0]*pvPoint[i].x + matrix[1][1]*pvPoint[i].y + matrix[1][2]*pvPoint[i].z;
		result[2] = matrix[2][0]*pvPoint[i].x + matrix[2][1]*pvPoint[i].y + matrix[2][2]*pvPoint[i].z;

		pvPoint[i].x = (float)result[0];
		pvPoint[i].y = (float)result[1];
		pvPoint[i].z = (float)result[2];

		if ((pvPoint[i].nx == 0) && (pvPoint[i].ny == 0) && (pvPoint[i].nx == 0)) //rlc optimize, often 0,0,0
			{
			result[0] = matrix[0][0]*pvPoint[i].nx + matrix[0][1]*pvPoint[i].ny + matrix[0][2]*pvPoint[i].nz;
			result[1] = matrix[1][0]*pvPoint[i].nx + matrix[1][1]*pvPoint[i].ny + matrix[1][2]*pvPoint[i].nz;
			result[2] = matrix[2][0]*pvPoint[i].nx + matrix[2][1]*pvPoint[i].ny + matrix[2][2]*pvPoint[i].nz;

			pvPoint[i].nx = (float)result[0];
			pvPoint[i].ny = (float)result[1];
			pvPoint[i].nz = (float)result[2];
			}		
		}
	}

void CrossProduct(Vertex3D *pv1, Vertex3D *pv2, Vertex3D *pvCross)
	{
	pvCross->x = pv1->y * pv2->z - pv1->z * pv2->y;
	pvCross->y = pv1->z * pv2->x - pv1->x * pv2->z;
	pvCross->z = pv1->x * pv2->y - pv1->y * pv2->x;
	}
