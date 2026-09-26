// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "physics/cabinet/CabinetPhysics.h"
#include "physics/cabinet/DampedHarmonicOscillator.h"
#include "physics/cabinet/KeyboardNudge.h"
#include "physics/cabinet/NudgeHandler.h"
#include "physics/cabinet/NudgeIntentHandler.h"

#include "doctest.h"

using namespace VPX::Physics;

TEST_CASE("Damped harmonic oscillator")
{
   SUBCASE("initial state is at rest")
   {
      DampedHarmonicOscillator osc(2.f, 1.f, 0.5f);
      CHECK(osc.GetMass() == doctest::Approx(2.f));
      CHECK(osc.GetDisplacement() == doctest::Approx(0.f));
      CHECK(osc.GetVelocity() == doctest::Approx(0.f));
      CHECK(osc.GetAcceleration() == doctest::Approx(0.f));
   }

   SUBCASE("critically damped oscillator settles at the equilibrium displacement")
   {
      DampedHarmonicOscillator osc(1.f, 1.f, 1.f);
      for (int i = 0; i < 5000; ++i)
         osc.StepOneMillisecond(10.f, 0.001f);
      // F = k.x with k = m.w0^2 = (2.pi)^2
      CHECK(osc.GetDisplacement() == doctest::Approx(10.f / (4.f * static_cast<float>(M_PI) * static_cast<float>(M_PI))).epsilon(0.05));
      CHECK(std::fabs(osc.GetVelocity()) < 1e-3f);
   }

   SUBCASE("undamped oscillator keeps oscillating")
   {
      DampedHarmonicOscillator osc(1.f, 1.f, 0.f);
      for (int i = 0; i < 10; ++i)
         osc.StepOneMillisecond(10.f, 0.001f);
      float minX = 0.f, maxX = 0.f;
      for (int i = 0; i < 3000; ++i)
      {
         osc.StepOneMillisecond(0.f, 0.001f);
         minX = std::min(minX, osc.GetDisplacement());
         maxX = std::max(maxX, osc.GetDisplacement());
      }
      CHECK(maxX > 0.f);
      CHECK(minX < 0.f);
   }

   SUBCASE("reset clears the state")
   {
      DampedHarmonicOscillator osc(1.f, 1.f, 1.f);
      for (int i = 0; i < 100; ++i)
         osc.StepOneMillisecond(10.f, 0.001f);
      osc.Reset();
      CHECK(osc.GetDisplacement() == doctest::Approx(0.f));
      CHECK(osc.GetVelocity() == doctest::Approx(0.f));
      CHECK(osc.GetAcceleration() == doctest::Approx(0.f));
   }
}

TEST_CASE("Cabinet physics")
{
   SUBCASE("no force means no motion")
   {
      CabinetPhysics physics;
      for (int i = 0; i < 100; ++i)
         physics.StepOneMillisecond(Vertex2D(0.f, 0.f));
      CHECK(physics.GetCabinetOffset().x == doctest::Approx(0.f));
      CHECK(physics.GetCabinetOffset().y == doctest::Approx(0.f));
      CHECK(physics.GetCabinetAcceleration().x == doctest::Approx(0.f));
      CHECK(physics.GetCabinetAcceleration().y == doctest::Approx(0.f));
   }

   SUBCASE("constant force displaces the cabinet and release lets it decay")
   {
      CabinetPhysics physics;
      for (int i = 0; i < 25; ++i)
         physics.StepOneMillisecond(Vertex2D(0.f, 500.f));
      const float peak = std::fabs(physics.GetCabinetOffset().y);
      CHECK(peak > 0.f);
      CHECK(std::fabs(physics.GetCabinetAcceleration().y) > 0.f);

      for (int i = 0; i < 5000; ++i)
         physics.StepOneMillisecond(Vertex2D(0.f, 0.f));
      CHECK(std::fabs(physics.GetCabinetOffset().y) < peak * 0.5f);
   }
}

