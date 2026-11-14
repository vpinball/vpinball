// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"
#include "physics-harness.h"

#include "parts/ball.h"
#include "parts/gate.h"
#include "parts/pintable.h"
#include "physics/PhysicsEngine.h"
#include "physics/collide.h"
#include "physics/hitball.h"
#include "physics/physconst.h"

#include "doctest.h"

// Gate setup used by all tests: axis along x at (500,1000), pivot at z = playfield + 60,
// the hanging wire/plate swings along y (negative applied angle tips it toward -y)
static Gate *AddTestGate(PhysicsTestHarness &harness, const bool twoWay, const float minDeg = 0.f, const float maxDeg = 90.f, const GateType type = GateWireW)
{
   Gate *const gate = Gate::COMCreate();
   gate->Init(500.f, 1000.f, false);
   gate->SetName("Gate");
   gate->m_d.m_length = 90.f;
   gate->m_d.m_height = 60.f;
   gate->m_d.m_rotation = 0.f;
   gate->m_d.m_twoWay = twoWay;
   gate->m_d.m_collidable = true;
   gate->m_d.m_showBracket = false;
   gate->m_d.m_type = type;
   gate->m_d.m_angleMin = ANGTORAD(minDeg);
   gate->m_d.m_angleMax = ANGTORAD(maxDeg);
   gate->m_d.m_damping = 0.999f;
   gate->m_d.m_gravityfactor = 1.f;
   harness.GetTable()->AddPart(gate);
   gate->Release();
   return gate;
}

static float GateAngleDeg(const Gate *gate)
{
   float angle = 0.f;
   const_cast<Gate *>(gate)->get_CurrentAngle(&angle);
   return angle;
}

// The dedicated one-sided segment standing for the wire/plate pressed on its
// stop (10.9 profile): the only LineSeg collider owned by the gate
static LineSeg *FindBlockSeg(PhysicsTestHarness &harness, const Gate *gate)
{
   for (HitObject *const hobj : harness.GetEngine()->GetHitObjects())
      if (LineSeg *const seg = dynamic_cast<LineSeg *>(hobj); seg && seg->m_editable == gate)
         return seg;
   return nullptr;
}

static float MaxAngleDuring(PhysicsTestHarness &harness, const Gate *gate, const uint32_t ms)
{
   float peak = 0.f;
   for (uint32_t i = 0; i < ms; ++i)
   {
      harness.AdvanceMs(1);
      peak = max(peak, fabsf(GateAngleDeg(gate)));
   }
   return peak;
}

