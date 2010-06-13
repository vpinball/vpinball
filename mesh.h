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
	void SetCurve(const Vertex2D * const pv0, const Vertex2D * const pv1, const Vertex2D * const pv2, const Vertex2D * const pv3);
	void GetPointAt(const float t, Vertex2D * const pv) const;
	
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

class RenderVertex;

void PolygonToTriangles(const RenderVertex * const rgv, Vector<void> * const pvpoly, Vector<Triangle> * const pvtri);

BOOL AdvancePoint(const RenderVertex * const rgv, Vector<void> * const pvpoly, const int a, const int b, const int c, const int pre, const int post);

float GetDot(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2);
float GetCos(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2);
float GetAngle(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2);

void Calc2DNormal(const Vertex2D * const pv1, const Vertex2D * const pv2, Vertex2D * const pnormal);

BOOL FLinesIntersect(const Vertex2D * const Start1, const Vertex2D * const Start2, const Vertex2D * const End1, const Vertex2D * const End2);

//void SetNormal(Vertex3D *rgv, WORD *rgi, int count);

void SetNormal(Vertex3D * rgv, const WORD * rgi, const int count, Vertex3D *rgvApply, const WORD *rgiApply, int applycount);

void SetHUDVertices(Vertex3D * const rgv, const int count);
void SetDiffuseFromMaterial(Vertex3D * const rgv, const int count, const D3DMATERIAL7 * const pmtrl);

BOOL Flat(const Vertex2D * const pvt1, const Vertex2D * const pvt2, const Vertex2D * const pvtMid);
void RecurseSmoothLine(const CatmullCurve * const pcc, const float t1, const float t2, const RenderVertex * const pvt1, const RenderVertex * const pvt2, Vector<RenderVertex> * const pvv);

void ClosestPointOnPolygon(const Vertex2D * const rgv, const int count, const Vertex2D * const pvin, Vertex2D * const pvout, int * const piseg, const BOOL fClosed);

void RotateAround(const Vertex3Ds * const pvAxis, Vertex3D * const pvPoint, const int count, const float angle);
void RotateAround(const Vertex3Ds * const pvAxis, Vertex3Ds * const pvPoint, const int count, const float angle);

void CrossProduct(const Vertex3D * const pv1, const Vertex3D * const pv2, Vertex3D * const pvCross);
void CrossProduct(const Vertex3Ds * const pv1, const Vertex3Ds * const pv2, Vertex3Ds * const pvCross);
