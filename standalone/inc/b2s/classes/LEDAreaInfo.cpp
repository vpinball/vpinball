#include "stdafx.h"

#include "LEDAreaInfo.h"

LEDAreaInfo::LEDAreaInfo(const SDL_Rect& rect, bool isOnDMD)
{
   m_rect = rect;
   m_onDMD = isOnDMD;
}