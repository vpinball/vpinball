#include "stdafx.h"

#include "LEDDisplayDigitLocation.h"

LEDDisplayDigitLocation::LEDDisplayDigitLocation(Dream7Display* pLEDDisplay, const int digit, const int ledDisplayID)
{
   m_pLEDDisplay = pLEDDisplay;
   m_digit = digit;
   m_ledDisplayID = ledDisplayID;
}