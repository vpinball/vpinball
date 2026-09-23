// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "physics/collide.h"
#include "physics/hitball.h"
#include "physics/physconst.h"

#include "doctest.h"

namespace
{

BallS MakeBall(const Vertex3Ds& pos, const Vertex3Ds& vel)
{
   BallS ball;
   ball.m_pos = pos;
   ball.m_vel = vel;
   ball.m_radius = DEFAULT_BALL_SIZE;
   ball.m_mass = 1.f;
   ball.m_vpVolObjs = nullptr; // only dereferenced for trigger/kicker hit objects
   return ball;
}

} // namespace

TEST_CASE("Elasticity with falloff")
{
   SUBCASE("no falloff leaves elasticity unchanged")
   {
      CHECK(ElasticityWithFalloff(0.4f, 0.f, 100.f) == doctest::Approx(0.4f));
      CHECK(ElasticityWithFalloff(0.8f, 0.f, -500.f) == doctest::Approx(0.8f));
   }

   SUBCASE("falloff reduces effective elasticity with velocity")
   {
      CHECK(ElasticityWithFalloff(0.5f, 1.f, 0.f) == doctest::Approx(0.5f));
      CHECK(ElasticityWithFalloff(0.5f, 1.f, 18.53f) == doctest::Approx(0.25f));
      CHECK(ElasticityWithFalloff(0.5f, 2.f, 18.53f) == doctest::Approx(0.5f / 3.f));
   }

   SUBCASE("falloff uses absolute velocity") { CHECK(ElasticityWithFalloff(0.5f, 1.f, -18.53f) == doctest::Approx(0.25f)); }
}

TEST_CASE("Line segment hit test")
{
   // Vertical wall at x=100, spanning y=[0,100], z=[0,50]
   LineSeg wall(nullptr, Vertex2D(100.f, 0.f), Vertex2D(100.f, 100.f), 0.f, 50.f);
   CollisionEvent coll;

   SUBCASE("ball approaching wall collides")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      const float hittime = wall.HitTest(ball, 1.f, coll);
      CHECK(hittime == doctest::Approx(0.25f)); // 25 VPU to travel at 100 VPU/s
      CHECK(coll.m_hitnormal.x == doctest::Approx(-1.f));
      CHECK(coll.m_hitnormal.y == doctest::Approx(0.f));
      CHECK(coll.m_hitdistance == doctest::Approx(25.f));
      CHECK(!coll.m_isContact);
   }

   SUBCASE("ball moving away does not collide")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(-100.f, 0.f, 0.f));
      CHECK(wall.HitTest(ball, 1.f, coll) < 0.f);
   }

   SUBCASE("ball travelling beyond the segment end does not collide")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 200.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      CHECK(wall.HitTest(ball, 1.f, coll) < 0.f);
   }

   SUBCASE("ball above the wall does not collide")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 100.f), Vertex3Ds(100.f, 0.f, 0.f));
      CHECK(wall.HitTest(ball, 1.f, coll) < 0.f);
   }

   SUBCASE("hit after dtime is ignored")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      CHECK(wall.HitTest(ball, 0.1f, coll) < 0.f);
   }

   SUBCASE("ball in contact reports a contact event")
   {
      const BallS ball = MakeBall(Vertex3Ds(75.02f, 50.f, 25.f), Vertex3Ds(0.f, 0.f, 0.f));
      const float hittime = wall.HitTest(ball, 1.f, coll);
      CHECK(hittime == doctest::Approx(0.f));
      CHECK(coll.m_isContact);
   }

   SUBCASE("disabled wall never collides")
   {
      wall.m_enabled = false;
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      CHECK(wall.HitTest(ball, 1.f, coll) < 0.f);
   }
}

