#pragma once
#define MAXBALLS 256
 
//#define c_Gravity 0.65f //0.574f

#define GRAVITYCONST 0.86543f

#define BALL_NUMBLURS				5				// The number of ball blur images to draw.

 
class HitObject;
class Level;
class Ball;

class BallAnimObject : public AnimObject
	{
public:
	virtual BOOL FMover() {return fFalse;} // We add ourselves to the mover list.  
											// If we allow the table to do that, we might get added twice, 
											// if we get created in Init code
	virtual void UpdateDisplacements(PINFLOAT dtime);
	virtual void UpdateVelocities(PINFLOAT dtime);

	Ball *m_pball;
	};

class Ball : public HitObject 
	{
public:
	Ball();
	~Ball();

	void Init();

    static int NumInitted( void );

	virtual void UpdateDisplacements(PINFLOAT dtime);
	virtual void UpdateVelocities(PINFLOAT dtime);

	// From HitObject
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);	
	virtual int GetType() {return eBall;}
	virtual void Draw(HDC hdc) {} // Dead function
	virtual void Collide(Ball *pball, Vertex3D *phitnormal);
	virtual void CalcHitRect();

	//semi-generic collide methods
	void CollideWall(Vertex3D *phitnormal, float elasticity, float antifriction, float scatter_angle);
	void Collide3DWall(Vertex3D *phitnormal, float m_elasticity, float antifriction, float scatter_angle);

	void AngularAcceleration(Vertex3D *phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_ballanim;}

	//~Ball();
	//Ball(Ball &pball);

	Vertex3D m_rgv3D[4];						// Last vertices of the ball texture
	Vertex3D m_rgv3DShadow[4];					// Last vertices of the ball shadow
	BOOL m_fErase;								// set after the ball has been drawn for the first time
	RECT m_rcScreen[BALL_NUMBLURS];				// rect where the ball appears on the screen
	RECT m_rcScreenShadow[BALL_NUMBLURS];
	
	COLORREF m_color;

	// Per frame info
	FRect brc; // bounding rectangle

	void CalcBoundingRect();

	void EnsureOMObject();

	HitObject *m_pho;	//pointer to hit object trial, may not be a actual hit if something else happens first
	VectorVoid* m_vpVolObjs;	// vector of triggers we are now inside
	PINFLOAT m_hittime; // time at which this ball will hit something
	PINFLOAT m_hitx, m_hity; // position of the ball at hit time (saved to avoid floating point errors with multiple time slices)
	PINFLOAT m_HitDist;	// hit distance 
	PINFLOAT m_HitNormVel;	// hit normal Velocity
	BOOL m_HitRigid; // Rigid = 1, Non-Rigid = 0	
	int m_fDynamic; // used to determine static ball conditions and velocity quenching, 
	Vertex3D m_hitnormal[5];//rlc 0: hit normal, 1: hit object velocity, 2: monent and angular rate, 4: contact distance

	//Level *m_plevel; // Level this ball is rolling on

	CCO(BallEx) *m_pballex; // Object model version of the ball

	char m_szImage[MAXTOKEN];
	char m_szImageFront[MAXTOKEN];
	char m_szImageBack[MAXTOKEN];
	PinImage *m_pin;
	PinImage *m_pinFront;
	PinImage *m_pinBack;

	BallAnimObject m_ballanim;

	PINFLOAT x;
	PINFLOAT y;
	PINFLOAT z;
	//float GetZ() {return z - radius;}
	//void PutZ(float newz) {z = newz + radius;}
	PINFLOAT vx;
	PINFLOAT vy;
	PINFLOAT vz;

	PINFLOAT dsx;	//delta static ball
	PINFLOAT dsy;
	PINFLOAT dsz;
	PINFLOAT drsq;	// square of distance moved
	int	m_contacts; // number of static contacts during the physics frame 

	float radius;

	bool IsBlurReady;
	PINFLOAT prev_x;
	PINFLOAT prev_y;
	PINFLOAT prev_z;

//	PINFLOAT x_min, x_max;	// world limits on ball displacements
//	PINFLOAT y_min, y_max;
	PINFLOAT z_min, z_max;

	Vertex3D m_Event_Pos;

	BOOL fFrozen;
	//BOOL fTempFrozen; //if the ball is stuck and we are avoiding hittesting
	//unsigned int  fFrozenCount; // number of sequential zero hit times

	Matrix3 m_orientation;
	Vertex3D m_angularmomentum;
	Vertex3D m_angularvelocity;
	Matrix3 m_inverseworldinertiatensor;
	Matrix3 m_inversebodyinertiatensor;
	};

class Level
	{
public:
	float m,n,b; // Plane equation z = mx + ny + b
	Vertex3D m_gravity; // Gravity vector (2-D - cheating)

	Vector<HitObject> m_vho;
	};

inline BOOL fIntRectIntersect(RECT &rc1, RECT &rc2)
	{
	if (rc1.right < rc2.left || rc1.bottom < rc2.top || rc1.left > rc2.right || rc1.top > rc2.bottom)
		{
		return fFalse;
		}
	return fTrue;
	}

inline BOOL fRectIntersect(FRect &rc1, FRect &rc2)
	{
	if (rc1.right < rc2.left || rc1.bottom < rc2.top || rc1.left > rc2.right || rc1.top > rc2.bottom)
		{
		return fFalse;
		}
	return fTrue;
	}

inline BOOL fRectIntersect3D(FRect3D &rc1, FRect3D &rc2)
	{
	if (rc1.right < rc2.left || rc1.bottom < rc2.top || rc1.left > rc2.right || rc1.top > rc2.bottom || rc1.zlow > rc2.zhigh || rc1.zhigh < rc2.zlow)
		{
		return fFalse;
		}
	return fTrue;
	}
