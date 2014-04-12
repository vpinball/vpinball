#pragma once
class FlipperAnimObject : public AnimObject
	{
public:
	void SetObjects(const float angle);
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	virtual bool FMover() const {return true;}

	virtual void Check3D()                              { }

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

	float m_anglespeed;
	float m_angleCur;
	float m_angleEnd;

	float m_flipperradius;
	float m_force;
	float m_mass;
	int m_fAcc;				//rotational acceleration  -1, 0, +1
	float m_elasticity;

	float m_height;

	float m_maxvelocity;

	float m_angleMin, m_angleMax;

#if 0
	float m_inertia;	//moment of inertia
#endif

	int m_EnableRotateEvent;

	Vertex2D zeroAngNorm; // base norms at zero degrees	

	bool m_fEnabled;
   bool m_fVisible;
	bool m_lastHitFace;
   bool m_fCompatibility;
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

	virtual float HitTestFlipperFace(const Ball * pball, const float dtime, CollisionEvent& coll, const bool face1);

	virtual float HitTestFlipperEnd(const Ball * pball, const float dtime, CollisionEvent& coll);

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	
	virtual int GetType() const {return eFlipper;}

	virtual void Collide(CollisionEvent *coll);

	virtual AnimObject *GetAnimObject() {return &m_flipperanim;}

	virtual void CalcHitRect();

	Flipper *m_pflipper;

	float m_forcemass; // Force of the flipper, treated as the mass of the moving object;

	FlipperAnimObject m_flipperanim;
	int m_last_hittime;
	};
