#include "../common.h"

#include "LEDDisplayDigitLocation.h"

namespace B2SLegacy {

LEDDisplayDigitLocation::LEDDisplayDigitLocation(Dream7Display* pLEDDisplay, const int digit, const int ledDisplayID)
{
   m_pLEDDisplay = pLEDDisplay;
   m_digit = digit;
   m_ledDisplayID = ledDisplayID;
}

}
