#pragma once

#include "../b2s_i.h"

#include "B2SBaseBox.h"

class B2SLEDBox : public B2SBaseBox
{
public:
   B2SLEDBox();

   void OnPaint(VP::RendererGraphics* pGraphics) override;
   void OnResize();

   OLE_COLOR GetLitLEDSegmentColor() const { return m_litLEDSegmentColor; }
   void SetLitLEDSegmentColor(OLE_COLOR litLEDSegmentColor) { m_litLEDSegmentColor = litLEDSegmentColor; }
   OLE_COLOR GetDarkLEDSegmentColor() const { return m_darkLEDSegmentColor; }
   void SetDarkLEDSegmentColor(OLE_COLOR darkLEDSegmentColor) { m_darkLEDSegmentColor = darkLEDSegmentColor; }
   eLEDType GetLEDType() const { return m_ledType; }
   void SetLEDType(eLEDType ledType);
   int GetValue() const { return m_value; }
   void SetValue(int value, bool refresh = false);
   string GetText() const { return m_text; }
   void SetText(const string& text);

private:
   vector<vector<SDL_FPoint>> m_currentSeg;
   OLE_COLOR m_litLEDSegmentColor;
   OLE_COLOR m_darkLEDSegmentColor;
   eLEDType m_ledType;
   int m_value;
   string m_text;

   B2SData* m_pB2SData;
};
