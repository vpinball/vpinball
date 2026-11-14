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

#include "math/dragpoint.h"
#include "parts/ball.h"
#include "parts/flipper.h"
#include "parts/pintable.h"
#include "parts/ramp.h"
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

// ---------------------------------------------------------------------------
// Ramp side walls (https://github.com/vpinball/vpinball/issues/222): V10_8 walls are LineSeg rectangles subdivided
// along the slope, and the segments walking the slope from high to low got their subdivided height windows
// swapped, so the collider keeping the ball on the ramp is dead wherever the floor height falls outside the
// swapped window: the ball escapes through the wall on steep sections, while balls rolling on the playfield
// below the raised wall are phantom-blocked. V10_9 builds the exact sloped quads.
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: ramp side wall blocks the ball along the whole slope")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;

      // The issue's repro: a straight 2-point ramp, heights 0 -> 105, widths 115 -> 65, wall heights 60
      Ramp *const ramp = Ramp::COMCreate();
      ramp->Init(500.f, 1000.f, false);
      ramp->SetName("TestRamp");
      ramp->m_curve.ClearPoints();
      ramp->m_curve.PushPoint(std::make_unique<DragPoint>(&ramp->m_curve, 500.f, 1400.f, 0.f, true));
      ramp->m_curve.PushPoint(std::make_unique<DragPoint>(&ramp->m_curve, 500.f, 600.f, 0.f, true));
      ramp->m_d.m_type = RampTypeFlat;
      ramp->m_d.m_heightbottom = 0.f;
      ramp->m_d.m_heighttop = 105.f;
      ramp->m_d.m_widthbottom = 115.f;
      ramp->m_d.m_widthtop = 65.f;
      ramp->m_d.m_leftwallheight = 60.f;
      ramp->m_d.m_rightwallheight = 60.f;
      ramp->m_d.m_collidable = true;
      ramp->m_d.m_overwritePhysics = true;
      ramp->m_d.m_elasticity = 0.3f;
      ramp->m_d.m_friction = 0.3f;
      ramp->m_d.m_scatter = 0.f;
      harness.GetTable()->AddPart(ramp);
      ramp->Release(); // owned by the table

      const PhysicProfile running = StartWithProfile(harness, profile);

      // x of the right outline at height y (the wall line): the outline is a trapezoid, the edge
      // widens to 557.5 at the bottom, so "outside" must be tested against the edge at the ball's y
      const auto rightEdge = [](const float y)
      {
         const float t = (1400.f - y) / 800.f;
         return 500.f + 0.5f * (115.f - 50.f * (t < 0.f ? 0.f : (t > 1.f ? 1.f : t)));
      };

      // t ~= 0.9 of the ramp: floor height ~94.5, right outline x = 535 — inside the deepest V10_8 dead zone
      SUBCASE("a ball on the ramp cannot escape through the wall")
      {
         Ball *const ball = harness.AddBall(520.f, 680.f, 94.5f, 300.f, 0.f, 0.f);
         // "escaped" = outside the outline above playfield height (a ball that fell to the playfield and rolls under the wall is not an escape)
         const auto escaped = [&ball, &rightEdge]()
         {
            const Vertex3Ds &p = ball->m_hitBall.m_d.m_pos;
            return p.x > rightEdge(p.y) && p.z > 60.f;
         };
         CAPTURE(ball->m_hitBall.m_d.m_pos);
         if (running == PhysicProfile::V10_9)
            CHECK(!harness.AdvanceUntil(escaped, 2000));
         else
            CHECK(harness.AdvanceUntil(escaped, 2000)); // the dead zone lets it leave through the wall
      }

      SUBCASE("a ball on the playfield rolls under the raised wall")
      {
         Ball *const ball = harness.AddBall(520.f, 680.f, 0.f, 300.f, 0.f, 0.f);
         const auto passed = [&ball, &rightEdge]()
         {
            const Vertex3Ds &p = ball->m_hitBall.m_d.m_pos;
            return p.x > rightEdge(p.y);
         };
         CAPTURE(ball->m_hitBall.m_d.m_pos);
         if (running == PhysicProfile::V10_9)
            CHECK(harness.AdvanceUntil(passed, 2000)); // the wall's bottom edge follows the ramp, the ball passes under
         else
            CHECK(!harness.AdvanceUntil(passed, 2000)); // the swapped height window phantom-blocks it at the wall line
      }
   }
}

// ---------------------------------------------------------------------------
// The cabinet nudge pseudo-force is part of the ball's external acceleration
// (PhysicsEngine::GetFrameAcceleration), feeding the contact model under all
// profiles (nudge-only: it is zero without an active nudge, see physconst.h).
// It is an acceleration, so V10_8's 1/mass scaling quirk stays limited to gravity.
// ---------------------------------------------------------------------------

TEST_CASE("PhysicsProfile: the ball acceleration includes the nudge pseudo-force")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      harness.GetTable()->m_angletiltMin = harness.GetTable()->m_angletiltMax = 0.f; // flat projection
      const PhysicProfile running = StartWithProfile(harness, profile); // flat gravity (0, 0, -G)
      harness.GetEngine()->SetCabinetAcceleration(Vertex2D(2.f, 0.f)); // pushes the balls toward -x

      HitBall ball;
      ball.SetPhysics(harness.GetEngine());
      ball.m_d.m_mass = 2.f;
      ball.m_angularmomentum.SetZero(); // no centripetal term

      const float f = MS2TOVPUVPT2(2.f); // pseudo-acceleration magnitude
      const Vertex3Ds acc = ball.SurfaceAcceleration(Vertex3Ds(0.f, 0.f, -DEFAULT_BALL_SIZE));
      const float gravityScale = running == PhysicProfile::V10_8 ? 0.5f : 1.f; // the 1/mass quirk only applies to gravity
      CHECK(acc.x == doctest::Approx(-f));
      CHECK(fabsf(acc.y) < 1e-6f);
      CHECK(acc.z == doctest::Approx(-GRAVITYCONST * gravityScale));
   }
}

