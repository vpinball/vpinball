// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "input/PlungerKalmanFilter.h"

#include "doctest.h"

TEST_CASE("Plunger Kalman filter")
{
   PlungerKalmanFilter filter;

   SUBCASE("reset sets the state")
   {
      filter.Reset(0.25f, 0.f, 1000);
      CHECK(filter.GetPosition() == doctest::Approx(0.25f));
      CHECK(filter.GetVelocity() == doctest::Approx(0.f));
      CHECK(filter.GetTimeNs() == 1000);
   }

   SUBCASE("prediction advances position with velocity")
   {
      filter.Reset(0.5f, 2.f, 1000000ull);
      filter.PredictTo(2000000ull); // 1ms
      CHECK(filter.GetPosition() == doctest::Approx(0.502f));
      CHECK(filter.GetTimeNs() == 2000000ull);

      filter.PredictTo(500000ull); // in the past: no-op
      CHECK(filter.GetPosition() == doctest::Approx(0.502f));
      CHECK(filter.GetTimeNs() == 2000000ull);
   }

   SUBCASE("position measurements converge to the measured value")
   {
      filter.Reset(0.f, 0.f, 1000000ull);
      for (uint64_t i = 1; i <= 200; ++i)
         filter.UpdatePosition(1000000ull + i * 1000000ull, 0.5f);
      CHECK(filter.GetPosition() == doctest::Approx(0.5f).epsilon(0.05));
      CHECK(std::fabs(filter.GetVelocity()) < 0.5f);
   }

   SUBCASE("first position update anchors the time base")
   {
      PlungerKalmanFilter fresh;
      fresh.UpdatePosition(5000000ull, 0.3f);
      CHECK(fresh.GetTimeNs() == 5000000ull);
      CHECK(fresh.GetPosition() > 0.1f); // pulled toward the measurement
   }

   SUBCASE("position limits clamp the estimate")
   {
      PlungerKalmanFilter::Config config;
      config.enablePositionLimits = true;
      config.minPosition = 0.f;
      config.maxPosition = 1.f;
      filter.SetConfig(config);

      filter.Reset(1.5f, 0.f, 0);
      CHECK(filter.GetPosition() == doctest::Approx(1.f));
      filter.Reset(-0.5f, 0.f, 0);
      CHECK(filter.GetPosition() == doctest::Approx(0.f));
   }

   SUBCASE("late measurements rewind and replay the filter")
   {
      filter.Reset(0.f, 0.f, 1000000ull);
      filter.PredictTo(3000000ull);
      filter.UpdatePosition(2000000ull, 1.f); // timestamped in the past
      CHECK(filter.GetTimeNs() == 3000000ull); // present time is preserved
      CHECK(filter.GetPosition() > 0.5f); // pulled toward the late measurement
      CHECK(filter.GetPosition() <= 1.05f);
   }

   SUBCASE("velocity-zero pseudo measurement damps the velocity")
   {
      filter.Reset(0.5f, 2.f, 1000000ull);
      filter.UpdateVelocityZero(0.01f);
      CHECK(std::fabs(filter.GetVelocity()) < 2.f);
   }

   SUBCASE("non finite measurements are ignored")
   {
      filter.Reset(0.5f, 0.f, 1000000ull);
      filter.UpdatePosition(2000000ull, std::numeric_limits<float>::quiet_NaN());
      CHECK(filter.GetPosition() == doctest::Approx(0.5f));
      filter.UpdatePosition(2000000ull, std::numeric_limits<float>::infinity());
      CHECK(filter.GetPosition() == doctest::Approx(0.5f));
   }
}
