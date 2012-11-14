#pragma once
class Surface;
class Level;

class PrimitiveAnimObject : public AnimObject
{
	//m_fInvalid, Check3D and m_rcBounds
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}
	virtual BOOL FMover() {return fTrue;}
	virtual void Check3D();

	BOOL m_fVisible;
	BOOL m_fDisabled;
};

class HitPrimitive : public HitObject
{
public:
	HitPrimitive();

	virtual AnimObject *GetAnimObject() {return &m_primitiveAnim;}
	PrimitiveAnimObject m_primitiveAnim;

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);
	virtual int GetType() {return ePrimitive;}
	virtual void Draw(HDC hdc);
	virtual void CalcHitRect();

};

class BumperAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(const RECT * const prc);
	virtual void Reset();

	int m_iframe;
	int m_TimeReset; // Time at which to turn off light
	int m_iframedesired;
	BOOL m_fAutoTurnedOff;
	BOOL m_fDisabled;
	BOOL m_fVisible;

	ObjFrame *m_pobjframe[2];
	};

class BumperHitCircle : public HitCircle
	{
public:

	BumperHitCircle();
	virtual ~BumperHitCircle();

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_bumperanim;}

	BumperAnimObject m_bumperanim;

	Bumper *m_pbumper;
	};

class SlingshotAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(const RECT * const prc);
	virtual void Reset();

	int m_iframe;
	int m_TimeReset; // Time at which to pull in slingshot
	bool m_fAnimations;

	ObjFrame *m_pobjframe;
	};

class LineSegSlingshot : public LineSeg
	{
public:

	LineSegSlingshot();
	virtual ~LineSegSlingshot();

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_slingshotanim;}

	SlingshotAnimObject m_slingshotanim;

	float m_force;

	Surface *m_psurface;
	};

class LineSegLevelEdge : public LineSeg
	{
public:
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	Level *m_plevel1, *m_plevel2;
	};

class Hit3DPoly : public HitObject
	{
public:
	Hit3DPoly(Vertex3Ds * const rgv, const int count, const bool keepptr);
	virtual ~Hit3DPoly();
	virtual float HitTestBasicPolygon(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool direction, const bool rigid);
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual int GetType() {return e3DPoly;}
	virtual void Draw(HDC hdc);
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);
	void CalcNormal();
	virtual void CalcHitRect();

	Vertex3Ds *m_rgv;
	Vertex3Ds normal;
	float D; // D for the plane equation.  A,B, and C are the plane normal (A x' + B y' +C z' = D, normal= x',y'+z'
	int m_cvertex;
	BOOL m_fVisible; // for ball shadows
	};

class SpinnerAnimObject : public AnimObject
	{
public:
	virtual void UpdateDisplacements(const float dtime);
	//virtual void ResetFrameTime();
	//virtual void UpdateTimePermanent();
	virtual void UpdateVelocities(const float dtime);

	virtual BOOL FMover() {return fTrue;}
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(const RECT * const prc);
	virtual void Reset();

	Vector<ObjFrame> m_vddsFrame;

	Spinner *m_pspinner;

	int m_iframe; //Frame index that this spinner is currently displaying

	float m_anglespeed;
	float m_angle;
	float m_angleMax;
	float m_angleMin;
	float m_elasticity;
	float m_friction;
	float m_scatter;
	BOOL m_fVisible;
	};

class HitSpinner : public HitObject
	{
public:

	HitSpinner(Spinner * const pspinner, const float height);

	virtual void Draw(HDC hdc);

	virtual int GetType() {return eSpinner;}

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	virtual AnimObject *GetAnimObject() {return &m_spinneranim;}

	LineSeg m_lineseg[2];

	SpinnerAnimObject m_spinneranim;
	};

class GateAnimObject : public AnimObject
	{
public:
	virtual void UpdateDisplacements(const float dtime);
	//virtual void ResetFrameTime();
	//virtual void UpdateTimePermanent();
	virtual void UpdateVelocities(const float dtime);

	virtual BOOL FMover() {return fTrue;}
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(const RECT * const prc);
	virtual void Reset();

	Vector<ObjFrame> m_vddsFrame;

	Gate *m_pgate;

	int m_iframe; //Frame index that this flipper is currently displaying

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

	virtual int GetType() {return eGate;}

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_gateanim;}

	Gate *m_pgate;
	
	GateAnimObject m_gateanim;
	};

class TriggerLineSeg : public LineSeg
	{
public:

	TriggerLineSeg();

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual int GetType() {return eTrigger;}

	Trigger *m_ptrigger;
	};

class TriggerHitCircle : public HitCircle
	{
public:

	TriggerHitCircle();

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual int GetType() {return eTrigger;}

	Trigger *m_ptrigger;
	};

class Hit3DCylinder : public HitCircle
	{
public:

	Hit3DCylinder(const Vertex3Ds * const pv1, const Vertex3Ds * const pv2, const Vertex3Ds * const pvnormal);

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	virtual int GetType() {return e3DLine;}

	void CacheHitTransform();

	Vertex3Ds v1, v2;
	Vertex3Ds normal;

	Vertex3Ds vtrans[2];
	Vertex3Ds transaxis;
	float transangle;
	};

class PolyDropAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(const RECT * const prc);
	virtual void Reset();

	int m_iframe;
	int m_TimeReset; // Time at which to turn off light

	int m_iframedesire; // Frame we want to be at

	ObjFrame *m_pobjframe[2];
	};

class Hit3DPolyDrop : public Hit3DPoly
	{
public:
	Hit3DPolyDrop(Vertex3Ds * const rgv, const int count, const bool keepptr);

	virtual AnimObject *GetAnimObject() {return &m_polydropanim;}

	PolyDropAnimObject m_polydropanim;
	};

class TextboxAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D() {}
	virtual ObjFrame *Draw3D(const RECT * const prc);
	virtual void Reset();

	Textbox *m_ptextbox;
	};

class TextboxUpdater : public HitObject
	{
public:

	TextboxUpdater(Textbox * const ptb) {m_textboxanim.m_ptextbox = ptb;}
	virtual ~TextboxUpdater() {}

	virtual int GetType() {return eTextbox;}

	// Bogus methods
	virtual void Draw(HDC hdc) {}
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal) {}
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_textboxanim;}

	TextboxAnimObject m_textboxanim;
	};

class DispReelAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

    virtual void Check3D(); //{}
	virtual ObjFrame *Draw3D(const RECT * const prc);
	virtual void Reset();

	DispReel *m_pDispReel;
	};

class DispReelUpdater : public HitObject
	{
public:

	DispReelUpdater(DispReel * const ptb) {m_dispreelanim.m_pDispReel = ptb;}
	virtual ~DispReelUpdater() {}

	virtual int GetType() {return eDispReel;}

	// Bogus methods
	virtual void Draw(HDC hdc) {}
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal) {}
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_dispreelanim;}

	DispReelAnimObject m_dispreelanim;
	};

class LightSeqAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

    virtual void Check3D(); //{}
	virtual ObjFrame *Draw3D(const RECT * const prc);

	LightSeq *m_pLightSeq;
	};

class LightSeqUpdater : public HitObject
	{
public:

	LightSeqUpdater(LightSeq * const ptb) {m_lightseqanim.m_pLightSeq = ptb;}
	virtual ~LightSeqUpdater() {}

	virtual int GetType() {return eLightSeq;}

	// Bogus methods
	virtual void Draw(HDC hdc) {}
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal) {}
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_lightseqanim;}

	LightSeqAnimObject m_lightseqanim;
	};
