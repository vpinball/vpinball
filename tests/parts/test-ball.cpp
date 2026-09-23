// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/ball.h"

#include "doctest.h"

TEST_CASE("Ball part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Ball* const ball = Ball::COMCreate();
      ball->Init(100.f, 200.f, false);
      ball->SetName(L"Ball1");
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

      CHECK(copy->GetWName() == L"Ball1");
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
      ball->SetName(L"Ball2");
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
      ball->SetName(L"Ball3");
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
