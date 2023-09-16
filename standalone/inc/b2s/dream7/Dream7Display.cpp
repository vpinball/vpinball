#include "stdafx.h"

#include "Dream7Display.h"

Dream7Display::Dream7Display() : Control()
{
   m_hidden = false;
   m_szText = "";
   m_scaleMode = ScaleMode_Stretch;
   m_spacing = 20.0f;
   m_transparentBackground = false;
   m_segmentNumberType = SegmentNumberType_SevenSegment;
   m_offColor = RGB(20, 20, 20);
   m_shear = 0.1f;
   m_thickness = 16.0f;
   m_scaleFactor = 0.5f;
   m_offsetWidth = 0;
   m_angle = 0.0f;
}

Dream7Display::~Dream7Display()
{
}

void Dream7Display::SetText(const string& szText)
{
   m_szText = szText;
   int nLen = 0;
   if (!m_szText.empty())
      nLen = m_szText.length() - 1;
    
   int nIndex = 0;
   for (int nSegment = 0; nSegment <= nLen; nSegment++) {
      if (nIndex >= m_segmentNumbers.size())
         break;
      char sChar = ' ';
      //if (!m_szText.empty() && m_szText.length() > nSegment)
      //   sChar = m_szText.substr(nSegment, 1);
      //if (sChar == '.' && nIndex > 0 && !m_segmentNumbers[nIndex - 1].Character.EndsWith('.')) 
      //   m_segmentNumbers[nIndex - 1].Character += '.';
      //else {
      //   m_segmentNumbers[nIndex].Character = sChar;
      //   nIndex++;
      //}
   }
}

void Dream7Display::SetTransparentBackground(const bool transparentBackground)
{
   m_transparentBackground = transparentBackground;
   if (!transparentBackground) {
      // Me.DoubleBuffered = True
      // Me.SetStyle(ControlStyles.Opaque, False)
      // Me.SetStyle(ControlStyles.OptimizedDoubleBuffer, True)    
   }
   else {
      // Me.DoubleBuffered = False
      // Me.SetStyle(ControlStyles.Opaque, True)
      // Me.SetStyle(ControlStyles.OptimizedDoubleBuffer, False)
   }
}

void Dream7Display::SetOffColor(const OLE_COLOR offColor)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.OffColor = offColor;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetLightColor(const OLE_COLOR lightColor)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.LightColor = lightColor;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetGlassColor(const OLE_COLOR glassColor)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.GlassColor = glassColor;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetGlassColorCenter(const OLE_COLOR glassColorCenter)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.GlassColorCenter = glassColorCenter;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetGlassAlpha(const int glassAlpha)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.GlassAlpha = glassAlpha;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetGlassAlphaCenter(const int glassAlphaCenter)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.GlassAlphaCenter = glassAlphaCenter;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetGlow(const float glow)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.Glow = glow;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetBulbSize(const SDL_FRect& sizeF)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.BulbSize = sizeF;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetWireFrame(const bool wireFrame)
{
   for (int i = 0; i < m_segmentNumbers.size(); i++) {
      //m_segmentNumbers[i].Style.WireFrame = wireFrame;
      //m_segmentNumbers[i].AssignStyle();
   }
}

void Dream7Display::SetValue(int segment, string value)
{
}

void Dream7Display::SetValue(int segment, int value)
{
}
