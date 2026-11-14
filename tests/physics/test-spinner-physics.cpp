// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"
#include "physics-harness.h"

#include "parts/ball.h"
#include "parts/spinner.h"
#include "parts/pintable.h"
#include "physics/PhysicsEngine.h"
#include "physics/collide.h"
#include "physics/hitball.h"
#include "physics/physconst.h"

#include "doctest.h"

// Spinner setup used by all tests: axis along x at (500,1000), axle at z = playfield + 60,
// the plate centered on the axle swings along y (negative angle tips it toward +y)
static Spinner* AddTestSpinner(PhysicsTestHarness& harness, const float minDeg = 0.f, const float maxDeg = 0.f)
{
   Spinner* const spinner = Spinner::COMCreate();
   spinner->Init(500.f, 1000.f, false);
   spinner->SetName("Spinner");
   spinner->m_d.m_length = 90.f;
   spinner->m_d.m_height = 60.f;
   spinner->m_d.m_rotation = 0.f;
   spinner->m_d.m_showBracket = false;
   spinner->m_d.m_angleMin = minDeg;
   spinner->m_d.m_angleMax = maxDeg;
   spinner->m_d.m_damping = 0.999f;
   spinner->m_d.m_elasticity = 0.5f;
   harness.GetTable()->AddPart(spinner);
   spinner->Release();
   return spinner;
}

static float SpinnerAngleDeg(const Spinner* spinner)
{
   float angle = 0.f;
   const_cast<Spinner*>(spinner)->get_CurrentAngle(&angle);
   return angle;
}

