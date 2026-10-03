// license:GPLv3+
//
// Regression tests for physics-engine defects and contract inconsistencies 
// between collider types identified through tests & audits, and pending fixes.
//
// Tests that pin *intended* behavior for a confirmed-but-unfixed defect are
// decorated with doctest::should_fail(): they fail today (which doctest counts
// as an expected failure, keeping the suite green) and start failing the suite
// the day the underlying bug is fixed — at which point the decorator must be
// removed. Do NOT "fix" them by relaxing the expectations.

#include "core/stdafx.h"
#include "../vpx-test.h"
#include "physics-harness.h"

#include "core/Settings.h"
#include "parts/ball.h"
#include "parts/flipper.h"
#include "parts/kicker.h"
#include "parts/pintable.h"
#include "parts/surface.h"
#include "parts/trigger.h"
#include "physics/PhysicsEngine.h"
#include "physics/cabinet/NudgeHandler.h"
#include "physics/collide.h"
#include "physics/collideex.h"
#include "physics/hitball.h"
#include "physics/hitflipper.h"
#include "physics/kdtree.h"
#include "physics/physconst.h"

#include "doctest.h"

namespace
{

BallS MakeBallState(const Vertex3Ds &pos, const Vertex3Ds &vel)
{
   BallS ball;
   ball.m_pos = pos;
   ball.m_vel = vel;
   ball.m_radius = DEFAULT_BALL_SIZE;
   ball.m_mass = 1.f;
   ball.m_vpVolObjs = nullptr; // only dereferenced for trigger/kicker hit objects
   return ball;
}

// HitObject counting how many times the broadphase asks for a HitTest.
class CountingHitObject final : public HitObject
{
public:
   CountingHitObject()
      : HitObject(nullptr)
   {
   }
   float HitTest(const BallS &ball, const float dtime, CollisionEvent &coll) const override
   {
      ++m_hitTestCount;
      return -1.f;
   }
   int GetType() const override { return eLineSeg; }
   void Collide(const CollisionEvent &coll) override { }
   void CalcHitBBox() override { }
   void DrawUI(std::function<Vertex2D(Vertex3Ds)> project, ImDrawList *drawList, bool fill) const override { }

   mutable int m_hitTestCount = 0;
};

} // namespace

// ---------------------------------------------------------------------------
// HitBall::HandleStaticContact must add the gravity compensation impulse and
// the original normal velocity in consistent units, and SurfaceAcceleration
// must return gravity (already an acceleration) unscaled by mass. Both were
// only correct for m_mass == 1.
// ---------------------------------------------------------------------------

TEST_CASE("Resting contact cancels gravity exactly for a unit-mass ball")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   HitBall ball;
   ball.m_physics = harness.GetEngine();
   ball.m_d.m_mass = 1.f;
   ball.m_d.m_vel.SetZero();
   ball.m_angularmomentum.SetZero();

   CollisionEvent coll;
   coll.m_hitnormal = Vertex3Ds(0.f, 0.f, 1.f); // resting on the playfield
   coll.m_hitdistance = 1.f; // outside PHYS_TOUCH: no embed kick
   coll.m_hit_org_normalvelocity = 0.f;

   ball.HandleStaticContact(coll, 0.3f, (float)PHYS_FACTOR);
   // A resting ball must gain exactly |g|*dt upward velocity per step.
   CHECK(ball.m_d.m_vel.z == doctest::Approx(GRAVITYCONST * PHYS_FACTOR));
}

TEST_CASE("Resting contact velocity gain must be mass-independent")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   HitBall ball;
   ball.m_physics = harness.GetEngine();
   ball.m_d.m_mass = 4.f; // a heavier resting ball still needs only |g|*dt of normal velocity
   ball.m_d.m_vel.SetZero();
   ball.m_angularmomentum.SetZero();

   CollisionEvent coll;
   coll.m_hitnormal = Vertex3Ds(0.f, 0.f, 1.f);
   coll.m_hitdistance = 1.f;
   coll.m_hit_org_normalvelocity = 0.f;

   ball.HandleStaticContact(coll, 0.3f, (float)PHYS_FACTOR);
   CHECK(ball.m_d.m_vel.z == doctest::Approx(GRAVITYCONST * PHYS_FACTOR));
}

TEST_CASE("Contact cancels an incoming normal velocity for any mass")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   HitBall ball;
   ball.m_physics = harness.GetEngine();
   ball.m_d.m_mass = 4.f;
   ball.m_d.m_vel.SetZero();
   ball.m_angularmomentum.SetZero();

   CollisionEvent coll;
   coll.m_hitnormal = Vertex3Ds(0.f, 0.f, 1.f);
   coll.m_hitdistance = 1.f;
   coll.m_hit_org_normalvelocity = -0.05f; // still moving into the surface at hit time

   ball.HandleStaticContact(coll, 0.3f, (float)PHYS_FACTOR);
   // The applied delta-v is the gravity compensation plus the approach
   // cancellation, independent of mass.
   CHECK(ball.m_d.m_vel.z == doctest::Approx(GRAVITYCONST * PHYS_FACTOR + 0.05f));
}

