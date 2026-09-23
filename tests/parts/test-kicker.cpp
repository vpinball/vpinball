// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/kicker.h"

#include "doctest.h"

TEST_CASE("Kicker part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Kicker* const kicker = Kicker::COMCreate();
      kicker->Init(100.f, 200.f, false);
      kicker->SetName(L"Kicker1");
      kicker->m_d.m_radius = 30.f;
      kicker->m_d.m_szSurface = "surface1";
      kicker->m_d.m_kickertype = KickerHole;
      kicker->m_d.m_hitAccuracy = 0.6f;
      kicker->m_d.m_orientation = 45.f;
      kicker->m_d.m_hit_height = 40.f;
      kicker->m_d.m_enabled = false;
      kicker->m_d.m_fallThrough = true;
      kicker->m_d.m_legacyMode = true;
      kicker->m_d.m_szMaterial = "kickerMat";
      kicker->m_d.m_scatter = 0.15f;
      table->AddPart(kicker);
      kicker->Release();

      const InMemStream saved = SavePartToStream(kicker);

      Kicker* const copy = Kicker::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Kicker1");
      CHECK(copy->m_d.m_vCenter.x == 100.f);
      CHECK(copy->m_d.m_vCenter.y == 200.f);
      CHECK(copy->m_d.m_radius == 30.f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_kickertype == KickerHole);
      CHECK(copy->m_d.m_hitAccuracy == 0.6f);
      CHECK(copy->m_d.m_orientation == 45.f);
      CHECK(copy->m_d.m_hit_height == 40.f);
      CHECK(copy->m_d.m_enabled == false);
      CHECK(copy->m_d.m_fallThrough == true);
      CHECK(copy->m_d.m_legacyMode == true);
      CHECK(copy->m_d.m_szMaterial == "kickerMat");
      CHECK(copy->m_d.m_scatter == 0.15f);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Kicker* const kicker = Kicker::COMCreate();
      kicker->Init(50.f, 60.f, false);
      kicker->SetName(L"Kicker2");
      kicker->m_d.m_kickertype = KickerCup;
      table->AddPart(kicker);
      kicker->Release();

      Kicker* const liveCopy = kicker->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(kicker), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Kicker* const kicker = Kicker::COMCreate();
      kicker->Init(10.f, 20.f, false);
      kicker->SetName(L"Kicker3");
      table->AddPart(kicker);
      kicker->Release();

      CHECK(kicker->GetCenter().x == 10.f);
      CHECK(kicker->GetCenter().y == 20.f);
      kicker->Translate(Vertex2D(-10.f, 5.f));
      CHECK(kicker->GetCenter().x == 0.f);
      CHECK(kicker->GetCenter().y == 25.f);
   }

   table->Release();
}
