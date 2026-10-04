// license:GPLv3+

#pragma once

#include "math/vector.h"
#include "physics/physconst.h"

// For defects FIX_PHYSICS fixes: TEST_CASE("..." * doctest::should_fail(!kFixPhysics))
#ifdef FIX_PHYSICS
inline constexpr bool kFixPhysics = true;
#else
inline constexpr bool kFixPhysics = false;
#endif

class PinTable;
class PhysicsEngine;
class HitBall;
class HitObject;
class LineSeg;
class HitCircle;
class Ball;
class Surface;

// Headless test harness around PhysicsEngine.
//
// The harness owns an empty PinTable on which the test builds a situation:
// - walls as Surface parts (their PhysicSetup generates LineSeg / HitLineZ /
//   HitPoint / Hit3DPoly colliders, like a real table),
// - raw HitObjects (LineSeg, HitCircle, ...) injected through a minimal
//   IEditable/IHitable shim so they go through the regular engine add path,
// - balls as Ball parts owning their HitBall, like Player::CreateBall.
//
// Start() creates the PhysicsEngine (which collects all colliders in its
// constructor) and anchors the simulated clock. Step()/AdvanceMs() then drive
// UpdatePhysics() with virtual target times, so the simulation is stepped
// deterministically, independently of the real clock.
class PhysicsTestHarness final
{
public:
   // Creates a table of the given size in VP units (defaults to a typical playfield size).
   explicit PhysicsTestHarness(float tableWidth = 1000.f, float tableHeight = 2000.f);
   ~PhysicsTestHarness();

   PinTable *GetTable() const { return m_table; }
   PhysicsEngine *GetEngine() const { return m_physics.get(); } // valid after Start()

   // --- Scenario definition ---

   // Adds a wall as a closed polygon (a Surface part). The outline points are the wall
   // footprint, the wall extends from heightBottom to heightTop. Physics properties are
   // applied directly (overwritePhysics), so the test does not depend on materials.
   // Returns the part for further tweaking. Only supported before Start().
   Surface *AddWall(const vector<Vertex2D> &outline, float heightBottom = 0.f, float heightTop = 50.f, float elasticity = 0.3f, float friction = 0.3f);

   // Adds a straight wall between two points (a thin polygon). Only supported before Start().
   Surface *AddWallSegment(
      float x1, float y1, float x2, float y2, float heightBottom = 0.f, float heightTop = 50.f, float halfThickness = 1.f, float elasticity = 0.3f, float friction = 0.3f);

   // Registers a raw collider created by the test (LineSeg, HitCircle, HitTriangle, ...).
   // The engine takes ownership of it. The object's m_editable is overwritten to point to
   // an internal shim editable. Can be called before or after Start() (the latter goes
   // through the live-edit PhysicsEngine::Add path).
   void AddHitObject(std::unique_ptr<HitObject> hitObject);

   // Adds a ball to the simulation, like Player::CreateBall. The returned Ball part is
   // owned by the table and owns the simulated HitBall (access it via ball->m_hitBall).
   // z is the height of the bottom of the ball, so the ball center rests at z + radius.
   // Can be called before or after Start() (balls live in the dynamic quadtree).
   Ball *AddBall(float x, float y, float z, float vx = 0.f, float vy = 0.f, float vz = 0.f, float radius = DEFAULT_BALL_SIZE, float mass = 1.f);
   void RemoveBall(Ball *ball);

   // Gravity in VP units (see PhysicsEngine::SetGravity). Defaults to the table's configured
   // slope and gravity strength, like the player does. Call before or after Start().
   void SetGravity(float slopeDeg, float strength);

   // Enables the ground HitPlane hit test, like the implicit playfield mesh does in a real
   // game (enabled by default). Call before Start().
   void SetPlayfieldFloorEnabled(bool enabled) { m_playfieldFloor = enabled; }

   // --- Simulation ---

   // Builds the physics engine over the current table content and starts the simulated clock.
   void Start();

   // Advances the simulation by a single physics step (PHYSICS_STEPTIME = 1ms).
   void Step() { AdvanceMs(1); }

   // Advances the simulation by the given number of physics steps (1ms each).
   void AdvanceMs(uint32_t ms);

   // Advances one step at a time until the predicate returns true or maxMs steps were run.
   // Returns true if the predicate was satisfied.
   bool AdvanceUntil(const std::function<bool()> &predicate, uint32_t maxMs);

   // Number of physics steps executed so far (also the simulated time in milliseconds).
   uint32_t GetTimeMs() const;
   const vector<HitBall *> &GetBalls() const;

private:
   // Minimal IEditable/IHitable owning the raw hit objects registered through AddHitObject,
   // so they are set up/released through the regular engine path like any part's colliders.
   class HitObjectSet;

   PinTable *m_table;
   std::unique_ptr<PhysicsEngine> m_physics;
   std::unique_ptr<HitObjectSet> m_hitObjectSet;
   float m_gravitySlopeDeg;
   float m_gravityStrength;
   bool m_playfieldFloor = true;
   uint64_t m_stepCount = 0;
   unsigned int m_partCounter = 0;
};
