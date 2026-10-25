#pragma once

#include "../common.h"
#include "SegDisplay.h"
#include "SegGraphics.h"
#include "GraphicsPath.h"

namespace B2S::seg
{

typedef enum
{
   eLEDType_Undefined = 0,
   eLEDType_Standard = 1,
   eLEDType_Segment = 2,
   eLEDType_LED8 = 3,
   eLEDType_LED10 = 4,
   eLEDType_LED14 = 5,
   eLEDType_LED16 = 6
} eLEDType;

// Simple rendered LED digit: fills the segment polygons of its led type with the
// lit/dark segment colors, matching the reference B2SLEDBox.
class LEDBox final : public SegDisplay
{
public:
   LEDBox(const VPXPluginAPI* vpxApi);

   void Render(VPXRenderContext2D* const ctx);
   void OnResize() override;
   uint32_t GetLitLEDSegmentColor() const { return m_litLEDSegmentColor; }
   void SetLitLEDSegmentColor(uint32_t litLEDSegmentColor) { m_litLEDSegmentColor = litLEDSegmentColor; }
   uint32_t GetDarkLEDSegmentColor() const { return m_darkLEDSegmentColor; }
   void SetDarkLEDSegmentColor(uint32_t darkLEDSegmentColor) { m_darkLEDSegmentColor = darkLEDSegmentColor; }
   eLEDType GetLEDType() const { return m_ledType; }
   void SetLEDType(eLEDType ledType);
   int GetValue() const { return m_value; }
   void SetValue(int value, bool refresh = false);
   const string& GetText() const { return m_text; }
   void SetText(const string& text);

private:
   vector<vector<FPoint>> m_currentSeg;
   uint32_t m_litLEDSegmentColor = 0;
   uint32_t m_darkLEDSegmentColor = 0;
   eLEDType m_ledType = eLEDType_Undefined;
   int m_value = 0;
   string m_text;
};

}
