#pragma once

class Surface;

class BumperAnimObject : public AnimObject
{
public:
   virtual void Animate() { }

   float m_ringAnimOffset;
   bool m_fHitEvent;
   bool m_fDisabled;
   bool m_fVisible;
};

class BumperHitCircle : public HitCircle
{
public:
   BumperHitCircle(const Vertex2D& c, const float r, const float zlow, const float zhigh)
      : HitCircle(c,r,zlow,zhigh)
   {
      m_bumperanim.m_fHitEvent = true;
      m_bumperanim.m_ringAnimOffset = 0.0f;
      m_elasticity = 0.3f;
      SetFriction(0.3f);
      m_scatter = 0;
      m_bumperanim.m_fVisible = true;
      m_pbumper = NULL;
   }

   virtual void Collide(CollisionEvent& coll);

   virtual AnimObject *GetAnimObject() { return &m_bumperanim; }

   BumperAnimObject m_bumperanim;

   Bumper *m_pbumper;
};

class SlingshotAnimObject : public AnimObject
{
public:
   virtual void Animate();

   int m_iframe;
   U32 m_TimeReset; // Time at which to pull in slingshot
   bool m_fAnimations;
};

class LineSegSlingshot : public LineSeg
{
public:
   LineSegSlingshot(const Vertex2D& p1, const Vertex2D& p2, const float _zlow, const float _zhigh)
      : LineSeg(p1, p2, _zlow, _zhigh)
   {
      m_slingshotanim.m_iframe = 0;
      m_slingshotanim.m_TimeReset = 0; // zero means the slingshot is currently reset
      m_elasticity = 0.3f;
      SetFriction(0.3f);
      m_scatter = 0;
      m_doHitEvent=false;
      m_force = 0.f;
      m_EventTimeReset=0;
      m_psurface = NULL;
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual void Collide(CollisionEvent& coll);

   virtual AnimObject *GetAnimObject() { return &m_slingshotanim; }

   SlingshotAnimObject m_slingshotanim;

   float m_force;
   unsigned int m_EventTimeReset;
   Surface *m_psurface;
   bool m_doHitEvent;
};

class Hit3DPoly : public HitObject
{
public:
   Hit3DPoly(Vertex3Ds * const rgv, const int count);
   virtual ~Hit3DPoly();
   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return e3DPoly; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitRect();

   Vertex3Ds *m_rgv;
   Vertex3Ds normal;
   int m_cvertex;
};

class HitTriangle : public HitObject
{
public:
   HitTriangle(const Vertex3Ds rgv[3]);    // vertices in counterclockwise order
   virtual ~HitTriangle() {}
   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return eTriangle; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitRect();

   bool IsDegenerate() const       { return normal.IsZero(); }

   Vertex3Ds m_rgv[3];
   Vertex3Ds normal;
};


class HitPlane : public HitObject
{
public:
   HitPlane() {}
   HitPlane(const Vertex3Ds& normal_, const float d_)
      : normal(normal_), d(d_)
   {
      m_elasticity = 0.2f;
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return ePlane; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitRect() {}  // TODO: this is needed if we want to put it in the quadtree

   Vertex3Ds normal;
   float d;
};


class SpinnerAnimObject : public AnimObject
{
public:
   virtual void UpdateDisplacements(const float dtime);
   virtual void UpdateVelocities();

   virtual bool FMover() const { return true; }

   Spinner *m_pspinner;

   float m_anglespeed;
   float m_angle;
   float m_angleMax;
   float m_angleMin;
   float m_elasticity;
   float m_damping;
   bool m_fVisible;
};

class HitSpinner : public HitObject
{
public:
   HitSpinner(Spinner * const pspinner, const float height);

   virtual int GetType() const { return eSpinner; }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);

   virtual void Collide(CollisionEvent& coll);

   virtual void CalcHitRect();

   virtual AnimObject *GetAnimObject() { return &m_spinneranim; }

   LineSeg m_lineseg[2];

   SpinnerAnimObject m_spinneranim;
};

class GateAnimObject : public AnimObject
{
public:
   virtual void UpdateDisplacements(const float dtime);
   virtual void UpdateVelocities();

   virtual bool FMover() const { return true; }

   Gate *m_pgate;

   float m_anglespeed;
   float m_angle;
   float m_angleMin, m_angleMax;
   float m_friction;
   float m_damping;
   bool m_fVisible;
   bool m_fOpen; // True when the table logic is opening the gate, not just the ball passing through
   bool m_forcedMove; // is true if the table logic is opening/closing the gate
};

class HitGate : public HitObject
{
public:
   HitGate(Gate * const pgate, const float height);

   virtual int GetType() const { return eGate; }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);

   virtual void Collide(CollisionEvent& coll);
   virtual void CalcHitRect();

   virtual AnimObject *GetAnimObject() { return &m_gateanim; }

   Gate *m_pgate;
   LineSeg m_lineseg[3];
   GateAnimObject m_gateanim;
   bool m_twoWay;
};

class TriggerLineSeg : public LineSeg
{
public:
   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual void Collide(CollisionEvent& coll);

   virtual int GetType() const { return eTrigger; }

   Trigger *m_ptrigger;
};

class TriggerHitCircle : public HitCircle
{
public:
   TriggerHitCircle(const Vertex2D& c, float r, float zlow, float zhigh)
     : HitCircle(c,r,zlow,zhigh) {}
   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual void Collide(CollisionEvent& coll);

   virtual int GetType() const { return eTrigger; }

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

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual void Collide(CollisionEvent& coll);

   virtual void CalcHitRect();

   virtual int GetType() const { return e3DLine; }

private:
   Matrix3 matTrans;
   float m_zlow, m_zhigh;
};

class DispReelAnimObject : public AnimObject
{
public:
   virtual void Animate();

   DispReel *m_pDispReel;
};

class LightSeqAnimObject : public AnimObject
{
public:
   virtual void Animate();

   LightSeq *m_pLightSeq;
};
