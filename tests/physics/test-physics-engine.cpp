// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"
#include "physics-harness.h"

#include "parts/ball.h"
#include "parts/pintable.h"
#include "physics/PhysicsEngine.h"
#include "physics/collide.h"
#include "physics/hitball.h"
#include "physics/physconst.h"

#include "doctest.h"

TEST_CASE("PhysicsEngine: engine setup and stepping")
{
   PhysicsTestHarness harness;
   harness.Start();

   PhysicsEngine *const physics = harness.GetEngine();

   SUBCASE("an empty table has the cabinet bounding hit shapes")
   {
      // 4 side walls + top glass polygon (the playfield and top glass planes are not in the quadtree)
      CHECK(physics->GetHitObjects().size() == 5);
      CHECK(physics->GetBalls().empty());
   }

   SUBCASE("AdvanceMs steps the simulated clock")
   {
      CHECK(physics->GetTimeMsec() == 0);
      harness.AdvanceMs(250);
      CHECK(physics->GetTimeSec() == doctest::Approx(0.25).epsilon(1e-3));
      harness.Step();
      CHECK(physics->GetTimeSec() == doctest::Approx(0.251).epsilon(1e-3));
   }
}

TEST_CASE("PhysicsEngine: gravity")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST); // straight down, no slope

   SUBCASE("ball free fall")
   {
      harness.SetPlayfieldFloorEnabled(false); // no ground: pure free fall
      Ball *const ball = harness.AddBall(500.f, 1000.f, 800.f);
      harness.Start();

      harness.AdvanceMs(200);

      const BallS &d = ball->m_hitBall.m_d;
      // After n steps: v = -g * (n * PHYS_FACTOR) and p = p0 - g * PHYS_FACTOR^2 * n * (n + 1) / 2
      CHECK(d.m_vel.z == doctest::Approx(-GRAVITYCONST * 200.f * PHYS_FACTOR).epsilon(0.02));
      CHECK(d.m_vel.x == doctest::Approx(0.f));
      CHECK(d.m_vel.y == doctest::Approx(0.f));
      CHECK(d.m_pos.z == doctest::Approx(825.f - GRAVITYCONST * PHYS_FACTOR * PHYS_FACTOR * 200.f * 201.f * 0.5f).epsilon(0.02));
   }

   SUBCASE("ball falls, bounces and comes to rest on the playfield")
   {
      Ball *const ball = harness.AddBall(500.f, 1000.f, 100.f); // center starts at z = 100 + radius
      harness.Start();

      // Let the ball bounce (playfield elasticity is 0.25) until it rests in the contact layer
      harness.AdvanceMs(4000);

      const BallS &d = ball->m_hitBall.m_d;
      CHECK(d.m_pos.z == doctest::Approx(DEFAULT_BALL_SIZE).epsilon(0.1));
      CHECK(d.m_vel.Length() < 1.f);
   }

   SUBCASE("ball rolls down the slope")
   {
      harness.SetGravity(6.5f, GRAVITYCONST); // slope toward the bottom of the table (+y)
      Ball *const ball = harness.AddBall(500.f, 500.f, 0.f);
      harness.Start();

      harness.AdvanceMs(2000);

      const BallS &d = ball->m_hitBall.m_d;
      CHECK(d.m_pos.y > 500.f); // moved down-table
      CHECK(d.m_vel.y > 0.f); // still rolling/sliding down-table
      CHECK(d.m_pos.z == doctest::Approx(DEFAULT_BALL_SIZE).epsilon(0.1)); // stays on the playfield
   }
}

TEST_CASE("PhysicsEngine: wall collisions")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST); // flat playfield: gravity only holds the ball down

   SUBCASE("ball bounces back off a wall")
   {
      // Vertical wall across the whole table at x=800
      harness.AddWallSegment(800.f, 0.f, 800.f, 2000.f, 0.f, 100.f);
      Ball *const ball = harness.AddBall(300.f, 500.f, 0.f, 100.f, 0.f, 0.f); // +100 VPU/T toward the wall
      harness.Start();

      float maxX = 0.f;
      const bool bounced = harness.AdvanceUntil(
         [&]()
         {
            maxX = max(maxX, ball->m_hitBall.m_d.m_pos.x);
            return ball->m_hitBall.m_d.m_vel.x < 0.f;
         },
         2000);

      CHECK(bounced);
      CHECK(maxX < 800.f); // the ball never went through the wall (contact at ~775)
      CHECK(ball->m_hitBall.m_d.m_pos.x < 775.f);
   }

   SUBCASE("raw hit object: ball bounces off a circle")
   {
      harness.AddHitObject(std::make_unique<HitCircle>(nullptr, Vertex2D(500.f, 800.f), 50.f, 0.f, 100.f));
      Ball *const ball = harness.AddBall(500.f, 300.f, 0.f, 0.f, 100.f, 0.f);
      harness.Start();

      const bool bounced = harness.AdvanceUntil([&]() { return ball->m_hitBall.m_d.m_vel.y < 0.f; }, 2000);

      CHECK(bounced);
      CHECK(ball->m_hitBall.m_d.m_pos.y < 725.f + 5.f); // contact distance is 50 + 25
   }

   SUBCASE("cabinet bounds keep the ball inside the table")
   {
      // No wall added: the engine adds the table's bounding walls by itself
      Ball *const ball = harness.AddBall(500.f, 1800.f, 0.f, 0.f, 100.f, 0.f); // toward the bottom edge y=2000
      harness.Start();

      float maxY = 0.f;
      const bool bounced = harness.AdvanceUntil(
         [&]()
         {
            maxY = max(maxY, ball->m_hitBall.m_d.m_pos.y);
            return ball->m_hitBall.m_d.m_vel.y < 0.f;
         },
         2000);

      CHECK(bounced);
      CHECK(maxY < 2000.f); // contact at ~1975
   }
}

