#include "../common.h"

#include "../classes/B2SData.h"
#include "ControlCollection.h"
#include "../classes/ControlInfo.h"
#include "../classes/B2SReelDisplay.h"
#include "../controls/B2SLEDBox.h"
#include "../dream7/Dream7Display.h"

namespace B2SLegacy {

void ControlCollection::Add(ControlInfo* pControlInfo)
{
   bool add = true;

   for (const auto& it : *this) {
      if (it->GetStartDigit() == pControlInfo->GetStartDigit()) {
         add = false;
         break;
      }
   }
   if (add) {
      push_back(pControlInfo);
      m_digits += pControlInfo->GetDigits();
   }
   else
      delete pControlInfo;
}

void ControlCollection::SetScore(B2SData* pB2SData, int score)
{
   string scoreAsString = std::to_string(score);
   if (m_digits > (int)scoreAsString.length())
      scoreAsString = string(m_digits - (int)scoreAsString.length(), ' ') + scoreAsString;
   else if ((int)scoreAsString.length() > m_digits)
      scoreAsString = scoreAsString.substr(scoreAsString.length() - m_digits);

   for (const auto& pControl : *this) {
      // get the part of the score
      const string partofscore = scoreAsString.substr(0, pControl->GetDigits());

      // pass matching score part to real control
      switch (pControl->GetType()) {
         case eControlType_LEDBox:
            // nothing to do
            break;
         case eControlType_Dream7LEDDisplay:
            if (!pControl->GetLEDDisplay()->IsVisible()) {
               for (int i = pControl->GetStartDigit(); i < pControl->GetStartDigit() + pControl->GetDigits(); i++)
                  (*pB2SData->GetLEDs())["LEDBox" + std::to_string(i)]->SetText(string(1,partofscore[i - pControl->GetStartDigit()]));
            }
            else {
               for (int i = 0; i < pControl->GetDigits(); i++)
                  pControl->GetLEDDisplay()->SetValue(i, string(1,partofscore[i]));
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

}
