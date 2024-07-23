#pragma once

#include "../b2s_i.h"

class B2SLEDBox;
class Dream7Display;
class B2SReelBox;
class B2SReelDisplay;

class ControlInfo
{
public:
   ControlInfo(int startDigit, int digits, eControlType type, B2SLEDBox* pLEDBox);
   ControlInfo(int startDigit, int digits, eControlType type, Dream7Display* pLEDDisplay);
   ControlInfo(int startDigit, int digits, eControlType type, B2SReelBox* pReelBox);
   ControlInfo(int startDigit, int digits, eControlType type, B2SReelDisplay* pReelDisplay);
   virtual ~ControlInfo();

   int GetStartDigit() const { return m_startDigit; }
   void SetStartDigit(int startDigit) { m_startDigit = startDigit; }
   int GetDigits() const { return m_digits; }
   void SetDigits(int digits) { m_digits = digits; }
   eControlType GetType() const { return m_type; }
   void SetType(eControlType type) { m_type = type; }
   B2SLEDBox* GetLEDBox() const { return m_pLEDBox; }
   void SetLEDBox(B2SLEDBox* pLEDBox) { m_pLEDBox = pLEDBox; }
   Dream7Display* GetLEDDisplay() const { return m_pLEDDisplay; }
   void SetLEDDisplay(Dream7Display* pLEDDisplay) { m_pLEDDisplay = pLEDDisplay; }
   B2SReelBox* GetReelBox() const { return m_pReelBox; }
   void SetReelBox(B2SReelBox* pReelBox) { m_pReelBox = pReelBox; }
   B2SReelDisplay* GetReelDisplay() const { return m_pReelDisplay; }
   void SetReelDisplay(B2SReelDisplay* pReelDisplay) { m_pReelDisplay = pReelDisplay; }

private:
   int m_startDigit;
   int m_digits;
   eControlType m_type;
   B2SLEDBox* m_pLEDBox;
   Dream7Display* m_pLEDDisplay;
   B2SReelBox* m_pReelBox;
   B2SReelDisplay* m_pReelDisplay;
};