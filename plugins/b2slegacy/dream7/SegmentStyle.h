#pragma once


#include <map>


namespace B2SLegacy {

class SegmentStyle
{
public:
   SegmentStyle();

   uint32_t GetOffColor() const { return m_offColor; }
   void SetOffColor(uint32_t color) { m_offColor = color; }
   uint32_t GetLightColor() const { return m_lightColor; }
   void SetLightColor(uint32_t color) { m_lightColor = color; }
   uint32_t GetGlassColor() const { return m_glassColor; }
   void SetGlassColor(uint32_t color) { m_glassColor = color; }
   uint32_t GetGlassColorCenter() const { return m_glassColorCenter; }
   void SetGlassColorCenter(uint32_t color) { m_glassColorCenter = color; }
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
   uint32_t m_offColor;
   uint32_t m_lightColor;
   uint32_t m_glassColor;
   uint32_t m_glassColorCenter;
   int m_glassAlpha;
   int m_glassAlphaCenter;
   float m_glow;
   SDL_FRect m_bulbSize;
   bool m_wireFrame;
};

}
