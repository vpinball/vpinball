// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/spinner.h"

#include "doctest.h"

TEST_CASE("Spinner part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Spinner* const spinner = Spinner::COMCreate();
      spinner->Init(100.f, 200.f, false);
      spinner->SetName(L"Spinner1");
      spinner->m_d.m_length = 85.f;
      spinner->m_d.m_rotation = 30.f;
      spinner->m_d.m_height = 70.f;
      spinner->m_d.m_damping = 0.97f;
      spinner->m_d.m_angleMax = 1.2f;
      spinner->m_d.m_angleMin = -1.2f;
      spinner->m_d.m_szSurface = "surface1";
      spinner->m_d.m_showBracket = false;
      spinner->m_d.m_szImage = "spinner.png";
      spinner->m_d.m_szMaterial = "spinnerMat";
      spinner->m_d.m_visible = false;
      // spinner->m_d.m_collidable = ...; // TODO refactor unused property
      spinner->m_d.m_reflectionEnabled = false;
      table->AddPart(spinner);
      spinner->Release();

      const InMemStream saved = SavePartToStream(spinner);

      Spinner* const copy = Spinner::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Spinner1");
      CHECK(copy->m_d.m_vCenter.x == 100.f);
      CHECK(copy->m_d.m_vCenter.y == 200.f);
      CHECK(copy->m_d.m_length == 85.f);
      CHECK(copy->m_d.m_rotation == 30.f);
      CHECK(copy->m_d.m_height == 70.f);
      CHECK(copy->m_d.m_damping == 0.97f);
      CHECK(copy->m_d.m_angleMax == 1.2f);
      CHECK(copy->m_d.m_angleMin == -1.2f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_showBracket == false);
      CHECK(copy->m_d.m_szImage == "spinner.png");
      CHECK(copy->m_d.m_szMaterial == "spinnerMat");
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_reflectionEnabled == false);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Spinner* const spinner = Spinner::COMCreate();
      spinner->Init(50.f, 60.f, false);
      spinner->SetName(L"Spinner2");
      spinner->m_d.m_angleMin = -0.5f;
      table->AddPart(spinner);
      spinner->Release();

      Spinner* const liveCopy = spinner->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(spinner), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Spinner* const spinner = Spinner::COMCreate();
      spinner->Init(10.f, 20.f, false);
      spinner->SetName(L"Spinner3");
      table->AddPart(spinner);
      spinner->Release();

      CHECK(spinner->GetCenter().x == 10.f);
      CHECK(spinner->GetCenter().y == 20.f);
      spinner->Translate(Vertex2D(5.f, 5.f));
      CHECK(spinner->GetCenter().x == 15.f);
      CHECK(spinner->GetCenter().y == 25.f);
   }

   table->Release();
}
