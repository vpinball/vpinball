// license:GPLv3+
//
// Tests pinning the effect of C_BALL_SPIN_HACK, the heuristic that quenches
// residual ball spin on resting balls. The define gates two things:
//
// - in HitBall::ApplyFriction, the extra condition "(normVel <= 0.025 &&
//   hitnormal.z > 0.5)" that forces the static (acceleration-directed)
//   friction branch for resting support contacts, so contact friction
//   responds to slip *acceleration* and not to slip *velocity*,
// - in PhysicsEngine::PhysicsSimulateCycle, the post-contact pass that damps
//   m_angularmomentum of in-contact balls whose recent displacement is small
//   relative to their horizontal spin: ratio = |Lxy|^2 / dist^2 over the
//   last 80/90ms, damped by up to x0.23 per physics step above ratio 666.
//
// The engine has no rolling-resistance model: without some quench, residual
// spin persists forever (frictionless case), gets injected by the static
// friction on sloped playfields, or is converted back into motion by slip-
// directed friction. These tests document what the hack does, which parts
// are still reachable, and which behavior a replacement must keep.

#include "core/stdafx.h"
#include "../vpx-test.h"
#include "physics-harness.h"

#include "parts/ball.h"
#include "parts/pintable.h"
#include "parts/surface.h"
#include "physics/PhysicsEngine.h"
#include "physics/collide.h"
#include "physics/hitball.h"
#include "physics/physconst.h"

#include "doctest.h"

namespace
{

// Playfield physics override on the test table (like PinTable::m_overridePhysics):
// must be set before Start() which reads it when creating the HitPlane collider.
void OverridePlayfieldPhysics(PinTable *table, const float friction, const float elasticity)
{
   table->m_overridePhysics = 1;
   table->m_fOverrideContactFriction = friction;
   table->m_fOverrideElasticity = elasticity;
   table->m_fOverrideElasticityFalloff = 0.f;
   table->m_fOverrideScatterAngle = 0.f;
}

HitBall MakeBall(PhysicsEngine *physics, const Vertex3Ds &vel, const Vertex3Ds &angularMomentum)
{
   HitBall ball;
   ball.m_physics = physics;
   ball.m_d.m_mass = 1.f;
   ball.m_d.m_vel = vel;
   ball.m_angularmomentum = angularMomentum;
   return ball;
}

} // namespace

// ---------------------------------------------------------------------------
// The ApplyFriction half: on resting support contacts (normVel <= 0.025 with
// a support-like normal), the hack forces the static friction branch, which
// reacts to slip acceleration only. With the define removed, the dynamic
// branch fires instead and answers the slip *velocity*: a resting ball with
// residual spin is gripped back into motion (measured: vy = -0.055/step and
// Lx -1.36/step for a 50 VPU/T slip), and a sliding ball is decelerated by
// the contact (measured: -0.055/step on a 10 VPU/T slide).
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: resting support contact friction ignores slip velocity")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();
   const float normalImpulse = GRAVITYCONST * (float)PHYS_FACTOR; // the Δv a resting contact applies per step

   SUBCASE("a resting ball keeps its residual spin")
   {
      HitBall ball = MakeBall(harness.GetEngine(), Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds(500.f, 0.f, 0.f)); // 50 VPU/T slip at the floor contact
      ball.ApplyFriction(Vertex3Ds(0.f, 0.f, 1.f), (float)PHYS_FACTOR, 0.3f, normalImpulse);
      // static branch: tangential slip acceleration is ~0 on a flat support -> early return, nothing applied
      CHECK(ball.m_d.m_vel.LengthSquared() == doctest::Approx(0.f));
      CHECK(ball.m_angularmomentum.x == doctest::Approx(500.f));
   }

   SUBCASE("a sliding ball is not decelerated by the contact")
   {
      HitBall ball = MakeBall(harness.GetEngine(), Vertex3Ds(0.f, 10.f, 0.f), Vertex3Ds(0.f, 0.f, 0.f));
      ball.ApplyFriction(Vertex3Ds(0.f, 0.f, 1.f), (float)PHYS_FACTOR, 0.3f, normalImpulse);
      // same static branch: the slide is not slowed by contact friction (decay happens through collision events)
      CHECK(ball.m_d.m_vel.y == doctest::Approx(10.f));
      CHECK(ball.m_angularmomentum.LengthSquared() == doctest::Approx(0.f));
   }
}

