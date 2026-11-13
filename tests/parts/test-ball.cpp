// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/ball.h"

#include "doctest.h"

// The raytraced ball impostor quad is sized from the projected fit points: once scaled, they must reach the sphere
// surface in every direction, or the ball outline gets clipped
TEST_CASE("Ball impostor bounds enclose the sphere")
{
   const AntiStretchHelper ash;
   float worst = 0.f;
   // Directions on a dense Fibonacci sphere
   constexpr int n = 20000;
   for (int k = 0; k < n; k++)
   {
      const float z = 1.f - 2.f * ((float)k + 0.5f) / (float)n;
      const float r = sqrtf(1.f - z * z);
      const float phi = (float)k * 2.39996323f;
      const float ux = r * cosf(phi), uy = r * sinf(phi), uz = z;
      float reach = -FLT_MAX;
      for (int i = 0; i < AntiStretchHelper::npts * 3; i += 3)
         reach = max(reach, ux * ash.m_stretchFitPoints[i] + uy * ash.m_stretchFitPoints[i + 1] + uz * ash.m_stretchFitPoints[i + 2]);
      worst = max(worst, 1.f - reach);
   }
   CHECK(worst > 0.09f); // The unscaled points really fall short (else boundScale is not needed anymore)
   CHECK(AntiStretchHelper::boundScale * (1.f - worst) >= 1.f);

   // Through an orthographic projection, the bounds of the scaled points enclose the projected sphere in any orientation
   for (int k = 0; k < 64; k++)
   {
      const Matrix3D rot = Matrix3D::MatrixRotateX((float)k * 0.37f) * Matrix3D::MatrixRotateZ((float)k * 0.91f);
      float xMin = FLT_MAX, xMax = -FLT_MAX, yMin = FLT_MAX, yMax = -FLT_MAX, zMin = FLT_MAX;
      REQUIRE_FALSE(ash.computeProjBounds(rot, xMin, xMax, yMin, yMax, zMin));
      CHECK(xMin <= -1.f);
      CHECK(xMax >= 1.f);
      CHECK(yMin <= -1.f);
      CHECK(yMax >= 1.f);
   }
}

TEST_CASE("Ball part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Ball* const ball = Ball::COMCreate();
      ball->Init(100.f, 200.f, false);
      ball->SetName("Ball1");
      ball->m_hitBall.m_d.m_pos.z = 25.f;
      ball->m_hitBall.m_d.m_radius = 30.f;
      ball->m_hitBall.m_d.m_mass = 2.5f;
      ball->m_d.m_forceReflection = true;
      ball->m_d.m_decalMode = true;
      ball->m_d.m_szImage = "ball.png";
      ball->m_d.m_imageDecal = "decal.png";
      ball->m_d.m_bulb_intensity_scale = 0.5f;
      ball->m_d.m_playfieldReflectionStrength = 0.25f;
      ball->m_d.m_color = RGB(200, 100, 50);
      ball->m_d.m_pinballEnvSphericalMapping = true;
      ball->m_d.m_reflectionEnabled = false;
      ball->m_timerEnabled = true;
      ball->m_timerInterval = 77;
      table->AddPart(ball);
      ball->Release();

      const InMemStream saved = SavePartToStream(ball);

      Ball* const copy = Ball::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetName() == "Ball1");
      CHECK(copy->m_hitBall.m_d.m_pos.x == 100.f);
      CHECK(copy->m_hitBall.m_d.m_pos.y == 200.f);
      CHECK(copy->m_hitBall.m_d.m_pos.z == 25.f);
      CHECK(copy->m_hitBall.m_d.m_radius == 30.f);
      CHECK(copy->m_hitBall.m_d.m_mass == 2.5f);
      CHECK(copy->m_d.m_forceReflection == true);
      CHECK(copy->m_d.m_decalMode == true);
      CHECK(copy->m_d.m_szImage == "ball.png");
      CHECK(copy->m_d.m_imageDecal == "decal.png");
      CHECK(copy->m_d.m_bulb_intensity_scale == 0.5f);
      CHECK(copy->m_d.m_playfieldReflectionStrength == 0.25f);
      CHECK(copy->m_d.m_color == RGB(200, 100, 50));
      CHECK(copy->m_d.m_pinballEnvSphericalMapping == true);
      CHECK(copy->m_d.m_reflectionEnabled == false);
      CHECK(copy->m_timerEnabled == true);
      CHECK(copy->m_timerInterval == 77);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Ball* const ball = Ball::COMCreate();
      ball->Init(10.f, 20.f, false);
      ball->SetName("Ball2");
      ball->m_hitBall.m_d.m_pos.z = 5.f;
      ball->m_hitBall.m_d.m_radius = 42.f;
      ball->m_hitBall.m_d.m_mass = 3.f;
      ball->m_d.m_decalMode = true;
      table->AddPart(ball);
      ball->Release();

      Ball* const liveCopy = ball->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(ball), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Ball* const ball = Ball::COMCreate();
      ball->Init(10.f, 20.f, false);
      ball->SetName("Ball3");
      table->AddPart(ball);
      ball->Release();

      CHECK(ball->GetCenter().x == 10.f);
      CHECK(ball->GetCenter().y == 20.f);
      ball->Translate(Vertex2D(-5.f, 10.f));
      CHECK(ball->GetCenter().x == 5.f);
      CHECK(ball->GetCenter().y == 30.f);
      CHECK(ball->m_hitBall.m_d.m_pos.x == 5.f);
      CHECK(ball->m_hitBall.m_d.m_pos.y == 30.f);
   }

   table->Release();
}
