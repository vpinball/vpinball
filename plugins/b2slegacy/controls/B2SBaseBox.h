#pragma once

#include "Control.h"
#include "../classes/B2SData.h"

namespace B2SLegacy {

class B2SBaseBox : public Control
{
public:
   B2SBaseBox(VPXPluginAPI* vpxApi, B2SData* pB2SData);

   B2SData* GetB2SData() const { return m_pB2SData; }
   eType_2 GetType() const { return m_type; }
   void SetType(eType_2 type) { m_type = type; }
   int GetID() const { return m_id; }
   void SetID(const int id) { m_id = id; }
   int GetDisplayID() const { return m_displayId; }
   void SetDisplayID(const int displayId) { m_displayId = displayId; }
   int GetRomID() const { return m_romid; }
   void SetRomID(const int romid) { m_romid = romid; }
   eRomIDType GetRomIDType() const { return m_romidtype; }
   void SetRomIDType(const eRomIDType romidtype) { m_romidtype = romidtype; }
   int GetRomIDValue() const { return m_romidvalue; }
   void SetRomIDValue(const int romidvalue) { m_romidvalue = romidvalue; }
   bool IsRomInverted() const { return m_rominverted; }
   void SetRomInverted(const bool rominverted) { m_rominverted = rominverted; }
   const SDL_FRect& GetRectangleF() const { return m_rectangleF; }
   void SetRectangleF(const SDL_FRect& rectangleF) { m_rectangleF = rectangleF; }
   int GetStartDigit() const { return m_startDigit; }
   void SetStartDigit(const int startDigit) { m_startDigit = startDigit; }
   int GetDigits() const { return m_digits; }
   void SetDigits(const int digits) { m_digits = digits; }
   bool IsHidden() const { return m_hidden; }
   void SetHidden(const bool hidden) { m_hidden = hidden; }

protected:
   B2SData* m_pB2SData = nullptr;

private:
   eType_2 m_type = eType_2_NotDefined;
   int m_id = 0;
   int m_displayId = 0;
   int m_romid = 0;
   eRomIDType m_romidtype = eRomIDType_NotDefined;
   int m_romidvalue = 0;
   bool m_rominverted = false;
   SDL_FRect m_rectangleF = { 0.0f, 0.0f, 0.0f, 0.0f };
   int m_startDigit = 0;
   int m_digits = 0;
   bool m_hidden = false;
};

}
