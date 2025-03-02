// license:GPLv3+

#pragma once

enum eObjType : unsigned char
{
   eNull,
   ePoint,
   eLineSeg,
   eLineSegSlingshot,
   eJoint,
   eCircle,
   eFlipper,
   ePlunger,
   eSpinner,
   eBall,
   e3DPoly,
   eTriangle,
   ePlane,
   e3DLine,
   eGate,
   eTextbox,
   eDispReel,
   eLightSeq,
   ePrimitive,  // also (ab)used for Rubbers and Ramps (as both are made out of triangles, too)
   eHitTarget,
   eTrigger,    // this value and greater are volume set tested, add rigid or non-volume set above
   eKicker      // this is done to limit to one test
};

extern float c_hardScatter;

// forward declarations
struct BallS;
class HitBall;
class HitObject;
struct ImDrawList;
class IEditable;

class MoverObject // Spinner, Gate, Flipper, Plunger and Ball
{
public:
   virtual bool AddToList() const = 0;
   virtual void UpdateDisplacements(const float dtime) = 0;
   virtual void UpdateVelocities() = 0;
};

// Ported at: VisualPinball.Engine/Math/Functions.cs

// Rubber has a coefficient of restitution which decreases with the impact velocity.
// We use a heuristic model which decreases the COR according to a falloff parameter:
// 0 = no falloff, 1 = half the COR at 1 m/s (18.53 speed units)
inline float ElasticityWithFalloff(const float elasticity, const float falloff, const float vel)
{
   if (falloff > 0.f)
      return elasticity / (1.0f + falloff * fabsf(vel) * (float)(1.0 / 18.53));
   else
      return elasticity;
}

// Ported at: VisualPinball.Engine/Physics/CollisionEvent.cs

struct CollisionEvent
{
   CollisionEvent() : m_ball(0), m_obj(0), m_hittime(0.0f), m_hitdistance(0.0f), /*m_hitmoment(0.0f)*/ m_hitmoment_bit(true), m_hitflag(false), /*m_hitRigid(false),*/ m_isContact(false) {}

   HitBall* m_ball;      // the ball that collided with smth
   HitObject* m_obj;     // what the ball collided with

   float m_hittime;      // when the collision happens (relative to current physics state)
   float m_hitdistance;  // hit distance 

   // additional collision information
   Vertex3Ds m_hitnormal;
   Vertex2D  m_hitvel; // only "correctly" used by plunger and flipper
   float m_hit_org_normalvelocity; // only set if isContact is true

   //float m_hitangularrate; //!! angular rate is only assigned but never used

   //float m_hitmoment; //!! currently only one bit is used (hitmoment == 0 or not)
   bool m_hitmoment_bit;

   bool m_hitflag; // UnHit signal/direction of hit/side of hit (spinner/gate)

   //bool m_hitRigid; // rigid body collision? //!! this is almost never ever triggered (as 99.999999% true when actually handled), and if then only once while rolling over a trigger, etc, with a very minimalistic special handling afterwards (if false), so for now removed

   bool m_isContact; // set to true if impact velocity is ~0
};


class HitObject
{
public:
   HitObject(IEditable* const editable) : m_editable(editable) {}
   virtual ~HitObject() {}

   virtual float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const { return -1.f; } //!! shouldn't need to do this, but for whatever reason there is a pure virtual function call triggered otherwise that refuses to be debugged (all derived classes DO implement this one!)
   virtual int GetType() const = 0;
   virtual void Collide(const CollisionEvent& coll) = 0;
   virtual void Contact(CollisionEvent& coll, const float dtime); // apply contact forces for the given time interval. Ball, Spinner and Gate do nothing here, Flipper has a specialized handling
   virtual void CalcHitBBox() = 0;

   virtual MoverObject *GetMoverObject() { return nullptr; }

   void SetFriction(const float friction)  { m_friction = friction; }
   void FireHitEvent(HitBall* const pball);

   virtual void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const = 0;

