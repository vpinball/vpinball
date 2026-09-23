// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "physics/cabinet/MotionKalmanAxis.h"
#include "physics/cabinet/MotionGainCalibratorAxis.h"

#include "doctest.h"

using VPX::Physics::MotionKalmanAxis;

TEST_CASE("Motion Kalman axis")
{
   MotionKalmanAxis::Config config;
   MotionKalmanAxis filter(config);

   SUBCASE("filter starts uninitialized")
   {
      CHECK(!filter.IsInitialized());
      CHECK(filter.GetTimeNs() == 0);
      CHECK(filter.GetPosition() == doctest::Approx(0.f));
      CHECK(filter.GetVelocity() == doctest::Approx(0.f));
      CHECK(filter.GetBiasedVelocity() == doctest::Approx(0.f));
      CHECK(filter.GetBiasedAcceleration() == doctest::Approx(0.f));
   }

   SUBCASE("first samples initialize the velocity and acceleration biases")
   {
      filter.UpdateVelocity(1000, 2.f);
      CHECK(filter.IsInitialized());
      CHECK(filter.GetTimeNs() == 1000);
      CHECK(filter.GetVelocity() == doctest::Approx(0.f));
      CHECK(filter.GetVelocityBias() == doctest::Approx(2.f));
      CHECK(filter.GetBiasedVelocity() == doctest::Approx(2.f));

      filter.UpdateAcceleration(2000, 0.5f);
      CHECK(filter.GetTimeNs() == 2000);
      CHECK(filter.GetBiasedAcceleration() == doctest::Approx(0.5f).epsilon(0.1));
   }

   SUBCASE("constant acceleration builds up velocity and displacement")
   {
      filter.Reset(0, 0.f, 0.f, 0.f);
      for (uint64_t i = 1; i <= 1000; ++i)
      {
         filter.PredictTo(i * 1000000ull); // 1ms steps
         filter.UpdateAcceleration(i * 1000000ull, 1.f);
      }
      CHECK(filter.GetTimeNs() == 1000000000ull);
      CHECK(filter.GetBiasedAcceleration() == doctest::Approx(1.f).epsilon(0.2));
      CHECK(filter.GetVelocity() > 0.f);
      CHECK(filter.GetPosition() > 0.f);
   }

   SUBCASE("rest constraints pull the state back to rest")
   {
      filter.Reset(0, 0.5f, 1.f, 0.2f);
      for (uint64_t i = 1; i <= 500; ++i)
      {
         filter.PredictTo(i * 1000000ull);
         filter.UpdateRestConstraints(i * 1000000ull);
      }
      CHECK(std::fabs(filter.GetPosition()) < 0.05f);
      CHECK(std::fabs(filter.GetVelocity()) < 0.05f);
      CHECK(std::fabs(filter.GetAcceleration()) < 0.05f);
   }

   SUBCASE("late velocity measurements rewind and replay the filter")
   {
      filter.Reset(0, 0.f, 0.f, 0.f);
      filter.PredictTo(2000000ull);
      filter.UpdateVelocity(1000000ull, 3.f); // measurement timestamped in the past
      CHECK(filter.GetTimeNs() == 2000000ull);
      CHECK(std::fabs(filter.GetBiasedVelocity() - 3.f) < 1.5f);
   }

   SUBCASE("prediction is limited to 1ms steps")
   {
      filter.Reset(0, 0.f, 1.f, 0.f);
      filter.PredictTo(5000000ull); // 5ms
      CHECK(filter.GetTimeNs() == 5000000ull);
      CHECK(filter.GetPosition() == doctest::Approx(0.005f).epsilon(0.05));
   }
}

