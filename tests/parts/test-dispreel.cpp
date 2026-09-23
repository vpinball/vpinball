// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/dispreel.h"

#include "doctest.h"

TEST_CASE("DispReel part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      DispReel* const reel = DispReel::COMCreate();
      reel->Init(100.f, 200.f, false);
      reel->SetName(L"Reel1");
      reel->m_d.m_v2 = Vertex2D(300.f, 400.f);
      reel->m_d.m_imagesPerGridRow = 10;
      reel->m_d.m_reelcount = 5;
      reel->m_d.m_width = 32.f;
      reel->m_d.m_height = 48.f;
      reel->m_d.m_reelspacing = 4.f;
      reel->m_d.m_motorsteps = 2;
      reel->m_d.m_digitrange = 9;
      reel->m_d.m_szSound = "reelsound";
      reel->m_d.m_updateinterval = 60;
      reel->m_d.m_backcolor = RGB(10, 20, 30);
      reel->m_d.m_transparent = true;
      reel->m_d.m_useImageGrid = false;
      reel->m_d.m_szImage = "reels.png";
      reel->m_timerEnabled = true;
      reel->m_timerInterval = 33;
      table->AddPart(reel);
      reel->Release();

      const InMemStream saved = SavePartToStream(reel);

      DispReel* const copy = DispReel::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Reel1");
      CHECK(copy->m_d.m_v1.x == 100.f);
      CHECK(copy->m_d.m_v1.y == 200.f);
      CHECK(copy->m_d.m_v2.x == 300.f);
      CHECK(copy->m_d.m_v2.y == 400.f);
      CHECK(copy->m_d.m_imagesPerGridRow == 10);
      CHECK(copy->m_d.m_reelcount == 5);
      CHECK(copy->m_d.m_width == 32.f);
      CHECK(copy->m_d.m_height == 48.f);
      CHECK(copy->m_d.m_reelspacing == 4.f);
      CHECK(copy->m_d.m_motorsteps == 2);
      CHECK(copy->m_d.m_digitrange == 9);
      CHECK(copy->m_d.m_szSound == "reelsound");
      CHECK(copy->m_d.m_updateinterval == 60);
      CHECK(copy->m_d.m_backcolor == RGB(10, 20, 30));
      CHECK(copy->m_d.m_transparent == true);
      CHECK(copy->m_d.m_useImageGrid == false);
      CHECK(copy->m_d.m_szImage == "reels.png");
      CHECK(copy->m_timerEnabled == true);
      CHECK(copy->m_timerInterval == 33);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      DispReel* const reel = DispReel::COMCreate();
      reel->Init(10.f, 20.f, false);
      reel->SetName(L"Reel2");
      reel->m_d.m_reelcount = 3;
      table->AddPart(reel);
      reel->Release();

      DispReel* const liveCopy = reel->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(reel), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      DispReel* const reel = DispReel::COMCreate();
      reel->Init(10.f, 20.f, false);
      reel->SetName(L"Reel3");
      table->AddPart(reel);
      reel->Release();

      const Vertex2D center = reel->GetCenter();
      reel->Translate(Vertex2D(5.f, 5.f));
      CHECK(reel->GetCenter().x == center.x + 5.f);
      CHECK(reel->GetCenter().y == center.y + 5.f);
   }

   table->Release();
}
