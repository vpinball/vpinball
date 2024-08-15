// license:GPLv3+

#pragma once

#include "physics/kdtree.h"
#include "physics/quadtree.h"
#include "physics/NudgeFilter.h"

class PhysicsEngine final
{
public:
   PhysicsEngine(PinTable *const table);
   ~PhysicsEngine();

   void SetGravity(float slopeDeg, float strength);
   const Vertex3Ds& GetGravity() const { return m_gravity; }

   // For the time being, beside the colliders loaded from the table in the constructor, only ball can added/removed
   void AddBall(HitBall *const ball);
   void RemoveBall(HitBall *const ball);
   void AddCollider(HitObject * collider, IEditable * editable, const bool isUI);

   void OnPrepareFrame();
   void OnFinishFrame();

   void StartPhysics();
   void UpdatePhysics();

   bool IsBallCollisionHandlingSwapped() const { return m_swap_ball_collision_handling; }
   bool RecordContact(const CollisionEvent& newColl);

   void Nudge(float angle, float force);
   Vertex3Ds GetNudge() const; // More or less nudge velocity (table velocity and "nudge force")
   Vertex2D GetScreenNudge() const; // Table displacement
   const Vertex2D& GetPlumbPos() const { return m_plumb; } // Plumb position on the tilt circle plane
   void ReadNudgeSettings(Settings& settings);

   void RayCast(const Vertex3Ds &source, const Vertex3Ds &target, const bool uiCast, vector<HitTestResult> &vhoHit);

   void ResetStats() { m_phys_max = 0; m_phys_max_iterations = 0; m_count = 0; m_phys_total_iterations = 0; }
   int GetPerfNIterations() const { return m_phys_iterations; }
   int GetPerfLengthMax() const { return m_phys_max; }
   string GetPerfInfo(bool resetMax);

   const vector<HitObject *>& GetHitObjects() const { return m_vho; }
   const vector<HitObject *> &GetUIObjects() const { return m_vUIHitObjects; }

private:
   void AddCabinetBoundingHitShapes(PinTable *const table);
   void PhysicsSimulateCycle(float dtime); // Perform continuous collision detection for the given amount of delta time

   Vertex3Ds m_gravity;
   
   unsigned int m_physicsMaxLoops;

   bool m_swap_ball_collision_handling = false; // Swaps the order of ball-ball collision handling around each physics cycle (in regard to the RLC comment block in quadtree.cpp (hopefully ;)))

   bool m_recordContacts = false; // flag for DoHitTest()
   vector<CollisionEvent> m_contacts;

   U64 m_StartTime_usec; // Time when the simulation started (creation of this object)
   U64 m_curPhysicsFrameTime; // Time where the last machine simulation (physics, timers, scripts,...) stopped
   U64 m_nextPhysicsFrameTime; // Time at which the next physics update should be
   U64 m_lastFlipTime = 0;

   vector<HitFlipper *> m_vFlippers;
   HitPlane m_hitPlayfield; // HitPlanes cannot be part of octree (infinite size)
   HitPlane m_hitTopGlass;

   vector<MoverObject *> m_vmover; // moving objects for physics simulation

   vector<HitObject *> m_vho;
   /*HitKD*/ HitQuadtree m_hitoctree;
   vector<HitObject *> m_vho_dynamic;
#ifdef USE_EMBREE
   HitQuadtree m_hitoctree_dynamic; // should be generated from scratch each time something changes
#else
   HitKD m_hitoctree_dynamic; // should be generated from scratch each time something changes
#endif

   vector<HitObject *> m_vUIHitObjects;
   HitQuadtree m_UIOctree;

   void UpdateNudge(float dtime);

   Vertex2D m_nudge; // filtered nudge acceleration acquired from hardware or resulting of keyboard nudge
   bool m_enableNudgeFilter = false; // Located in physic engine instead of input since it is applied at physics cycle rate, on hardware input but also on keyboard nudge
   NudgeFilter m_nudgeFilterX;
   NudgeFilter m_nudgeFilterY;
   bool m_enablePlumbTilt = false;
   bool m_plumbTiltHigh = false;
   Vertex2D m_plumb;
   Vertex2D m_plumbVel;
   float m_plumbTiltThreshold;

   // Table modeled as a spring
   Vertex3Ds m_tableVel;
   Vertex3Ds m_tableDisplacement;
   Vertex3Ds m_tableVelOld;
   Vertex3Ds m_tableVelDelta;
   float m_nudgeSpring;
   float m_nudgeDamping;

   // legacy/VP9 style keyboard nudging
   bool m_legacyNudge = false;
   float m_legacyNudgeStrength = 0.f;
   Vertex2D m_legacyNudgeBack;
   int m_legacyNudgeTime = 0;

   // Physics stats
   U32 m_phys_iterations;
   U32 m_phys_max_iterations;
   U32 m_phys_max;
   U64 m_phys_total_iterations;
   U64 m_count; // Number of frames included in the total variant of the counters

#ifdef DEBUGPHYSICS
public:
   U32 c_hitcnts;
   U32 c_collisioncnt;
   U32 c_contactcnt;
   #ifdef C_DYNAMIC
   U32 c_staticcnt;
   #endif
   U32 c_embedcnts;
   U32 c_timesearch;

   U32 c_traversed;
   U32 c_tested;
   U32 c_deepTested;
#endif
};