TEST_CASE("Ball surface acceleration is independent of mass")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   HitBall ball;
   ball.m_physics = harness.GetEngine();
   ball.m_d.m_mass = 4.f;
   ball.m_d.m_vel.SetZero();
   ball.m_angularmomentum.SetZero(); // no rotation: only the linear acceleration remains

   const Vertex3Ds acc = ball.SurfaceAcceleration(Vertex3Ds(0.f, 0.f, -DEFAULT_BALL_SIZE));
   CHECK(acc.x == doctest::Approx(0.f));
   CHECK(acc.y == doctest::Approx(0.f));
   CHECK(acc.z == doctest::Approx(-GRAVITYCONST));
}

TEST_CASE("Contact friction counters the tangential gravity component for any mass")
{
   PhysicsTestHarness harness;
   harness.SetGravity(6.f, GRAVITYCONST); // gravity gains a +y tangential component on the flat playfield
   harness.Start();

   HitBall ball;
   ball.m_physics = harness.GetEngine();
   ball.m_d.m_mass = 4.f;
   ball.m_d.m_vel.SetZero();
   ball.m_angularmomentum.SetZero(); // no rotation: SurfaceAcceleration is gravity alone

   // Static friction branch (normVel <= 0.025): with I = 2/5 m r^2 the impulse
   // denominator is 1/m + r^2/I = 3.5/m, so the applied delta-v is
   // dtime * tangential g / 3.5, which is mass-independent. The normal impulse
   // argument is the delta-v a resting contact applies per step: -g.n * dtime.
   const float normalImpulse = cosf(ANGTORAD(6.f)) * GRAVITYCONST * (float)PHYS_FACTOR;
   ball.ApplyFriction(Vertex3Ds(0.f, 0.f, 1.f), (float)PHYS_FACTOR, 0.3f, normalImpulse);
   const float tangentialG = sinf(ANGTORAD(6.f)) * GRAVITYCONST;
   CHECK(ball.m_d.m_vel.y == doctest::Approx(-PHYS_FACTOR * tangentialG / 3.5f));
}

TEST_CASE("A heavy ball resting on the playfield is not launched upward")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   Ball *const ball = harness.AddBall(500.f, 1000.f, 0.f, 0.f, 0.f, 0.f, DEFAULT_BALL_SIZE, 10.f);
   harness.Start();

   // The contact normal impulse must just cancel gravity: it must never throw
   // the ball upward (the mass == 1 assumption made it add m*|g|*dt per step).
   const bool launched = harness.AdvanceUntil([&]() { return ball->m_hitBall.m_d.m_vel.z > 0.3f; }, 500);
   CHECK(!launched);
   CHECK(ball->m_hitBall.m_d.m_pos.z == doctest::Approx(DEFAULT_BALL_SIZE).epsilon(0.1));
}

TEST_CASE("Unequal-mass ball/ball collision conserves momentum")
{
   // The impulse path of HitBall::Collide is mass-correct; this guards it while
   // the mass==1 defects above stay open.
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);

   Ball *const moving = harness.AddBall(300.f, 500.f, 0.f, 100.f, 0.f, 0.f, DEFAULT_BALL_SIZE, 4.f);
   Ball *const still = harness.AddBall(600.f, 500.f, 0.f);
   harness.Start();

   harness.AdvanceMs(50);

   const BallS &dMoving = moving->m_hitBall.m_d;
   const BallS &dStill = still->m_hitBall.m_d;
   // restitution 0.8, m=4 vs m=1: still takes v' = 1.8*4*100/5 = 144, moving keeps 64
   CHECK(dStill.m_vel.x == doctest::Approx(144.f).epsilon(0.15));
   CHECK(dMoving.m_vel.x == doctest::Approx(64.f).epsilon(0.15));
   CHECK(4.f * dMoving.m_vel.x + dStill.m_vel.x == doctest::Approx(400.f).epsilon(0.1)); // momentum
}

// ---------------------------------------------------------------------------
// Hit3DPoly::HitTest performs its inside-polygon test in the XY plane even
// though the polygon may have any orientation; a polygon perpendicular to the
// playfield projects onto a line and rejects every hit (the source comment
// acknowledges this: "this need to be changed to a point in polygon on 3D
// plane").
// ---------------------------------------------------------------------------

