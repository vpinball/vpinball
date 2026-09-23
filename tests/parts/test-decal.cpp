// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/decal.h"

#include "doctest.h"

TEST_CASE("Decal part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Decal* const decal = Decal::COMCreate();
      decal->Init(100.f, 200.f, false);
      decal->SetName(L"Decal1");
      decal->m_d.m_width = 300.f;
      decal->m_d.m_height = 50.f;
      decal->m_d.m_rotation = 45.f;
      decal->m_d.m_szSurface = "surface1";
      decal->m_d.m_decaltype = DecalImage;
      decal->m_d.m_text = "High Score";
      decal->m_d.m_sizingtype = ManualSize;
      decal->m_d.m_color = RGB(255, 128, 0);
      decal->m_d.m_verticalText = true;
      decal->m_d.m_szImage = "decal.png";
      decal->m_d.m_szMaterial = "decalMat";
      decal->m_d.m_font.name = "Courier New";
      decal->m_d.m_font.size = 120000;
      decal->m_d.m_font.weight = 700;
      decal->m_d.m_font.charset = 1;
      decal->m_d.m_font.attributes = 0x02; // italic
      table->AddPart(decal);
      decal->Release();

      const InMemStream saved = SavePartToStream(decal);

      Decal* const copy = Decal::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Decal1");
      CHECK(copy->m_d.m_vCenter.x == 100.f);
      CHECK(copy->m_d.m_vCenter.y == 200.f);
      CHECK(copy->m_d.m_width == 300.f);
      CHECK(copy->m_d.m_height == 50.f);
      CHECK(copy->m_d.m_rotation == 45.f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_decaltype == DecalImage);
      CHECK(copy->m_d.m_text == "High Score");
      CHECK(copy->m_d.m_sizingtype == ManualSize);
      CHECK(copy->m_d.m_color == RGB(255, 128, 0));
      CHECK(copy->m_d.m_verticalText == true);
      CHECK(copy->m_d.m_szImage == "decal.png");
      CHECK(copy->m_d.m_szMaterial == "decalMat");
      CHECK(copy->m_d.m_font.name == "Courier New");
      CHECK(copy->m_d.m_font.size == 120000);
      CHECK(copy->m_d.m_font.weight == 700);
      CHECK(copy->m_d.m_font.charset == 1);
      CHECK(copy->m_d.m_font.attributes == 0x02);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Decal* const decal = Decal::COMCreate();
      decal->Init(10.f, 20.f, false);
      decal->SetName(L"Decal2");
      decal->m_d.m_text = "TEST";
      decal->m_d.m_decaltype = DecalText;
      table->AddPart(decal);
      decal->Release();

      Decal* const liveCopy = decal->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(decal), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Decal* const decal = Decal::COMCreate();
      decal->Init(10.f, 20.f, false);
      decal->SetName(L"Decal3");
      table->AddPart(decal);
      decal->Release();

      CHECK(decal->GetCenter().x == 10.f);
      CHECK(decal->GetCenter().y == 20.f);
      decal->Translate(Vertex2D(-10.f, -20.f));
      CHECK(decal->GetCenter().x == 0.f);
      CHECK(decal->GetCenter().y == 0.f);
   }

   table->Release();
}