TEST_CASE("Circle hit test")
{
   HitCircle circle(nullptr, Vertex2D(100.f, 50.f), 25.f, 0.f, 50.f);
   CollisionEvent coll;

   SUBCASE("ball approaching circle collides")
   {
      const BallS ball = MakeBall(Vertex3Ds(40.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      const float hittime = circle.HitTest(ball, 1.f, coll);
      CHECK(hittime == doctest::Approx(0.1f)); // 10 VPU to travel at 100 VPU/s
      CHECK(coll.m_hitnormal.x == doctest::Approx(-1.f));
      CHECK(coll.m_hitdistance == doctest::Approx(10.f));
   }

   SUBCASE("ball moving away does not collide")
   {
      const BallS ball = MakeBall(Vertex3Ds(40.f, 50.f, 25.f), Vertex3Ds(-100.f, 0.f, 0.f));
      CHECK(circle.HitTest(ball, 1.f, coll) < 0.f);
   }

   SUBCASE("stationary ball does not collide")
   {
      const BallS ball = MakeBall(Vertex3Ds(40.f, 50.f, 25.f), Vertex3Ds(0.f, 0.f, 0.f));
      CHECK(circle.HitTest(ball, 1.f, coll) < 0.f);
   }

   SUBCASE("ball on top of the circle center has no valid normal")
   {
      const BallS ball = MakeBall(Vertex3Ds(100.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      CHECK(circle.HitTest(ball, 1.f, coll) < 0.f);
   }
}

TEST_CASE("Line joint hit test")
{
   HitLineZ joint(nullptr, Vertex2D(100.f, 50.f), 0.f, 50.f);
   CollisionEvent coll;

   SUBCASE("ball approaching joint collides")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      const float hittime = joint.HitTest(ball, 1.f, coll);
      CHECK(hittime == doctest::Approx(0.25f));
      CHECK(coll.m_hitnormal.x == doctest::Approx(-1.f));
   }

   SUBCASE("ball out of z range does not collide")
   {
      HitLineZ highJoint(nullptr, Vertex2D(100.f, 50.f), 40.f, 50.f);
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      CHECK(highJoint.HitTest(ball, 1.f, coll) < 0.f);
   }
}

TEST_CASE("Point hit test")
{
   HitPoint point(nullptr, 100.f, 50.f, 25.f);
   CollisionEvent coll;

   SUBCASE("ball approaching point collides")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(100.f, 0.f, 0.f));
      const float hittime = point.HitTest(ball, 1.f, coll);
      CHECK(hittime == doctest::Approx(0.25f));
      CHECK(coll.m_hitnormal.x == doctest::Approx(-1.f));
   }

   SUBCASE("ball moving away does not collide")
   {
      const BallS ball = MakeBall(Vertex3Ds(50.f, 50.f, 25.f), Vertex3Ds(-100.f, 0.f, 0.f));
      CHECK(point.HitTest(ball, 1.f, coll) < 0.f);
   }
}

TEST_CASE("Hit ball")
{
   SUBCASE("ball to ball collision time")
   {
      // HitBall owns m_d.m_vpVolObjs (allocated in its ctor, deleted in the dtor): set fields, do not overwrite m_d
      HitBall moving;
      moving.m_d.m_pos = Vertex3Ds(0.f, 0.f, 25.f);
      moving.m_d.m_vel = Vertex3Ds(100.f, 0.f, 0.f);
      HitBall still;
      still.m_d.m_pos = Vertex3Ds(100.f, 0.f, 25.f);
      still.m_d.m_vel = Vertex3Ds(0.f, 0.f, 0.f);

      CollisionEvent coll;
      const float hittime = still.HitTest(moving.m_d, 1.f, coll);
      CHECK(hittime == doctest::Approx(0.5f)); // 50 VPU to travel at 100 VPU/s
      CHECK(coll.m_hitnormal.x == doctest::Approx(-1.f));
      CHECK(coll.m_hitdistance == doctest::Approx(50.f));
   }

   SUBCASE("receding balls do not collide")
   {
      HitBall moving;
      moving.m_d.m_pos = Vertex3Ds(0.f, 0.f, 25.f);
      moving.m_d.m_vel = Vertex3Ds(-100.f, 0.f, 0.f);
      HitBall still;
      still.m_d.m_pos = Vertex3Ds(100.f, 0.f, 25.f);
      still.m_d.m_vel = Vertex3Ds(0.f, 0.f, 0.f);

      CollisionEvent coll;
      CHECK(still.HitTest(moving.m_d, 1.f, coll) < 0.f);
   }

   SUBCASE("displacement integrates velocity")
   {
      HitBall ball;
      ball.m_d.m_pos = Vertex3Ds(0.f, 0.f, 25.f);
      ball.m_d.m_vel = Vertex3Ds(100.f, 50.f, 0.f);
      ball.UpdateDisplacements(0.5f);
      CHECK(ball.m_d.m_pos.x == doctest::Approx(50.f));
      CHECK(ball.m_d.m_pos.y == doctest::Approx(25.f));
      CHECK(ball.m_d.m_pos.z == doctest::Approx(25.f));
   }

   SUBCASE("hit bounding box follows position")
   {
      HitBall ball;
      ball.m_d.m_pos = Vertex3Ds(100.f, 200.f, 25.f);
      ball.m_d.m_vel = Vertex3Ds(0.f, 0.f, 0.f);
      ball.CalcHitBBox();
      // CalcHitBBox extends the box by |vel| + radius + 0.05 paranoia margin
      CHECK(ball.m_hitBBox.left == doctest::Approx(100.f - DEFAULT_BALL_SIZE - 0.05f));
      CHECK(ball.m_hitBBox.right == doctest::Approx(100.f + DEFAULT_BALL_SIZE + 0.05f));
      CHECK(ball.HitRadiusSqr() == doctest::Approx((DEFAULT_BALL_SIZE + 0.05f) * (DEFAULT_BALL_SIZE + 0.05f)));
   }
}

TEST_CASE("DoHitTest picks the earliest collision")
{
   LineSeg nearWall(nullptr, Vertex2D(100.f, 0.f), Vertex2D(100.f, 100.f), 0.f, 50.f);
   LineSeg farWall(nullptr, Vertex2D(200.f, 0.f), Vertex2D(200.f, 100.f), 0.f, 50.f);
   HitBall ball;
   ball.m_d.m_pos = Vertex3Ds(50.f, 50.f, 25.f);
   ball.m_d.m_vel = Vertex3Ds(100.f, 0.f, 0.f);

   SUBCASE("nearest hit object is reported")
   {
      CollisionEvent coll;
      coll.m_hittime = 1.f;
      DoHitTest(&ball, &farWall, coll);
      DoHitTest(&ball, &nearWall, coll);
      CHECK(coll.m_obj == &nearWall);
      CHECK(coll.m_ball == &ball);
      CHECK(coll.m_hittime == doctest::Approx(0.25f));
   }

   SUBCASE("null hit object is ignored")
   {
      CollisionEvent coll;
      coll.m_hittime = 1.f;
      DoHitTest(&ball, nullptr, coll);
      CHECK(coll.m_obj == nullptr);
      CHECK(coll.m_hittime == doctest::Approx(1.f));
   }
}