TEST_CASE("A vertical HitTriangle registers hits")
{
   // Same geometry through the 3D path: proves the defect is Hit3DPoly's XY
   // projection, not the vertical plane itself.
   const Vertex3Ds rgv[3] = { Vertex3Ds(600.f, 400.f, 0.f), Vertex3Ds(600.f, 600.f, 0.f), Vertex3Ds(600.f, 600.f, 100.f) };
   HitTriangle triangle(nullptr, rgv);
   triangle.CalcHitBBox();

   CollisionEvent coll;
   const BallS ball = MakeBallState(Vertex3Ds(575.f, 500.f, 50.f), Vertex3Ds(100.f, 0.f, 0.f));
   CHECK(triangle.HitTest(ball, 1.f, coll) >= 0.f);
   CHECK(coll.m_hitnormal.x == doctest::Approx(-1.f));
}

TEST_CASE("A vertical Hit3DPoly registers hits" * doctest::should_fail())
{
   // Quad perpendicular to the playfield (x=600): its XY projection collapses
   // to a line, so the winding test always rejects the hit.
   Vertex3Ds *const rgv = new Vertex3Ds[4] { Vertex3Ds(600.f, 400.f, 0.f), Vertex3Ds(600.f, 600.f, 0.f), //
      Vertex3Ds(600.f, 600.f, 100.f), Vertex3Ds(600.f, 400.f, 100.f) };
   Hit3DPoly poly(nullptr, rgv, 4);
   poly.CalcHitBBox();

   CollisionEvent coll;
   const BallS ball = MakeBallState(Vertex3Ds(575.f, 500.f, 50.f), Vertex3Ds(100.f, 0.f, 0.f));
   CHECK(poly.HitTest(ball, 1.f, coll) >= 0.f);
}

// ---------------------------------------------------------------------------
// DoHitTest keeps the earliest hit but uses <=, so a second object reporting
// the same hit time overwrites the first: equal-time collisions depend on the
// (randomized) traversal order.
// ---------------------------------------------------------------------------

TEST_CASE("Equal-time collisions keep the first tested object" * doctest::should_fail())
{
   HitBall ball;
   ball.m_d.m_pos = Vertex3Ds(50.f, 50.f, 25.f);
   ball.m_d.m_vel = Vertex3Ds(100.f, 0.f, 0.f);

   LineSeg wallA(nullptr, Vertex2D(100.f, 0.f), Vertex2D(100.f, 100.f), 0.f, 50.f);
   LineSeg wallB(nullptr, Vertex2D(100.f, 0.f), Vertex2D(100.f, 100.f), 0.f, 50.f);

   CollisionEvent coll;
   coll.m_hittime = 1.f;
   DoHitTest(&ball, &wallA, coll);
   DoHitTest(&ball, &wallB, coll); // same hittime: currently overwrites wallA

   CHECK(coll.m_obj == &wallA);
}

// ---------------------------------------------------------------------------
// HitBall::Collide3DWall positional hacks: the embed branch rewrites the normal
// velocity to -C_EMBEDSHOT, and the C_DISP_GAIN block teleports the ball by
// -C_DISP_GAIN*hitdistance along the normal — a position change that no
// velocity ever integrated.
// ---------------------------------------------------------------------------

TEST_CASE("A resting embedded ball stays put" * doctest::should_fail())
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   HitBall ball;
   ball.m_physics = harness.GetEngine();
   ball.m_d.m_pos = Vertex3Ds(570.f, 500.f, 25.f);
   ball.m_d.m_vel.SetZero();
   ball.m_coll.m_hitdistance = -2.f; // 2 VPU embedded at hit time

   ball.Collide3DWall(Vertex3Ds(-1.f, 0.f, 0.f), 0.5f, 0.f, 0.f, -1.f);
   // Two position/velocity hacks fire on a resting ball here:
   // - the embed branch rewrites the (zero) normal velocity to -C_EMBEDSHOT,
   //   producing a phantom bounce velocity,
   // - the C_DISP_GAIN block teleports the ball by -0.9875*hitdistance (~1.98
   //   VPU) along the normal, a position change no velocity ever integrated.
   CHECK(ball.m_d.m_pos.x == doctest::Approx(570.f));
   CHECK(ball.m_d.m_vel.x == doctest::Approx(0.f).epsilon(0.01));
}

// ---------------------------------------------------------------------------
// Ball-ball contacts: a pair inside the touch layer (|bnd| <= PHYS_TOUCH)
// with a slow approach (|bnv| <= C_CONTACTVEL) reports a contact at hittime 0
// like the other colliders; a deeper overlap stays a hittime-0 collision so
// the displacement correction still separates the balls. Recorded contacts are
// dispatched to the contacted HitBall's Contact() (the inherited base
// implementation), which applies the usual static-contact handling to
// coll.m_ball — each ball produces its own record, so the pair is handled on
// both sides.
// ---------------------------------------------------------------------------

