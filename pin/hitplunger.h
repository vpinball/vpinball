#pragma once
class PlungerAnimObject : public AnimObject
	{
public:
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	virtual bool FMover() const {return true;}

	virtual void Check3D() { }

	void SetObjects(const float len);

	float mechPlunger() const; // Returns mechanical plunger position 0 at rest, +1 pulled (fully extended)

	LineSeg m_linesegBase;
	LineSeg m_linesegEnd;
	LineSeg m_linesegSide[2];

	HitLineZ m_jointBase[2];
	HitLineZ m_jointEnd[2];

	float m_speed;
	float m_pos;
	float m_posdesired;
	float m_mass;

	float m_x,m_x2,m_y;

	float m_force;

	float m_frameStart;
	float m_frameEnd;
	int m_mechTimeOut;

	float err_fil;	// integrate error over multiple update periods 

	float m_parkPosition;
	float m_scatterVelocity;
	float m_breakOverVelocity;
	Plunger* m_plunger;

	bool recock;
	bool m_fAcc;
};

class HitPlunger :
	public HitObject
	{
public:

	HitPlunger(const float x, const float y, const float x2, const float pos, const float zheight, Plunger * const pPlunger);
	~HitPlunger() {}

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);

	virtual int GetType() const {return ePlunger;}

	virtual void Collide(CollisionEvent *coll);
    virtual void Contact(CollisionEvent& coll, float dtime);

	virtual void CalcHitRect();

	virtual AnimObject *GetAnimObject() {return &m_plungeranim;}

	PlungerAnimObject m_plungeranim;

	Plunger *m_pplunger;
	};
