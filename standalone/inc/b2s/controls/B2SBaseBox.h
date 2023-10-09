#pragma once

#include "../b2s_i.h"

#include "Control.h"

class B2SBaseBox : public Control {
public:
   B2SBaseBox();

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
   SDL_FRect& GetRectangleF() { return m_rectangleF; }
   void SetRectangleF(const SDL_FRect& rectangleF) { m_rectangleF = rectangleF; }
   int GetStartDigit() const { return m_startDigit; }
   void SetStartDigit(const int startDigit) { m_startDigit = startDigit; }
   int GetDigits() const { return m_digits; }
   void SetDigits(const int digits) { m_digits = digits; }
   bool IsHidden() const { return m_hidden; }
   void SetHidden(const bool hidden) { m_hidden = hidden; }

private:
   eType_2 m_type;
   int m_id;
   int m_displayId;
   int m_romid;
   eRomIDType m_romidtype;
   int m_romidvalue;
   bool m_rominverted;
   SDL_FRect m_rectangleF;
   int m_startDigit;
   int m_digits;
   bool m_hidden;
};