TEST_CASE("A slowly touching ball pair produces an event")
{
   HitBall moving;
   moving.m_d.m_pos = Vertex3Ds(0.f, 0.f, 25.f);
   moving.m_d.m_vel = Vertex3Ds(0.05f, 0.f, 0.f); // below C_CONTACTVEL
   HitBall still;
   still.m_d.m_pos = Vertex3Ds(50.02f, 0.f, 25.f); // bnd = 0.02: inside PHYS_TOUCH
   still.m_d.m_vel.SetZero();

   CollisionEvent coll;
   CHECK(still.HitTest(moving.m_d, (float)PHYS_FACTOR, coll) == doctest::Approx(0.f));
   CHECK(coll.m_isContact);
   CHECK(coll.m_hit_org_normalvelocity == doctest::Approx(-0.05f));
   CHECK(coll.m_hitdistance == doctest::Approx(0.02f));
}

TEST_CASE("A deeply overlapped ball pair still reports a collision")
{
   HitBall moving;
   moving.m_d.m_pos = Vertex3Ds(0.f, 0.f, 25.f);
   moving.m_d.m_vel = Vertex3Ds(0.05f, 0.f, 0.f); // below C_CONTACTVEL
   HitBall still;
   still.m_d.m_pos = Vertex3Ds(49.94f, 0.f, 25.f); // bnd = -0.06: resting band, contact drains it
   still.m_d.m_vel.SetZero();

   CollisionEvent coll;
   // Shallow overlap at resting speed is a persistent contact: the contact drain
   // separates the pair gently while a collision would pop them apart and the
   // pair would keep bouncing (the slow jitter loop of resting stacks).
   CHECK(still.HitTest(moving.m_d, (float)PHYS_FACTOR, coll) == doctest::Approx(0.f));
   CHECK(coll.m_isContact);

   // But a pathological overlap deep past the resting band still reports a real
   // collision so the displacement correction can dig the balls out.
   still.m_d.m_pos = Vertex3Ds(49.4f, 0.f, 25.f); // bnd = -0.6
   CHECK(still.HitTest(moving.m_d, (float)PHYS_FACTOR, coll) == doctest::Approx(0.f));
   CHECK(!coll.m_isContact);
}

TEST_CASE("Ball-ball contact handling supports the resting ball")
{
   // Ball-ball contacts are recorded like any other contact and dispatched to
   // the contacted HitBall's Contact(). It must apply static contact handling
   // to coll.m_ball, not silently drop the event: a ball that just gained one
   // step of gravity must come out with zero normal velocity (the impulse is
   // computed from the current velocity, not from a gravity pre-compensation,
   // so that simultaneous contacts on stacked balls do not overshoot).
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   HitBall top;
   top.m_physics = harness.GetEngine();
   top.m_d.m_mass = 1.f;
   top.m_d.m_vel = Vertex3Ds(0.f, 0.f, -GRAVITYCONST * PHYS_FACTOR);
   top.m_angularmomentum.SetZero();
   HitBall bottom;

   CollisionEvent coll;
   coll.m_ball = &top;
   coll.m_obj = &bottom;
   coll.m_hitnormal = Vertex3Ds(0.f, 0.f, 1.f); // top resting on bottom
   coll.m_hitdistance = 0.f;
   coll.m_hit_org_normalvelocity = 0.f;

   bottom.Contact(coll, (float)PHYS_FACTOR);
   CHECK(top.m_d.m_vel.z == doctest::Approx(0.f));
}

TEST_CASE("A ball pressed against a locked ball comes to rest")
{
   // The slope keeps pressing the ball into the anchored one; the ball-ball
   // contact must cancel the approach every step so the ball comes to rest.
   // A kicker-locked ball is used as the anchor so wall-contact noise cannot
   // contaminate the measurement.
   PhysicsTestHarness harness;
   harness.SetGravity(6.f, GRAVITYCONST); // slope downhill toward +y

   Ball *const anchor = harness.AddBall(500.f, 600.f, 0.f);
   Ball *const ball = harness.AddBall(500.f, 549.98f, 0.f); // touching: bnd = 0.02, uphill of anchor
   anchor->m_hitBall.m_d.m_lockedInKicker = true; // frozen: acts as a fixed target
   harness.Start();

   harness.AdvanceMs(500); // let the pair settle

   // Only the lateral velocity is checked: the z component jitters on the
   // playfield contact in any case (a resting ball gains ~0.18/step of downward
   // velocity, above C_CONTACTVEL, so the floor is always a micro-collision).
   float maxLateralSpeed = 0.f;
   for (int i = 0; i < 50; ++i)
   {
      harness.Step();
      const Vertex3Ds v = ball->m_hitBall.m_d.m_vel;
      maxLateralSpeed = std::max(maxLateralSpeed, sqrtf(v.x * v.x + v.y * v.y));
   }
   CHECK(maxLateralSpeed < 0.05f);
}

