#pragma once


namespace B2SLegacy {

class Dream7Display;

class LEDDisplayDigitLocation final
{
public:
   LEDDisplayDigitLocation(Dream7Display* pLEDDisplay, const int digit, const int ledDisplayID);

   void SetLEDDisplay(Dream7Display* pLEDDisplay) { m_pLEDDisplay = pLEDDisplay; }
   Dream7Display* GetLEDDisplay() const { return m_pLEDDisplay; }
   int GetDigit() const { return m_digit; }
   void SetDigit(const int digit) { m_digit = digit; }
   int GetLEDDisplayID() const { return m_ledDisplayID; }
   void SetLEDDisplayID(const int ledDisplayID) { m_ledDisplayID = ledDisplayID; }

private:
   Dream7Display* m_pLEDDisplay = nullptr;
   int m_digit = 0;
   int m_ledDisplayID = 0;
};

}