// The plate is driven by the ball contact under all physics profiles (animation only)
TEST_CASE("Spinner: ball contact drives the plate")
{
   for (const PhysicProfile profile : { PhysicProfile::V10_8, PhysicProfile::V10_9 })
   {
      SUBCASE("a ball parked under the plate keeps it tilted")
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f); // flat: isolate the plate interaction
         harness.GetTable()->m_physicProfile = profile;
         Spinner* const spinner = AddTestSpinner(harness);
         harness.AddBall(500.f, 1000.f, 0.f); // resting right under the axle
         harness.Start();

         CHECK(harness.AdvanceUntil([&] { return fabsf(SpinnerAngleDeg(spinner)) > 20.f; }, 1000));
      }

      SUBCASE("a ball parked beside the axle keeps the plate resting on it")
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = profile;
         Spinner* const spinner = AddTestSpinner(harness);
         harness.AddBall(500.f, 985.f, 0.f); // against the plate's hanging tip
         harness.Start();

         CHECK(harness.AdvanceUntil([&] { return SpinnerAngleDeg(spinner) > 10.f; }, 1000)); // leans toward the ball
      }

      SUBCASE("a ball slowly pushed through still moves the plate")
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = profile;
         Spinner* const spinner = AddTestSpinner(harness);
         Ball* const ball = harness.AddBall(500.f, 1040.f, 0.f, 0.f, -10.f, 0.f); // slow push through the plate
         harness.Start();

         // The plate must tip away from the incoming ball (bottom end toward -y,
         // which is a positive current angle), not lean into it
         float peak = 0.f;
         harness.AdvanceUntil(
            [&]
            {
               peak = max(peak, SpinnerAngleDeg(spinner));
               return ball->m_hitBall.m_d.m_pos.y < 940.f;
            },
            2000);
         INFO("peak plate angle: ", peak, " ball y: ", ball->m_hitBall.m_d.m_pos.y);
         CHECK(peak > 10.f); // the plate is pushed aside while the ball slides under
      }

      SUBCASE("a ball pressed against a low plate still moves it")
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = profile;
         Spinner* const spinner = AddTestSpinner(harness);
         spinner->m_d.m_height = 30.f; // axle near ball-center height: contact lands on the plate's face near the pivot
         Ball* const ball = harness.AddBall(500.f, 1040.f, 0.f, 0.f, -10.f, 0.f);
         harness.Start();

         float peak = 0.f;
         harness.AdvanceUntil(
            [&]
            {
               peak = max(peak, SpinnerAngleDeg(spinner));
               return ball->m_hitBall.m_d.m_pos.y < 940.f;
            },
            2000);
         INFO("peak plate angle: ", peak, " ball y: ", ball->m_hitBall.m_d.m_pos.y);
         CHECK(peak > 5.f);
      }

      SUBCASE("a ball overlapping the plate's swing still rotates it")
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = profile;
         Spinner* const spinner = AddTestSpinner(harness);
         harness.AddBall(500.f, 975.f, 0.f); // deep inside the plate's swept disc: every rest angle sweeps it through the ball
         harness.Start();

         // The plate must still rotate to lean on the ball instead of freezing inside it
         CHECK(harness.AdvanceUntil([&] { return SpinnerAngleDeg(spinner) > 10.f; }, 1000));
      }

      SUBCASE("a slow ball leaves almost no spin on the plate")
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = profile;
         Spinner* const spinner = AddTestSpinner(harness);
         Ball* const ball = harness.AddBall(500.f, 1040.f, 0.f, 0.f, -2.f, 0.f); // slow push through the plate
         harness.Start();

         // Once the ball has cleared the plate's swept disc, the leftover spin
         // must stay proportional to the ball speed (v/h ~ 0.067 rad/10ms), not
         // keep the steering rate used to track the contact
         CHECK(harness.AdvanceUntil([&] { return SpinnerAngleDeg(spinner) > 10.f; }, 3000));
         CHECK(harness.AdvanceUntil([&] { return 1000.f - ball->m_hitBall.m_d.m_pos.y > 60.f; }, 3000));
         float maxRate = 0.f, prev = SpinnerAngleDeg(spinner);
         for (uint32_t i = 0; i < 100; ++i)
         {
            harness.AdvanceMs(1);
            float d = SpinnerAngleDeg(spinner) - prev;
            d -= 360.f * floorf(d / 360.f + 0.5f); // wrapped delta: the angle loops at 360
            maxRate = max(maxRate, fabsf(d));
            prev = SpinnerAngleDeg(spinner);
         }
         INFO("leftover plate rotation rate: ", maxRate, " deg/ms");
         CHECK(maxRate < 1.f); // ~0.4 deg/ms expected: a slow ball cannot fling the plate
      }

      SUBCASE("the plate goes back down once the ball leaves")
      {
         PhysicsTestHarness harness;
         harness.SetGravity(0.f, 0.f);
         harness.GetTable()->m_physicProfile = profile;
         Spinner* const spinner = AddTestSpinner(harness);
         Ball* const ball = harness.AddBall(500.f, 1300.f, 0.f, 0.f, -30.f, 0.f);
         harness.Start();

         CHECK(harness.AdvanceUntil([&] { return SpinnerAngleDeg(spinner) > 20.f; }, 500));
         CHECK(harness.AdvanceUntil([&] { return ball->m_hitBall.m_d.m_pos.y < 600.f; }, 500));
         CHECK(ball->m_hitBall.m_d.m_vel.y == doctest::Approx(-30.f).epsilon(0.05)); // the plate does not slow the ball
      }
   }
}

TEST_CASE("Spinner: plate respects its angle limits")
{
   PhysicsTestHarness harness;
   harness.SetGravity(0.f, 0.f);
   Spinner* const spinner = AddTestSpinner(harness, -15.f, 15.f);
   harness.AddBall(500.f, 1000.f, 0.f); // resting right under the axle
   harness.Start();

   float peak = 0.f;
   harness.AdvanceUntil(
      [&]
      {
         peak = max(peak, fabsf(SpinnerAngleDeg(spinner)));
         return false;
      },
      1000);
   CHECK(peak == doctest::Approx(15.f).epsilon(0.1)); // clamped at the limit, not parked on the ball
}