// ---------------------------------------------------------------------------
// Each collider type defines its own contact window and receding-velocity
// gate, so identical situations are reported as contact, collision, or "no
// event" depending on the shape:
// - HitLineZ has no lower bound on bnd: a deeply embedded ball is reported as
//   a sticky contact that is never ejected,
// - HitCircle drops balls embedded deeper than one ball radius entirely,
// - LineSeg rejects receding balls at C_LOWNORMVEL before the touch-layer
//   test, while HitLineZ accepts receding contact up to C_CONTACTVEL.
// ---------------------------------------------------------------------------

TEST_CASE("A ball deep inside a wall joint is pushed out, not kept" * doctest::should_fail())
{
   // HitLineZ has no lower bound on bnd: a ball 20 VPU inside the joint is
   // reported as a contact (hittime 0), so HandleStaticContact never ejects it.
   HitLineZ joint(nullptr, Vertex2D(100.f, 50.f), 0.f, 50.f);
   CollisionEvent coll;
   const BallS ball = MakeBallState(Vertex3Ds(105.f, 50.f, 25.f), Vertex3Ds(0.f, 0.f, 0.f));

   const float hittime = joint.HitTest(ball, 1.f, coll);
   REQUIRE(hittime >= 0.f); // an event is produced...
   CHECK(!coll.m_isContact); // ...but it must be a collision, not a sticky contact
}

TEST_CASE("A ball deep inside a post is not invisible" * doctest::should_fail())
{
   // HitCircle rejects bnd < -ball.m_radius outright (post radius 25 + ball
   // radius 25 -> center 5 VPU off axis gives bnd = -45): the deeply embedded
   // ball is invisible to the collider, contrasting with HitLineZ above.
   HitCircle post(nullptr, Vertex2D(100.f, 50.f), 25.f, 0.f, 50.f);
   CollisionEvent coll;
   const BallS ball = MakeBallState(Vertex3Ds(105.f, 50.f, 25.f), Vertex3Ds(0.f, 0.f, 0.f));

   CHECK(post.HitTest(ball, 1.f, coll) >= 0.f);
}

TEST_CASE("A slowly receding ball in the touch layer keeps wall contact" * doctest::should_fail())
{
   // LineSeg rejects receding balls before the touch-layer test (bUnHit at
   // C_LOWNORMVEL), so a ball slowly separating while still touching gets no
   // contact support. HitLineZ uses C_CONTACTVEL for this gate instead.
   LineSeg wall(nullptr, Vertex2D(100.f, 0.f), Vertex2D(100.f, 100.f), 0.f, 50.f);
   CollisionEvent coll;
   const BallS ball = MakeBallState(Vertex3Ds(75.f, 50.f, 25.f), Vertex3Ds(-0.05f, 0.f, 0.f));

   const float hittime = wall.HitTest(ball, 1.f, coll);
   CHECK(hittime >= 0.f);
   CHECK(coll.m_isContact);
}

TEST_CASE("A wall joint keeps contact for a slowly receding ball")
{
   // Contrast: HitLineZ does report the same contact (its receding gate is
   // C_CONTACTVEL, not C_LOWNORMVEL). This documents the inconsistent contract.
   HitLineZ joint(nullptr, Vertex2D(100.f, 50.f), 0.f, 50.f);
   CollisionEvent coll;
   const BallS ball = MakeBallState(Vertex3Ds(75.f, 50.f, 25.f), Vertex3Ds(-0.05f, 0.f, 0.f));

   const float hittime = joint.HitTest(ball, 1.f, coll);
   CHECK(hittime == doctest::Approx(0.f));
   CHECK(coll.m_isContact);
}

TEST_CASE("A ball spawned inside a wall joint is ejected" * doctest::should_fail())
{
   // Engine-level symptom of the missing lower bound on HitLineZ's contact
   // window: the joint reports a contact every step, nothing ever pushes the
   // ball out, so it stays embedded forever.
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.AddHitObject(std::make_unique<HitLineZ>(nullptr, Vertex2D(100.f, 50.f), 0.f, 50.f));
   Ball *const ball = harness.AddBall(105.f, 50.f, 0.f); // center 5 VPU off the joint axis
   harness.Start();

   harness.AdvanceMs(200);

   const Vertex3Ds d = ball->m_hitBall.m_d.m_pos - Vertex3Ds(100.f, 50.f, ball->m_hitBall.m_d.m_pos.z);
   CHECK(d.Length() >= doctest::Approx(DEFAULT_BALL_SIZE).epsilon(0.2));
}

// ---------------------------------------------------------------------------
// The KD-tree SSE leaf loop used to scan 4-item groups covering
// [0, start+items+3) instead of the node's own [start, start+items) range, so
// items at group boundaries were HitTest'ed multiple times per query. For
// contact events that multiplied recorded contacts (and therefore contact
// impulses).
// ---------------------------------------------------------------------------

