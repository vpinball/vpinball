#pragma once
class FlipperAnimObject : public AnimObject
	{
public:
	void SetObjects(const float angle);
	//BOOL FIsInside(Ball *pball);
	virtual void UpdateDisplacements(float dtime);
	//virtual void ResetFrameTime();
	//virtual void UpdateTimePermanent();
	virtual void UpdateVelocities(float dtime);

	virtual BOOL FMover() {return fTrue;}
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(RECT *prc);

	Flipper *m_pflipper;

	LineSeg m_lineseg1;
	LineSeg m_lineseg2;
	HitCircle m_hitcircleEnd;
	HitCircle m_hitcircleBase;
	float m_endradius;
	float faceNormOffset; 

	// rlc New Flipper motion basis, uses Green's transform to rotate these valuse to curAngle
	bool m_lastHitFace;
	Vertex2D m_leftFaceNormal, m_rightFaceNormal, m_leftFaceBase, m_rightFaceBase;
	Vertex2D m_endRadiusCenter;
	float m_lengthFace;
	//rlc end

	float m_anglespeed;
	float m_angleCur;
	float m_angleEnd;

	float m_lastAngspd;
		
	float m_flipperradius;
	float m_force;
	float m_mass;
	int m_fAcc;				//rotational acceleration  -1, 0, +1
	float m_elasticity;

	float m_height;

	float m_maxvelocity;

	float m_angleMin, m_angleMax;

	Vector<ObjFrame> m_vddsFrame;
	float m_frameStart;
	float m_frameEnd;

	float m_angleFrame; // angle at the beginning of this frame

	float m_inertia;	//moment of inertia

	int m_iframe; //Frame index that this flipper is currently displaying

	bool m_fEnabled;

	int m_EnableRotateEvent;

	Vertex2D zeroAngNorm; // base norms at zero degrees	
	};

class HitFlipper :
	public HitObject
	{
public:
	Vertex2D v;
	//float rad1, rad2;

	HitFlipper(const float x, const float y, float baser, float endr, float flipr, const float angle,
		       const float zlow, const float zhigh, float strength, const float mass);
	~HitFlipper();

	//void GetHitShapes(Vector<HitObject> *pvho);

	virtual float HitTestFlipperFace(Ball *pball, float dtime, Vertex3Ds *phitnormal, bool face1);

	virtual float HitTestFlipperEnd(Ball *pball, float dtime, Vertex3Ds *phitnormal); 

	virtual float HitTest(Ball *pball, float dtime, Vertex3Ds *phitnormal);
	
	virtual int GetType() {return eFlipper;}

	virtual void Draw(HDC hdc);

	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_flipperanim;}

	virtual void CalcHitRect();

	//LineSeg m_linesegConnect;

	//Vector<HitObject> m_vho;

	Flipper *m_pflipper;

	//float m_forcemass; // Force of the flipper, treated as the mass of the moving object;
	float m_forcemass; // Force of the flipper, treated as the mass of the moving object;

	FlipperAnimObject m_flipperanim;
	int m_last_hittime;
	};
