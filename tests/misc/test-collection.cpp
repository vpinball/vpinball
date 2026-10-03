// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/Collection.h"
#include "parts/bumper.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"
#include "core/vpversion.h"

#include "doctest.h"

TEST_CASE("Collection")
{
   PinTable* const table = CreateTestTable();

   Bumper* const bump1 = Bumper::COMCreate();
   bump1->Init(0.f, 0.f, false);
   bump1->SetName("Bump1");
   table->AddPart(bump1);
   bump1->Release();

   Bumper* const bump2 = Bumper::COMCreate();
   bump2->Init(0.f, 0.f, false);
   bump2->SetName("Bump2");
   table->AddPart(bump2);
   bump2->Release();

   SUBCASE("membership, indexed access and enumeration")
   {
      CComObject<Collection>* col;
      CComObject<Collection>::CreateInstance(&col);
      col->AddRef();
      col->m_name = "Col1";

      col->AddPart(bump1);
      col->AddPart(bump2);
      CHECK(col->GetParts().size() == 2);

      LONG count = 0;
      CHECK(SUCCEEDED(col->get_Count(&count)));
      CHECK(count == 2);

      // Indexed access is bounds checked and returns the part's dispatch interface
      IDispatch* disp = nullptr;
      CHECK(SUCCEEDED(col->get_Item(1, &disp)));
      CHECK(disp == static_cast<IDispatch*>(bump2));
      disp->Release();
      CHECK(col->get_Item(-1, &disp) == TYPE_E_OUTOFBOUNDS);
      CHECK(col->get_Item(2, &disp) == TYPE_E_OUTOFBOUNDS);

      // IEnumVARIANT iteration (used by scripts' "For Each")
      IUnknown* unk = nullptr;
      REQUIRE(SUCCEEDED(col->get__NewEnum(&unk)));
      IEnumVARIANT* enumerator = nullptr;
      REQUIRE(SUCCEEDED(unk->QueryInterface(IID_IEnumVARIANT, reinterpret_cast<void**>(&enumerator))));
      unk->Release();

      VARIANT values[2];
      VariantInit(&values[0]);
      VariantInit(&values[1]);
      ULONG fetched = 0;
      CHECK(enumerator->Next(2, values, &fetched) == S_OK);
      CHECK(fetched == 2);
      CHECK(V_VT(&values[0]) == VT_DISPATCH);
      CHECK(V_DISPATCH(&values[0]) == static_cast<IDispatch*>(bump1));
      CHECK(V_DISPATCH(&values[1]) == static_cast<IDispatch*>(bump2));
      VariantClear(&values[0]);
      VariantClear(&values[1]);

      // End of enumeration: partial/empty results
      VARIANT extra;
      VariantInit(&extra);
      fetched = 99;
      CHECK(enumerator->Next(1, &extra, &fetched) == S_FALSE);
      CHECK(fetched == 0);
      VariantClear(&extra);

      // Reset + Skip
      CHECK(enumerator->Reset() == S_OK);
      CHECK(enumerator->Skip(1) == S_OK);
      VARIANT one;
      VariantInit(&one);
      CHECK(enumerator->Next(1, &one, &fetched) == S_OK);
      CHECK(fetched == 1);
      CHECK(V_DISPATCH(&one) == static_cast<IDispatch*>(bump2));
      VariantClear(&one);
      CHECK(enumerator->Skip(10) == S_FALSE);
      enumerator->Release();

      col->RemovePart(bump1);
      CHECK(col->GetParts().size() == 1);
      col->ClearParts();
      CHECK(col->GetParts().empty());

      col->Release();
   }

   SUBCASE("save/load resolves member names against the table")
   {
      CComObject<Collection>* col;
      CComObject<Collection>::CreateInstance(&col);
      col->AddRef();
      col->m_name = "MyCol";
      col->m_fireEvents = true;
      col->m_stopSingleEvents = true;
      col->AddPart(bump1);
      col->AddPart(bump2);

      InMemStream stream;
      {
         BiffWriter writer(&stream, nullptr);
         col->Save(writer, false);
         CHECK_FALSE(writer.HasError());
      }

      CComObject<Collection>* loaded;
      CComObject<Collection>::CreateInstance(&loaded);
      loaded->AddRef();
      BiffReader reader(stream.Data(), static_cast<uint32_t>(stream.Size()), CURRENT_FILE_FORMAT_VERSION, nullptr, 0);
      loaded->Load(reader);
      CHECK_FALSE(reader.HasError());

      CHECK(loaded->m_name == "MyCol");
      CHECK(loaded->m_fireEvents == true);
      CHECK(loaded->m_stopSingleEvents == true);
      // Members are stored by name and only resolved once the whole table is loaded
      CHECK(loaded->GetParts().empty());

      CHECK(SUCCEEDED(loaded->InitPostLoad(table)));
      REQUIRE(loaded->GetParts().size() == 2);
      CHECK(loaded->GetParts()[0] == bump1);
      CHECK(loaded->GetParts()[1] == bump2);

      // The reverse link is registered on the parts
      CHECK(std::ranges::find(bump1->m_vCollection, loaded) != bump1->m_vCollection.end());
      CHECK(std::ranges::find(bump2->m_vCollection, loaded) != bump2->m_vCollection.end());

      loaded->Release();
      col->Release();
   }

   SUBCASE("membership edition keeps the parts' back references in sync")
   {
      CComObject<Collection>* col;
      CComObject<Collection>::CreateInstance(&col);
      col->AddRef();
      col->m_name = "Col1";
      table->AddCollection(col);
      col->Release();

      table->AddPartToCollection(col, bump1);
      table->AddPartToCollection(col, bump2);
      CHECK(col->GetParts().size() == 2);
      REQUIRE(bump1->m_vCollection.size() == 1);
      CHECK(bump1->m_vCollection[0] == col);
      CHECK(bump1->m_viCollection[0] == 0);
      CHECK(bump2->m_viCollection[0] == 1);

      // Adding the same part twice is a no-op
      table->AddPartToCollection(col, bump1);
      CHECK(col->GetParts().size() == 2);

      // Removing a member fixes the member index of the parts that were after it
      table->RemovePartFromCollection(col, bump1);
      CHECK(col->GetParts() == vector<IEditable*> { bump2 });
      CHECK(bump1->m_vCollection.empty());
      CHECK(bump1->m_viCollection.empty());
      CHECK(bump2->m_viCollection[0] == 0);

      // Content rebuild keeps both sides of the cross reference consistent
      table->SetCollectionContent(col, { bump2, bump1 });
      REQUIRE(col->GetParts().size() == 2);
      CHECK(col->GetParts()[0] == bump2);
      CHECK(col->GetParts()[1] == bump1);
      CHECK(bump2->m_viCollection[0] == 0);
      CHECK(bump1->m_vCollection[0] == col);
      CHECK(bump1->m_viCollection[0] == 1);

      // Toggle membership used by the context menus also maintains the back references
      table->ToggleCollectionMembership(0, { bump1 });
      CHECK(col->GetParts() == vector<IEditable*> { bump2 });
      CHECK(bump1->m_vCollection.empty());
      table->ToggleCollectionMembership(0, { bump1 });
      REQUIRE(col->GetParts().size() == 2);
      CHECK(bump1->m_vCollection[0] == col);
      CHECK(bump1->m_viCollection[0] == 1);

      table->RemoveCollection(col);
      CHECK(bump1->m_vCollection.empty());
      CHECK(bump2->m_vCollection.empty());
   }

   table->Release();
}
