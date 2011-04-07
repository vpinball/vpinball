#pragma once
#define MAXBALLS 256
 
#define GRAVITYCONST 0.86543f

class HitObject;
class Level;
class Ball;

class BallAnimObject : public AnimObject
	{
public:
	virtual BOOL FMover() {return fFalse;} // We add ourselves to the mover list.  
											// If we allow the table to do that, we might get added twice, 
											// if we get created in Init code
	virtual void UpdateDisplacements(float dtime);
	virtual void UpdateVelocities(float dtime);

	Ball *m_pball;
	};

class Ball : public HitObject 
	{
public:
	Ball();
	~Ball();

	void Init();

    static int NumInitted();

	virtual void UpdateDisplacements(float dtime);
	virtual void UpdateVelocities(float dtime);

	// From HitObject
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);	
	virtual int GetType() {return eBall;}
	virtual void Draw(HDC hdc) {} // Dead function
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);
	virtual void CalcHitRect();
	virtual AnimObject *GetAnimObject() {return &m_ballanim;}

	//semi-generic collide methods
	void CollideWall(const Vertex3Ds * const phitnormal, const float elasticity, float antifriction, float scatter_angle);
	void Collide3DWall(const Vertex3Ds * const phitnormal, const float m_elasticity, float antifriction, float scatter_angle);

	void AngularAcceleration(const Vertex3Ds * const phitnormal);

	void CalcBoundingRect();

	void EnsureOMObject();

	//~Ball();
	//Ball(Ball &pball);

	Vertex3D m_rgv3D[4];						// Last vertices of the ball texture
	Vertex3D m_rgv3DShadow[4];					// Last vertices of the ball shadow

	RECT m_rcScreen;							// rect where the ball appears on the screen
	RECT m_rcScreenShadow;
	
	COLORREF m_color;

	// Per frame info
	FRect brc; // bounding rectangle

	CCO(BallEx) *m_pballex; // Object model version of the ball

	char m_szImage[MAXTOKEN];
	char m_szImageFront[MAXTOKEN];
	char m_szImageBack[MAXTOKEN];

	PinImage *m_pin;
	PinImage *m_pinFront;
	PinImage *m_pinBack;

	HitObject *m_pho;		//pointer to hit object trial, may not be a actual hit if something else happens first
	VectorVoid* m_vpVolObjs;// vector of triggers we are now inside
	float m_hittime;		// time at which this ball will hit something
	float m_hitx, m_hity;// position of the ball at hit time (saved to avoid floating point errors with multiple time slices)
	float m_HitDist;		// hit distance 
	float m_HitNormVel;	// hit normal Velocity
	int m_fDynamic;			// used to determine static ball conditions and velocity quenching, 
	Vertex3Ds m_hitnormal[5];//rlc 0: hit normal, 1: hit object velocity, 2: monent and angular rate, 4: contact distance

	//Level *m_plevel; // Level this ball is rolling on

	BallAnimObject m_ballanim;

	float x;
	float y;
	float z;
	
	//float GetZ() const {return z - radius;}
	//void PutZ(const float newz) {z = newz + radius;}

	float vx;
	float vy;
	float vz;

	float drsq;	// square of distance moved

	float radius;
	float collisionMass;

//	float x_min, x_max;	// world limits on ball displacements
//	float y_min, y_max;
	float z_min, z_max;

	Vertex3Ds m_Event_Pos;
	
	Matrix3 m_orientation;
	Vertex3Ds m_angularmomentum;
	Vertex3Ds m_angularvelocity;
	Matrix3 m_inverseworldinertiatensor;
	Matrix3 m_inversebodyinertiatensor;

	bool m_HitRigid;	// Rigid = 1, Non-Rigid = 0

	bool m_fErase;		// set after the ball has been drawn for the first time

	bool fFrozen;
	//bool fTempFrozen; //if the ball is stuck and we are avoiding hittesting
	//unsigned int  fFrozenCount; // number of sequential zero hit times
	};

class Level
	{
public:
	float m,n,b; // Plane equation z = mx + ny + b
	Vertex3Ds m_gravity; // Gravity vector (2-D - cheating)

	Vector<HitObject> m_vho;
	};

inline bool fIntRectIntersect(const RECT &rc1, const RECT &rc2)
	{
	return (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom);
	}

inline bool fRectIntersect(const FRect &rc1, const FRect &rc2)
	{
	return (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom);
	}

inline bool fRectIntersect3D(const FRect3D &rc1, const FRect3D &rc2)
	{
	return (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow);
	}
