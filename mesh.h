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
	void SetCurve(Vertex *pv0, Vertex *pv1, Vertex *pv2, Vertex *pv3);
	void GetPointAt(float t, Vertex *pv);
	
	float x1,x2,x3,x4;
	float y1,y2,y3,y4;
	};

class LightProjected
	{
public:
	float inclination;
	float rotation;
	float spin;

	Vertex3D m_v;

	void CalcCoordinates(Vertex3D *pv);
	};

class RenderVertex;

void PolygonToTriangles(RenderVertex *rgv, Vector<void> *pvpoly, Vector<Triangle> *pvtri);

BOOL AdvancePoint(RenderVertex *rgv, Vector<void> *pvpoly, int a, int b, int c, int pre, int post);

float GetDot(Vertex *pvEnd1, Vertex *pvJoint, Vertex *pvEnd2);
float GetCos(Vertex *pvEnd1, Vertex *pvJoint, Vertex *pvEnd2);
float GetAngle(Vertex *pvEnd1, Vertex *pvJoint, Vertex *pvEnd2);

void Calc2DNormal(Vertex *pv1, Vertex *pv2, Vertex *pnormal);

BOOL FLinesIntersect(Vertex *Start1, Vertex *Start2, Vertex *End1, Vertex *End2);

//void SetNormal(Vertex3D *rgv, WORD *rgi, int count);

void SetNormal(Vertex3D *rgv, WORD *rgi, int count, Vertex3D *rgvApply, WORD *rgiApply, int applycount);

void SetHUDVertices(Vertex3D *rgv, int count);
void SetDiffuseFromMaterial(Vertex3D *rgv, int count, D3DMATERIAL7 *pmtrl);

BOOL Flat(Vertex *pvt1, Vertex *pvt2, Vertex *pvtMid);
void RecurseSmoothLine(CatmullCurve *pcc, float t1, float t2, RenderVertex *pvt1, RenderVertex *pvt2, Vector<RenderVertex> *pvv);

void ClosestPointOnPolygon(Vertex *rgv, int count, Vertex *pvin, Vertex *pvout, int *piseg, BOOL fClosed);

void RotateAround(Vertex3D *pvAxis, Vertex3D *pvPoint, int count, float angle);

void CrossProduct(Vertex3D *pv1, Vertex3D *pv2, Vertex3D *pvCross);