// ---------------------------------------------------------------------------
// Under FIX_PHYSICS the clause additionally requires a support-like normal:
// wall contacts (hitnormal.z <= 0.5) keep the dynamic, slip-directed branch.
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: wall contacts keep slip-directed friction")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   // Ball resting against a vertical wall, spinning about +y: 50 VPU/T slip
   // along +z at the contact -> friction pushes the contact down (-z).
   HitBall ball = MakeBall(harness.GetEngine(), Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds(0.f, 500.f, 0.f));
#ifdef FIX_PHYSICS
   ball.ApplyFriction(Vertex3Ds(1.f, 0.f, 0.f), (float)PHYS_FACTOR, 0.3f, 100.f); // generous impulse budget (bound 30), no clamping
   // FIX_PHYSICS requests the full slip-removing impulse: jt = -slipspeed / (1/m + r^2/I) = -50 / 3.5
   CHECK(ball.m_d.m_vel.z == doctest::Approx(-14.286f).epsilon(0.01));
   CHECK(ball.m_angularmomentum.y == doctest::Approx(500.f - 25.f * 14.286f).epsilon(0.01));
#else
   ball.ApplyFriction(Vertex3Ds(1.f, 0.f, 0.f), (float)PHYS_FACTOR, 0.3f, 10.f); // generous impulse budget, no clamping
   // jt = dtime * (-slipspeed) / (1/m + r^2/I) = 0.1 * -50 / 3.5
   CHECK(ball.m_d.m_vel.z == doctest::Approx(-1.4286f).epsilon(0.01));
   CHECK(ball.m_angularmomentum.y == doctest::Approx(500.f - 25.f * 1.4286f).epsilon(0.01));
#endif
}

// ---------------------------------------------------------------------------
// The static branch impulse also applies the rolling-constraint torque: on a
// sloped playfield a resting ball gains angular momentum from the contact
// every step (here ~-0.14 of Lx per step at 6 degrees). This is what feeds
// residual spin into resting balls on real (sloped) tables — the input the
// engine-level quench exists to bleed.
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: sloped support contact pumps spin into a resting ball")
{
   PhysicsTestHarness harness;
   harness.SetGravity(6.f, GRAVITYCONST); // tangential gravity is +y
   harness.Start();

   HitBall ball = MakeBall(harness.GetEngine(), Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds(0.f, 0.f, 0.f));
   const float normalImpulse = cosf(ANGTORAD(6.f)) * GRAVITYCONST * (float)PHYS_FACTOR;
   ball.ApplyFriction(Vertex3Ds(0.f, 0.f, 1.f), (float)PHYS_FACTOR, 0.3f, normalImpulse);

   // static branch impulse = -dtime * tangentialG / (1/m + r^2/I) applied at the contact
   const float stepImpulse = (float)PHYS_FACTOR * sinf(ANGTORAD(6.f)) * GRAVITYCONST / 3.5f;
   CHECK(ball.m_d.m_vel.y == doctest::Approx(-stepImpulse).epsilon(0.05));
   CHECK(ball.m_angularmomentum.x == doctest::Approx(-stepImpulse * DEFAULT_BALL_SIZE).epsilon(0.05));
}

