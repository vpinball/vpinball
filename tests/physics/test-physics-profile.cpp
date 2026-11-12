// license:GPLv3+
//
// Physics profiles (PinTable::m_physicProfile, see physconst.h): V10_8 must reproduce the VPX 10.8 physics existing tables were
// tuned for, V10_9 enables the fixes. The other physics tests run the harness default (V10_9): each test here runs both profiles
// and pins the V10_8 behavior next to the V10_9 one.
//
// Expectations follow the profile the engine runs (PhysicsEngine::GetPhysicProfile): NEW_PHYSICS builds always run V10_9.

#include "core/stdafx.h"
#include "../vpx-test.h"
#include "physics-harness.h"

#include "parts/flipper.h"
#include "parts/pintable.h"
#include "physics/PhysicsEngine.h"
#include "physics/collide.h"
#include "physics/collideex.h"
#include "physics/hitball.h"
#include "physics/hitflipper.h"
#include "physics/physconst.h"

#include "doctest.h"

namespace
{

constexpr PhysicProfile kProfiles[] = { PhysicProfile::V10_8, PhysicProfile::V10_9 };

const char *ProfileName(const PhysicProfile profile) { return profile == PhysicProfile::V10_8 ? "10.8" : "10.9"; }

BallS MakeBallState(const Vertex3Ds &pos, const Vertex3Ds &vel)
{
   BallS ball;
   ball.m_pos = pos;
   ball.m_vel = vel;
   ball.m_radius = DEFAULT_BALL_SIZE;
   ball.m_mass = 1.f;
   ball.m_vpVolObjs = nullptr;
   return ball;
}

// Starts a harness on a table of the given profile, returning the profile its engine runs
PhysicProfile StartWithProfile(PhysicsTestHarness &harness, const PhysicProfile profile)
{
   harness.GetTable()->m_physicProfile = profile;
   harness.SetGravity(0.f, GRAVITYCONST); // flat: gravity is (0, 0, -GRAVITYCONST)
   harness.Start();
   return harness.GetEngine()->GetPhysicProfile();
}

} // namespace

// ---------------------------------------------------------------------------
// The profile is fixed when the engine is created: the colliders and movers are built for it (flipper inertia model, thin
// triangle normals), so a change of the table's profile while playing (live editor) only applies to the next play
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: the engine keeps the profile it was created with")
{
   PhysicsTestHarness harness;
   harness.GetTable()->m_physicProfile = PhysicProfile::V10_8;
#ifdef NEW_PHYSICS
   constexpr PhysicProfile expected = PhysicProfile::V10_9; // NEW_PHYSICS is built on top of the V10_9 fixes
#else
   constexpr PhysicProfile expected = PhysicProfile::V10_8;
#endif
   CHECK(PhysicsEngine::ResolvePhysicProfile(harness.GetTable()) == expected);
   const PhysicProfile running = StartWithProfile(harness, PhysicProfile::V10_8);
   CHECK(running == PhysicsEngine::ResolvePhysicProfile(harness.GetTable()));

   harness.GetTable()->m_physicProfile = PhysicProfile::V10_9; // changed while playing
   CHECK(harness.GetEngine()->GetPhysicProfile() == running);

   // An object added afterward still follows the engine
   HitBall ball;
   ball.SetPhysics(harness.GetEngine());
   CHECK(ball.GetPhysicProfile() == running);

   // Detached objects exercise the latest behavior
   HitBall detached;
   CHECK(detached.GetPhysicProfile() == PhysicProfile::V10_9);
}

// ---------------------------------------------------------------------------
// Walls (LineSeg): a slow ball within the touch layer is a contact under V10_9. V10_8 reports a fake hit time
// (bnd * 10 + 0.5), beyond the physics step, so the touch is dropped until the ball actually penetrates
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: slow shallow wall touch")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      const PhysicProfile running = StartWithProfile(harness, profile);

      LineSeg wall(nullptr, Vertex2D(100.f, 0.f), Vertex2D(100.f, 100.f), 0.f, 50.f); // normal faces -x
      wall.SetPhysics(harness.GetEngine());
      CollisionEvent coll;
      const BallS ball = MakeBallState(Vertex3Ds(75.f - 0.02f, 50.f, 25.f), Vertex3Ds(0.05f, 0.f, 0.f)); // 0.02 away, approaching slowly
      const float hittime = wall.HitTest(ball, (float)PHYS_FACTOR, coll);
      if (running == PhysicProfile::V10_9)
      {
         CHECK(hittime == 0.f);
         CHECK(coll.m_isContact);
      }
      else
         CHECK(hittime < 0.f); // 0.02 * 10 + 0.5 = 0.7 > the 0.1 step: no event
   }
}

// ---------------------------------------------------------------------------
// Playfield (HitPlane): a slow ball embedded deeper than PHYS_TOUCH is ignored by V10_8 (and can sink through), V10_9
// collides it out
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: slow ball embedded in the playfield")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      const PhysicProfile running = StartWithProfile(harness, profile);

      HitPlane playfield(harness.GetTable(), Vertex3Ds(0.f, 0.f, 1.f), 0.f);
      playfield.SetPhysics(harness.GetEngine());
      CollisionEvent coll;
      const BallS ball = MakeBallState(Vertex3Ds(500.f, 500.f, DEFAULT_BALL_SIZE - 1.f), Vertex3Ds(0.f, 0.f, 0.f)); // 1 VPU deep, at rest
      const float hittime = playfield.HitTest(ball, (float)PHYS_FACTOR, coll);
      if (running == PhysicProfile::V10_9)
         CHECK(hittime == 0.f);
      else
         CHECK(hittime < 0.f);
   }
}

