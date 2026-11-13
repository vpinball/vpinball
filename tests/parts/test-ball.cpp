// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/ball.h"

#include "doctest.h"

// The raytraced ball impostor quad is sized from the exact clip space bounds of the (anti stretched) ball: they must
// match the bounds of densely sampled surface points, which they enclose
TEST_CASE("Ball impostor bounds are the exact projected bounds")
{
   // Perspective projection (row vectors, depth 0..1 like Matrix3D::MatrixPerspectiveFovLH)
   constexpr float f = 1.5f, aspect = 1.6f, zn = 10.f, zf = 5000.f;
   const Matrix3D proj(f / aspect, 0.f, 0.f, 0.f, 0.f, f, 0.f, 0.f, 0.f, 0.f, zf / (zf - zn), 1.f, 0.f, 0.f, -zn * zf / (zf - zn), 0.f);
   for (int k = 0; k < 24; k++)
   {
      // Ellipsoids (anti stretch scales one axis), rotated, at various places in front of the camera
      const Matrix3D toClip = Matrix3D::MatrixScale(25.f * (1.f + 0.1f * (float)(k % 3)), 25.f, 25.f) * Matrix3D::MatrixRotateX((float)k * 0.37f)
         * Matrix3D::MatrixRotateZ((float)k * 0.91f) * Matrix3D::MatrixTranslate(-300.f + 25.f * (float)k, 200.f - 17.f * (float)k, 100.f + 40.f * (float)k) * proj;
      float xMin = FLT_MAX, xMax = -FLT_MAX, yMin = FLT_MAX, yMax = -FLT_MAX, zMin = FLT_MAX;
      REQUIRE(AccumulateUnitSphereClipBounds(toClip, xMin, xMax, yMin, yMax, zMin));

      // Points on a dense Fibonacci sphere, in double so that they lie on the sphere whatever the float model of the build
      double sxMin = DBL_MAX, sxMax = -DBL_MAX, syMin = DBL_MAX, syMax = -DBL_MAX, szMin = DBL_MAX;
      constexpr int n = 20000;
      for (int i = 0; i < n; i++)
      {
         const double z = 1. - 2. * (i + 0.5) / n;
         const double r = sqrt(1. - z * z);
         const double phi = fmod(i * 2.399963229728653, 2. * M_PI);
         const double p[3] = { r * cos(phi), r * sin(phi), z };
         double c[4];
         for (int j = 0; j < 4; j++)
            c[j] = p[0] * toClip.m[0][j] + p[1] * toClip.m[1][j] + p[2] * toClip.m[2][j] + toClip.m[3][j];
         sxMin = min(sxMin, c[0] / c[3]);
         sxMax = max(sxMax, c[0] / c[3]);
         syMin = min(syMin, c[1] / c[3]);
         syMax = max(syMax, c[1] / c[3]);
         szMin = min(szMin, c[2] / c[3]);
      }
      // The bounds are rounded to float
      CHECK(xMin <= sxMin + 1e-6);
      CHECK(xMax >= sxMax - 1e-6);
      CHECK(yMin <= syMin + 1e-6);
      CHECK(yMax >= syMax - 1e-6);
      CHECK(zMin <= szMin + 1e-6);
      CHECK(xMin >= sxMin - 1e-3f);
      CHECK(xMax <= sxMax + 1e-3f);
      CHECK(yMin >= syMin - 1e-3f);
      CHECK(yMax <= syMax + 1e-3f);
      CHECK(zMin >= szMin - 1e-4f);
   }

   // A ball reaching the camera plane has no usable bounds
   float xMin = FLT_MAX, xMax = -FLT_MAX, yMin = FLT_MAX, yMax = -FLT_MAX, zMin = FLT_MAX;
   CHECK_FALSE(AccumulateUnitSphereClipBounds(Matrix3D::MatrixScale(25.f) * Matrix3D::MatrixTranslate(0.f, 0.f, 20.f) * proj, xMin, xMax, yMin, yMax, zMin));
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
