#pragma once

#include "../b2s_i.h"

#include <map>

class SegmentStyle 
{
public:
   SegmentStyle();
   
   OLE_COLOR GetOffColor() const { return m_offColor; }
   void SetOffColor(OLE_COLOR color) { m_offColor = color; }
   OLE_COLOR GetLightColor() const { return m_lightColor; }
   void SetLightColor(OLE_COLOR color) { m_lightColor = color; }
   OLE_COLOR GetGlassColor() const { return m_glassColor; }
   void SetGlassColor(OLE_COLOR color) { m_glassColor = color; }
   OLE_COLOR GetGlassColorCenter() const { return m_glassColorCenter; }
   void SetGlassColorCenter(OLE_COLOR color) { m_glassColorCenter = color; }
   int GetGlassAlpha() const { return m_glassAlpha; }
   void SetGlassAlpha(int alpha) { m_glassAlpha = alpha; }
   int GetGlassAlphaCenter() const { return m_glassAlphaCenter; }
   void SetGlassAlphaCenter(int alpha) { m_glassAlphaCenter = alpha; }
   float GetGlow() const { return m_glow; }
   void SetGlow(float glow) { m_glow = glow; }
   SDL_FRect& GetBulbSize() { return m_bulbSize; }
   void SetBulbSize(const SDL_FRect& bulbSize) { m_bulbSize = bulbSize; }
   bool IsWireFrame() const { return m_wireFrame; }
   void SetWireFrame(bool wireFrame) { m_wireFrame = wireFrame; }

private:
   OLE_COLOR m_offColor;
   OLE_COLOR m_lightColor;
   OLE_COLOR m_glassColor;
   OLE_COLOR m_glassColorCenter;
   int m_glassAlpha;
   int m_glassAlphaCenter;
   float m_glow;
   SDL_FRect m_bulbSize;
   bool m_wireFrame;
};