// ---------------------------------------------------------------------------
// Reachability: ApplyFriction is only ever called at the end of
// HandleStaticContact, after the contact normal impulse. That impulse leaves
// the residual normal velocity at -(g.n).dtime (~ +0.18 on the playfield,
// above the 0.025 gate), so under FIX_PHYSICS the resting-support clause
// never actually fires through the real call path: the ApplyFriction half of
// the hack is dead code, and the PhysicsSimulateCycle quench is the only
// live part. This test drives the full path to pin that.
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: the resting-contact clause does not fire through HandleStaticContact")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   HitBall ball = MakeBall(harness.GetEngine(), Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds(500.f, 0.f, 0.f));

   CollisionEvent coll;
   coll.m_hitnormal = Vertex3Ds(0.f, 0.f, 1.f); // resting on the playfield
   coll.m_hitdistance = 0.f;
   coll.m_hit_org_normalvelocity = 0.f;

   ball.HandleStaticContact(coll, 0.3f, (float)PHYS_FACTOR);

   // The contact impulse leaves vel.z at +|g|.dtime > 0.025: ApplyFriction
   // then takes the dynamic branch like without the hack, gripping the slip.
   CHECK(ball.m_d.m_vel.z == doctest::Approx(GRAVITYCONST * PHYS_FACTOR));
   CHECK(ball.m_d.m_vel.y < -0.01f); // spin was converted into a -y kick
   CHECK(ball.m_angularmomentum.x < 500.f); // and the spin decayed
}

// ---------------------------------------------------------------------------
// The engine-level quench: for a ball in contact, Lxy^2 / displacement^2
// over the last ~90ms is damped once above 666. On a frictionless playfield
// nothing else can change the angular momentum, so the quench is isolated:
// - a slow slider is clamped down to ~sqrt(666 * dist^2) (~92 measured),
// - a fast slider is never touched,
// - spin about the vertical axis never enters the ratio (Lxy only) and is
//   never quenched.
// With the define removed the slow slider keeps its full 400 of Lx.
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: the quench only fires while recent motion is small")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   OverridePlayfieldPhysics(harness.GetTable(), 0.f, 0.25f);

   Ball *const slow = harness.AddBall(300.f, 1000.f, 0.f, 0.f, 0.5f, 0.f);
   slow->m_hitBall.m_angularmomentum = Vertex3Ds(400.f, 0.f, 0.f);
   Ball *const fast = harness.AddBall(550.f, 1000.f, 0.f, 0.f, 30.f, 0.f);
   fast->m_hitBall.m_angularmomentum = Vertex3Ds(500.f, 0.f, 0.f);
   Ball *const axial = harness.AddBall(800.f, 1000.f, 0.f, 0.f, 0.5f, 0.f);
   axial->m_hitBall.m_angularmomentum = Vertex3Ds(0.f, 0.f, 400.f);
   harness.Start();

   harness.AdvanceMs(250); // > 90ms of position history, fast ball stays clear of the walls

   // clamped by the quench down to ratio 666 * (0.5 VPU/T * 90ms)^2 territory
   INFO("slow Lx = ", slow->m_hitBall.m_angularmomentum.x);
   CHECK(slow->m_hitBall.m_angularmomentum.x < 150.f);
   CHECK(slow->m_hitBall.m_angularmomentum.x > 30.f); // but not killed completely ("do not kill spin completely")
   CHECK(slow->m_hitBall.m_d.m_vel.y == doctest::Approx(0.5f)); // the quench only touches the spin

   // a moving ball's ratio stays far below 666: full spin retained
   CHECK(fast->m_hitBall.m_angularmomentum.x == doctest::Approx(500.f));

   // axial spin does not produce contact-point slip and never enters the ratio
   CHECK(axial->m_hitBall.m_angularmomentum.z == doctest::Approx(400.f));
}

