#pragma once


#include "B2SBaseBox.h"
#include "../utils/VPXGraphics.h"

namespace B2SLegacy {

class B2SLEDBox final : public B2SBaseBox
{
public:
   B2SLEDBox(VPXPluginAPI* vpxApi, B2SData* pB2SData);

   void OnPaint(VPXRenderContext2D* const ctx) override;
   void OnResize();
   uint32_t GetLitLEDSegmentColor() const { return m_litLEDSegmentColor; }
   void SetLitLEDSegmentColor(uint32_t litLEDSegmentColor) { m_litLEDSegmentColor = litLEDSegmentColor; }
   uint32_t GetDarkLEDSegmentColor() const { return m_darkLEDSegmentColor; }
   void SetDarkLEDSegmentColor(uint32_t darkLEDSegmentColor) { m_darkLEDSegmentColor = darkLEDSegmentColor; }
   eLEDType GetLEDType() const { return m_ledType; }
   void SetLEDType(eLEDType ledType);
   int GetValue() const { return m_value; }
   void SetValue(int value, bool refresh = false);
   string GetText() const { return m_text; }
   void SetText(const string& text);

private:
   vector<vector<SDL_FPoint>> m_currentSeg;
   uint32_t m_litLEDSegmentColor;
   uint32_t m_darkLEDSegmentColor;
   eLEDType m_ledType;
   int m_value;
   string m_text;

   std::unique_ptr<VPXGraphics> m_pGraphics;
};

}
