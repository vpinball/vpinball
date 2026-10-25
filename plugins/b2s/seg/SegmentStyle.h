#pragma once

#include "SegTypes.h"

namespace B2S::seg
{

class SegmentStyle final
{
public:
   uint32_t GetOffColor() const { return m_offColor; }
   void SetOffColor(uint32_t color) { m_offColor = color; }
   uint32_t GetLightColor() const { return m_lightColor; }
   void SetLightColor(uint32_t lightColor) { m_lightColor = lightColor; }
   uint32_t GetGlassColor() const { return m_glassColor; }
   void SetGlassColor(uint32_t glassColor) { m_glassColor = glassColor; }
   uint32_t GetGlassColorCenter() const { return m_glassColorCenter; }
   void SetGlassColorCenter(uint32_t glassColorCenter) { m_glassColorCenter = glassColorCenter; }
   int GetGlassAlpha() const { return m_glassAlpha; }
   void SetGlassAlpha(int alpha) { m_glassAlpha = alpha; }
   uint8_t GetGlassAlphaCenter() const { return m_glassAlphaCenter; }
   void SetGlassAlphaCenter(const uint8_t alpha) { m_glassAlphaCenter = alpha; }
   float GetGlow() const { return m_glow; }
   void SetGlow(float glow) { m_glow = glow; }
   const FRect& GetBulbSize() { return m_bulbSize; }
   void SetBulbSize(const FRect& bulbSize) { m_bulbSize = bulbSize; }
   bool IsWireFrame() const { return m_wireFrame; }
   void SetWireFrame(bool wireFrame) { m_wireFrame = wireFrame; }

private:
   uint32_t m_offColor = RGB(20, 20, 20);
   uint32_t m_lightColor = RGB(254, 90, 50);
   uint32_t m_glassColor = RGB(254, 50, 25);
   uint32_t m_glassColorCenter = RGB(254, 50, 25);
   int m_glassAlpha = 140;
   uint8_t m_glassAlphaCenter = 255;
   float m_glow = 10.0f;
   FRect m_bulbSize = { 0.0f, 0.0f, 0.0f, 0.0f };
   bool m_wireFrame = false;
};

}
