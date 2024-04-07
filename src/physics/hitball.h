#pragma once

#include "physics/collide.h"


class BallMoverObject : public MoverObject
{
public:
   bool AddToList() const override { return false; } // We add ourselves to the mover list.
                                                     // If we allow the table to do that, we might get added twice, if we get created in the player Init code
   void UpdateDisplacements(const float dtime) override;
   void UpdateVelocities() override;

   Ball *m_pball;
};

struct BallS
{
   vector<IFireEvents*>* m_vpVolObjs; // vector of triggers and kickers we are now inside (stored as IFireEvents* though, as HitObject.m_obj stores it like that!)
   Vertex3Ds m_pos;
   Vertex3Ds m_vel; // ball velocity
   float m_radius;
   float m_mass;
   bool m_lockedInKicker;
};

class Ball : public HitObject
{
public:
   Ball();

   void Init(const float mass);

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

   // Per frame info
   CCO(BallEx) *m_pballex;   // Object model version of the ball

   CollisionEvent m_coll;    // collision information, may not be a actual hit if something else happens first

   BallMoverObject m_mover;

   BallS m_d;

   Vertex3Ds m_oldVel;       // hack for kicker hole handling only

   U32 m_lastEventTime; // last hit event time (to filter hit 'equal' hit events)
   Vertex3Ds m_lastEventPos; // last hit event position (to filter hit 'equal' hit events)

   Vertex3Ds m_angularmomentum;

#ifdef C_DYNAMIC
   int m_dynamic;            // used to determine static ball conditions and velocity quenching
   float m_drsq;             // square of distance moved
#endif

   unsigned int m_id;        // unique ID for each ball
   static unsigned int ballID; // increased for each ball created to have an unique ID for scripts for each ball

   Matrix3 m_orientation;

   Vertex3Ds m_oldpos[MAX_BALL_TRAIL_POS]; // used for killing spin and for ball trails
   unsigned int m_ringcounter_oldpos;

   // rendering only (to be moved to parts/ball):
   bool m_reflectionEnabled;
   bool m_forceReflection;
   bool m_visible;
   bool m_decalMode;
   string m_image;
   string m_imageDecal;
   float m_bulb_intensity_scale; // to dampen/increase contribution of the bulb lights (locally/by script)
   float m_playfieldReflectionStrength;
   COLORREF m_color;
   Texture* m_pinballEnv;
   bool m_pinballEnvSphericalMapping;
   Texture* m_pinballDecal;
};
