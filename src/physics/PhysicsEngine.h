// license:GPLv3+

#pragma once

#include "physics/kdtree.h"
#include "physics/quadtree.h"
#include "physics/AsyncDynamicQuadTree.h"
#include "physics/collideex.h"
#include "physics/cabinet/PlumbHandler.h"

class BallControl;
class InputAction;
class PlungerHandler;

class PhysicsEngine final
{
public:
   PhysicsEngine(PinTable *const table);
   ~PhysicsEngine();

   void SetGravity(float slopeDeg, float strength);
   const Vertex3Ds& GetGravity() const { return m_gravity; } // Gravity expressed in VP units. Earth gravity 9.81 m.s^-2 is approximately 1.81751 VPU.VPT^-2 (see physconst.h)

   // Only supported for UI for the time being
   void SetDynamic(IEditable *editable) { GetUIQuadTree()->SetDynamic(editable); }
   void SetStatic(IEditable *editable) { GetUIQuadTree()->SetStatic(editable); }

   // Allow to update/add/remove parts after initial setup (live edit). Editing suspends the simulation,
   // immediately updates the colliders (they may be displayed in the editor), and defers the static
   // quadtree rebuild to the next physics update (see FlushStaticQuadTree)
   void Update(IEditable *editable);
   void Add(IEditable *editable);
   void Remove(IEditable *editable);

   // Add or remove a collider, as a consequence of PhysicSetup/Release
   // Colliders are given to the physics engine which owns them, except for balls which always owns their HitBall (therefore, being the only one using RemoveCollider)
   void AddCollider(HitObject * collider, const bool isUI);
   void RemoveCollider(HitObject * collider, const bool isUI);
   void CollectColliders(IEditable *editable, vector<HitObject *> *hitObjects, bool isUI);

   void OnFinishFrame();

   void StartPhysics();
   void UpdatePhysics(uint64_t targetTimeUs);

   bool IsBallCollisionHandlingSwapped() const { return m_swap_ball_collision_handling; }
   bool RecordContact(const CollisionEvent& newColl);

   void RayCast(const Vertex3Ds &source, const Vertex3Ds &target, const bool uiCast, vector<HitTestResult> &vhoHit);

   void ResetStats() { m_phys_max = 0; m_phys_max_iterations = 0; m_count = 0; m_phys_total_iterations = 0; }
   void ResetPerFrameStats();
   int GetPerfNIterations() const { return m_phys_iterations; }
   int GetPerfLengthMax() const { return m_phys_max; }
   string GetPerfInfo(bool resetMax);

   const vector<HitObject *>& GetHitObjects() const { return m_hitoctree.GetHitObjects(); }
   vector<HitObject *> GetUIHitObjects(IEditable *editable);

   uint64_t GetStartTime() const { return m_startTime_usec; }
   uint64_t GetCurrentTime() const { return m_curPhysicsFrameTime; }

   // Table being simulated
   PinTable *GetTable() const { return m_table; }

   // Simulated time, updated by UpdatePhysics
   uint32_t GetTimeMsec() const { return m_time_msec; }
   double GetTimeSec() const { return m_time_sec; }

   // Balls registered in the simulation
   const vector<HitBall *> &GetBalls() const { return m_vballs; }

   // True when the player added an implicit (non collidable) playfield mesh, meaning the ground HitPlane must be simulated
   void SetImplicitPlayfieldMesh(const bool hasImplicitMesh) { m_implicitPlayfieldMesh = hasImplicitMesh; }

   // Last simulated time at which a ball hit the plunger vicinity (feedback for UI and toys)
   uint32_t GetLastPlungerHit() const { return m_lastPlungerHit; }
   void NotifyPlungerBallContact() { m_lastPlungerHit = m_time_msec; }

   // Access to the player services used by the physics objects. These are no-ops / null
   // when there is no active player (e.g. headless simulation).
   BallControl *GetBallControl() const;
   PlungerHandler *GetPlungerHandler() const;
   InputAction *GetLaunchBallAction() const;
   Vertex2D GetCabinetAcceleration() const;
   void PlayBallBallRumble(float impactSpeed) const;
   void PlaySlingshotRumble() const;
   void PlayFlipperContactRumble(float impactSpeed) const;
   void PlayPlungerRumble(float fireSpeed) const;
   void PlayPlungerLaunchRumble(float impact) const;

