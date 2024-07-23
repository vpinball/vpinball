#include "stdafx.h"

#include "SegmentStyle.h"

SegmentStyle::SegmentStyle()
{
   m_offColor = RGB(20, 20, 20);
   m_lightColor = RGB(254, 90, 50);
   m_glassColor = RGB(254, 50, 25);
   m_glassColorCenter = RGB(254, 50, 25);
   m_glassAlpha = 140;
   m_glassAlphaCenter = 255;
   m_glow = 10.0f;
   m_bulbSize = { 0.0f, 0.0f, 0.0f, 0.0f };
   m_wireFrame = false;
}