#include "stdafx.h"

#include "B2SBaseBox.h"

B2SBaseBox::B2SBaseBox() : Control()
{
   m_type = eType_2_NotDefined;
   m_romid = 0;
   m_romidtype = eRomIDType_NotDefined;
   m_romidvalue = 0;
   m_rominverted = false;
   m_rectangleF = { 0.0f, 0.0f, 0.0f, 0.0f };
   m_startDigit = 0;
   m_digits = 0;
   m_hidden = false;
}