TEST_CASE("PhysicsEngine: ball to ball collision")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);

   Ball *const moving = harness.AddBall(300.f, 500.f, 0.f, 100.f, 0.f, 0.f);
   Ball *const still = harness.AddBall(600.f, 500.f, 0.f);
   harness.Start();

   // Contact happens ~25ms in (250 VPU gap at 10 VPU/ms); check shortly after,
   // before the still ball reaches the right cabinet bound.
   harness.AdvanceMs(50);

   const BallS &dMoving = moving->m_hitBall.m_d;
   const BallS &dStill = still->m_hitBall.m_d;
   // Momentum transfer with restitution 0.8 on equal masses: the hit ball takes ~90% of the speed
   CHECK(dStill.m_vel.x == doctest::Approx(90.f).epsilon(0.3));
   CHECK(dMoving.m_vel.x < dStill.m_vel.x);
   CHECK(dMoving.m_vel.x + dStill.m_vel.x == doctest::Approx(100.f).epsilon(0.2)); // momentum conservation
   CHECK((dMoving.m_pos - dStill.m_pos).Length() >= doctest::Approx(50.f).epsilon(0.1)); // balls do not overlap
}

TEST_CASE("PhysicsEngine: live changes during simulation")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST);
   harness.Start();

   SUBCASE("ball added after Start is simulated")
   {
      Ball *const ball = harness.AddBall(500.f, 1000.f, 500.f);
      CHECK(harness.GetBalls().size() == 1);
      harness.AdvanceMs(200);
      CHECK(ball->m_hitBall.m_d.m_pos.z < 525.f); // fell
   }

   SUBCASE("hit object added after Start collides")
   {
      const size_t hitObjectCount = harness.GetEngine()->GetHitObjects().size();
      harness.AddHitObject(std::make_unique<HitCircle>(nullptr, Vertex2D(500.f, 800.f), 50.f, 0.f, 100.f));
      Ball *const ball = harness.AddBall(500.f, 300.f, 0.f, 0.f, 100.f, 0.f);

      const bool bounced = harness.AdvanceUntil([&]() { return ball->m_hitBall.m_d.m_vel.y < 0.f; }, 2000);

      CHECK(bounced);
      CHECK(harness.GetEngine()->GetHitObjects().size() == hitObjectCount + 1);
   }

   SUBCASE("ball removed from the simulation is no longer stepped")
   {
      Ball *const ball = harness.AddBall(500.f, 1000.f, 500.f);
      harness.RemoveBall(ball);
      CHECK(harness.GetBalls().empty());
      harness.AdvanceMs(100); // stepping a ball-free simulation works
   }
}

TEST_CASE("PhysicsEngine: ray cast")
{
   PhysicsTestHarness harness;
   harness.AddWallSegment(800.f, 0.f, 800.f, 2000.f, 0.f, 100.f);
   harness.Start();

   vector<HitTestResult> hits;

   SUBCASE("ray crossing a wall reports it as the first hit")
   {
      harness.GetEngine()->RayCast(Vertex3Ds(100.f, 100.f, 25.f), Vertex3Ds(900.f, 100.f, 25.f), false, hits);
      REQUIRE(!hits.empty());
      CHECK(hits[0].m_obj->GetType() == eLineSeg);
      CHECK(hits[0].m_time == doctest::Approx(0.875).epsilon(0.05));
   }

   SUBCASE("ray inside the playfield does not hit anything")
   {
      harness.GetEngine()->RayCast(Vertex3Ds(100.f, 100.f, 25.f), Vertex3Ds(700.f, 100.f, 25.f), false, hits);
      CHECK(hits.empty());
   }
}