TEST_CASE("Motion gain calibrator axis")
{
   MotionGainCalibratorAxis::Config config;

   SUBCASE("samples are rejected outside a segment")
   {
      MotionGainCalibratorAxis calibrator(config);
      CHECK(!calibrator.IsSegmentActive());
      CHECK(!calibrator.AddSample(1000, 1.f, 1.f));
   }

   SUBCASE("non increasing timestamps are rejected")
   {
      MotionGainCalibratorAxis calibrator(config);
      calibrator.StartSegment(1000);
      CHECK(calibrator.IsSegmentActive());
      CHECK(calibrator.AddSample(2000, 1.f, 1.f));
      CHECK(!calibrator.AddSample(2000, 1.f, 1.f));
      CHECK(!calibrator.AddSample(1000, 1.f, 1.f));
   }

   SUBCASE("segments with too few samples are rejected")
   {
      MotionGainCalibratorAxis calibrator(config);
      calibrator.StartSegment(0);
      for (uint64_t i = 1; i <= 5; ++i)
         calibrator.AddSample(i * 10000000ull, 1.f, 1.f); // 10ms steps, 5 samples < min 8
      CHECK(!calibrator.EndSegment());
      CHECK(calibrator.GetAcceptedSegmentCount() == 0);
      CHECK(calibrator.GetRejectedSegmentCount() == 1);
      CHECK(calibrator.GetGain() == doctest::Approx(1.f));
   }

   SUBCASE("clean sine segment recovers the input gain")
   {
      MotionGainCalibratorAxis calibrator(config);

      // Acceleration a(t) = 10.sin(pi.t/T), velocity fed as g.integral(a) with g = 2
      const float gain = 2.f;
      const float duration = 0.2f; // 200ms
      const int steps = 200;
      float integratedAccel = 0.f;
      float prevAccel = 0.f;

      calibrator.StartSegment(0);
      for (int i = 1; i <= steps; ++i)
      {
         const float t = duration * i / steps;
         const float dt = duration / steps;
         const float accel = 10.f * sinf(static_cast<float>(M_PI) * t / duration);
         integratedAccel += 0.5f * (prevAccel + accel) * dt;
         prevAccel = accel;
         REQUIRE(calibrator.AddSample(static_cast<uint64_t>(t * 1e9f), gain * integratedAccel, accel));
      }
      CHECK(calibrator.EndSegment());

      CHECK(calibrator.GetAcceptedSegmentCount() == 1);
      CHECK(calibrator.GetRejectedSegmentCount() == 0);
      CHECK(calibrator.GetGain() == doctest::Approx(gain).epsilon(0.1));
      CHECK(calibrator.GetGlobalConfidence() > 0.f);
      CHECK(calibrator.ScaleAcceleration(3.f) == doctest::Approx(3.f * calibrator.GetGain()));
   }

   SUBCASE("low excitation segment is rejected")
   {
      MotionGainCalibratorAxis calibrator(config);
      calibrator.StartSegment(0);
      for (uint64_t i = 1; i <= 200; ++i)
         calibrator.AddSample(i * 1000000ull, 1e-5f * i, 1e-5f);
      CHECK(!calibrator.EndSegment());
      CHECK(calibrator.GetAcceptedSegmentCount() == 0);
      CHECK(calibrator.GetRejectedSegmentCount() == 1);
   }

   SUBCASE("out of range segment gain is rejected")
   {
      MotionGainCalibratorAxis::Config narrowConfig;
      narrowConfig.m_minGain = 0.5f;
      narrowConfig.m_maxGain = 1.5f;
      MotionGainCalibratorAxis calibrator(narrowConfig);

      // Segment measured with gain 2 is above the maximum gain and must be rejected
      const float duration = 0.2f;
      const int steps = 200;
      float integratedAccel = 0.f;
      float prevAccel = 0.f;
      calibrator.StartSegment(0);
      for (int i = 1; i <= steps; ++i)
      {
         const float t = duration * i / steps;
         const float dt = duration / steps;
         const float accel = 10.f * sinf(static_cast<float>(M_PI) * t / duration);
         integratedAccel += 0.5f * (prevAccel + accel) * dt;
         prevAccel = accel;
         calibrator.AddSample(static_cast<uint64_t>(t * 1e9f), 2.f * integratedAccel, accel);
      }
      CHECK(!calibrator.EndSegment());
      CHECK(calibrator.GetAcceptedSegmentCount() == 0);
   }
}
