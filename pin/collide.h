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
	eKicker,	// this is done to limit to one test
	};

// test near zero conditions in linear, well behaved, conditions
#define C_PRECISION 0.01f

// tolerance for line segment endpoint and point radii collisions

#define C_TOL_ENDPNTS 0

#define C_TOL_RADIUS 0.005f

#define SHOWNORMAL 1

// Physical Skin ... postive contact layer. Any contact (collision) in this layer reports zero time.
// layer is used to calculate contact effects ... beyond this and objects pass through each other
// Default 25.0

#define PHYS_SKIN 25.0

extern PINFLOAT c_Gravity;
extern PINFLOAT c_hardFriction; 
extern PINFLOAT c_hardScatter; 
extern PINFLOAT c_maxBallSpeedSqed; 
extern PINFLOAT c_dampingFriction;

extern PINFLOAT c_plungerNormalize;  //Adjust Mech-Plunger, useful for component change or weak spring etc.
extern bool c_plungerFilter;

extern U32 c_PostCheck;

// Layer outside object which increases it's size for contact measurements. Used to determine clearances.
// Setting this value during testing to 0.1 will insure clearance. After testing set the value to 0.005
// Default 0.01

#define PHYS_TOUCH  0.05

// Low Normal speed collison is handled as contact process rather than impulse collision
#define C_LOWNORMVEL 0.0001f

#define C_CONTACTVEL 0.099f

// limit  ball speed to C_SPEEDLIMIT
#define C_SPEEDLIMIT 60.0f

// low velocity stabalization ... if embedding occurs add some velocity 
#define C_EMBEDDED 0.0f
#define C_EMBEDSHOT 0.05f

// Contact displacement corrections, hard ridgid contacts i.e. steel on hard plastic or hard wood
#define C_DISP_GAIN 0.9875f
#define C_DISP_LIMIT 5.0f

#define RC_FRICTIONCONST 2.5e-3f

#define C_DYNAMIC 2

//trigger/kicker boundary crossing hysterisis
#define STATICTIME 0.005f

//#define INFOARRAY vertex[2];
//#define INFONORMAL 0
//#define INFOMOVE 1

inline BOOL FQuickLineIntersect(PINFLOAT x1, PINFLOAT y1, PINFLOAT x2, PINFLOAT y2,
		PINFLOAT x3, PINFLOAT y3, PINFLOAT x4, PINFLOAT y4)
	{
	float d123, d124, d341, d342;

    d123 = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

	d124 = (x2 - x1)*(y4 - y1) - (x4 - x1)*(y2 - y1);

    d341 = (x3 - x1)*(y4 - y1) - (x4 - x1)*(y3 - y1);

    d342 = d123 - d124 + d341;

	return ((d123 * d124 <= 0) && (d341 * d342 <= 0));
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

HitObject *CreateCircularHitPoly(PINFLOAT x, PINFLOAT y, PINFLOAT z, PINFLOAT r, int sections);

class HitObject
	{
public:

	HitObject();
	virtual ~HitObject(){}

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal) = 0;

	virtual int GetType() = 0;

	virtual void Draw(HDC hdc) = 0;

	virtual void Collide(Ball *pball, Vertex3D *phitnormal) = 0;

	virtual void CalcHitRect() = 0;
	
	virtual AnimObject *GetAnimObject() {return NULL;}

	//void SetZBounds(char *szSurfaceName);

	//EventProxyBase *m_pep;
	IFireEvents *m_pfe;
	float m_threshold;
	
	//IDispatch *m_pdisp;
	IFireEvents *m_pfedebug;

	//RECT m_rcUpdate;
	//Vector<RECT> m_vrcupdate;

	FRect3D m_rcHitRect;

	BOOL m_fEnabled;
	int  m_ObjType;
	void* m_pObj;
	float m_elasticity;
	float m_antifriction;
	float m_scatter;

	};

class AnimObject
	{
public:
	virtual BOOL FMover() {return fFalse;}
	virtual void UpdateDisplacements(PINFLOAT dtime) {}
	//virtual void ResetFrameTime() {}
	//virtual void UpdateTimePermanent() {}
	virtual void UpdateVelocities(PINFLOAT dtime) {}

	virtual BOOL FNeedsScreenUpdate() {return fFalse;}
    virtual void Check3D() {}
	virtual ObjFrame *Draw3D(RECT *prc) {return NULL;}
	virtual void Reset() {}

	RECT m_rcBounds; // bounding box for invalidation
	float m_znear, m_zfar; // To tell which objects are closer and should be blitted last

	BOOL m_fInvalid;
	};

class HitNormal : public HitObject
	{
public:
	Vertex2D normal;
#ifdef SHOWNORMAL
	int check1;
#endif
	};

class LineSeg : public HitNormal
	{
public:
	Vertex2D v1, v2;
	float length;

	virtual PINFLOAT HitTestBasic(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal
									,bool direction, bool lateral, bool rigid);
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);
	//float VertHitTest(Ball *pball, float dtime, Vertex2D *phitnormal);
	virtual int GetType() {return eLineSeg;}
	virtual void Draw(HDC hdc);
	virtual void Collide(Ball *pball, Vertex3D *phitnormal);
	void CalcNormal();
	void CalcLength();
	virtual void CalcHitRect();
	};

class HitCircle : public HitObject
	{
public:

	Vertex2D center;
	float radius;
	float zlow;
	float zhigh;

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);

	PINFLOAT HitTestBasicRadius(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal
								,bool direction, bool lateral, bool rigid);

	PINFLOAT HitTestRadius(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);

	virtual int GetType() {return eCircle;}

	virtual void Draw(HDC hdc);

	virtual void Collide(Ball *pball, Vertex3D *phitnormal);

	virtual void CalcHitRect();
	};

class Joint : public HitCircle
	{
public:
	Joint();

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);

	virtual int GetType() {return eJoint;}

	virtual void Draw(HDC hdc);

	virtual void Collide(Ball *pball, Vertex3D *phitnormal);

	virtual void CalcHitRect();

	//Vertex2D v;
	Vertex2D normal;
	};

class HitOctree
	{
public:
	HitOctree():m_fLeaf(fTrue)
		{
		}
		
	~HitOctree();

	void HitTestXRay(Ball *pball, Vector<HitObject> *pvhoHit);

	void HitTestBall(Ball *pball);

	void CreateNextLevel();

	HitOctree *m_phitoct[8];

	Vector<HitObject> m_vho;

	FRect3D m_rectbounds;
	Vertex3Ds m_vcenter;

	BOOL m_fLeaf;
	};
