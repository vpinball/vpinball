// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/editablereg.h"
#include "core/ieditable.h"

#include "doctest.h"

TEST_CASE("EditableRegistry")
{
   SUBCASE("all registered part types can be created")
   {
      // This list must match the RegisterEditable calls in VPApp::InitInstance
      const ItemTypeEnum types[] = { eItemBall, eItemBumper, eItemDecal, eItemDispReel, eItemFlasher, eItemFlipper, eItemGate, eItemKicker, eItemLight, eItemLightSeq, eItemPlunger,
         eItemPrimitive, eItemRamp, eItemRubber, eItemSpinner, eItemSurface, eItemTextbox, eItemTimer, eItemTrigger, eItemHitTarget, eItemPartGroup };
      for (const ItemTypeEnum type : types)
      {
         IEditable* const part = EditableRegistry::Create(type);
         REQUIRE(part != nullptr);
         CHECK(part->GetItemType() == type);
         part->Release();
      }
   }

   SUBCASE("all registered part types have a type name string id")
   {
      const ItemTypeEnum types[] = { eItemBall, eItemBumper, eItemDecal, eItemDispReel, eItemFlasher, eItemFlipper, eItemGate, eItemKicker, eItemLight, eItemLightSeq, eItemPlunger,
         eItemPrimitive, eItemRamp, eItemRubber, eItemSpinner, eItemSurface, eItemTextbox, eItemTimer, eItemTrigger, eItemHitTarget, eItemPartGroup };
      for (const ItemTypeEnum type : types)
         CHECK(EditableRegistry::GetTypeNameStringID(type) > 0);
   }
}