TEST_CASE("Keyboard nudge implementations")
{
   SUBCASE("fresh push/retract nudge reports zero state")
   {
      PushRetractKeyboardNudge nudge(1.f);
      nudge.StepOneMillisecond();
      CHECK(nudge.GetCabinetOffset().x == doctest::Approx(0.f));
      CHECK(nudge.GetCabinetOffset().y == doctest::Approx(0.f));
      CHECK(nudge.GetCabinetAcceleration().x == doctest::Approx(0.f));
      CHECK(nudge.GetCabinetAcceleration().y == doctest::Approx(0.f));
   }

   SUBCASE("fresh box model nudge reports zero state")
   {
      BoxModelKeyboardNudge nudge(1.f);
      nudge.StepOneMillisecond();
      CHECK(nudge.GetCabinetOffset().x == doctest::Approx(0.f));
      CHECK(nudge.GetCabinetOffset().y == doctest::Approx(0.f));
      CHECK(nudge.GetCabinetAcceleration().x == doctest::Approx(0.f));
      CHECK(nudge.GetCabinetAcceleration().y == doctest::Approx(0.f));
   }

   SUBCASE("push/retract nudge produces a short impulse then settles")
   {
      PushRetractKeyboardNudge nudge(1.f);
      CHECK(!nudge.IsActive());
      nudge.Nudge(0.f, 1.f);
      CHECK(nudge.IsActive());

      bool sawAcceleration = false;
      for (int i = 0; i < 200; ++i)
      {
         nudge.StepOneMillisecond();
         sawAcceleration |= std::fabs(nudge.GetCabinetAcceleration().y) > 0.f;
      }
      CHECK(sawAcceleration);
      CHECK(nudge.GetCabinetOffset().y == doctest::Approx(0.f).epsilon(0.01));
   }

   SUBCASE("box model nudge moves the cabinet and decays")
   {
      BoxModelKeyboardNudge nudge(1.f);
      nudge.Nudge(0.f, 1.f);

      float peak = 0.f;
      for (int i = 0; i < 500; ++i)
      {
         nudge.StepOneMillisecond();
         peak = std::max(peak, std::fabs(nudge.GetCabinetOffset().y));
      }
      CHECK(peak > 0.f);

      for (int i = 0; i < 5000; ++i)
         nudge.StepOneMillisecond();
      CHECK(std::fabs(nudge.GetCabinetOffset().y) < peak * 0.5f);
   }

   SUBCASE("cab model nudge produces acceleration then settles")
   {
      CabModelKeyboardNudge nudge(1.f);
      nudge.Nudge(0.f, 1.f);

      bool sawAcceleration = false;
      float peak = 0.f;
      for (int i = 0; i < 500; ++i)
      {
         nudge.StepOneMillisecond();
         sawAcceleration |= std::fabs(nudge.GetCabinetAcceleration().y) > 0.f;
         peak = std::max(peak, std::fabs(nudge.GetCabinetOffset().y));
      }
      CHECK(sawAcceleration);
      CHECK(peak > 0.f);

      for (int i = 0; i < 5000; ++i)
         nudge.StepOneMillisecond();
      CHECK(std::fabs(nudge.GetCabinetOffset().y) < peak * 0.5f);
   }
}

TEST_CASE("Nudge intent handler")
{
   SUBCASE("no motion does not trigger an impulse")
   {
      NudgeIntentHandler handler(false);
      for (int i = 0; i < 100; ++i)
         handler.StepOneMillisecond(Vertex2D(0.f, 0.f));
      CHECK(!handler.IsImpulseInProgress());
      CHECK(handler.GetImpulseAcceleration().x == doctest::Approx(0.f));
      CHECK(handler.GetImpulseAcceleration().y == doctest::Approx(0.f));
   }

   SUBCASE("sustained negative acceleration triggers a delayed impulse")
   {
      NudgeIntentHandler handler(false);
      for (int i = 0; i < 10; ++i)
         handler.StepOneMillisecond(Vertex2D(0.f, -5.f));
      CHECK(handler.IsImpulseInProgress());

      bool sawImpulse = false;
      for (int i = 0; i < 40; ++i)
      {
         handler.StepOneMillisecond(Vertex2D(0.f, 0.f));
         sawImpulse |= std::fabs(handler.GetImpulseAcceleration().y) > 0.f;
      }
      CHECK(sawImpulse);
      CHECK(handler.GetImpulseAcceleration().x == doctest::Approx(0.f));
   }

   SUBCASE("positive y acceleration is filtered out")
   {
      NudgeIntentHandler handler(false);
      for (int i = 0; i < 20; ++i)
         handler.StepOneMillisecond(Vertex2D(0.f, 5.f));
      CHECK(!handler.IsImpulseInProgress());
   }
}

TEST_CASE("Nudge handler")
{
   Settings settings;
   NudgeHandler handler(nullptr, settings);

   SUBCASE("keyboard nudge mode round trip")
   {
      handler.SetKeyboardNudgeMode(NudgeHandler::VP9_PUSH_RETRACT);
      CHECK(handler.GetKeyboardNudgeMode() == NudgeHandler::VP9_PUSH_RETRACT);
      handler.SetKeyboardNudgeMode(NudgeHandler::VPX_BOX_MODEL);
      CHECK(handler.GetKeyboardNudgeMode() == NudgeHandler::VPX_BOX_MODEL);
      handler.SetKeyboardNudgeMode(NudgeHandler::VPX_CAB_MODEL);
      CHECK(handler.GetKeyboardNudgeMode() == NudgeHandler::VPX_CAB_MODEL);
   }

   SUBCASE("keyboard nudge strength round trip")
   {
      handler.SetKeyboardNudgeStrength(0.5f);
      CHECK(handler.GetKeyboardNudgeStrength() == doctest::Approx(0.5f));
      handler.SetKeyboardNudgeStrength(1.f);
   }

   SUBCASE("keyboard impulse produces cabinet acceleration")
   {
      handler.SetKeyboardNudgeMode(NudgeHandler::VPX_CAB_MODEL);
      handler.ApplyKeyboardImpulse(0.f, 1.f);

      bool sawAcceleration = false;
      for (int i = 0; i < 200; ++i)
      {
         handler.StepOneMillisecond();
         sawAcceleration |= std::fabs(handler.GetCabinetAcceleration().y) > 0.f;
      }
      CHECK(sawAcceleration);
   }
}
