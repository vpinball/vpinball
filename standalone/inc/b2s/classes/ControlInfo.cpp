#include "stdafx.h"

#include "ControlInfo.h"

ControlInfo::ControlInfo(int startDigit, int digits, eControlType type, B2SLEDBox* pLEDBox)
{
   m_startDigit = startDigit;
   m_digits = digits;
   m_type = type;
   m_pLEDBox = pLEDBox;
}

ControlInfo::ControlInfo(int startDigit, int digits, eControlType type, Dream7Display* pLEDDisplay)
{
   m_startDigit = startDigit;
   m_digits = digits;
   m_type = type;
   m_pLEDDisplay = pLEDDisplay;
}

ControlInfo::ControlInfo(int startDigit, int digits, eControlType type, B2SReelBox* pReelBox)
{
   m_startDigit = startDigit;
   m_digits = digits;
   m_type = type;
   m_pReelBox = pReelBox;
}

ControlInfo::ControlInfo(int startDigit, int digits, eControlType type, B2SReelDisplay* pReelDisplay)
{
   m_startDigit = startDigit;
   m_digits = digits;
   m_type = type;
   m_pReelDisplay = pReelDisplay;
}

ControlInfo::~ControlInfo()
{
}