#pragma once

enum
{
   eNull,
   ePoint,
   eLineSeg,
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
class AnimObject;


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
   CollisionEvent() : ball(0), obj(0), isContact(false), hittime(0.0f), hitdistance(0.0f), /*hitmoment(0.0f)*/ hitmoment_bit(true), hitflag(false) /*, hitRigid(false)*/ {}

   Ball* ball;         // the ball that collided with smth
   HitObject* obj;     // what the ball collided with

   float hittime;      // when the collision happens (relative to current physics state)
   float hitdistance;  // hit distance 

   // additional collision information
   Vertex3Ds hitnormal;
   Vertex2D  hitvel; // only "correctly" used by plunger and flipper
   float hit_org_normalvelocity; // only set if isContact is true

   //float hitangularrate; //!! angular rate is only assigned but never used

   //float hitmoment; //!! currently only one bit is used (hitmoment == 0 or not)
   bool hitmoment_bit;

   bool hitflag; // UnHit signal/direction of hit/side of hit (spinner/gate)

   //bool hitRigid; // rigid body collision? //!! this is almost never ever triggered (as 99.999999% true when actually handled), and if then only once while rolling over a trigger, etc, with a very minimalistic special handling afterwards (if false), so for now removed

   bool isContact; // set to true if impact velocity is ~0
};


HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections);

class HitObject
{
public:

   HitObject() : m_fEnabled(fTrue), m_ObjType(eNull), m_pObj(NULL),
      m_elasticity(0.3f), m_elasticityFalloff(0.0f), m_friction(0.3f), m_scatter(0.0f),
      m_pfe(NULL), m_threshold(0.f), m_pfedebug(NULL), m_pe(NULL), m_objHitEvent(NULL) {}
   virtual ~HitObject() {}

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) { return -1.f; } //!! shouldn't need to do this, but for whatever reason there is a pure virtual function call triggered otherwise that refuses to be debugged
   virtual int GetType() const = 0;
   virtual void Collide(CollisionEvent& coll) = 0;
   virtual void Contact(CollisionEvent& coll, const float dtime)  { }  // apply contact forces for the given time interval
   virtual void CalcHitRect() = 0;
   virtual AnimObject *GetAnimObject() { return NULL; }

   void SetFriction(const float friction)  { m_friction = friction; }
   void FireHitEvent(Ball * const pball);

   IFireEvents *m_pfe;
   float m_threshold;

   //IDispatch *m_pdisp;
   IFireEvents *m_pfedebug;

   void *m_pe; // currently only used to determine which HitTriangles/HitLines/HitPoints are being part of the same Primitive element

   FRect3D m_rcHitRect;

   int   m_ObjType;
   void* m_pObj;
   void* m_objHitEvent;
   float m_elasticity;
   float m_elasticityFalloff;
   float m_friction;
   float m_scatter;
   bool  m_fEnabled;
};

class AnimObject
{
public:
   virtual bool FMover() const { return false; }
   virtual void UpdateDisplacements(const float dtime) {}
   virtual void UpdateVelocities() {}

   virtual void Animate() {}
};

// 2D 'Wall' (a simple 2D line with additional height limitation)
class LineSeg : public HitObject
{
public:
   LineSeg() { }
   LineSeg(const Vertex2D& p1, const Vertex2D& p2, const float zlow, const float zhigh)
       : v1(p1), v2(p2)
   {
      m_rcHitRect.zlow = zlow;
      m_rcHitRect.zhigh = zhigh;
      CalcNormal();
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return eLineSeg; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitRect();

   float HitTestBasic(const Ball * const pball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid);
   void CalcNormal();
   void CalcLength();

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
      m_rcHitRect.zlow = zlow;
      m_rcHitRect.zhigh = zhigh;
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return eCircle; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitRect();

   float HitTestBasicRadius(const Ball * const pball, const float dtime, CollisionEvent& coll,
                            const bool direction, const bool lateral, const bool rigid);

   Vertex2D center;
   float radius;
};


// collision object which is a line segment parallel to the z axis
class HitLineZ : public HitObject
{
public:
   HitLineZ() { }
   HitLineZ(const Vertex2D& xy, const float zlow, const float zhigh) : m_xy(xy)
   {
      m_rcHitRect.zlow = zlow;
      m_rcHitRect.zhigh = zhigh;
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return eJoint; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitRect();

   Vertex2D m_xy;
};


class HitPoint : public HitObject
{
public:
   HitPoint(const Vertex3Ds& p) : m_p(p) {}
   HitPoint(const float x, const float y, const float z) : m_p(Vertex3Ds(x,y,z)) {}

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return ePoint; }
   virtual void Collide(CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime);
   virtual void CalcHitRect();

   Vertex3Ds m_p;
};


// Callback for the broadphase collision test.
// Perform the actual hittest between ball and hit object and update
// collision information if a hit occurred.
void DoHitTest(Ball * const pball, HitObject * const pho, CollisionEvent& coll);