namespace
{

void CheckKdLeafVisitsItemsOnce()
{
   CountingHitObject objects[5];
   // x bounds: one split-plane straddler, two left items, two right items.
   const float boxes[5][2] = { { -40.f, 40.f }, { -25.f, -15.f }, { -34.f, -24.f }, { 15.f, 25.f }, { 24.f, 34.f } };
   vector<HitObject *> vho;
   for (int i = 0; i < 5; ++i)
   {
      objects[i].m_hitBBox.left = boxes[i][0];
      objects[i].m_hitBBox.right = boxes[i][1];
      objects[i].m_hitBBox.top = -5.f;
      objects[i].m_hitBBox.bottom = 5.f;
      objects[i].m_hitBBox.zlow = 0.f;
      objects[i].m_hitBBox.zhigh = 50.f;
      vho.push_back(&objects[i]);
   }

   HitKD tree;
   tree.Reset(vho);
   REQUIRE(tree.GetNLevels() >= 1); // 5 items must subdivide for the test to be meaningful

   HitBall ball;
   ball.m_d.m_pos = Vertex3Ds(0.f, 0.f, 25.f);
   ball.m_d.m_vel.SetZero();
   ball.CalcHitBBox(); // query sphere ~25 VPU around the split plane: overlaps every item

   CollisionEvent coll;
   coll.m_hittime = 1.f;
   tree.HitTestBall(&ball, coll);

   for (int i = 0; i < 5; ++i)
      CHECK(objects[i].m_hitTestCount == 1);
}

} // namespace

// The duplicate visits only happened under the SSE leaf path.
TEST_CASE("KD leaf scan visits each overlapping item exactly once")
{
   CheckKdLeafVisitsItemsOnce();
}

// ---------------------------------------------------------------------------
// LineSegSlingshot::Collide adds its m_force kick to the incoming velocity
// *before* Collide3DWall reflects it, so the delivered kick is multiplied by
// (1 + elasticity) although kick and restitution are independent inputs.
// ---------------------------------------------------------------------------

TEST_CASE("Slingshot kick does not scale with elasticity" * doctest::should_fail())
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);

   Surface *const surface = harness.AddWallSegment(600.f, 400.f, 600.f, 600.f);
   surface->m_d.m_slingshot_threshold = 0.f;
   surface->m_disabled = false;
   harness.Start();

   LineSegSlingshot sling(surface, Vertex2D(600.f, 400.f), Vertex2D(600.f, 600.f), 0.f, 100.f);
   sling.m_force = 10.f;
   sling.m_elasticity = 0.5f;
   sling.m_elasticityFalloff = 0.f;
   sling.SetFriction(0.f);
   sling.m_scatter = 0.f;

   HitBall ball;
   ball.m_physics = harness.GetEngine();
   ball.m_d.m_pos = Vertex3Ds(570.f, 500.f, 25.f); // mid segment: force shape = 0.5
   ball.m_d.m_vel = Vertex3Ds(100.f, 0.f, 0.f);

   CollisionEvent coll;
   coll.m_ball = &ball;
   coll.m_hitnormal = Vertex3Ds(-1.f, 0.f, 0.f);
   coll.m_hittime = 0.f;

   sling.Collide(coll);
   // kick = 0.5*10 = 5 applied pre-reflection -> approach 105 -> out 105*0.5 = 52.5.
   // A post-reflection kick (its actual contract) gives 0.5*100 + 5 = 55.
   CHECK(ball.m_d.m_vel.x == doctest::Approx(-55.f));
}

// ---------------------------------------------------------------------------
// KickerHitCircle::DoCollide computes the capture ceiling as
// (zlow + ballRadius) * hitAccuracy. With the default sub-1 hit accuracy a
// ball rolling at floor height sits above grabHeight and is deflected instead
// of captured by a non-legacy kicker.
// ---------------------------------------------------------------------------

TEST_CASE("A floor-height ball entering a non-legacy kicker is captured" * doctest::should_fail())
{
   PhysicsTestHarness harness;

   Kicker *const kicker = Kicker::COMCreate();
   kicker->Init(500.f, 500.f, false);
   kicker->SetName("TestKicker");
   harness.GetTable()->AddPart(kicker);
   kicker->Release(); // owned by the table
   kicker->m_d.m_legacyMode = false;
   kicker->m_d.m_fallThrough = true; // avoids the locked-ball path (which dereferences g_pplayer)
   kicker->m_d.m_hitAccuracy = 0.5f; // saturated [0,1] property, default 0.5

   KickerHitCircle sling(kicker, Vertex2D(500.f, 500.f), 50.f, 0.f, 100.f);
   sling.m_pkicker = kicker;
   sling.m_obj = static_cast<IFireEvents *>(kicker);
   sling.CalcHitBBox();

   HitBall ball;
   ball.m_d.m_pos = Vertex3Ds(500.f, 500.f, DEFAULT_BALL_SIZE); // resting at the kicker floor
   ball.m_d.m_vel = Vertex3Ds(1.f, 0.f, 0.f);

   // Entering the kicker volume (hitflag false = Hit, not UnHit).
   sling.DoCollide(&ball, Vertex3Ds(0.f, 1.f, 0.f), false, false);

   // grabHeight = (0 + 25) * 0.5 = 12.5 < pos.z = 25 -> !hitEvent -> deflection
   // instead of capture. On capture a fall-through kicker teleports the ball
   // below the hole (zlow - radius - 5) and zeroes its velocity.
   CHECK(ball.m_d.m_pos.z == doctest::Approx(-DEFAULT_BALL_SIZE - 5.f).epsilon(0.01));
   CHECK(ball.m_d.m_vel.LengthSquared() == doctest::Approx(0.f));
}