// ---------------------------------------------------------------------------
// Thin triangles: V10_9 discards nearly degenerate normals (precision), V10_8 normalizes them like any other
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: thin triangle normal")
{
   const Vertex3Ds rgv[3] = { Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds(100.f, 0.f, 0.f), Vertex3Ds(50.f, 0.0001f, 0.f) }; // |e0 x e1|^2 = 1e-4
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      const HitTriangle triangle(nullptr, rgv, profile);
      CHECK(triangle.GetPhysicProfile() == profile);
      if (profile == PhysicProfile::V10_9)
         CHECK(triangle.m_normal.LengthSquared() == 0.f);
      else
         CHECK(triangle.m_normal.LengthSquared() == doctest::Approx(1.f));
   }
}

// ---------------------------------------------------------------------------
// Static contacts: V10_8 computed the normal impulse in a mixed impulse/velocity unit, so a resting ball of mass m gained
// m times the gravity compensation per step. V10_9 applies |g| dt (plus the approach cancellation) whatever the mass
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: resting contact velocity gain")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      const PhysicProfile running = StartWithProfile(harness, profile);

      HitBall ball;
      ball.SetPhysics(harness.GetEngine());
      ball.m_d.m_mass = 4.f;
      ball.m_d.m_vel.SetZero();
      ball.m_angularmomentum.SetZero();

      CollisionEvent coll;
      coll.m_hitnormal = Vertex3Ds(0.f, 0.f, 1.f);
      coll.m_hitdistance = 1.f;
      coll.m_hit_org_normalvelocity = -0.05f; // still moving into the surface at hit time

      ball.HandleStaticContact(coll, 0.3f, (float)PHYS_FACTOR);
      if (running == PhysicProfile::V10_9)
         CHECK(ball.m_d.m_vel.z == doctest::Approx(GRAVITYCONST * PHYS_FACTOR + 0.05f));
      else
         CHECK(ball.m_d.m_vel.z == doctest::Approx(4.f * GRAVITYCONST * PHYS_FACTOR + 0.05f));
   }
}

// ---------------------------------------------------------------------------
// Contact friction against a wall: V10_8 bounds it by the gravity component pressing the ball on the surface, which is
// none on a vertical wall of a flat table, so a ball spinning against a wall keeps its spin. V10_9 bounds it by the normal
// impulse of the contact, and requests the full slip-removing impulse
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: wall contact friction")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      const PhysicProfile running = StartWithProfile(harness, profile);

      HitBall ball;
      ball.SetPhysics(harness.GetEngine());
      ball.m_d.m_mass = 1.f;
      ball.m_d.m_vel.SetZero();
      ball.m_angularmomentum = Vertex3Ds(0.f, 500.f, 0.f); // 50 VPU/T slip along +z at the wall contact
      ball.ApplyFriction(Vertex3Ds(1.f, 0.f, 0.f), (float)PHYS_FACTOR, 0.3f, 100.f); // generous impulse budget (bound 30)
      if (running == PhysicProfile::V10_9)
      {
         // jt = -slipspeed / (1/m + r^2/I) = -50 / 3.5
         CHECK(ball.m_d.m_vel.z == doctest::Approx(-14.286f).epsilon(0.01));
         CHECK(ball.m_angularmomentum.y == doctest::Approx(500.f - 25.f * 14.286f).epsilon(0.01));
      }
      else
      {
         CHECK(ball.m_d.m_vel.LengthSquared() == doctest::Approx(0.f));
         CHECK(ball.m_angularmomentum.y == doctest::Approx(500.f));
      }
   }
}

// ---------------------------------------------------------------------------
// Flipper inertia: V10_8 models the flipper as a rod (mass * length^2 / 3), V10_9 uses its real shape. Mass reads and
// writes (Flipper.Mass from scripts) must go through the same model as the inertia, whatever the profile
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: flipper inertia model and mass round-trip")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      Flipper *const flipperPart = Flipper::COMCreate();
      flipperPart->Init(0.f, 0.f, false);
      flipperPart->SetName("TestFlipperProfile");
      harness.GetTable()->AddPart(flipperPart);
      flipperPart->Release(); // owned by the table
      const PhysicProfile running = StartWithProfile(harness, profile);

      HitFlipper flipper(Vertex2D(500.f, 500.f), 30.f, 15.f, 80.f, 0.f, 0.f, 0.f, 100.f, flipperPart, running);
      flipper.SetPhysics(harness.GetEngine());
      FlipperMoverObject &mover = flipper.m_flipperMover;
      REQUIRE(mover.GetPhysicProfile() == running);

      const float partMass = flipperPart->m_d.m_mass;
      const float rod = 80.f * 80.f / 3.f;
      if (running == PhysicProfile::V10_9)
         CHECK(mover.m_inertia == doctest::Approx(partMass * mover.m_inertiaShape));
      else
         CHECK(mover.m_inertia == doctest::Approx(partMass * rod));
      CHECK(mover.GetMass() == doctest::Approx(partMass));

      mover.SetMass(2.f * partMass);
      CHECK(std::isfinite(mover.m_inertia));
      CHECK(mover.GetMass() == doctest::Approx(2.f * partMass));
      CHECK(mover.m_inertia == doctest::Approx(2.f * partMass * (running == PhysicProfile::V10_9 ? mover.m_inertiaShape : rod)));
   }
}
