#include "stdafx.h"

#include "B2SLEDBox.h"
#include "../classes/B2SData.h"
#include "../../common/GraphicsPath.h"

B2SLEDBox::B2SLEDBox() : B2SBaseBox()
{
   SetBackColor(RGB(0, 0, 0));
   SetVisible(true);

   m_pB2SData = B2SData::GetInstance();
}

void B2SLEDBox::OnResize()
{
   B2SData* pB2SData = B2SData::GetInstance();

   // resize the led segments and load it into the current segment array
   float width = (float)GetWidth() / (float)pB2SData->GetLEDCoordMax();
   float height = (float)GetHeight() / (float)pB2SData->GetLEDCoordMax();
   m_currentSeg.clear();

   vector<vector<SDL_FPoint>>* pLedData;
   switch (m_ledType) {
      case eLEDType_LED10:
         pLedData = pB2SData->GetLED10Seg();
         break;
      case eLEDType_LED14:
         pLedData = pB2SData->GetLED14Seg();
         break;
      case eLEDType_LED16:
         pLedData = pB2SData->GetLED16Seg();
         break;
      default:
         pLedData = pB2SData->GetLED8Seg();
         break;
    }
    for (const auto& entry : *pLedData) {
       vector<SDL_FPoint> scaled(entry.size());
       for (size_t i = 0; i < entry.size(); ++i)
          scaled[i] = { entry[i].x * width, entry[i].y * height };
       m_currentSeg.push_back(scaled);
    }
}

void B2SLEDBox::OnPaint(VP::RendererGraphics* pGraphics)
{
   if (IsVisible()) {
      pGraphics->TranslateTransform(GetLeft(), GetTop());

      // draw LED segments
      float width = (float)GetWidth() / (float)m_pB2SData->GetLEDCoordMax();
      float height = (float)GetHeight() / (float)m_pB2SData->GetLEDCoordMax();

      SDL_BlendMode blendMode = pGraphics->GetBlendMode();
      pGraphics->SetBlendMode(SDL_BLENDMODE_BLEND);

      // draw LED segments
      for (int i = 0; i < m_currentSeg.size(); i++) {
         GraphicsPath* pPath = new GraphicsPath();
         pPath->AddPolygon(&m_currentSeg[i]);
         if ((m_value & (1 << i)) != 0)
            pGraphics->SetColor(m_litLEDSegmentColor);
         else
            pGraphics->SetColor(m_darkLEDSegmentColor);

         pGraphics->FillPath(pPath);
         delete pPath;
      }

      pGraphics->SetBlendMode(blendMode);

      pGraphics->TranslateTransform(-GetLeft(), -GetTop());
   }

   Control::OnPaint(pGraphics);
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