// ---------------------------------------------------------------------------
// HitTestFlipperEnd rejects every receding contact (bnv >= 0) before the
// contact flag is considered, while HitTestFlipperFace accepts contacts with
// |bnv| <= C_CONTACTVEL: a ball resting on the tip gets no contact support.
// ---------------------------------------------------------------------------

TEST_CASE("A resting ball is held by the flipper face")
{
   // No simulation needed: HitFlipper's mover only reads the part/table physics
   // overrides, which Init+AddPart provide.
   PhysicsTestHarness harness;

   Flipper *const flipperPart = Flipper::COMCreate();
   flipperPart->Init(0.f, 0.f, false);
   flipperPart->SetName("TestFlipperFace");
   harness.GetTable()->AddPart(flipperPart);
   flipperPart->Release(); // owned by the table

   // Flipper at angle 0, pointing toward -y: face2 (face1=false) on the +x side.
   HitFlipper flipper(Vertex2D(500.f, 500.f), 30.f, 15.f, 80.f, 0.f, 0.f, 0.f, 100.f, flipperPart);
   flipper.CalcHitBBox();

   const Vertex2D base(500.f, 500.f);
   const Vertex2D F(flipper.m_flipperMover.m_zeroAngNorm); // face2 normal at angle 0
   const Vertex2D T(F.y, -F.x); // face tangent toward the tip
   const float faceLen = 80.f * F.x; // flipperradius * zeroAngNorm.x
   const Vertex2D facePoint = base + F * 30.f + T * (faceLen * 0.5f); // middle of the face segment

   SUBCASE("resting ball")
   {
      BallS ball = MakeBallState(Vertex3Ds(facePoint.x + F.x * (DEFAULT_BALL_SIZE + 0.02f), facePoint.y + F.y * (DEFAULT_BALL_SIZE + 0.02f), 25.f), Vertex3Ds(0.f, 0.f, 0.f));
      CollisionEvent coll;
      const float hittime = flipper.HitTestFlipperFace(ball, 1.f, coll, false);
      CHECK(hittime >= 0.f);
      CHECK(coll.m_isContact);
   }

   SUBCASE("slowly receding ball")
   {
      BallS ball = MakeBallState(
         Vertex3Ds(facePoint.x + F.x * (DEFAULT_BALL_SIZE + 0.02f), facePoint.y + F.y * (DEFAULT_BALL_SIZE + 0.02f), 25.f), Vertex3Ds(F.x * 0.05f, F.y * 0.05f, 0.f)); // below C_CONTACTVEL
      CollisionEvent coll;
      const float hittime = flipper.HitTestFlipperFace(ball, 1.f, coll, false);
      CHECK(hittime >= 0.f);
      CHECK(coll.m_isContact);
   }
}

TEST_CASE("The flipper end cap provides the same contact support" * doctest::should_fail())
{
   PhysicsTestHarness harness;

   Flipper *const flipperPart = Flipper::COMCreate();
   flipperPart->Init(0.f, 0.f, false);
   flipperPart->SetName("TestFlipperEnd");
   harness.GetTable()->AddPart(flipperPart);
   flipperPart->Release(); // owned by the table

   // End cap at angle 0 sits at base + (0, -flipperradius) = (500, 420).
   HitFlipper flipper(Vertex2D(500.f, 500.f), 30.f, 15.f, 80.f, 0.f, 0.f, 0.f, 100.f, flipperPart);
   flipper.CalcHitBBox();
   const Vertex2D endCap(500.f, 420.f);
   const float touchDist = 15.f + DEFAULT_BALL_SIZE + 0.02f; // end radius + ball radius, touching

   SUBCASE("resting ball")
   {
      const BallS ball = MakeBallState(Vertex3Ds(endCap.x, endCap.y - touchDist, 25.f), Vertex3Ds(0.f, 0.f, 0.f));
      CollisionEvent coll;
      CHECK(flipper.HitTestFlipperEnd(ball, 1.f, coll) >= 0.f); // bnv = 0 -> rejected today
   }

   SUBCASE("slowly receding ball")
   {
      const BallS ball = MakeBallState(Vertex3Ds(endCap.x, endCap.y - touchDist, 25.f), Vertex3Ds(0.f, -0.05f, 0.f));
      CollisionEvent coll;
      CHECK(flipper.HitTestFlipperEnd(ball, 1.f, coll) >= 0.f); // bnv > 0 -> rejected today
   }
}