TEST_CASE("Gate: one-way gate (10.9)")
{
   SUBCASE("ball rolling through the open side passes and drags the wire")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f); // flat: isolate the gate interaction
      Gate *const gate = AddTestGate(harness, false);
      Ball *const ball = harness.AddBall(500.f, 1300.f, 0.f, 0.f, -30.f, 0.f); // one-way gates open toward -y
      harness.Start();

      const float peak = MaxAngleDuring(harness, gate, 160);
      CHECK(peak > 60.f); // the ball dragged the wire to follow its contact
      CHECK(harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_pos.y < 600.f; }, 300));
      CHECK(ball->m_hitBall.m_d.m_vel.y == doctest::Approx(-30.f).epsilon(0.01)); // the wire follows kinematically, ball unaffected
   }

   SUBCASE("an open one-way gate does not block: the segment follows the wire position")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false);
      Ball *const ball = harness.AddBall(500.f, 1300.f, 0.f, 0.f, -60.f, 0.f);
      harness.Start();

      CHECK(harness.AdvanceUntil([&] { return fabsf(GateAngleDeg(gate)) > 40.f; }, 300));
      ball->m_hitBall.m_d.m_vel = Vertex3Ds(0.f, 60.f, 0.f); // sent back down-table while the wire is still up
      // While the wire is off its stop, the blocking segment must not stand,
      // even when no ball is being followed
      LineSeg *const blockseg = FindBlockSeg(harness, gate);
      REQUIRE(blockseg != nullptr);
      CHECK(harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_pos.y > 1150.f && fabsf(GateAngleDeg(gate)) > 5.f && !blockseg->m_enabled; }, 300));
      ball->m_hitBall.m_d.m_vel = Vertex3Ds(0.f, -60.f, 0.f); // and straight back through the still-raised wire
      bool passed = harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_pos.y < 950.f; }, 500);
      INFO("wire angle when stopped: ", GateAngleDeg(gate), " ball y: ", ball->m_hitBall.m_d.m_pos.y);
      CHECK(passed);
   }

   SUBCASE("ball rolling toward the closed side bounces on the wire")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false);
      Ball *const ball = harness.AddBall(500.f, 700.f, 0.f, 0.f, 30.f, 0.f);
      harness.Start();

      harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_vel.y < 0.f; }, 500);
      CHECK(ball->m_hitBall.m_d.m_vel.y < 0.f);
      harness.AdvanceMs(100);
      CHECK(ball->m_hitBall.m_d.m_pos.y < 1000.f); // never crossed the gate axis
   }

   SUBCASE("fast ball rolling toward the closed side does not tunnel through the wire")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false);
      Ball *const ball = harness.AddBall(500.f, 700.f, 0.f, 0.f, 600.f, 0.f); // 60 VPU per substep: jumps over the wire's contact disc
      harness.Start();

      CHECK(harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_vel.y < 0.f; }, 500)); // bounced
      bool crossed = false;
      harness.AdvanceUntil(
         [&]
         {
            crossed = crossed || ball->m_hitBall.m_d.m_pos.y > 1000.f;
            return false;
         },
         400);
      CHECK(!crossed);
   }

   SUBCASE("ball dropped on the wire on the blocked side does not cross")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 1.f); // straight down
      Gate *const gate = AddTestGate(harness, false);
      Ball *const ball = harness.AddBall(500.f, 985.f, 60.f, 0.f, 0.f, 0.f); // dropped on the wire, blocked side
      harness.Start();

      float peakAngle = 0.f;
      bool crossed = false;
      harness.AdvanceUntil(
         [&]
         {
            peakAngle = max(peakAngle, fabsf(GateAngleDeg(gate)));
            crossed = crossed || ball->m_hitBall.m_d.m_pos.y > 1000.f;
            return false;
         },
         1500);
      CHECK(peakAngle < 10.f); // pressing the wire/plate through must not swing it open
      CHECK(!crossed);
   }

   SUBCASE("ball resting against the closed side never creeps through")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(8.f, 4.f); // tilted table: sustained pressure into the gate
      AddTestGate(harness, false);
      Ball *const ball = harness.AddBall(500.f, 960.f, 0.f, 0.f, 0.f, 0.f); // just short of the wire
      harness.Start();

      int phase = 0;
      bool crossed = false;
      harness.AdvanceUntil(
         [&]
         {
            ++phase;
            ball->m_hitBall.m_d.m_vel.x = (phase % 60 < 30) ? 8.f : -8.f; // table shaken side to side
            crossed = crossed || ball->m_hitBall.m_d.m_pos.y > 1000.f;
            return false;
         },
         2000);
      CHECK(!crossed);
   }

   SUBCASE("plate gate blocks the closed side on its bottom rim")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false, 0.f, 90.f, GatePlate);
      Ball *const ball = harness.AddBall(500.f, 700.f, 0.f, 0.f, 30.f, 0.f);
      harness.Start();

      harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_vel.y < 0.f; }, 500);
      CHECK(ball->m_hitBall.m_d.m_vel.y < 0.f);
      harness.AdvanceMs(100);
      CHECK(ball->m_hitBall.m_d.m_pos.y < 1000.f);
   }

   SUBCASE("plate gate lets the ball through the open side")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false, 0.f, 90.f, GatePlate);
      Ball *const ball = harness.AddBall(500.f, 1300.f, 0.f, 0.f, -30.f, 0.f);
      harness.Start();

      const float peak = MaxAngleDuring(harness, gate, 160);
      CHECK(peak > 45.f);
      CHECK(harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_pos.y < 600.f; }, 300));
   }

   SUBCASE("wire returns to its rest position once the ball leaves")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false);
      harness.AddBall(500.f, 1300.f, 0.f, 0.f, -30.f, 0.f);
      harness.Start();

      harness.AdvanceMs(500); // pass + settle
      CHECK(harness.AdvanceUntil([&] { return fabsf(GateAngleDeg(gate) - 0.f) < 2.f; }, 3000));
   }

   SUBCASE("wire lifted by a ball that falls back returns to its rest position")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false);
      Ball *const ball = harness.AddBall(500.f, 1080.f, 0.f, 0.f, -60.f, 0.f); // pushes the wire up...
      harness.Start();

      CHECK(harness.AdvanceUntil([&] { return fabsf(GateAngleDeg(gate)) > 30.f; }, 300));
      ball->m_hitBall.m_d.m_vel = Vertex3Ds(0.f, 60.f, 0.f); // ...but rolls back before clearing the wire
      harness.AdvanceMs(300); // ball retreats out of the zone
      CHECK(harness.AdvanceUntil([&] { return fabsf(GateAngleDeg(gate) - 0.f) < 2.f; }, 3000)); // wire falls back to its stop
   }
}

