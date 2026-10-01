// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"
#include "physics-harness.h"

#include "parts/ball.h"
#include "parts/pintable.h"
#include "parts/trigger.h"
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

   SUBCASE("ball bounces off a wall with a reversed point order")
   {
      // Same wall outline wound the other way around, like when the drag points get
      // reversed in the editor: the colliders must still face outward
      harness.AddWall({ Vertex2D(700.f, 400.f), Vertex2D(900.f, 400.f), Vertex2D(900.f, 600.f), Vertex2D(700.f, 600.f) }, 0.f, 100.f);
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
      CHECK(maxX < 700.f); // the ball never entered the wall (contact at ~675)
   }

   SUBCASE("ball lands on a wall top with a reversed point order")
   {
      harness.AddWall({ Vertex2D(700.f, 400.f), Vertex2D(900.f, 400.f), Vertex2D(900.f, 600.f), Vertex2D(700.f, 600.f) }, 0.f, 100.f);
      Ball *const ball = harness.AddBall(800.f, 500.f, 400.f); // dropped above the wall top face
      harness.Start();

      harness.AdvanceMs(2000);

      const BallS &d = ball->m_hitBall.m_d;
      CHECK(d.m_pos.z == doctest::Approx(100.f + DEFAULT_BALL_SIZE).epsilon(0.1)); // rests on the wall top
      CHECK(d.m_vel.Length() < 1.f);
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

TEST_CASE("PhysicsEngine: trigger volume")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, GRAVITYCONST); // flat playfield: gravity only holds the ball down

   const auto checkTriggerCrossing = [&](const vector<Vertex2D> &outline)
   {
      Trigger *const trigger = Trigger::COMCreate();
      trigger->Init(0.f, 0.f, false);
      trigger->SetName("Trigger");
      trigger->m_d.m_shape = TriggerNone; // use the custom outline, not a predefined shape
      trigger->m_curve.ClearPoints();
      for (const Vertex2D &v : outline)
         trigger->m_curve.PushPoint(std::make_unique<DragPoint>(&trigger->m_curve, v.x, v.y, 0.f, false));
      harness.GetTable()->AddPart(trigger);
      trigger->Release(); // owned by the table

      Ball *const ball = harness.AddBall(400.f, 500.f, 0.f, 100.f, 0.f, 0.f); // +x through the [700,900]x[400,600] footprint
      harness.Start();

      // The ball must be registered as inside the trigger volume
      REQUIRE(harness.AdvanceUntil([&]() { return ball->m_hitBall.m_d.m_pos.x > 750.f; }, 2000));
      CHECK(!ball->m_hitBall.m_d.m_vpVolObjs->empty());

      // and unregistered after leaving it
      REQUIRE(harness.AdvanceUntil([&]() { return ball->m_hitBall.m_d.m_pos.x > 925.f; }, 2000));
      CHECK(ball->m_hitBall.m_d.m_vpVolObjs->empty());
   };

   SUBCASE("canonical point order") { checkTriggerCrossing({ Vertex2D(700.f, 400.f), Vertex2D(700.f, 600.f), Vertex2D(900.f, 600.f), Vertex2D(900.f, 400.f) }); }

   SUBCASE("reversed point order")
   {
      // Outline wound the other way around: Hit and UnHit must not be swapped
      checkTriggerCrossing({ Vertex2D(700.f, 400.f), Vertex2D(900.f, 400.f), Vertex2D(900.f, 600.f), Vertex2D(700.f, 600.f) });
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

TEST_CASE("PhysicsEngine: stacked balls come to rest")
{
   PhysicsTestHarness harness;
   harness.SetGravity(6.5f, GRAVITYCONST); // slope toward the bottom of the table (+y)

   // A single U-shaped wall (concave outline): a pocket opened up-table, 104 wide
   // inside so two balls fit side by side on the playfield.
   harness.AddWall({ Vertex2D(448.f, 1500.f), Vertex2D(448.f, 1898.f), Vertex2D(552.f, 1898.f), Vertex2D(552.f, 1500.f), Vertex2D(560.f, 1500.f), Vertex2D(560.f, 1910.f),
                      Vertex2D(440.f, 1910.f), Vertex2D(440.f, 1500.f) },
      0.f, 100.f);
   // A front bar closes the pocket, making the box only 55 deep: the third ball
   // can neither pass between the two others nor rest on the playfield in front of
   // them, so it must stack on top of them, wedged by the slope.
   harness.AddWallSegment(444.f, 1837.f, 556.f, 1837.f, 0.f, 100.f, 6.f);

   // Two balls at the bottom of the pocket, the third one dropped on top of them, all spinning
   Ball *const bottomLeft = harness.AddBall(475.f, 1870.f, 0.f);
   Ball *const bottomRight = harness.AddBall(525.f, 1870.f, 0.f);
   Ball *const top = harness.AddBall(500.f, 1870.f, 55.f);
   bottomLeft->m_hitBall.m_angularmomentum = Vertex3Ds(400.f, -500.f, 300.f);
   bottomRight->m_hitBall.m_angularmomentum = Vertex3Ds(-350.f, 600.f, -250.f);
   top->m_hitBall.m_angularmomentum = Vertex3Ds(500.f, -300.f, 450.f);
   harness.Start();

   // Let the pile settle
   harness.AdvanceMs(12000);

   const HitBall *const balls[3] = { &bottomLeft->m_hitBall, &bottomRight->m_hitBall, &top->m_hitBall };
   for (const HitBall *const ball : balls)
   {
      INFO("ball pos ", ball->m_d.m_pos.x, ' ', ball->m_d.m_pos.y, ' ', ball->m_d.m_pos.z, " vel ", ball->m_d.m_vel.x, ' ', ball->m_d.m_vel.y, ' ', ball->m_d.m_vel.z, " angvel ",
         (ball->m_angularmomentum / ball->Inertia()).x, ' ', (ball->m_angularmomentum / ball->Inertia()).y, ' ', (ball->m_angularmomentum / ball->Inertia()).z);

      // the mutual contacts led to a stable situation: the ball lost its speed and its spin
      CHECK(ball->m_d.m_vel.Length() < 0.5f);
      CHECK((ball->m_angularmomentum / ball->Inertia()).Length() < 0.1f);

      // and stayed inside the pocket
      CHECK(ball->m_d.m_pos.x > 460.f);
      CHECK(ball->m_d.m_pos.x < 540.f);
      CHECK(ball->m_d.m_pos.y > 1840.f);
      CHECK(ball->m_d.m_pos.y < 1880.f);
   }

   const BallS &dLeft = bottomLeft->m_hitBall.m_d;
   const BallS &dRight = bottomRight->m_hitBall.m_d;
   const BallS &dTop = top->m_hitBall.m_d;

   // The stack held: two balls rest on the playfield, the third on top of them
   CHECK(dLeft.m_pos.z == doctest::Approx(DEFAULT_BALL_SIZE).epsilon(0.05));
   CHECK(dRight.m_pos.z == doctest::Approx(DEFAULT_BALL_SIZE).epsilon(0.05));
   CHECK(dTop.m_pos.z > DEFAULT_BALL_SIZE + 25.f);

   // Mutual contacts are maintained without interpenetration
   CHECK((dTop.m_pos - dLeft.m_pos).Length() == doctest::Approx(2.f * DEFAULT_BALL_SIZE).epsilon(0.1));
   CHECK((dTop.m_pos - dRight.m_pos).Length() == doctest::Approx(2.f * DEFAULT_BALL_SIZE).epsilon(0.1));
   CHECK((dLeft.m_pos - dRight.m_pos).Length() > 2.f * DEFAULT_BALL_SIZE - 1.f);

   // The pile is stable: positions do not change over time
   const Vertex3Ds posLeft = dLeft.m_pos, posRight = dRight.m_pos, posTop = dTop.m_pos;
   harness.AdvanceMs(1000);
   CHECK((dLeft.m_pos - posLeft).Length() < 0.5f);
   CHECK((dRight.m_pos - posRight).Length() < 0.5f);
   CHECK((dTop.m_pos - posTop).Length() < 0.5f);
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
