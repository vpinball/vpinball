#pragma once

enum
	{
	eNull,
	eLineSeg,
	eJoint,
	eCircle,
	eFlipper,
	ePlunger,
	eSpinner,
	eBall,
	e3DPoly,
	e3DLine,
	eGate,
	eTextbox,
    eDispReel,
	eLightSeq,
	eTrigger,	// this value and greater are volume set tested, add rigid or non-volume set above
	eKicker,		// this is done to limit to one test
	ePrimitive
	};

//extern float c_Gravity;
extern float c_hardFriction; 
extern float c_hardScatter; 
extern float c_maxBallSpeedSqed; 
extern float c_dampingFriction;

extern float c_plungerNormalize;  //Adjust Mech-Plunger, useful for component change or weak spring etc.
extern bool c_plungerFilter;

inline bool FQuickLineIntersect(const float x1, const float y1, const float x2, const float y2,
								const float x3, const float y3, const float x4, const float y4)
	{
	const float d123 = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);
	const float d124 = (x2 - x1)*(y4 - y1) - (x4 - x1)*(y2 - y1);

	if(d123 * d124 > 0.0f)
	    return false;

    const float d341 = (x3 - x1)*(y4 - y1) - (x4 - x1)*(y3 - y1);
    const float d342 = d123 - d124 + d341;

	return (d341 * d342 <= 0.0f);
	}

class Ball;
class HitObject;
class AnimObject;

class UpdateRect
	{
public:
	RECT m_rcupdate;
	Vector<AnimObject> m_vobject;
	BOOL m_fSeeThrough;
	};

HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections);

class HitObject
	{
public:

	HitObject();
	virtual ~HitObject() {}

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) = 0;

	virtual int GetType() const = 0;

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal) = 0;

	virtual void CalcHitRect() = 0;
	
	virtual AnimObject *GetAnimObject() {return NULL;}

	IFireEvents *m_pfe;
	float m_threshold;
	
	//IDispatch *m_pdisp;
	IFireEvents *m_pfedebug;

	//RECT m_rcUpdate;
	FRect3D m_rcHitRect;

	BOOL  m_fEnabled;
	int   m_ObjType;
	void* m_pObj;
	float m_elasticity;
	float m_antifriction;
	float m_scatter;
	};

class AnimObject
	{
public:
	virtual bool FMover() const {return false;}
	virtual void UpdateDisplacements(const float dtime) {}
	virtual void UpdateVelocities() {}

    virtual void Check3D() {}
	virtual ObjFrame *Draw3D(const RECT * const prc) {return NULL;}
	virtual void Reset() {}

	RECT m_rcBounds; // bounding box for invalidation
	float m_znear, m_zfar; // To tell which objects are closer and should be blitted last

	bool m_fInvalid;
	};

class LineSeg : public HitObject
	{
public:
	Vertex2D normal;
	Vertex2D v1, v2;
	float length;

	virtual float HitTestBasic(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool direction, const bool lateral, const bool rigid);
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual int GetType() const {return eLineSeg;}
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);
	void CalcNormal();
	void CalcLength();
	virtual void CalcHitRect();
	};

class HitCircle : public HitObject
	{
public:
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	float HitTestBasicRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal,
									const bool direction, const bool lateral, const bool rigid);

	float HitTestRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual int GetType() const {return eCircle;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	Vertex2D center;
	float radius;
	float zlow;
	float zhigh;
	};

class Joint : public HitCircle
	{
public:
	Joint();

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual int GetType() const {return eJoint;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	//Vertex2D v;
	Vertex2D normal;
	};

class HitOctree
	{
public:
	inline HitOctree() : m_fLeaf(true)
		{
    lefts = 0;
    rights = 0;
    tops = 0;
    bottoms = 0;
    zlows = 0;
    zhighs = 0;
		}
		
	~HitOctree();

	void HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const;

	void HitTestBall(Ball * const pball) const;
	void HitTestBallSse(Ball * const pball) const;
	void HitTestBallSseInner(Ball * const pball, const int i) const;

	void CreateNextLevel();

	HitOctree * __restrict m_phitoct[8];

	Vector<HitObject> m_vho;

	// helper arrays for SSE boundary checks
	void InitSseArrays();
	float* __restrict lefts;
	float* __restrict rights;
	float* __restrict tops;
	float* __restrict bottoms;
	float* __restrict zlows;
	float* __restrict zhighs;

	FRect3D m_rectbounds;
	Vertex3Ds m_vcenter;

	bool m_fLeaf;
	};