// ---------------------------------------------------------------------------
// Trigger/kicker volume-edge events teleport the ball by STATICTIME*vel —
// a position change that no velocity ever integrated. The same hack
// exists in the Hit3DPoly volume path, TriggerHitLine and the kicker.
// ---------------------------------------------------------------------------

TEST_CASE("Entering a trigger volume does not teleport the ball" * doctest::should_fail())
{
   // No simulation needed: TriggerHitCircle::Collide only touches the ball and
   // the trigger's event bookkeeping.
   PhysicsTestHarness harness;

   Trigger *const trigger = Trigger::COMCreate();
   trigger->Init(500.f, 500.f, false, false);
   trigger->SetName("TestTrigger");
   harness.GetTable()->AddPart(trigger);
   trigger->Release(); // owned by the table

   TriggerHitCircle triggerVolume(trigger, Vertex2D(500.f, 500.f), 50.f, 0.f, 100.f);
   triggerVolume.m_ObjType = eTrigger;
   triggerVolume.m_obj = static_cast<IFireEvents *>(trigger);

   HitBall ball;
   ball.m_d.m_pos = Vertex3Ds(500.f, 500.f, DEFAULT_BALL_SIZE);
   ball.m_d.m_vel = Vertex3Ds(100.f, 0.f, 0.f);

   CollisionEvent coll;
   coll.m_ball = &ball;
   coll.m_hitflag = false; // Hit: ball was not inside the volume yet

   triggerVolume.Collide(coll);

   // The event does record the ball inside the volume ...
   REQUIRE(ball.m_d.m_vpVolObjs->size() == 1);
   // ... but reporting the edge crossing must not displace it. The code adds
   // STATICTIME*vel (~0.2 ms of travel) "to move ball slightly forward" — a
   // position change outside the integrator that also shifts where the ball's
   // next hit tests run from.
   CHECK(ball.m_d.m_pos.x == doctest::Approx(500.f));
}

// ---------------------------------------------------------------------------
// HitFlipper::Contact feeds HitBall::SurfaceAcceleration — which includes the
// centripetal term w x (w x rB) of the ball's *material* surface point — into
// the contact separation test. A fast spinning ball reads as "accelerating
// away" and the contact early-outs without even cancelling the approach
// velocity.
// ---------------------------------------------------------------------------

TEST_CASE("A spinning ball keeps flipper contact support" * doctest::should_fail())
{
   PhysicsTestHarness harness;

   Flipper *const flipperPart = Flipper::COMCreate();
   flipperPart->Init(0.f, 0.f, false);
   flipperPart->SetName("TestFlipperSpin");
   harness.GetTable()->AddPart(flipperPart);
   flipperPart->Release(); // owned by the table
   harness.Start();

   HitFlipper flipper(Vertex2D(500.f, 500.f), 30.f, 15.f, 80.f, 0.f, 0.f, 0.f, 100.f, flipperPart);
   const Vertex2D F(flipper.m_flipperMover.m_zeroAngNorm);
   const Vertex3Ds normal(F.x, F.y, 0.f);

   // Same contact state for both balls: slowly approaching the flipper face.
   auto makeContact = [&](HitBall &ball)
   {
      ball.m_physics = harness.GetEngine();
      ball.m_d.m_vel = -0.05f * normal;

      CollisionEvent coll;
      coll.m_ball = &ball;
      coll.m_hitnormal = normal;
      coll.m_hitdistance = 0.02f;
      coll.m_hit_org_normalvelocity = -0.05f;
      flipper.Contact(coll, (float)PHYS_FACTOR);
   };

   HitBall nonSpinning;
   nonSpinning.m_angularmomentum.SetZero();
   makeContact(nonSpinning);
   // Sanity check: without spin the contact kills the approach velocity.
   CHECK(nonSpinning.m_d.m_vel.Dot(normal) > -0.01f);

   HitBall spinning;
   // angular velocity w = am/I with I = 0.4*r^2*m = 250: am=1000 gives w=4,
   // a centripetal acceleration of w^2*r = 400 along the normal >> gravity.
   spinning.m_angularmomentum = Vertex3Ds(0.f, 0.f, 1000.f);
   makeContact(spinning);
   // The material surface point's centripetal acceleration is not gap
   // acceleration: spin must not release the contact. Today normAcc >= 0
   // early-outs before the approach-velocity cancellation, leaving the full
   // -0.05 approach velocity in place.
   CHECK(spinning.m_d.m_vel.Dot(normal) > -0.01f);
}
