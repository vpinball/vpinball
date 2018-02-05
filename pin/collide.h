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
   eTrigger,	// this value and greater are volume set tested, add rigid or non-volume set above
   eKicker,		// this is done to limit to one test
};

extern float c_hardScatter;

// forward declarations
class Ball;
class HitObject;


class MoverObject // Spinner, Gate, Flipper, Plunger and Ball
{
public:
   virtual bool AddToList() const = 0;
   virtual void UpdateDisplacements(const float dtime) = 0;
   virtual void UpdateVelocities() = 0;
};

class AnimObject // DispReel, LightSeq and Slingshot
{
public:
   virtual void Animate() = 0;
};


/*
 * Rubber has a coefficient of restitution which decreases with the impact velocity.
 * We use a heuristic model which decreases the COR according to a falloff parameter:
 * 0 = no falloff, 1 = half the COR at 1 m/s (18.53 speed units)
 */
inline float ElasticityWithFalloff(const float elasticity, const float falloff, const float vel)
{
   if (falloff > 0.f)
      return elasticity / (1.0f + falloff * fabsf(vel) * (float)(1.0 / 18.53));
   else
      return elasticity;
}


struct CollisionEvent
{
   CollisionEvent() : m_ball(0), m_obj(0), m_isContact(false), m_hittime(0.0f), m_hitdistance(0.0f), /*m_hitmoment(0.0f)*/ m_hitmoment_bit(true), m_hitflag(false) /*, m_hitRigid(false)*/ {}

   Ball* m_ball;         // the ball that collided with smth
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
   HitObject() : m_fEnabled(true), m_ObjType(eNull), m_obj(NULL),
      m_elasticity(0.3f), m_elasticityFalloff(0.0f), m_friction(0.3f), m_scatter(0.0f),
      m_threshold(0.f), m_pfedebug(NULL), m_fe(false), m_e(false) {}
   virtual ~HitObject() {}

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const { return -1.f; } //!! shouldn't need to do this, but for whatever reason there is a pure virtual function call triggered otherwise that refuses to be debugged (all derived classes DO implement this one!)
   virtual int GetType() const = 0;
   virtual void Collide(CollisionEvent& coll) = 0;
   virtual void Contact(CollisionEvent& coll, const float dtime); // apply contact forces for the given time interval. Ball, Spinner and Gate do nothing here, Flipper has a specialized handling
   virtual void CalcHitBBox() = 0;

   virtual MoverObject *GetMoverObject() { return NULL; }

   void SetFriction(const float friction)  { m_friction = friction; }
   void FireHitEvent(Ball * const pball);

   IFireEvents *m_pfedebug;

   IFireEvents *m_obj; // base object pointer (mainly used as IFireEvents, but also as HitTarget or Primitive or Trigger or Kicker or Gate, see below)

   float m_threshold;  // threshold for firing an event (usually (always??) normal dot ball-velocity)

   FRect3D m_hitBBox;

   float m_elasticity;
   float m_elasticityFalloff;
   float m_friction;
   float m_scatter;

   eObjType m_ObjType;

   bool  m_fEnabled;

   bool  m_fe;  // FireEvents for m_obj?
   bool  m_e;   // currently only used to determine which HitTriangles/HitLines/HitPoints are being part of the same Primitive element m_obj, to be able to early out intersection traversal if primitive is flagged as not collidable
};

//

// 2D 'Wall' (a simple 2D line with additional height limitation)
class LineSeg : public HitObject
{
public:
   LineSeg() { }
   LineSeg(const Vertex2D& p1, const Vertex2D& p2, const float zlow, const float zhigh)
       : v1(p1), v2(p2)
   {
      m_hitBBox.zlow = zlow;
      m_hitBBox.zhigh = zhigh;
      CalcNormal();
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const;
   virtual int GetType() const { return eLineSeg; }
   virtual void Collide(CollisionEvent& coll);
   virtual void CalcHitBBox();

   float HitTestBasic(const Ball * const pball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid) const;
   void CalcNormal(); // and also does update length!

   Vertex2D normal;
   Vertex2D v1, v2;
   float length;
};

class HitCircle : public HitObject
{
public:
   HitCircle() { }
   HitCircle(const Vertex2D& c, const float r, const float zlow, const float zhigh) : center(c), radius(r)
   {
      m_hitBBox.zlow = zlow;
      m_hitBBox.zhigh = zhigh;
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const;
   virtual int GetType() const { return eCircle; }
   virtual void Collide(CollisionEvent& coll);
   virtual void CalcHitBBox();

   float HitTestBasicRadius(const Ball * const pball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid) const;

   Vertex2D center;
   float radius;
};


// collision object which is a line segment parallel to the z axis
class HitLineZ : public HitObject
{
public:
   HitLineZ() {}
   HitLineZ(const Vertex2D& xy, const float zlow, const float zhigh) : m_xy(xy)
   {
      m_hitBBox.zlow = zlow;
      m_hitBBox.zhigh = zhigh;
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const;
   virtual int GetType() const { return eJoint; }
   virtual void Collide(CollisionEvent& coll);
   virtual void CalcHitBBox();

   Vertex2D m_xy;
};


class HitPoint : public HitObject
{
public:
   HitPoint(const Vertex3Ds& p) : m_p(p) {}
   HitPoint(const float x, const float y, const float z) : m_p(Vertex3Ds(x,y,z)) {}

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const;
   virtual int GetType() const { return ePoint; }
   virtual void Collide(CollisionEvent& coll);
   virtual void CalcHitBBox();

   Vertex3Ds m_p;
};


// Callback for the broadphase collision test.
// Perform the actual hittest between ball and hit object and update
// collision information if a hit occurred.
void DoHitTest(Ball * const pball, HitObject * const pho, CollisionEvent& coll);
