#pragma once
class Surface;
class Level;

class BumperAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(RECT *prc);
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

	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_bumperanim;}

	BumperAnimObject m_bumperanim;

	Bumper *m_pbumper;
	};

class SlingshotAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(RECT *prc);
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

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_slingshotanim;}

	SlingshotAnimObject m_slingshotanim;

	float m_force;

	Surface *m_psurface;
	};

class LineSegLevelEdge : public LineSeg
	{
public:
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	Level *m_plevel1, *m_plevel2;
	};

class Hit3DPoly : public HitObject
	{
public:
	Hit3DPoly(Vertex3D *rgv, int count, bool keepptr);
	virtual ~Hit3DPoly();
	virtual PINFLOAT HitTestBasicPolygon(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal,bool direction, bool rigid);
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);
	virtual int GetType() {return e3DPoly;}
	virtual void Draw(HDC hdc);
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);
	void CalcNormal();
	virtual void CalcHitRect();

	Vertex3D *m_rgv;
	Vertex3Ds normal;
	float D; // D for the plane equation.  A,B, and C are the plane normal (A x' + B y' +C z' = D, normal= x',y'+z'
	int m_cvertex;
	BOOL m_fVisible; // for ball shadows
	};

class SpinnerAnimObject : public AnimObject
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

	HitSpinner(Spinner *pspinner, float height);

	virtual void Draw(HDC hdc);

	virtual int GetType() {return eSpinner;}

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);

	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual void CalcHitRect();

	virtual AnimObject *GetAnimObject() {return &m_spinneranim;}

	LineSeg m_lineseg[2];

	SpinnerAnimObject m_spinneranim;
	};

class GateAnimObject : public AnimObject
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
	virtual void Reset();

	Vector<ObjFrame> m_vddsFrame;

	Gate *m_pgate;

	int m_iframe; //Frame index that this flipper is currently displaying

	float m_anglespeed;
	float m_angle;
	PINFLOAT m_angleMin, m_angleMax;
	PINFLOAT m_friction;
	BOOL m_fVisible; 
	
	BOOL m_fOpen; // True when the table logic is opening the gate, not just the ball passing through
	};

class HitGate : public LineSeg
	{
public:

	HitGate(Gate *pgate);

	virtual int GetType() {return eGate;}

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);

	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual AnimObject *GetAnimObject() {return &m_gateanim;}

	Gate *m_pgate;
	
	GateAnimObject m_gateanim;
	};

class TriggerLineSeg : public LineSeg
	{
public:

	TriggerLineSeg();

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual int GetType() {return eTrigger;}

	Trigger *m_ptrigger;
	};

class TriggerHitCircle : public HitCircle
	{
public:

	TriggerHitCircle();

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual int GetType() {return eTrigger;}

	Trigger *m_ptrigger;
	};

class Hit3DCylinder : public HitCircle
	{
public:

	Hit3DCylinder(Vertex3D *pv1, Vertex3D *pv2, Vertex3Ds *pvnormal);

	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal);
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal);

	virtual void CalcHitRect();

	virtual int GetType() {return e3DLine;}

	void CacheHitTransform();

	Vertex3Ds v1, v2;
	Vertex3Ds normal;

	Vertex3Ds vtrans[2];
	Vertex3Ds transaxis;
	PINFLOAT transangle;
	};

class PolyDropAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(RECT *prc);
	virtual void Reset();

	int m_iframe;
	int m_TimeReset; // Time at which to turn off light

	int m_iframedesire; // Frame we want to be at

	ObjFrame *m_pobjframe[2];
	};

class Hit3DPolyDrop : public Hit3DPoly
	{
public:
	Hit3DPolyDrop(Vertex3D *rgv, int count, bool keepptr);

	virtual AnimObject *GetAnimObject() {return &m_polydropanim;}

	PolyDropAnimObject m_polydropanim;
	};

class TextboxAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

	virtual void Check3D() {}
	virtual ObjFrame *Draw3D(RECT *prc);
	virtual void Reset();

	Textbox *m_ptextbox;
	};

class TextboxUpdater : public HitObject
	{
public:

	TextboxUpdater(Textbox *ptb) {m_textboxanim.m_ptextbox = ptb;}
	virtual ~TextboxUpdater() {}

	virtual int GetType() {return eTextbox;}

	// Bogus methods
	virtual void Draw(HDC hdc) {}
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal) {}
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_textboxanim;}

	TextboxAnimObject m_textboxanim;
	};

class DispReelAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

    virtual void Check3D(); //{}
	virtual ObjFrame *Draw3D(RECT *prc);
	virtual void Reset();

	DispReel *m_pDispReel;
	};

class DispReelUpdater : public HitObject
	{
public:

	DispReelUpdater(DispReel *ptb) {m_dispreelanim.m_pDispReel = ptb;}
	virtual ~DispReelUpdater() {}

	virtual int GetType() {return eDispReel;}

	// Bogus methods
	virtual void Draw(HDC hdc) {}
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal) {}
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_dispreelanim;}

	DispReelAnimObject m_dispreelanim;
	};

class LightSeqAnimObject : public AnimObject
	{
public:
	virtual BOOL FNeedsScreenUpdate() {return fTrue;}

    virtual void Check3D(); //{}
	virtual ObjFrame *Draw3D(RECT *prc);

	LightSeq *m_pLightSeq;
	};

class LightSeqUpdater : public HitObject
	{
public:

	LightSeqUpdater(LightSeq *ptb) {m_lightseqanim.m_pLightSeq = ptb;}
	virtual ~LightSeqUpdater() {}

	virtual int GetType() {return eLightSeq;}

	// Bogus methods
	virtual void Draw(HDC hdc) {}
	virtual void Collide(Ball *pball, Vertex3Ds *phitnormal) {}
	virtual PINFLOAT HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal) {return -1;}
	virtual void CalcHitRect() {}

	virtual AnimObject *GetAnimObject() {return &m_lightseqanim;}

	LightSeqAnimObject m_lightseqanim;
	};

