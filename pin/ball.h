
#define MAXBALLS 256

//#define GRAVITY 0.65f //0.574f

#define GRAVITY 0.86543

class HitObject;
class Level;
class Ball;

class BallAnimObject : public AnimObject
	{
public:
	virtual BOOL FMover() {return fFalse;} // We add ourselves to the mover list.  If we allow the table to do that, we might get added twice, if we get created in Init code
	virtual void UpdateTimeTemp(PINFLOAT dtime);
	virtual void UpdateAcceleration(PINFLOAT dtime);

	Ball *m_pball;
	};

class Ball : public HitObject
	{
public:
	Ball();
	~Ball();

	void Init();

	virtual void UpdateTimeTemp(PINFLOAT dtime);
	virtual void UpdateAcceleration(PINFLOAT dtime);

	// From HitObject
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);	
	virtual int GetType() {return eBall;}
	virtual void Draw(HDC hdc) {} // Dead function
	virtual void Collide(Ball *pball, Vertex3D *phitnormal);
	virtual void CalcHitRect();

	void CollideWall(Vertex3D *phitnormal, float elasticity, float antifriction, float scatter_angle);
	void Collide3DWall(Vertex3D *phitnormal, float elasticity, float antifriction, float scatter_angle);
	void AngularAcceleration(Vertex3D *phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_ballanim;}

	//~Ball();
	//Ball(Ball &pball);

	Vertex3D m_rgv3D[4]; // Last vertices of the ball texture
	Vertex3D m_rgv3DShadow[4]; // Last vertices of the ball shadow
	BOOL m_fErase; // set after the ball has been drawn for the first time
	RECT m_rcScreen; // rect where the ball appears on the screen
	RECT m_rcScreenShadow;
	
	COLORREF m_color;

	// Per frame info
	FRect brc; // bounding rectangle

	//BOOL fSync; // Whether this ball is on the synchronous list

	HitObject *phoHitLast; // Hit object the ball hit last - make sure it doesn't hit same thing again right away - for round-off problems

	void CalcBoundingRect();

	void EnsureOMObject();

	BOOL m_fCalced; // Whether this ball has had its next collision determined
	HitObject *m_pho;
	PINFLOAT m_hittime; // time at which this ball will hit something
	PINFLOAT m_hitx, m_hity; // position of the ball at hit time (saved to avoid floating point errors with multiple time slices)

	PINFLOAT m_HitDist;			// hit distance 
	PINFLOAT m_HitNormVel;		// hit normal Velocity
	BOOL m_HitRigid;			// Rigid = 1, Non-Rigid = 0	
	int m_fDynamic;				// used to determine static ball conditions and velocity quenching, 
	Vertex3D m_hitnormal[5];	//rlc 0: hit normal, 1: hit object velocity, 2: monent and angular rate, 4: contact distance

	VectorVoid* m_vpVolObjs;	// vector of triggers we are now inside
	Vertex3D m_Event_Pos;	// ultracade physics thing

	//Vertex3D m_hitnormal[10/*2*/];

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

	float radius;

	BOOL fFrozen;
	BOOL fTempFrozen; //if the ball is stuck and we are avoiding hittesting

	Matrix3 m_orientation;
	Vertex3D m_angularmomentum;
	Vertex3D m_angularvelocity;
	Matrix3 m_inverseworldinertiatensor;
	Matrix3 m_inversebodyinertiatensor;
	};

class Level
	{
public:
	double m,n,b; // Plane equation z = mx + ny + b
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