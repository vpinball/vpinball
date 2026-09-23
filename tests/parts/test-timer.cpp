// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/timer.h"
#include "parts/PartGroup.h"

#include "doctest.h"

TEST_CASE("Timer part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Timer* const timer = Timer::COMCreate();
      timer->Init(100.f, 200.f, false);
      timer->SetName(L"Timer1");
      timer->m_timerEnabled = true;
      timer->m_timerInterval = 250;
      timer->m_desktopBackdrop = true;
      timer->SetUILock(true);
      timer->SetUIVisible(false);
      table->AddPart(timer);
      timer->Release();

      const InMemStream saved = SavePartToStream(timer);

      Timer* const copy = Timer::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Timer1");
      CHECK(copy->m_d.m_v.x == 100.f);
      CHECK(copy->m_d.m_v.y == 200.f);
      CHECK(copy->m_timerEnabled == true);
      CHECK(copy->m_timerInterval == 250);
      CHECK(copy->m_desktopBackdrop == true);
      CHECK(copy->IsUILocked() == true);
      CHECK(copy->IsUIVisible(false) == false);

      // Re-saving the loaded part must produce the exact same stream
      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Timer* const timer = Timer::COMCreate();
      timer->Init(30.f, 40.f, false);
      timer->SetName(L"Timer2");
      timer->m_timerEnabled = true;
      timer->m_timerInterval = 42;
      timer->m_desktopBackdrop = true;
      table->AddPart(timer);
      timer->Release();

      Timer* const liveCopy = timer->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(timer), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("part group reference is persisted through its name")
   {
      PartGroup* const group = PartGroup::COMCreate();
      group->Init(0.f, 0.f, false);
      group->SetName(L"Layer_01");
      table->AddPart(group);
      group->Release();

      Timer* const timer = Timer::COMCreate();
      timer->Init(10.f, 10.f, false);
      timer->SetName(L"Timer3");
      timer->SetPartGroup(group);
      table->AddPart(timer);
      timer->Release();

      const InMemStream saved = SavePartToStream(timer);

      Timer* const copy = Timer::COMCreate();
      LoadPartFromStream(copy, saved);

      // The group is resolved by name after the whole table has been loaded
      CHECK(copy->m_onLoadExpectedPartGroup == L"Layer_01");

      copy->Release();
   }

   SUBCASE("editor transforms")
   {
      Timer* const timer = Timer::COMCreate();
      timer->Init(10.f, 20.f, false);
      timer->SetName(L"Timer4");
      table->AddPart(timer);
      timer->Release();

      CHECK(timer->GetCenter().x == 10.f);
      CHECK(timer->GetCenter().y == 20.f);

      timer->Translate(Vertex2D(5.f, -5.f));
      CHECK(timer->GetCenter().x == 15.f);
      CHECK(timer->GetCenter().y == 15.f);

      timer->Rotate(90.f, Vertex2D(0.f, 0.f), false);
      CHECK(timer->GetCenter().x == doctest::Approx(-15.f));
      CHECK(timer->GetCenter().y == doctest::Approx(15.f));

      timer->FlipX(Vertex2D(0.f, 0.f));
      CHECK(timer->GetCenter().x == doctest::Approx(15.f));
      CHECK(timer->GetCenter().y == doctest::Approx(15.f));

      timer->FlipY(Vertex2D(0.f, 0.f));
      CHECK(timer->GetCenter().x == doctest::Approx(15.f));
      CHECK(timer->GetCenter().y == doctest::Approx(-15.f));
   }

   table->Release();
}
