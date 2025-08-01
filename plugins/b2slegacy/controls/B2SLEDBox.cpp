#include "../common.h"

#include "B2SLEDBox.h"
#include "../utils/GraphicsPath.h"

namespace B2SLegacy {

B2SLEDBox::B2SLEDBox(VPXPluginAPI* vpxApi, B2SData* pB2SData)
   : B2SBaseBox(vpxApi, pB2SData)
{
   SetBackColor(RGB(0, 0, 0));
   SetVisible(true);
}

void B2SLEDBox::OnResize()
{
   if (GetWidth() > 0 && GetHeight() > 0)
      m_pGraphics = std::make_unique<VPXGraphics>(m_vpxApi, GetWidth(), GetHeight());

   // resize the led segments and load it into the current segment array
   float width = (float)GetWidth() / (float)m_pB2SData->GetLEDCoordMax();
   float height = (float)GetHeight() / (float)m_pB2SData->GetLEDCoordMax();
   m_currentSeg.clear();

   vector<vector<SDL_FPoint>>* pLedData;
   switch (m_ledType) {
      case eLEDType_LED10:
         pLedData = m_pB2SData->GetLED10Seg();
         break;
      case eLEDType_LED14:
         pLedData = m_pB2SData->GetLED14Seg();
         break;
      case eLEDType_LED16:
         pLedData = m_pB2SData->GetLED16Seg();
         break;
      default:
         pLedData = m_pB2SData->GetLED8Seg();
         break;
    }
    for (const auto& entry : *pLedData) {
       vector<SDL_FPoint> scaled(entry.size());
       for (size_t i = 0; i < entry.size(); ++i)
          scaled[i] = { entry[i].x * width, entry[i].y * height };
       m_currentSeg.push_back(scaled);
    }
}

void B2SLEDBox::OnPaint(VPXRenderContext2D* const ctx)
{
   if (IsVisible()) {
      if (!m_pGraphics) {
         if (GetWidth() > 0 && GetHeight() > 0) {
            m_pGraphics = std::make_unique<VPXGraphics>(m_vpxApi, GetWidth(), GetHeight());
         } else {
            Control::OnPaint(ctx);
            return;
         }
      }

      m_pGraphics->Clear();
      m_pGraphics->TranslateTransform(GetLeft(), GetTop());

      float width = (float)GetWidth() / (float)m_pB2SData->GetLEDCoordMax();
      float height = (float)GetHeight() / (float)m_pB2SData->GetLEDCoordMax();

      for (int i = 0; i < m_currentSeg.size(); i++) {
         GraphicsPath* pPath = new GraphicsPath();
         pPath->AddPolygon(&m_currentSeg[i]);
         if ((m_value & (1 << i)) != 0)
            m_pGraphics->SetColor(m_litLEDSegmentColor);
         else
            m_pGraphics->SetColor(m_darkLEDSegmentColor);

         m_pGraphics->FillPath(pPath);
         delete pPath;
      }

      m_pGraphics->TranslateTransform(-GetLeft(), -GetTop());
      m_pGraphics->DrawToContext(ctx, GetLeft(), GetTop());
   }

   Control::OnPaint(ctx);
}

void B2SLEDBox::SetLEDType(eLEDType ledType)
{
   if (m_ledType != ledType) {
      m_ledType = ledType;
      Invalidate();
   }
}

void B2SLEDBox::SetValue(int value, bool refresh)
{
   if (m_value != value) {
      m_value = value;
      Refresh();
   }
}

void B2SLEDBox::SetText(const string& text)
{
   if (m_text != text && !text.empty()) {
      m_text = text;
      switch (m_ledType) {
         case eLEDType_Undefined:
         case eLEDType_LED8:
            switch (text[0]) {
               case ' ' : m_value = 0; break;
               case '0' : m_value = 63; break;
               case '1' : m_value = 6; break;
               case '2' : m_value = 91; break;
               case '3' : m_value = 79; break;
               case '4' : m_value = 102; break;
               case '5' : m_value = 109; break;
               case '6' : m_value = 125; break;
               case '7' : m_value = 7; break;
               case '8' : m_value = 127; break;
               case '9' : m_value = 111; break;
            }
            break;
         case eLEDType_LED10:
            switch (text[0]) {
               case ' ' : m_value = 0; break;
               case '0' : m_value = 63; break;
               case '1' : m_value = 768; break;
               case '2' : m_value = 91; break;
               case '3' : m_value = 79; break;
               case '4' : m_value = 102; break;
               case '5' : m_value = 109; break;
               case '6' : m_value = 124; break;
               case '7' : m_value = 7; break;
               case '8' : m_value = 127; break;
               case '9' : m_value = 103; break;
            }
            break;
         case eLEDType_LED14:
            // TODO
            break;
         case eLEDType_LED16:
            // not implemented right now
            break;
      }
      Refresh();
   }
}

}
