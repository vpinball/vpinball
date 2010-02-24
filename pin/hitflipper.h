class FlipperAnimObject : public AnimObject
	{
public:
	void SetObjects(int i, PINFLOAT angle);
	void SetObjectsNewPhys(PINFLOAT angle);
	//BOOL FIsInside(Ball *pball);
	virtual void UpdateTimeTemp(PINFLOAT dtime);
	//virtual void ResetFrameTime();
	//virtual void UpdateTimePermanent();
	virtual void UpdateAcceleration(PINFLOAT dtime);

	virtual BOOL FMover() {return fTrue;}
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(RECT *prc);

	LineSeg m_lineseg1[2];
	LineSeg m_lineseg2[2];
	HitCircle m_hitcircleEnd[2];
	HitCircle m_hitcircleBase;

	PINFLOAT m_anglespeed;
	PINFLOAT m_angleCur;
	PINFLOAT m_angleEnd;
	PINFLOAT m_angleDelta;
	PINFLOAT m_angleSpan;
	PINFLOAT m_endradius;
	PINFLOAT m_flipperradius;
	PINFLOAT m_force;
	PINFLOAT m_mass;
	BOOL m_fAcc;
	PINFLOAT m_elasticity;

	PINFLOAT m_maxvelocity;

	PINFLOAT m_angleMin, m_angleMax;

	Vector<ObjFrame> m_vddsFrame;
	float m_frameStart;
	float m_frameEnd;

	float m_angleFrame; // angle at the beginning of this frame

	int m_iframe; //Frame index that this flipper is currently displaying

	BOOL m_fEnabled;
	
	/* Begin New Phys stuff */
	
	PINFLOAT m_height;
	
	PINFLOAT m_inertia;	//moment of inertia

	PINFLOAT faceNormOffset; 

	// rlc New Flipper motion basis, uses Green's transform to rotate these valuse to curAngle
	bool m_lastHitFace;
	Vertex m_leftFaceNormal, m_rightFaceNormal, m_leftFaceBase,m_rightFaceBase;
	Vertex m_endRadiusCenter;
	float m_lengthFace;
	//rlc end

	PINFLOAT m_lastAngspd;
	
	Vertex zeroAngNorm; // base norms at zero degrees	

	/* End New Phys stuff */
	};

class HitFlipper :
	public HitObject
	{
public:
	Vertex v;
	//float rad1, rad2;

	HitFlipper(float x, float y, float baser, float endr, float flipr, float angle, float zlow, float zhigh, float strength, float mass);
	~HitFlipper();

	//void GetHitShapes(Vector<HitObject> *pvho);

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);

	PINFLOAT HitTestFlipperFace(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal, bool face1);

	PINFLOAT HitTestFlipperEnd(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal); 

	virtual int GetType() {return eFlipper;}

	virtual void Draw(HDC hdc);

	virtual void Collide(Ball *pball, Vertex3D *phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_flipperanim;}

	virtual void CalcHitRect();

	//LineSeg m_linesegConnect;

	//Vector<HitObject> m_vho;

	Flipper *m_pflipper;

	PINFLOAT m_forcemass; // Force of the flipper, treated as the mass of the moving object;

	FlipperAnimObject m_flipperanim;
	
	/* Begin New Phys stuff */
	int m_last_hittime;
	/* End New Phys stuff */
	};