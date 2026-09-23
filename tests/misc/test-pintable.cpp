// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/timer.h"
#include "parts/bumper.h"
#include "parts/Collection.h"
#include "parts/Material.h"

#include "doctest.h"

TEST_CASE("PinTable part")
{
   SUBCASE("part add/remove lifecycle and name lookup")
   {
      PinTable* const table = CreateTestTable();
      CHECK(table->GetParts().empty());

      Timer* const timer = Timer::COMCreate();
      timer->Init(10.f, 20.f, false);
      timer->SetName(L"Part1");

      // AddPart takes its own reference on the part and registers the script name
      table->AddPart(timer);
      timer->Release();
      CHECK(table->HasPart(timer));
      CHECK(table->GetParts().size() == 1);
      CHECK(table->GetElementByName("Part1") == timer);
      CHECK(timer->GetPTable() == table);

      // Removing the part releases the table's reference and clears the name
      table->RemovePart(timer);
      CHECK_FALSE(table->HasPart(timer));
      CHECK(table->GetParts().empty());
      CHECK(table->GetElementByName("Part1") == nullptr);
      // 'timer' was deleted by RemovePart: do not touch it anymore

      table->Release();
   }

   SUBCASE("parts keep their creation order in the table list")
   {
      PinTable* const table = CreateTestTable();

      Timer* const t1 = Timer::COMCreate();
      t1->Init(0.f, 0.f, false);
      t1->SetName(L"A");
      table->AddPart(t1);
      t1->Release();

      Bumper* const b1 = Bumper::COMCreate();
      b1->Init(0.f, 0.f, false);
      b1->SetName(L"B");
      table->AddPart(b1);
      b1->Release();

      REQUIRE(table->GetParts().size() == 2);
      CHECK(table->GetParts()[0] == t1);
      CHECK(table->GetParts()[1] == b1);
      CHECK(table->GetParts()[0]->GetItemType() == eItemTimer);
      CHECK(table->GetParts()[1]->GetItemType() == eItemBumper);

      table->Release();
   }

   SUBCASE("material add, lookup and name uniquification")
   {
      PinTable* const table = CreateTestTable();
      CHECK(table->GetMaterialList().empty());

      Material* const wood = new Material();
      wood->m_name = "Wood";
      wood->m_fRoughness = 0.9f;
      table->AddMaterial(wood);

      CHECK(table->GetMaterial("Wood") == wood);
      // Lookup is case insensitive
      CHECK(table->GetMaterial("wood") == wood);
      // Missing names resolve to the dummy material
      CHECK(table->IsDummyMaterial(table->GetMaterial("DoesNotExist")));
      CHECK(table->IsDummyMaterial(table->GetMaterial("")));

      // Adding a material with a colliding name renames the new one
      Material* const dupe = new Material();
      dupe->m_name = "Wood";
      table->AddMaterial(dupe);
      CHECK(dupe->m_name == "Wood1");
      CHECK(table->GetMaterial("Wood1") == dupe);
      CHECK(table->GetMaterialList().size() == 2);

      // RemoveMaterial takes ownership back and deletes the material
      table->RemoveMaterial(dupe);
      CHECK(table->GetMaterialList().size() == 1);
      CHECK(table->IsDummyMaterial(table->GetMaterial("Wood1")));

      table->Release();
   }

   SUBCASE("collections are owned by the table")
   {
      PinTable* const table = CreateTestTable();
      CHECK(table->GetCollections().empty());

      CComObject<Collection>* col;
      CComObject<Collection>::CreateInstance(&col);
      col->AddRef();
      col->m_wzName = L"Col1";
      table->AddCollection(col);
      col->Release(); // the table now owns the only reference

      REQUIRE(table->GetCollections().size() == 1);
      CHECK(table->GetCollections()[0]->m_wzName == L"Col1");

      table->Release();
   }
}
