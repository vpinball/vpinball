// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/plunger.h"

#include "doctest.h"

TEST_CASE("Plunger part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Plunger* const plunger = Plunger::COMCreate();
      plunger->Init(100.f, 200.f, false);
      plunger->SetName(L"Plunger1");
      // plunger->m_d.m_color = ...; // TODO refactor unused property
      plunger->m_d.m_width = 30.f;
      plunger->m_d.m_height = 60.f;
      plunger->m_d.m_stroke = 80.f;
      plunger->m_d.m_zAdjust = 5.f;
      plunger->m_d.m_speedPull = 0.4f;
      plunger->m_d.m_speedFire = 0.8f;
      plunger->m_d.m_mechStrength = 85.f;
      plunger->m_d.m_type = PlungerTypeCustom;
      plunger->m_d.m_animFrames = 12;
      plunger->m_d.m_parkPosition = 0.4f;
      plunger->m_d.m_szSurface = "surface1";
      plunger->m_d.m_scatterVelocity = 0.5f;
      plunger->m_d.m_momentumXfer = 0.9f;
      plunger->m_d.m_szTipShape = "0 .34; 2 .6; 3 .64"; // plunger tip shape definition
      plunger->m_d.m_rodDiam = 0.7f;
      plunger->m_d.m_ringGap = 2.1f;
      plunger->m_d.m_ringDiam = 0.95f;
      plunger->m_d.m_ringWidth = 0.55f;
      plunger->m_d.m_springDiam = 0.77f;
      plunger->m_d.m_springGauge = 1.38f;
      plunger->m_d.m_springLoops = 2.5f;
      plunger->m_d.m_springEndLoops = 3.5f;
      plunger->m_d.m_mechPlunger = true;
      plunger->m_d.m_autoPlunger = true;
      plunger->m_d.m_szImage = "plunger.png";
      plunger->m_d.m_szMaterial = "plungerMat";
      plunger->m_d.m_visible = false;
      table->AddPart(plunger);
      plunger->Release();

      const InMemStream saved = SavePartToStream(plunger);

      Plunger* const copy = Plunger::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Plunger1");
      CHECK(copy->m_d.m_v.x == 100.f);
      CHECK(copy->m_d.m_v.y == 200.f);
      CHECK(copy->m_d.m_width == 30.f);
      CHECK(copy->m_d.m_height == 60.f);
      CHECK(copy->m_d.m_stroke == 80.f);
      CHECK(copy->m_d.m_zAdjust == 5.f);
      CHECK(copy->m_d.m_speedPull == 0.4f);
      CHECK(copy->m_d.m_speedFire == 0.8f);
      CHECK(copy->m_d.m_mechStrength == 85.f);
      CHECK(copy->m_d.m_type == PlungerTypeCustom);
      CHECK(copy->m_d.m_animFrames == 12);
      CHECK(copy->m_d.m_parkPosition == 0.4f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_scatterVelocity == 0.5f);
      CHECK(copy->m_d.m_momentumXfer == 0.9f);
      CHECK(copy->m_d.m_szTipShape == "0 .34; 2 .6; 3 .64");
      CHECK(copy->m_d.m_rodDiam == 0.7f);
      CHECK(copy->m_d.m_ringGap == 2.1f);
      CHECK(copy->m_d.m_ringDiam == 0.95f);
      CHECK(copy->m_d.m_ringWidth == 0.55f);
      CHECK(copy->m_d.m_springDiam == 0.77f);
      CHECK(copy->m_d.m_springGauge == 1.38f);
      CHECK(copy->m_d.m_springLoops == 2.5f);
      CHECK(copy->m_d.m_springEndLoops == 3.5f);
      CHECK(copy->m_d.m_mechPlunger == true);
      CHECK(copy->m_d.m_autoPlunger == true);
      CHECK(copy->m_d.m_szImage == "plunger.png");
      CHECK(copy->m_d.m_szMaterial == "plungerMat");
      CHECK(copy->m_d.m_visible == false);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Plunger* const plunger = Plunger::COMCreate();
      plunger->Init(50.f, 60.f, false);
      plunger->SetName(L"Plunger2");
      plunger->m_d.m_autoPlunger = true;
      table->AddPart(plunger);
      plunger->Release();

      Plunger* const liveCopy = plunger->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(plunger), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Plunger* const plunger = Plunger::COMCreate();
      plunger->Init(10.f, 20.f, false);
      plunger->SetName(L"Plunger3");
      table->AddPart(plunger);
      plunger->Release();

      CHECK(plunger->GetCenter().x == 10.f);
      CHECK(plunger->GetCenter().y == 20.f);
      plunger->Translate(Vertex2D(-4.f, 8.f));
      CHECK(plunger->GetCenter().x == 6.f);
      CHECK(plunger->GetCenter().y == 28.f);
   }

   table->Release();
}
