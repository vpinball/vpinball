#include "stdafx.h"

#include "../classes/B2SData.h"
#include "ReelBoxCollection.h"
#include "../controls/B2SReelBox.h"

void ReelBoxCollection::Add(B2SReelBox* pReelbox)
{
   if (!contains(pReelbox->GetName()))
      (*this)[pReelbox->GetName()] = pReelbox;

   if (pReelbox->GetRomID() > 0)
      (*pReelbox->GetB2SData()->GetUsedRomReelLampIDs())[pReelbox->GetRomID()].push_back(pReelbox);
}