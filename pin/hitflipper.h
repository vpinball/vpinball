#pragma once
class FlipperAnimObject : public AnimObject
	{
public:
	void SetObjects(const float angle);
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	virtual bool FMover() const {return true;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(const RECT * const prc);

	Flipper *m_pflipper;

	LineSeg m_lineseg1;
	LineSeg m_lineseg2;
	HitCircle m_hitcircleEnd;
	HitCircle m_hitcircleBase;
	float m_endradius;
	float faceNormOffset; 

	// New Flipper motion basis, uses Green's transform to rotate these valuse to curAngle
	Vertex2D m_leftFaceNormal, m_rightFaceNormal, m_leftFaceBase, m_rightFaceBase;
	Vertex2D m_endRadiusCenter;
	float m_lengthFace;

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

#if 0
	float m_inertia;	//moment of inertia
#endif

	int m_iframe;		//Frame index that this flipper is currently displaying

	int m_EnableRotateEvent;

	Vertex2D zeroAngNorm; // base norms at zero degrees	

	bool m_fEnabled;
   bool m_fVisible;
	bool m_lastHitFace;
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

	virtual float HitTestFlipperFace(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool face1);

	virtual float HitTestFlipperEnd(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	
	virtual int GetType() const {return eFlipper;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_flipperanim;}

	virtual void CalcHitRect();

	//LineSeg m_linesegConnect;

	//Vector<HitObject> m_vho;

	Flipper *m_pflipper;

	float m_forcemass; // Force of the flipper, treated as the mass of the moving object;

	FlipperAnimObject m_flipperanim;
	int m_last_hittime;
	};
