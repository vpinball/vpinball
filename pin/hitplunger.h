#pragma once
class PlungerAnimObject : public AnimObject
	{
public:
	virtual void UpdateDisplacements(PINFLOAT dtime);
	//virtual void ResetFrameTime();
	//virtual void UpdateTimePermanent();
	virtual void UpdateVelocities(PINFLOAT dtime);

	virtual BOOL FMover() {return fTrue;}
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(RECT *prc);

	void SetObjects(float len);

	float mechPlunger(void);// Returns mechanical plunger position 0 at rest, +1 pulled (fully extended)

	LineSeg m_linesegBase;
	LineSeg m_linesegEnd;
	LineSeg m_linesegSide[2];

	Joint m_jointBase[2];
	Joint m_jointEnd[2];

	Vector<ObjFrame> m_vddsFrame;

	int m_iframe; //Frame index that this flipper is currently displaying

	float m_speed;
	float m_pos;
	float m_posdesired;
	float m_posFrame; // Location of plunger at beginning of frame
	BOOL  m_fAcc;
	float m_mass;

	float m_x,m_x2,m_y;

	float m_force;

	float m_frameStart;
	float m_frameEnd;
	int m_mechTimeOut;

	bool recock;
	float err_fil;	// integrate error over multiple update periods 

	float m_parkPosition;
	float m_scatterVelocity;
	float m_breakOverVelocity;
	Plunger* m_plunger;
	};

class HitPlunger :
	public HitObject
	{
public:

	HitPlunger(float x, float y, float x2, float pos, float zheight, Plunger* pPlunger);
	~HitPlunger() {}

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal);

	virtual int GetType() {return ePlunger;}

	virtual void Draw(HDC hdc);

	virtual void Collide(Ball *pball, Vertex3D *phitnormal);

	virtual void CalcHitRect();

	virtual AnimObject *GetAnimObject() {return &m_plungeranim;}

	//Vector<HitObject> m_vho;

	//float m_acc;

	PlungerAnimObject m_plungeranim;

	Plunger *m_pplunger;
	};