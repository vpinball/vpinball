// license:GPLv3+

#pragma once

class Surface;

class Bumper;
class Spinner;
class Gate;
class Trigger;

class BumperHitCircle : public HitCircle
{
public:
   BumperHitCircle(Bumper* const pBumper, const Vertex2D& c, const float r, const float zlow, const float zhigh);
   void Collide(const CollisionEvent& coll) override;

   Vertex3Ds m_bumperanim_hitBallPosition;
   float m_bumperanim_ringAnimOffset;
   bool m_bumperanim_hitEvent;
};

class LineSegSlingshot : public LineSeg
{
public:
   LineSegSlingshot(Surface* const psurface, const Vertex2D& p1, const Vertex2D& p2, const float _zlow, const float _zhigh);
   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eLineSegSlingshot; }
   void Collide(const CollisionEvent& coll) override;

   Surface * const m_psurface;

   float m_force;
   unsigned int m_EventTimeReset;

   bool m_doHitEvent;

   void Animate();

   U32 m_TimeReset; // Time at which to pull in slingshot, Zero means the slingshot is currently reset
   bool m_animations;
   bool m_iframe;
};

class Hit3DPoly : public HitObject
{
public:
   Hit3DPoly(IEditable*const editable, Vertex3Ds * const rgv, const int count); // pointer is copied and content deleted in dtor
   Hit3DPoly(IEditable* const editable, const float x, const float y, const float z, const float r, const int sections); // creates a circular hit poly
   ~Hit3DPoly() override;

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return e3DPoly; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   void Init(Vertex3Ds * const rgv, const int count);

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList) const override { } // FIXME implement

private:
   Vertex3Ds *m_rgv;
   Vertex3Ds m_normal;
   int m_cvertex;
};

// Note that HitTriangle ONLY does include the plane and barycentric test, but NOT the edge and vertex test,
// thus one has to add these separately per mesh
class HitTriangle : public HitObject
{
public:
   HitTriangle(IEditable* const editable, const Vertex3Ds rgv[3]); // vertices in counterclockwise order
   ~HitTriangle() override {}

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eTriangle; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   bool IsDegenerate() const { return m_normal.IsZero(); }

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList) const override;

   Vertex3Ds m_rgv[3];
   Vertex3Ds m_normal;
};


class HitPlane : public HitObject
{
public:
   HitPlane(IEditable* const editable) : HitObject(editable) { }
   HitPlane(IEditable* const editable, const Vertex3Ds& normal, const float d)
      : HitObject(editable)
      , m_normal(normal)
      , m_d(d)
   {
   }

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return ePlane; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override {}  //!! TODO: this is needed if we want to put it in the quadtree, but then again impossible as infinite area

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList) const override { } // FIXME implement

private:
   Vertex3Ds m_normal;
   float m_d;
};


class SpinnerMoverObject : public MoverObject
{
public:
   void UpdateDisplacements(const float dtime) override;
   void UpdateVelocities() override;

   bool AddToList() const override { return true; }

   Spinner *m_pspinner;

   float m_anglespeed;
   float m_angle;
   float m_angleMax;
   float m_angleMin;
   float m_elasticity;
   float m_damping;
   bool m_visible;
};

class HitSpinner : public HitObject
{
public:
   HitSpinner(Spinner * const pspinner, const float height);

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eSpinner; }
   void Collide(const CollisionEvent& coll) override;
   void Contact(CollisionEvent& coll, const float dtime) override { }
   void CalcHitBBox() override;

   MoverObject *GetMoverObject() override { return &m_spinnerMover; }

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList) const override;

   LineSeg m_lineseg[2];

   SpinnerMoverObject m_spinnerMover;
};

class GateMoverObject : public MoverObject
{
public:
   void UpdateDisplacements(const float dtime) override;
   void UpdateVelocities() override;

   bool AddToList() const override { return true; }

   Gate *m_pgate;

   float m_anglespeed;
   float m_angle;
   float m_angleMin, m_angleMax;
   float m_friction;
   float m_damping;
   float m_gravityfactor;
   bool m_visible;
   bool m_open;         // True if the table logic is opening the gate, not just the ball passing through
   bool m_forcedMove;   // True if the table logic is opening/closing the gate
   bool m_hitDirection; // For the direction of the little bounce-back
};

class HitGate : public HitObject
{
public:
   HitGate(Gate * const pgate, const float height);

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eGate; }
   void Collide(const CollisionEvent& coll) override;
   void Contact(CollisionEvent& coll, const float dtime) override { }
   void CalcHitBBox() override;

   MoverObject *GetMoverObject() override { return &m_gateMover; }

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList) const override;

   GateMoverObject m_gateMover;
   bool m_twoWay;

private:
   Gate *m_pgate;
   LineSeg m_lineseg[2];
};

class TriggerLineSeg : public LineSeg
{
public:
   TriggerLineSeg(Trigger* const trigger, const Vertex2D& p1, const Vertex2D& p2, const float zlow, const float zhigh);
   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eTrigger; }
   void Collide(const CollisionEvent& coll) override;

   Trigger * const m_ptrigger;
};

class TriggerHitCircle : public HitCircle
{
public:
   TriggerHitCircle(Trigger* const trigger, const Vertex2D& c, const float r, const float zlow, const float zhigh);
   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eTrigger; }
   void Collide(const CollisionEvent& coll) override;
};

// Arbitrary line segment in 3D space.
// Implemented by transforming a HitLineZ to the desired orientation.
class HitLine3D : public HitLineZ
{
public:
   HitLine3D(IEditable* const editable, const Vertex3Ds& v1, const Vertex3Ds& v2);

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return e3DLine; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override { } // already done in constructor

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList) const override;

private:
   Matrix3 m_matrix;
   Vertex3Ds m_v1, m_v2;
};