   VPX::Physics::PlumbHandler m_plumbHandler;

private:
   void AddCabinetBoundingHitShapes(PinTable *const table);
   void PhysicsSimulateCycle(float dtime); // Perform continuous collision detection for the given amount of delta time

   void ReleaseVHO(const vector<HitObject *> &vho, bool isUI);

   void AddStaticColliders(IEditable *editable); // Create the gameplay colliders of an editable and add them to the static quadtree's hit object list
   void ReleaseStaticColliders(IEditable *editable); // Release the gameplay colliders of an editable and remove them from the static quadtree's hit object list
   void RegisterHitObject(HitObject *hitObject); // Register a collider with the simulation (flippers, plungers, movers)
   void UnregisterHitObject(HitObject *hitObject); // Unregister a collider from the simulation (flippers, plungers, movers)
   void FlushStaticQuadTree(); // Rebuild the static quadtree structure after its hit object list was modified (colliders are kept up to date)

   PinTable *const m_table;

   Vertex3Ds m_gravity;

   vector<HitBall *> m_vballs; // Balls registered in the simulation (add/remove through AddCollider/RemoveCollider)

   bool m_implicitPlayfieldMesh = false; // An implicit playfield mesh was created, the ground HitPlane must be hit tested

   uint32_t m_time_msec = 0; // Simulated time in milliseconds, published to the player when present
   double m_time_sec = 0.0; // Simulated time in seconds, published to the player when present

   uint32_t m_lastPlungerHit = 0;

   unsigned int m_physicsMaxLoops;

   bool m_swap_ball_collision_handling = false; // Swaps the order of ball-ball collision handling around each physics cycle (in regard to the RLC comment block in quadtree.cpp (hopefully ;)))

   bool m_recordContacts = false; // flag for DoHitTest()
   vector<CollisionEvent> m_contacts;

   uint64_t m_startTime_usec; // Time when the simulation started (creation of this object)
   uint64_t m_curPhysicsFrameTime; // Time where the last machine simulation (physics, timers, scripts,...) stopped
   uint64_t m_nextPhysicsFrameTime; // Time at which the next physics update should be
   uint64_t m_lastFlipTime = 0;

   unsigned int m_onUpdatePhysicsMsgId;

   vector<class HitFlipper *> m_vFlippers;
   vector<class HitPlunger *> m_vPlungers;

public:
   void OnBallWallHit(const class HitBall& ball, const Vertex3Ds& hitNormal, const float impactSpeed); // a ball hitting static geometry, offered to the plungers (shooter lane end)

private:
   HitPlane m_hitPlayfield; // HitPlanes cannot be part of octree (infinite size)
   HitPlane m_hitTopGlass;

   vector<MoverObject *> m_vmover; // moving objects for physics simulation

   vector<HitObject *>* m_pendingHitObjects = nullptr; // Hit objects pending insertion in quadtree, only defined while collecting through AddCollider callback method

   /*HitKD*/ HitQuadtree m_hitoctree;
   bool m_staticQuadTreeDirty = false; // Hit object list of m_hitoctree was modified without rebuilding its structure (rebuilt lazily, see FlushStaticQuadTree)
#ifdef USE_EMBREE
   HitQuadtree m_hitoctree_dynamic; // should be generated from scratch each time something changes
#else
   HitKD m_hitoctree_dynamic; // should be generated from scratch each time something changes
#endif

   AsyncDynamicQuadTree *GetUIQuadTree(); // Trigger UI quadtree creation/update
   AsyncDynamicQuadTree* m_UIQuadTtree = nullptr;

   // Physics stats
   uint32_t m_phys_iterations;
   uint32_t m_phys_max_iterations;
   uint32_t m_phys_max;
   uint64_t m_phys_total_iterations;
   uint64_t m_count; // Number of frames included in the total variant of the counters

#ifdef DEBUGPHYSICS
public:
   uint32_t c_hitcnts;
   uint32_t c_collisioncnt;
   uint32_t c_contactcnt;
   #ifdef C_DYNAMIC
   uint32_t c_staticcnt;
   #endif
   uint32_t c_embedcnts;
   uint32_t c_timesearch;

   uint32_t c_traversed;
   uint32_t c_tested;
   uint32_t c_deepTested;
#endif
};
