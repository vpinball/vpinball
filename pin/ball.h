#pragma once

#include "pin/collide.h"


class BallMoverObject : public MoverObject
{
public:
   virtual bool AddToList() const { return false; } // We add ourselves to the mover list.  
                                                    // If we allow the table to do that, we might get added twice, if we get created in the player Init code
   virtual void UpdateDisplacements(const float dtime);
   virtual void UpdateVelocities();

   Ball *m_pball;
};


class Ball : public HitObject
{
public:
   Ball();

   void Init(const float mass);
   void RenderSetup();

   void UpdateDisplacements(const float dtime);
   void UpdateVelocities();

   // From HitObject
   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const;
   virtual int GetType() const { return eBall; }
   virtual void Collide(const CollisionEvent& coll);
   virtual void Contact(CollisionEvent& coll, const float dtime) { }
   virtual void CalcHitBBox();

   float HitRadiusSqr() const { return sqrf((m_hitBBox.right - m_hitBBox.left)*0.5f); } // this returns the extended (by m_vel + magic) squared radius, as needed to be used in the collision detection
   void Collide3DWall(const Vertex3Ds& hitNormal, float elasticity, const float elastFalloff, const float friction, float scatter_angle);

   void ApplyFriction(const Vertex3Ds& hitnormal, const float dtime, const float fricCoeff);
   void HandleStaticContact(const CollisionEvent& coll, const float friction, const float dtime);

   Vertex3Ds SurfaceVelocity(const Vertex3Ds& surfP) const;
   Vertex3Ds SurfaceAcceleration(const Vertex3Ds& surfP) const;
   float Inertia() const { return (float)(2.0/5.0) * m_radius*m_radius * m_mass; }

   void ApplySurfaceImpulse(const Vertex3Ds& rotI, const Vertex3Ds& impulse);

   void EnsureOMObject();

   // Per frame info
   CCO(BallEx) *m_pballex; // Object model version of the ball

   vector<IFireEvents*>* m_vpVolObjs; // vector of triggers and kickers we are now inside (stored as IFireEvents* though, as HitObject.m_obj stores it like that!)

   CollisionEvent m_coll;  // collision information, may not be a actual hit if something else happens first

#ifdef C_DYNAMIC
   int m_dynamic;          // used to determine static ball conditions and velocity quenching
   float m_drsq;           // square of distance moved
#endif

   BallMoverObject m_ballMover;

   Vertex3Ds m_pos;
   Vertex3Ds m_vel;        // ball velocity
   Vertex3Ds m_oldVel;     // hack for kicker hole handling only

   float m_radius;
   float m_mass;

   Vertex3Ds m_Event_Pos;  // last hit event position (to filter hit 'equal' hit events)

   Vertex3Ds m_angularmomentum;

   unsigned int m_id; // unique ID for each ball

   bool m_frozen;

   // rendering only:
   bool m_reflectionEnabled;
   bool m_forceReflection;
   bool m_visible;
   bool m_decalMode;

   Matrix3 m_orientation;

   char m_szImage[MAXTOKEN];
   char m_szImageDecal[MAXTOKEN];

   float m_bulb_intensity_scale; // to dampen/increase contribution of the bulb lights (locally/by script)

   float m_playfieldReflectionStrength;

   COLORREF m_color;
   Texture *m_pinballEnv;
   Texture *m_pinballDecal;

   Vertex3Ds m_oldpos[MAX_BALL_TRAIL_POS]; // for the optional ball trails
   unsigned int m_ringcounter_oldpos;

   //
   static unsigned int ballID; // increased for each ball created to have an unique ID for scripts for each ball
};
