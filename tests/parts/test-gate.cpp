// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/gate.h"

#include "doctest.h"

TEST_CASE("Gate part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Gate* const gate = Gate::COMCreate();
      gate->Init(100.f, 200.f, false);
      gate->SetName(L"Gate1");
      gate->m_d.m_length = 110.f;
      gate->m_d.m_height = 60.f;
      gate->m_d.m_rotation = 90.f;
      gate->m_d.m_damping = 0.985f;
      gate->m_d.m_gravityfactor = 0.25f;
      gate->m_d.m_szSurface = "surface1";
      gate->m_d.m_angleMin = 0.1f;
      gate->m_d.m_angleMax = 1.5f;
      gate->m_d.m_type = GatePlate;
      gate->m_d.m_showBracket = false;
      gate->m_d.m_twoWay = false;
      gate->m_d.m_collidable = false;
      gate->m_d.m_visible = false;
      gate->m_d.m_szMaterial = "gateMat";
      gate->m_d.m_elasticity = 0.4f;
      gate->m_d.m_friction = 0.05f;
      // gate->m_d.m_scatter = ...; // TODO refactor unused property
      table->AddPart(gate);
      gate->Release();

      const InMemStream saved = SavePartToStream(gate);

      Gate* const copy = Gate::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Gate1");
      CHECK(copy->m_d.m_vCenter.x == 100.f);
      CHECK(copy->m_d.m_vCenter.y == 200.f);
      CHECK(copy->m_d.m_length == 110.f);
      CHECK(copy->m_d.m_height == 60.f);
      CHECK(copy->m_d.m_rotation == 90.f);
      CHECK(copy->m_d.m_damping == 0.985f);
      CHECK(copy->m_d.m_gravityfactor == 0.25f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_angleMin == 0.1f);
      CHECK(copy->m_d.m_angleMax == 1.5f);
      CHECK(copy->m_d.m_type == GatePlate);
      CHECK(copy->m_d.m_showBracket == false);
      CHECK(copy->m_d.m_twoWay == false);
      CHECK(copy->m_d.m_collidable == false);
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_szMaterial == "gateMat");
      CHECK(copy->m_d.m_elasticity == 0.4f);
      CHECK(copy->m_d.m_friction == 0.05f);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Gate* const gate = Gate::COMCreate();
      gate->Init(50.f, 60.f, false);
      gate->SetName(L"Gate2");
      gate->m_d.m_twoWay = true;
      table->AddPart(gate);
      gate->Release();

      Gate* const liveCopy = gate->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(gate), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Gate* const gate = Gate::COMCreate();
      gate->Init(10.f, 20.f, false);
      gate->SetName(L"Gate3");
      table->AddPart(gate);
      gate->Release();

      CHECK(gate->GetCenter().x == 10.f);
      CHECK(gate->GetCenter().y == 20.f);
      gate->Translate(Vertex2D(2.f, 4.f));
      CHECK(gate->GetCenter().x == 12.f);
      CHECK(gate->GetCenter().y == 24.f);
   }

   table->Release();
}
