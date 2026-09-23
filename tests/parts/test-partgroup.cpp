// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/PartGroup.h"
#include "parts/timer.h"

#include "doctest.h"

TEST_CASE("PartGroup part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      PartGroup* const group = PartGroup::COMCreate();
      group->Init(11.f, 22.f, false);
      group->SetName(L"Group1");
      group->m_timerEnabled = true;
      group->m_timerInterval = 33;
      group->m_d.m_playerModeVisibilityMask = PartGroupData::PMVM_DESKTOP | PartGroupData::PMVM_CABINET;
      group->m_d.m_spaceReference = PartGroupData::SpaceReference::SR_CABINET;
      group->SetUILock(true);
      table->AddPart(group);
      group->Release();

      const InMemStream saved = SavePartToStream(group);

      PartGroup* const copy = PartGroup::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Group1");
      CHECK(copy->m_d.m_v.x == 11.f);
      CHECK(copy->m_d.m_v.y == 22.f);
      CHECK(copy->m_timerEnabled == true);
      CHECK(copy->m_timerInterval == 33);
      CHECK(copy->m_d.m_playerModeVisibilityMask == (PartGroupData::PMVM_DESKTOP | PartGroupData::PMVM_CABINET));
      CHECK(copy->m_d.m_spaceReference == PartGroupData::SpaceReference::SR_CABINET);
      CHECK(copy->IsUILocked() == true);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      PartGroup* const group = PartGroup::COMCreate();
      group->Init(5.f, 6.f, false);
      group->SetName(L"Group2");
      group->m_d.m_playerModeVisibilityMask = PartGroupData::PMVM_VIRTUAL_REALITY;
      group->m_d.m_spaceReference = PartGroupData::SpaceReference::SR_ROOM;
      table->AddPart(group);
      group->Release();

      PartGroup* const liveCopy = group->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(group), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("group hierarchy, visibility mask and reference space inheritance")
   {
      PartGroup* const root = PartGroup::COMCreate();
      root->Init(0.f, 0.f, false);
      root->SetName(L"Root");
      root->m_d.m_playerModeVisibilityMask = PartGroupData::PMVM_DESKTOP | PartGroupData::PMVM_FSS;
      root->m_d.m_spaceReference = PartGroupData::SpaceReference::SR_CABINET;
      table->AddPart(root);
      root->Release();

      PartGroup* const child = PartGroup::COMCreate();
      child->Init(0.f, 0.f, false);
      child->SetName(L"Child");
      child->m_d.m_playerModeVisibilityMask = PartGroupData::PMVM_FSS | PartGroupData::PMVM_CABINET;
      child->m_d.m_spaceReference = PartGroupData::SpaceReference::SR_INHERIT;
      table->AddPart(child);
      child->Release();
      child->SetPartGroup(root);

      Timer* const part = Timer::COMCreate();
      part->Init(1.f, 2.f, false);
      part->SetName(L"GroupedPart");
      table->AddPart(part);
      part->Release();
      part->SetPartGroup(child);

      // Visibility masks are ANDed along the parent chain
      CHECK(child->GetPlayerModeVisibilityMask() == PartGroupData::PMVM_FSS);
      CHECK(root->GetPlayerModeVisibilityMask() == (PartGroupData::PMVM_DESKTOP | PartGroupData::PMVM_FSS));

      // SR_INHERIT resolves against the parent chain, root defaulting to playfield space
      CHECK(child->GetReferenceSpace() == PartGroupData::SpaceReference::SR_CABINET);
      root->m_d.m_spaceReference = PartGroupData::SpaceReference::SR_INHERIT;
      CHECK(child->GetReferenceSpace() == PartGroupData::SpaceReference::SR_PLAYFIELD);

      // Child detection and path strings follow the parenting
      CHECK(part->IsChild(child));
      CHECK(part->IsChild(root));
      CHECK_FALSE(child->IsChild(child));
      CHECK_FALSE(root->IsChild(child));
      CHECK(part->GetPathString(false) == "Root/Child/GroupedPart");
      CHECK(part->GetPathString(true) == "Root/Child/");
      CHECK(root->GetPathString(false) == "Root");

      // UI visibility is inherited through the group chain
      root->SetUIVisible(false);
      CHECK(part->IsUIVisible(false) == true);
      CHECK(part->IsUIVisible(true) == false);
      root->SetUIVisible(true);
      CHECK(part->IsUIVisible(true) == true);
      part->SetUIVisible(false);
      CHECK(part->IsUIVisible(true) == false);
   }

   SUBCASE("editor transforms")
   {
      PartGroup* const group = PartGroup::COMCreate();
      group->Init(10.f, 20.f, false);
      group->SetName(L"Group3");
      table->AddPart(group);
      group->Release();

      CHECK(group->GetCenter().x == 10.f);
      CHECK(group->GetCenter().y == 20.f);
      group->Translate(Vertex2D(-10.f, 5.f));
      CHECK(group->GetCenter().x == 0.f);
      CHECK(group->GetCenter().y == 25.f);
   }

   table->Release();
}
