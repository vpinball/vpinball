// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/lightseq.h"

#include "doctest.h"

TEST_CASE("LightSeq part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      LightSeq* const seq = LightSeq::COMCreate();
      seq->Init(100.f, 200.f, false);
      seq->SetName(L"LightSeq1");
      seq->m_d.m_vCenter = Vertex2D(150.f, 250.f);
      seq->m_d.m_wzCollection = L"MyLights";
      seq->m_d.m_updateinterval = 40;
      seq->m_timerEnabled = true;
      seq->m_timerInterval = 99;
      table->AddPart(seq);
      seq->Release();

      const InMemStream saved = SavePartToStream(seq);

      LightSeq* const copy = LightSeq::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"LightSeq1");
      CHECK(copy->m_d.m_v.x == 100.f);
      CHECK(copy->m_d.m_v.y == 200.f);
      CHECK(copy->m_d.m_vCenter.x == 150.f);
      CHECK(copy->m_d.m_vCenter.y == 250.f);
      CHECK(copy->m_d.m_wzCollection == L"MyLights");
      CHECK(copy->m_d.m_updateinterval == 40);
      CHECK(copy->m_timerEnabled == true);
      CHECK(copy->m_timerInterval == 99);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      LightSeq* const seq = LightSeq::COMCreate();
      seq->Init(10.f, 20.f, false);
      seq->SetName(L"LightSeq2");
      seq->m_d.m_wzCollection = L"Col";
      table->AddPart(seq);
      seq->Release();

      LightSeq* const liveCopy = seq->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(seq), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      LightSeq* const seq = LightSeq::COMCreate();
      seq->Init(10.f, 20.f, false);
      seq->SetName(L"LightSeq3");
      table->AddPart(seq);
      seq->Release();

      CHECK(seq->GetCenter().x == 10.f);
      CHECK(seq->GetCenter().y == 20.f);
      seq->Translate(Vertex2D(3.f, -3.f));
      CHECK(seq->GetCenter().x == 13.f);
      CHECK(seq->GetCenter().y == 17.f);
   }

   SUBCASE("CenterX/CenterY are bounded by the table size")
   {
      table->m_right = 1500.f;
      table->m_bottom = 3000.f;

      LightSeq* const seq = LightSeq::COMCreate();
      seq->Init(10.f, 20.f, false);
      seq->SetName(L"LightSeq4");
      table->AddPart(seq);
      seq->Release();

      // Beyond the legacy editor limits (1000x2000) but within the table
      CHECK(seq->put_CenterX(1200.f) == S_OK);
      CHECK(seq->GetX() == 1200.f);
      CHECK(seq->put_CenterY(2096.f) == S_OK);
      CHECK(seq->GetY() == 2096.f);

      // Outside the table bounds
      CHECK(seq->put_CenterX(-1.f) == E_FAIL);
      CHECK(seq->GetX() == 1200.f);
      CHECK(seq->put_CenterX(1500.f) == E_FAIL);
      CHECK(seq->put_CenterY(-1.f) == E_FAIL);
      CHECK(seq->GetY() == 2096.f);
      CHECK(seq->put_CenterY(3000.f) == E_FAIL);
   }

   table->Release();
}
