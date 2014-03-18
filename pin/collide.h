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
	eTriangle,
	e3DLine,
	eGate,
	eTextbox,
    eDispReel,
	eLightSeq,
	ePrimitive,
	eTrigger,	// this value and greater are volume set tested, add rigid or non-volume set above
	eKicker		// this is done to limit to one test
	};

extern float c_hardFriction; 
extern float c_hardScatter; 
extern float c_maxBallSpeedSqr; 
extern float c_dampingFriction;

extern float c_plungerNormalize;  //Adjust Mech-Plunger, useful for component change or weak spring etc.
extern bool c_plungerFilter;

// forward declarations
class Ball;
class HitObject;
class AnimObject;


struct CollisionEvent
{
    Ball* ball;         // the ball that collided with smth
    HitObject* obj;     // what the ball collided with

    float hittime;      // when the collision happens (relative to current physics state, units: 10 ms)
    float distance;     // hit distance 

    // additional collision information; typical use (not always the same):
    // 0: hit normal, 1: hit object velocity, 2: monent and angular rate, 4: contact distance
    Vertex3Ds normal[5];

    float hitx, hity;   // position of the ball at hit time (saved to avoid floating point errors with multiple time slices)

    bool hitRigid;      // rigid body collision?
};


HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections);

class HitObject
	{
public:

	HitObject();
	virtual ~HitObject() {}

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll) = 0;

	virtual int GetType() const = 0;

    virtual void Collide(CollisionEvent *hit) = 0;

	virtual void CalcHitRect() = 0;
	
	virtual AnimObject *GetAnimObject() {return NULL;}

	IFireEvents *m_pfe;
	float m_threshold;
	
	//IDispatch *m_pdisp;
	IFireEvents *m_pfedebug;

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
	};

class LineSeg : public HitObject
	{
public:
	virtual float HitTestBasic(const Ball * pball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid);
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual int GetType() const {return eLineSeg;}
	virtual void Collide(CollisionEvent *coll);
	void CalcNormal();
	void CalcLength();
	virtual void CalcHitRect();

	Vertex2D normal;
	Vertex2D v1, v2;
	float length;
	};

class HitCircle : public HitObject
	{
public:
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);

	float HitTestBasicRadius(const Ball * pball, const float dtime, CollisionEvent& coll,
									const bool direction, const bool lateral, const bool rigid);

	float HitTestRadius(const Ball * pball, const float dtime, CollisionEvent& coll);

	virtual int GetType() const {return eCircle;}

	virtual void Collide(CollisionEvent *coll);

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

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);

	virtual int GetType() const {return eJoint;}

	virtual void Collide(CollisionEvent *coll);

	virtual void CalcHitRect();

	//Vertex2D v;
	Vertex2D normal;
	};




// Callback for the broadphase collision test.
// Perform the actual hittest between ball and hit object and update
// collision information if a hit occurred.
void DoHitTest(Ball *pball, HitObject *pho);

