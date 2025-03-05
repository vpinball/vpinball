// license:GPLv3+

#pragma once

#include "physics/kdtree.h"
#include "physics/quadtree.h"
#include "physics/AsyncDynamicQuadTree.h"
#include "physics/NudgeFilter.h"

class PhysicsEngine final
{
public:
   PhysicsEngine(PinTable *const table);
   ~PhysicsEngine();

   void SetGravity(float slopeDeg, float strength);
   const Vertex3Ds& GetGravity() const { return m_gravity; } // Gravity expressed in VP units. Earth gravity 9.81 m.s^-2 is approximately 1.81751 VPU.VPT^-2 (see physconst.h)

   // Only supported for UI for the time being
   void SetDynamic(IEditable *editable) { GetUIQuadTree()->SetDynamic(editable); }
   void Update(IEditable *editable) { GetUIQuadTree()->Update(editable); }
   void SetStatic(IEditable *editable) { GetUIQuadTree()->SetStatic(editable); }

   // Add or remove a collider, as a consequence of PhysicSetup/Release
   // Colliders are given to the physics engine which owns them, except for balls which always owns their HitBall (therefore, being the only one using RemoveCollider)
   void AddCollider(HitObject * collider, const bool isUI);
   void RemoveCollider(HitObject * collider, const bool isUI);
   void CollectColliders(IEditable *editable, vector<HitObject *> *hitObjects, bool isUI);

   void OnFinishFrame();

   void StartPhysics();
   void UpdatePhysics();

   bool IsBallCollisionHandlingSwapped() const { return m_swap_ball_collision_handling; }
   bool RecordContact(const CollisionEvent& newColl);

   void Nudge(float angle, float force);
   Vertex3Ds GetNudgeAcceleration() const { return m_tableAcceleration + m_nudgeAcceleration; } // Table acceleration (due to nudge) expressed in VP units
   Vertex2D GetScreenNudge() const; // Table displacement
   const Vertex3Ds& GetPlumbPos() const { return m_plumbPos; }
   float GetPlumbPoleLength() const { return m_plumbPoleLength; }
   float GetPlumbTiltThreshold() const { return m_plumbTiltThreshold; }
   void ReadNudgeSettings(const Settings &settings);

   void RayCast(const Vertex3Ds &source, const Vertex3Ds &target, const bool uiCast, vector<HitTestResult> &vhoHit);

   void ResetStats() { m_phys_max = 0; m_phys_max_iterations = 0; m_count = 0; m_phys_total_iterations = 0; }
   void ResetPerFrameStats();
   int GetPerfNIterations() const { return m_phys_iterations; }
   int GetPerfLengthMax() const { return m_phys_max; }
   string GetPerfInfo(bool resetMax);

   const vector<HitObject *>& GetHitObjects() const { return m_hitoctree.GetHitObjects(); }
   vector<HitObject *> GetUIHitObjects(IEditable *editable);

private:
   void AddCabinetBoundingHitShapes(PinTable *const table);
   void PhysicsSimulateCycle(float dtime); // Perform continuous collision detection for the given amount of delta time

   void ReleaseVHO(const vector<HitObject *> &vho, bool isUI);

   Vertex3Ds m_gravity;

   unsigned int m_physicsMaxLoops;

   bool m_swap_ball_collision_handling = false; // Swaps the order of ball-ball collision handling around each physics cycle (in regard to the RLC comment block in quadtree.cpp (hopefully ;)))

   bool m_recordContacts = false; // flag for DoHitTest()
   vector<CollisionEvent> m_contacts;

   U64 m_startTime_usec; // Time when the simulation started (creation of this object)
   U64 m_curPhysicsFrameTime; // Time where the last machine simulation (physics, timers, scripts,...) stopped
   U64 m_nextPhysicsFrameTime; // Time at which the next physics update should be
   U64 m_lastFlipTime = 0;

   vector<HitFlipper *> m_vFlippers;
   HitPlane m_hitPlayfield; // HitPlanes cannot be part of octree (infinite size)
   HitPlane m_hitTopGlass;

   vector<MoverObject *> m_vmover; // moving objects for physics simulation

   vector<HitObject *>* m_pendingHitObjects = nullptr; // Hit objects pending insertion in quadtree, only defined while collecting through AddCollider callback method

   /*HitKD*/ HitQuadtree m_hitoctree;
#ifdef USE_EMBREE
   HitQuadtree m_hitoctree_dynamic; // should be generated from scratch each time something changes
#else
   HitKD m_hitoctree_dynamic; // should be generated from scratch each time something changes
#endif

   AsyncDynamicQuadTree *GetUIQuadTree(); // Trigger UI quadtree creation/update
   AsyncDynamicQuadTree* m_UIQuadTtree = nullptr;

#pragma region Nudge & Tilt Plumb
   void UpdateNudge(float dtime);

   Vertex3Ds m_nudgeAcceleration; // filtered nudge acceleration acquired from hardware or resulting of keyboard nudge
   bool m_enableNudgeFilter = false; // Located in physic engine instead of input since it is applied at physics cycle rate, on hardware input but also on keyboard nudge
   NudgeFilter m_nudgeFilterX;
   NudgeFilter m_nudgeFilterY;
   
   // Table modeled as a spring
   Vertex3Ds m_tableVel;
   Vertex3Ds m_tableDisplacement;
   Vertex3Ds m_tableVelOld;
   Vertex3Ds m_tableAcceleration;
   float m_nudgeSpring;
   float m_nudgeDamping;

   // External accelerometer velocity input.  This is for newer
   // pin cab I/O controllers that can integrate acceleration 
   // samples on the device side to compute the instantaneous
   // cabinet velocity, and pass the velocity data to the host.
   //
   // Velocities computed on the device side are applied to the
   // physics model the same way as the velocities computed from
   // the "spring model" for scripted nudge force inputs.
   Vertex3Ds m_prevSensorTableVelocity;

   // legacy/VP9 style keyboard nudging
   bool m_legacyNudge = false;
   float m_legacyNudgeStrength = 0.f;
   Vertex2D m_legacyNudgeBack;
   int m_legacyNudgeTime = 0;

   // Tilt plumb
   bool m_enablePlumbTilt = false;
   bool m_plumbTiltHigh = false;
   float m_plumbTiltThreshold;
   float m_plumbPoleLength;
   float m_plumbMassFactor;
   Vertex3Ds m_plumbPos;
   Vertex3Ds m_plumbVel;

#pragma endregion

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
