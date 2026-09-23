// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/textbox.h"

#include "doctest.h"

TEST_CASE("Textbox part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Textbox* const textbox = Textbox::COMCreate();
      textbox->Init(100.f, 200.f, false);
      textbox->SetName(L"Text1");
      textbox->m_d.m_v2 = Vertex2D(300.f, 400.f);
      textbox->m_d.m_backcolor = RGB(5, 10, 15);
      textbox->m_d.m_fontcolor = RGB(250, 240, 230);
      textbox->m_d.m_intensity_scale = 0.8f;
      textbox->m_d.m_text = "GAME OVER";
      textbox->m_d.m_talign = TextAlignRight;
      textbox->m_d.m_transparent = true;
      // textbox->m_d.m_visible = ...; // TODO refactor unused property
      textbox->m_d.m_isDMD = true;
      textbox->m_d.m_font.name = "Arial";
      textbox->m_d.m_font.size = 140000;
      textbox->m_d.m_font.weight = 400;
      textbox->m_d.m_font.charset = 0;
      textbox->m_d.m_font.attributes = 0x04; // underline
      table->AddPart(textbox);
      textbox->Release();

      const InMemStream saved = SavePartToStream(textbox);

      Textbox* const copy = Textbox::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Text1");
      CHECK(copy->m_d.m_v1.x == 100.f);
      CHECK(copy->m_d.m_v1.y == 200.f);
      CHECK(copy->m_d.m_v2.x == 300.f);
      CHECK(copy->m_d.m_v2.y == 400.f);
      CHECK(copy->m_d.m_backcolor == RGB(5, 10, 15));
      CHECK(copy->m_d.m_fontcolor == RGB(250, 240, 230));
      CHECK(copy->m_d.m_intensity_scale == 0.8f);
      CHECK(copy->m_d.m_text == "GAME OVER");
      CHECK(copy->m_d.m_talign == TextAlignRight);
      CHECK(copy->m_d.m_transparent == true);
      CHECK(copy->m_d.m_isDMD == true);
      CHECK(copy->m_d.m_font.name == "Arial");
      CHECK(copy->m_d.m_font.size == 140000);
      CHECK(copy->m_d.m_font.weight == 400);
      CHECK(copy->m_d.m_font.attributes == 0x04);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Textbox* const textbox = Textbox::COMCreate();
      textbox->Init(10.f, 20.f, false);
      textbox->SetName(L"Text2");
      textbox->m_d.m_text = "HELLO";
      table->AddPart(textbox);
      textbox->Release();

      Textbox* const liveCopy = textbox->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(textbox), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Textbox* const textbox = Textbox::COMCreate();
      textbox->Init(10.f, 20.f, false);
      textbox->SetName(L"Text3");
      table->AddPart(textbox);
      textbox->Release();

      const Vertex2D center = textbox->GetCenter();
      textbox->Translate(Vertex2D(-1.f, 6.f));
      CHECK(textbox->GetCenter().x == doctest::Approx(center.x - 1.f));
      CHECK(textbox->GetCenter().y == doctest::Approx(center.y + 6.f));
   }

   table->Release();
}
