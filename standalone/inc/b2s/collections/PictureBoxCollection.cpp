#include "stdafx.h"

#include "../classes/B2SData.h"
#include "PictureBoxCollection.h"
#include "../controls/B2SPictureBox.h"

void PictureBoxCollection::Add(B2SPictureBox* pPicbox, eDualMode dualMode)
{
   B2SData* pB2SData = B2SData::GetInstance();

   if (!contains(pPicbox->GetName()))
      (*this)[pPicbox->GetName()] = pPicbox;

   if (pPicbox->GetRomID() > 0) {
      std::map<int, vector<B2SBaseBox*>>* pUsedRomIDs4Authentic = NULL;
      std::map<int, vector<B2SBaseBox*>>* pUsedRomIDs4Fantasy = NULL;
      if (pPicbox->GetRomIDType() == eRomIDType_Lamp) {
         pUsedRomIDs4Authentic = pB2SData->GetUsedRomLampIDs4Authentic();
         pUsedRomIDs4Fantasy = pB2SData->GetUsedRomLampIDs4Fantasy();
      }
      else if (pPicbox->GetRomIDType() == eRomIDType_Solenoid) {
         pUsedRomIDs4Authentic = pB2SData->GetUsedRomSolenoidIDs4Authentic();
         pUsedRomIDs4Fantasy = pB2SData->GetUsedRomSolenoidIDs4Fantasy();
      }
      else if (pPicbox->GetRomIDType() == eRomIDType_GIString) {
         pUsedRomIDs4Authentic = pB2SData->GetUsedRomGIStringIDs4Authentic();
         pUsedRomIDs4Fantasy = pB2SData->GetUsedRomGIStringIDs4Fantasy();
      }
      else if (pPicbox->GetRomIDType() == eRomIDType_Mech) {
         pUsedRomIDs4Authentic = pB2SData->GetUsedRomMechIDs4Authentic();
         pUsedRomIDs4Fantasy = pB2SData->GetUsedRomMechIDs4Fantasy();
      }
      if (dualMode == eDualMode_Both || dualMode == eDualMode_Authentic)
          (*pUsedRomIDs4Authentic)[pPicbox->GetRomID()].push_back(pPicbox);
      if (dualMode == eDualMode_Both || dualMode == eDualMode_Fantasy)
          (*pUsedRomIDs4Fantasy)[pPicbox->GetRomID()].push_back(pPicbox);
   }
}