// ---------------------------------------------------------------------------
// The quench divides by the squared displacement since ~90ms ago. A ball
// that has not moved at all gives +inf, which is skipped by the infNaN
// guard: a perfectly still ball keeps its spin forever. On a frictional
// playfield this corner never lasts (collision friction converts spin into
// creep, producing the displacement that enables the quench); it shows up
// here on a frictionless playfield. Intended behavior is pinned with
// should_fail: a resting ball should lose residual spin, however still it
// is — a replacement mechanism (e.g. rolling resistance) would cover this
// unconditionally.
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: a bit-still ball loses its residual spin" * doctest::should_fail())
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   OverridePlayfieldPhysics(harness.GetTable(), 0.f, 0.25f);

   Ball *const ball = harness.AddBall(500.f, 1000.f, 0.f);
   ball->m_hitBall.m_angularmomentum = Vertex3Ds(500.f, 0.f, 0.f);
   harness.Start();

   harness.AdvanceMs(500); // well past the 90ms history window

   // the ball never moved -> displacement is 0 -> ratio is +inf -> skipped:
   // L stays at its full 500 instead of decaying
   CHECK(ball->m_hitBall.m_angularmomentum.x < 250.f);
}

// ---------------------------------------------------------------------------
// End-to-end effect on a real (frictional) playfield: collision-level
// friction keeps converting residual spin into motion, so a "resting" ball
// that still spins does not stay still. The quench bleeds L fast enough that
// the creep saturates low: measured stable creep ~3.9 VPU/T with L ~39, vs
// ~14 VPU/T and still accelerating without the hack.
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: residual spin does not become a persistent creep")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);

   Ball *const ball = harness.AddBall(500.f, 1000.f, 0.f);
   ball->m_hitBall.m_angularmomentum = Vertex3Ds(500.f, 0.f, 0.f);
   harness.Start();

   harness.AdvanceMs(1500);

   const BallS &d = ball->m_hitBall.m_d;
   INFO("creep vel = ", d.m_vel.x, ' ', d.m_vel.y, ' ', d.m_vel.z, " L = ", ball->m_hitBall.m_angularmomentum.x);
   CHECK(d.m_vel.Length() < 5.f); // saturates around 3.9 VPU/T (no rolling resistance: it never fully stops)
   CHECK(fabsf(ball->m_hitBall.m_angularmomentum.x) < 60.f);
}

// ---------------------------------------------------------------------------
// Semantic contract any replacement for the hack must keep: a ball that
// comes to rest holds no residual spin — on a sloped playfield the static
// contact friction keeps pumping it (see "sloped support contact pumps
// spin"), so something has to bleed it continuously.
// ---------------------------------------------------------------------------

TEST_CASE("BallSpin: a ball coming to rest keeps no residual spin")
{
   PhysicsTestHarness harness;
   harness.SetGravity(6.5f, GRAVITYCONST); // pushes the ball +y, into the wall
   harness.AddWallSegment(300.f, 1400.f, 700.f, 1400.f, 0.f, 100.f);

   Ball *const ball = harness.AddBall(500.f, 1000.f, 0.f);
   ball->m_hitBall.m_angularmomentum = Vertex3Ds(500.f, 0.f, 0.f);
   harness.Start();

   harness.AdvanceMs(3000); // roll down-slope, hit the wall, settle against it

   const BallS &d = ball->m_hitBall.m_d;
   const Vertex3Ds angvel = ball->m_hitBall.m_angularmomentum / ball->m_hitBall.Inertia();
   INFO("pos = ", d.m_pos.x, ' ', d.m_pos.y, ' ', d.m_pos.z, " vel = ", d.m_vel.x, ' ', d.m_vel.y, ' ', d.m_vel.z, " angvel = ", angvel.x, ' ', angvel.y, ' ', angvel.z);
   CHECK(d.m_pos.y == doctest::Approx(1375.f).epsilon(0.02)); // resting against the wall (1400 - radius)
   CHECK(d.m_vel.Length() < 0.5f);
   CHECK(angvel.Length() < 0.05f); // residual spin bled down to ~0.002

   // and it stays put: position stable over another 500ms
   const Vertex3Ds rest = d.m_pos;
   harness.AdvanceMs(500);
   CHECK((d.m_pos - rest).Length() < 1.f);
}
