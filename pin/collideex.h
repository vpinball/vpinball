#pragma once

class Surface;

class BumperAnimObject : public AnimObject
{
public:
	virtual void Check3D()   { }
	virtual void Reset();

	float m_fHeight;
	U32 m_TimeReset; // Time at which to turn off light
	int m_iframedesired;
	BOOL m_fHitEvent;
	BOOL m_fDisabled;
	BOOL m_fVisible;
};

class BumperHitCircle : public HitCircle
{
public:
	BumperHitCircle();

	virtual void Collide(CollisionEvent* coll);

	virtual AnimObject *GetAnimObject() {return &m_bumperanim;}

	BumperAnimObject m_bumperanim;

	Bumper *m_pbumper;
};

class SlingshotAnimObject : public AnimObject
{
public:
	virtual void Check3D();
	virtual void Reset();

	int m_iframe;
	U32 m_TimeReset; // Time at which to pull in slingshot
	bool m_fAnimations;
};

class LineSegSlingshot : public LineSeg
{
public:
	LineSegSlingshot();

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual void Collide(CollisionEvent* coll);

	virtual AnimObject *GetAnimObject() {return &m_slingshotanim;}

	SlingshotAnimObject m_slingshotanim;

	float m_force;

	Surface *m_psurface;
};

class Hit3DPoly : public HitObject
{
public:
	Hit3DPoly(Vertex3Ds * const rgv, const int count);
	virtual ~Hit3DPoly();
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual int GetType() const {return e3DPoly;}
	virtual void Collide(CollisionEvent *coll);
	virtual void CalcHitRect();

	Vertex3Ds *m_rgv;
	Vertex3Ds normal;
	int m_cvertex;
};

class HitTriangle : public HitObject
{
public:
	HitTriangle(const Vertex3Ds rgv[3]);
	virtual ~HitTriangle() {}
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual int GetType() const {return eTriangle;}
	virtual void Collide(CollisionEvent* coll);
    virtual void Contact(CollisionEvent& coll, float dtime);
	virtual void CalcHitRect();

    bool IsDegenerate() const       { return normal.IsZero(); }

	Vertex3Ds m_rgv[3];
	Vertex3Ds normal;
};


class HitPlane : public HitObject
{
public:
    HitPlane() {}
    HitPlane(const Vertex3Ds& normal_, const float d_);

    virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
    virtual int GetType() const { return ePlane; }
    virtual void Collide(CollisionEvent* coll);
    virtual void Contact(CollisionEvent& coll, float dtime);
    virtual void CalcHitRect() {}  // TODO: this is needed if we want to put it in the quadtree

    Vertex3Ds normal;
    float d;
};


class SpinnerAnimObject : public AnimObject
{
public:
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	virtual bool FMover() const {return true;}

	virtual void Reset();

	Spinner *m_pspinner;

	float m_anglespeed;
	float m_angle;
	float m_angleMax;
	float m_angleMin;
	float m_elasticity;
    float m_damping;
	BOOL m_fVisible;
};

class HitSpinner : public HitObject
{
public:
	HitSpinner(Spinner * const pspinner, const float height);

	virtual int GetType() const {return eSpinner;}

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);

	virtual void Collide(CollisionEvent* coll);

	virtual void CalcHitRect();

	virtual AnimObject *GetAnimObject() {return &m_spinneranim;}

	LineSeg m_lineseg[2];

	SpinnerAnimObject m_spinneranim;
};

class GateAnimObject : public AnimObject
{
public:
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	virtual bool FMover() const {return true;}

	virtual void Reset();

	Gate *m_pgate;

	float m_anglespeed;
	float m_angle;
	float m_angleMin, m_angleMax;
	float m_friction;
	BOOL m_fVisible; 
	
	BOOL m_fOpen; // True when the table logic is opening the gate, not just the ball passing through
};

class HitGate : public LineSeg
{
public:
	HitGate(Gate * const pgate);

	virtual int GetType() const {return eGate;}

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);

	virtual void Collide(CollisionEvent* coll);

	virtual AnimObject *GetAnimObject() {return &m_gateanim;}

	Gate *m_pgate;
	
	GateAnimObject m_gateanim;
};

class TriggerLineSeg : public LineSeg
{
public:
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual void Collide(CollisionEvent* coll);

	virtual int GetType() const {return eTrigger;}

	Trigger *m_ptrigger;
};

class TriggerHitCircle : public HitCircle
{
public:
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual void Collide(CollisionEvent* coll);

	virtual int GetType() const {return eTrigger;}

	Trigger *m_ptrigger;
};

/*
 * Arbitrary line segment in 3D space.
 *
 * Is implemented by transforming a HitLineZ to the desired orientation.
 */
class HitLine3D : public HitLineZ
{
public:
	HitLine3D(const Vertex3Ds& v1, const Vertex3Ds& v2);

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual void Collide(CollisionEvent* coll);

	virtual void CalcHitRect();

	virtual int GetType() const {return e3DLine;}

private:
	void CacheHitTransform(const Vertex3Ds& v1, const Vertex3Ds& v2);

    Matrix3 matTrans;
};


class DispReelAnimObject : public AnimObject
{
public:
    virtual void Check3D();
	virtual void Reset();

	DispReel *m_pDispReel;
};

class DispReelUpdater : public HitObject
{
public:
	DispReelUpdater(DispReel * const ptb) {m_dispreelanim.m_pDispReel = ptb;}
	virtual ~DispReelUpdater() {}

	virtual int GetType() const {return eDispReel;}

	// Bogus methods
	virtual void Collide(CollisionEvent* coll) {}
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_dispreelanim;}

	DispReelAnimObject m_dispreelanim;
};

class LightSeqAnimObject : public AnimObject
{
public:
    virtual void Check3D();

	LightSeq *m_pLightSeq;
};

class LightSeqUpdater : public HitObject
{
public:
	LightSeqUpdater(LightSeq * const ptb) {m_lightseqanim.m_pLightSeq = ptb;}
	virtual ~LightSeqUpdater() {}

	virtual int GetType() const {return eLightSeq;}

	// Bogus methods
	virtual void Collide(CollisionEvent* coll) {}
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_lightseqanim;}

	LightSeqAnimObject m_lightseqanim;
};
