#include "stdafx.h"

#include "../classes/B2SData.h"
#include "ControlCollection.h"
#include "../classes/ControlInfo.h"
#include "../classes/B2SReelDisplay.h"
#include "../controls/B2SLEDBox.h"
#include "../dream7/Dream7Display.h"

void ControlCollection::Add(ControlInfo* pControlInfo)
{
   bool add = true;

   for (auto& it : *this) {
      if (it->GetStartDigit() == pControlInfo->GetStartDigit()) {
         add = false;
         break;
      }
   }
   if (add) {
      push_back(pControlInfo);
      m_digits += pControlInfo->GetDigits();
   }
}

void ControlCollection::SetScore(int score)
{
   B2SData* pB2SData = B2SData::GetInstance();

   string scoreAsString = std::to_string(score);
   if (scoreAsString.length() < m_digits)
      scoreAsString = string(m_digits - scoreAsString.length(), ' ') + scoreAsString;
   else if (scoreAsString.length() > m_digits)
      scoreAsString = scoreAsString.substr(scoreAsString.length() - m_digits);

   for (auto& pControl : *this) {
      // get the part of the score
      string partofscore = scoreAsString.substr(0, pControl->GetDigits());
      
      // pass matching score part to real control
      switch (pControl->GetType()) {
         case eControlType_LEDBox:
            // nothing to do
            break;
         case eControlType_Dream7LEDDisplay:
            if (!pControl->GetLEDDisplay()->IsVisible()) {
               for (int i = pControl->GetStartDigit(); i < pControl->GetStartDigit() + pControl->GetDigits(); i++)
                  (*pB2SData->GetLEDs())["LEDBox" + std::to_string(i)]->SetText(partofscore.substr(i - pControl->GetStartDigit(), 1));
            }
            else {
               for (int i = 0; i < pControl->GetDigits(); i++)
                  pControl->GetLEDDisplay()->SetValue(i, partofscore.substr(i, 1));
            }
            break;
         case eControlType_ReelBox:
            // nothing to do
            break;
         case eControlType_ReelDisplay:
            pControl->GetReelDisplay()->SetScore(string_to_int(partofscore, 0));
            break;
         default: break;
      }
      // remove already passed score part
      scoreAsString = scoreAsString.substr(pControl->GetDigits());
   }
}