   // Editable that created this hitobject, used for by UI for selecting editables.
   // An hitobject is only valid if this part is not null (so a new HitObject is not valid at creation until this is set...)
   IEditable* m_editable = nullptr;

   // Collision events
   bool  m_fe = false;  // FireEvents for m_obj?
   float m_threshold = 0.f;  // threshold for firing an event (usually (always??) normal dot ball-velocity)
   // FIXME is there a situation where m_obj would be different m_editable ? if not, why do we keep both ?
   IFireEvents *m_obj = nullptr; // base object pointer (mainly used as IFireEvents, but also as HitTarget or Primitive or Trigger or Kicker or Gate, see below)

   FRect3D m_hitBBox;  // updated by CalcHitBBox, but for balls only on-demand when creating the collision hierarchies

   float m_elasticity = 0.3f;
   float m_elasticityFalloff = 0.f;
   float m_friction = 0.3f;
   float m_scatter = 0.f; // in radians

   // FIXME m_ObjType is used to check type conversion while it is abused and does not always correspond to m_editable type => remove it, and add parameters corresponding to the real use case (which in fact are the 'abuse ones')
   eObjType m_ObjType = eNull;

   bool  m_enabled = true;

protected:
   HitObject() { } // Hacky constructor used by HitBall which is still mixing the defining object (Ball) and the physics object (HitBall). This will be fixed but is a remaining of the previous state where Ball did not exist as a part of the data model but only as physics object
};

//

// 2D 'Wall' (a simple 2D line with additional height limitation)
class LineSeg : public HitObject
{
public:
   LineSeg(IEditable*const editable) : HitObject(editable) { }
   LineSeg(IEditable*const editable, const Vertex2D& p1, const Vertex2D& p2, const float zlow, const float zhigh)
       : HitObject(editable), v1(p1), v2(p2)
   {
      m_hitBBox.zlow = zlow; //!! abuses the hit bbox to store zlow and zhigh
      m_hitBBox.zhigh = zhigh;
      CalcNormalAndLength();
   }
   
   void Set(const Vertex2D& p1, const Vertex2D& p2)
   {
      v1 = p1;
      v2 = p2;
      CalcNormalAndLength();
   }

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eLineSeg; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   float HitTestBasic(const BallS& ball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid) const;
   void CalcNormalAndLength();

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

   Vertex2D v1, v2;
   Vertex2D normal;
   float length;
};

class HitCircle : public HitObject
{
public:
   HitCircle(IEditable*const editable, const Vertex2D& c, const float r, const float zlow, const float zhigh) : HitObject(editable), center(c), radius(r)
   {
      m_hitBBox.zlow = zlow;
      m_hitBBox.zhigh = zhigh;
   }

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eCircle; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   float HitTestBasicRadius(const BallS& ball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid) const;

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

   const Vertex2D center;
   const float radius;
};


// collision object which is a line segment parallel to the z axis
class HitLineZ : public HitObject
{
public:
   HitLineZ(IEditable*const editable) : HitObject(editable) {}
   HitLineZ(IEditable*const editable, const Vertex2D& xy, const float zlow, const float zhigh) : HitObject(editable), m_xy(xy), m_zlow(zlow), m_zhigh(zhigh) {}

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eJoint; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

   Vertex2D m_xy;
   float m_zlow;
   float m_zhigh;
};


class HitPoint : public HitObject
{
public:
   HitPoint(IEditable*const editable, const Vertex3Ds& p) : HitObject(editable), m_p(p) {}
   HitPoint(IEditable*const editable, const float x, const float y, const float z) : HitObject(editable), m_p(Vertex3Ds(x,y,z)) {}

   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return ePoint; }
   void Collide(const CollisionEvent& coll) override;
   void CalcHitBBox() override;

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList, bool fill) const override;

   const Vertex3Ds m_p;
};

struct HitTestResult
{
   HitObject* m_obj;
   float m_time;
};

// Callback for the broadphase collision test.
// Perform the actual hittest between ball and hit object and update
// collision information if a hit occurred.
void DoHitTest(const HitBall* const pball, const HitObject* const pho, CollisionEvent& coll);