TEST_CASE("PhysicsProfile: a contact anticipates the nudge pressing the ball into it")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      const PhysicProfile running = StartWithProfile(harness, profile); // flat gravity
      harness.GetEngine()->SetCabinetAcceleration(Vertex2D(2.f, 0.f)); // pushes the ball toward -x

      HitBall ball;
      ball.SetPhysics(harness.GetEngine());
      ball.m_d.m_vel.SetZero();
      ball.m_angularmomentum.SetZero();

      // Support on the ball's -x side: the contact must anticipate the next step's
      // pseudo-force injection the same way it anticipates gravity.
      CollisionEvent coll;
      coll.m_hitnormal = Vertex3Ds(1.f, 0.f, 0.f);
      coll.m_hitdistance = 1.f; // outside PHYS_TOUCH: no embed correction
      coll.m_hit_org_normalvelocity = 0.f;

      ball.HandleStaticContact(coll, 0.3f, (float)PHYS_FACTOR);
      CHECK(ball.m_d.m_vel.x == doctest::Approx(MS2TOVPUVPT2(2.f) * PHYS_FACTOR));

      // Without a nudge, the same contact applies no impulse
      harness.GetEngine()->SetCabinetAcceleration(Vertex2D(0.f, 0.f));
      HitBall idle;
      idle.SetPhysics(harness.GetEngine());
      idle.m_d.m_vel.SetZero();
      idle.m_angularmomentum.SetZero();
      idle.HandleStaticContact(coll, 0.3f, (float)PHYS_FACTOR);
      CHECK(fabsf(idle.m_d.m_vel.x) < 1e-6f);
   }
}

TEST_CASE("PhysicsProfile: the flipper contact sees the nudge pressing the ball into it")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      const PhysicProfile running = StartWithProfile(harness, profile); // flat gravity
      harness.GetEngine()->SetCabinetAcceleration(Vertex2D(2.f, 0.f)); // pushes the ball toward -x

      Flipper *const flipperPart = Flipper::COMCreate();
      flipperPart->Init(0.f, 0.f, false);
      flipperPart->SetName("TestFlipperNudge");
      harness.GetTable()->AddPart(flipperPart);
      flipperPart->Release(); // owned by the table

      // Flipper at angle 0 pointing toward -y: face2 on the +x side (same layout as the flipper face tests)
      HitFlipper flipper(Vertex2D(500.f, 500.f), 30.f, 15.f, 80.f, 0.f, 0.f, 0.f, 100.f, flipperPart, running);
      flipper.SetPhysics(harness.GetEngine());
      const Vertex2D F(flipper.m_flipperMover.m_zeroAngNorm);
      const Vertex2D T(F.y, -F.x);
      const Vertex2D facePoint = Vertex2D(500.f, 500.f) + F * 30.f + T * (80.f * F.x * 0.5f);

      HitBall ball;
      ball.SetPhysics(harness.GetEngine());
      ball.m_d.m_pos = Vertex3Ds(facePoint.x + F.x * DEFAULT_BALL_SIZE, facePoint.y + F.y * DEFAULT_BALL_SIZE, DEFAULT_BALL_SIZE);
      ball.m_d.m_vel.SetZero();
      ball.m_angularmomentum.SetZero();

      // The nudge presses the ball into the face: the gap-closing criterion must see
      // it, so the contact applies an impulse instead of early-outing (normAcc = 0
      // without the frame term: gravity is tangential to the vertical face).
      CollisionEvent coll;
      coll.m_ball = &ball;
      coll.m_hitnormal = Vertex3Ds(F.x, F.y, 0.f);
      coll.m_hitdistance = 0.f;
      coll.m_hit_org_normalvelocity = 0.f;

      flipper.Contact(coll, (float)PHYS_FACTOR);
      CHECK(ball.m_d.m_vel.Dot(coll.m_hitnormal) > 0.f);
   }
}

TEST_CASE("PhysicsProfile: a ball nudged into a wall keeps contact")
{
   for (const PhysicProfile profile : kProfiles)
   {
      CAPTURE(ProfileName(profile));
      PhysicsTestHarness harness;
      harness.GetTable()->m_physicProfile = profile;
      harness.AddWallSegment(100.f, 900.f, 100.f, 1100.f); // wall at x ~100, normal +-x
      Ball *const ball = harness.AddBall(101.f + DEFAULT_BALL_SIZE, 1000.f, 0.f); // resting against its +x face
      StartWithProfile(harness, profile); // flat gravity

      harness.AdvanceMs(500); // settle into the contact

      harness.GetEngine()->SetCabinetAcceleration(Vertex2D(3.f, 0.f)); // presses the ball into the wall
      harness.AdvanceMs(1000);

      const BallS &d = ball->m_hitBall.m_d;
      CHECK(d.m_pos.x > 101.f + DEFAULT_BALL_SIZE - 1.f); // did not sink into the wall
      CHECK(d.m_pos.x < 101.f + DEFAULT_BALL_SIZE + 5.f); // stays pressed against it
      CHECK(d.m_vel.Length() < 2.f); // no runaway velocity
   }
}