TEST_CASE("Gate: two-way gate (10.9)")
{
   SUBCASE("ball passes both ways and the wire swings in the travel direction")
   {
      for (const float dir : { -1.f, 1.f })
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         Gate *const gate = AddTestGate(harness, true);
         Ball *const ball = harness.AddBall(500.f, 1000.f - dir * 300.f, 0.f, 0.f, dir * 30.f, 0.f);
         harness.Start();

         float peak = 0.f;
         for (uint32_t i = 0; i < 160; ++i)
         {
            harness.AdvanceMs(1);
            peak = max(peak, GateAngleDeg(gate) * dir);
         }
         CHECK(peak > 60.f); // wire dragged downstream
         CHECK(harness.AdvanceUntil([&] { return (ball->m_hitBall.m_d.m_pos.y - 1000.f) * dir > 250.f; }, 300));
         CHECK(ball->m_hitBall.m_d.m_vel.y == doctest::Approx(dir * 30.f).epsilon(0.01));
      }
   }

   SUBCASE("wire stays between its limits when pushed to the max angle")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, true, 0.f, 45.f);
      Ball *const ball = harness.AddBall(500.f, 1300.f, 0.f, 0.f, -30.f, 0.f);
      harness.Start();

      const float peak = MaxAngleDuring(harness, gate, 200);
      CHECK(peak == doctest::Approx(45.f).epsilon(0.1));
      CHECK(ball->m_hitBall.m_d.m_pos.y > 900.f); // a wire clamped at 45 deg does not clear a full size ball: it bounces
   }
}

TEST_CASE("Gate: limits and tracking (10.9)")
{
   SUBCASE("nonzero min angle shifts the rest position but keeps the gate functional")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false, 20.f, 90.f);
      Ball *const ball = harness.AddBall(500.f, 1300.f, 0.f, 0.f, -30.f, 0.f);
      harness.Start();

      CHECK(GateAngleDeg(gate) == doctest::Approx(20.f).epsilon(0.01)); // rests at min
      const float peak = MaxAngleDuring(harness, gate, 160);
      CHECK(peak > 60.f);
      CHECK(harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_pos.y < 600.f; }, 500));
   }

   SUBCASE("several balls in the zone are tracked independently")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, true);
      Ball *const b1 = harness.AddBall(490.f, 1300.f, 0.f, 0.f, -30.f, 0.f);
      Ball *const b2 = harness.AddBall(510.f, 1400.f, 0.f, 0.f, -30.f, 0.f);
      harness.Start();

      CHECK(harness.AdvanceUntil([&] { return b1->m_hitBall.m_d.m_pos.y < 600.f; }, 800));
      CHECK(harness.AdvanceUntil([&] { return b2->m_hitBall.m_d.m_pos.y < 600.f; }, 800));
      CHECK(b1->m_hitBall.m_d.m_vel.y == doctest::Approx(-30.f).epsilon(0.01));
      CHECK(b2->m_hitBall.m_d.m_vel.y == doctest::Approx(-30.f).epsilon(0.01));
   }

   SUBCASE("non-collidable gate lets balls through untouched")
   {
      PhysicsTestHarness harness;
      harness.SetGravity(0.f, 0.f);
      Gate *const gate = AddTestGate(harness, false);
      gate->m_d.m_collidable = false;
      Ball *const ball = harness.AddBall(500.f, 700.f, 0.f, 0.f, 30.f, 0.f); // closed side for a one-way gate
      harness.Start();

      CHECK(harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_pos.y > 1400.f; }, 800));
      CHECK(GateAngleDeg(gate) == doctest::Approx(0.f).epsilon(0.01));
   }
}

TEST_CASE("Gate: legacy rigid segments (10.8)")
{
   SUBCASE("one-way gate blocks the closed side and lets the open side through")
   {
      for (const float dir : { -1.f, 1.f })
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = PhysicProfile::V10_8;
         Gate *const gate = AddTestGate(harness, false);
         Ball *const ball = harness.AddBall(500.f, 1000.f - dir * 300.f, 0.f, 0.f, dir * 30.f, 0.f);
         harness.Start();

         harness.AdvanceMs(500);
         if (dir < 0.f) // open side: passes
            CHECK(ball->m_hitBall.m_d.m_pos.y < 800.f);
         else // closed side: bounced back
            CHECK(ball->m_hitBall.m_d.m_pos.y < 1000.f);
      }
   }

   SUBCASE("two-way gate lets the ball through both ways")
   {
      for (const float dir : { -1.f, 1.f })
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = PhysicProfile::V10_8;
         Gate *const gate = AddTestGate(harness, true);
         Ball *const ball = harness.AddBall(500.f, 1000.f - dir * 300.f, 0.f, 0.f, dir * 30.f, 0.f);
         harness.Start();

         harness.AdvanceMs(500);
         CHECK((ball->m_hitBall.m_d.m_pos.y - 1000.f) * dir > 200.f);
      }
   }
}
