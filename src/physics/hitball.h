// license:GPLv3+

#pragma once

#include "physics/collide.h"


class BallMoverObject : public MoverObject
{
public:
   bool AddToList() const override { return false; } // We add ourselves to the mover list.
                                                     // If we allow the table to do that, we might get added twice, if we get created in the player Init code
   void UpdateDisplacements(const float dtime) override;
   void UpdateVelocities() override;

   HitBall* m_pHitBall;
};

struct BallS
{
   Vertex3Ds m_pos = Vertex3Ds(0.f, 0.f, 0.f);
   Vertex3Ds m_vel = Vertex3Ds(0.f, 0.f, 0.f); // ball velocity
   float m_radius = 25.f;
   float m_mass   = 1.f;
   vector<IFireEvents*>* m_vpVolObjs; // vector of triggers and kickers we are now inside (stored as IFireEvents* though, as HitObject.m_obj stores it like that!)
   bool m_lockedInKicker = false;
};

class HitBall : public HitObject
{
public:
   HitBall();
   ~HitBall();

   void UpdateDisplacements(const float dtime);
   void UpdateVelocities();

   // From HitObject
   float HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const override;
   int GetType() const override { return eBall; }
   void Collide(const CollisionEvent& coll) override;
   void Contact(CollisionEvent& coll, const float dtime) override { }
   void CalcHitBBox() override;

   float HitRadiusSqr() const { return sqrf((m_hitBBox.right - m_hitBBox.left)*0.5f); } // this returns the extended (by m_vel + magic) squared radius, as needed to be used in the collision detection
   void Collide3DWall(const Vertex3Ds& hitNormal, float elasticity, const float elastFalloff, const float friction, float scatter_angle);

   void ApplyFriction(const Vertex3Ds& hitnormal, const float dtime, const float fricCoeff);
   void HandleStaticContact(const CollisionEvent& coll, const float friction, const float dtime);

   Vertex3Ds SurfaceVelocity(const Vertex3Ds& surfP) const;
   Vertex3Ds SurfaceAcceleration(const Vertex3Ds& surfP) const;
   float Inertia() const { return (float)(2.0/5.0) * m_d.m_radius*m_d.m_radius * m_d.m_mass; }

   void ApplySurfaceImpulse(const Vertex3Ds& rotI, const Vertex3Ds& impulse);

   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList* drawList) const override { } // FIXME implement

   // Per frame info
   Ball *m_pBall = nullptr;  // Object model version of the ball

   CollisionEvent m_coll;    // collision information, may not be a actual hit if something else happens first

   BallMoverObject m_mover;

   BallS m_d;

   Vertex3Ds m_oldVel;       // hack for kicker hole handling only

   Vertex3Ds m_lastRenderedPos; // position where last render occured

   Vertex3Ds m_lastEventPos; // last hit event position (to filter hit 'equal' hit events)
   float m_lastEventSqrDist = 0.f; // distance travelled since last event

   Vertex3Ds m_angularmomentum;

   Matrix3 m_orientation;

   Vertex3Ds m_oldpos[MAX_BALL_TRAIL_POS]; // used for killing spin and for ball trails
   unsigned int m_ringcounter_oldpos = 0;

#ifdef C_DYNAMIC
   int m_dynamic = C_DYNAMIC; // used to determine static ball conditions and velocity quenching
   float m_drsq = 0.0f;       // square of distance moved
#endif
};
