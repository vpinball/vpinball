#include "../common.h"

#include "LEDBox.h"

namespace B2S::seg
{

// Maximum of the segment polygon coordinate space (reference B2SData::m_ledCoordMax)
static constexpr int c_ledCoordMax = 103;

// Segment polygon vertex tables (reference B2SData led segments), in a 103x103 box
static const vector<vector<FPoint>>& GetLEDSegments(eLEDType ledType)
{
   static const vector<vector<FPoint>> led8Seg = []()
   {
      constexpr int toleft = 8;
      vector<vector<FPoint>> segs;
      segs.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 29, 11 } });
      segs.push_back({ { 93, 7 }, { 96, 10 }, { 96 - toleft, 46 }, { 93 - toleft, 49 }, { 87 - toleft, 43 }, { 87, 12 } });
      segs.push_back({ { 92 - toleft, 51 }, { 95 - toleft, 54 }, { 96 - 2 * toleft, 90 }, { 93 - 2 * toleft, 93 }, { 87 - 2 * toleft, 88 }, { 86 - toleft, 57 } });
      segs.push_back({ { 22 - 2 * toleft, 95 }, { 29 - 2 * toleft, 89 }, { 85 - 2 * toleft, 89 }, { 92 - 2 * toleft, 95 }, { 88 - 2 * toleft, 98 }, { 26 - 2 * toleft, 98 } });
      segs.push_back({ { 20 - toleft, 51 }, { 26 - toleft, 57 }, { 27 - 2 * toleft, 88 }, { 21 - 2 * toleft, 93 }, { 17 - 2 * toleft, 90 }, { 17 - toleft, 54 } });
      segs.push_back({ { 21, 7 }, { 27, 12 }, { 27 - toleft, 43 }, { 21 - toleft, 49 }, { 18 - toleft, 46 }, { 18, 10 } });
      segs.push_back({ { 23 - toleft, 50 }, { 27 - toleft, 46 }, { 86 - toleft, 46 }, { 90 - toleft, 50 }, { 86 - toleft, 54 }, { 27 - toleft, 54 } });
      return segs;
   }();
   static const vector<vector<FPoint>> led10Seg = []()
   {
      constexpr int toleft = 8;
      vector<vector<FPoint>> segs;
      segs.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 72, 11 }, { 67, 6 }, { 62, 11 }, { 29, 11 } });
      segs.push_back({ { 93, 7 }, { 96, 10 }, { 96 - toleft, 46 }, { 93 - toleft, 49 }, { 87 - toleft, 43 }, { 87, 12 } });
      segs.push_back({ { 92 - toleft, 51 }, { 95 - toleft, 54 }, { 96 - 2 * toleft, 90 }, { 93 - 2 * toleft, 93 }, { 87 - 2 * toleft, 88 }, { 86 - toleft, 57 } });
      segs.push_back({ { 22 - 2 * toleft, 95 }, { 29 - 2 * toleft, 89 }, { 61 - 2 * toleft, 89 }, { 66 - 2 * toleft, 94 }, { 71 - 2 * toleft, 89 }, { 85 - 2 * toleft, 89 },
         { 92 - 2 * toleft, 95 }, { 88 - 2 * toleft, 98 }, { 26 - 2 * toleft, 98 } });
      segs.push_back({ { 20 - toleft, 51 }, { 26 - toleft, 57 }, { 27 - 2 * toleft, 88 }, { 21 - 2 * toleft, 93 }, { 17 - 2 * toleft, 90 }, { 17 - toleft, 54 } });
      segs.push_back({ { 21, 7 }, { 27, 12 }, { 27 - toleft, 43 }, { 21 - toleft, 49 }, { 18 - toleft, 46 }, { 18, 10 } });
      segs.push_back({ { 23 - toleft, 50 }, { 27 - toleft, 46 }, { 63 - toleft, 46 }, { 68 - toleft, 51 }, { 73 - toleft, 46 }, { 86 - toleft, 46 }, { 90 - toleft, 50 }, { 86 - toleft, 54 },
         { 72 - toleft, 54 }, { 67 - toleft, 49 }, { 62 - toleft, 54 }, { 27 - toleft, 54 } });
      segs.push_back({ { 67, 9 }, { 71, 13 }, { 71 - toleft, 45 }, { 67 - toleft, 49 }, { 63 - toleft, 45 }, { 63, 13 } });
      segs.push_back({ { 68, 7 }, { 72, 11 }, { 72 - toleft, 43 }, { 68 - toleft, 47 }, { 64 - toleft, 43 }, { 64, 7 } });
      segs.push_back({ { 66 - toleft, 51 }, { 70 - toleft, 55 }, { 70 - 2 * toleft, 88 }, { 66 - 2 * toleft, 92 }, { 62 - 2 * toleft, 88 }, { 62 - toleft, 51 } });
      return segs;
   }();
   static const vector<vector<FPoint>> led14Seg = []()
   {
      constexpr int toleft = 8;
      vector<vector<FPoint>> segs;
      segs.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 29, 11 } });
      segs.push_back({ { 93, 7 }, { 96, 10 }, { 96 - toleft, 46 }, { 93 - toleft, 49 }, { 87 - toleft, 43 }, { 87, 12 } });
      segs.push_back({ { 92 - toleft, 51 }, { 95 - toleft, 54 }, { 96 - 2 * toleft, 90 }, { 93 - 2 * toleft, 93 }, { 87 - 2 * toleft, 88 }, { 86 - toleft, 57 } });
      segs.push_back({ { 22 - 2 * toleft, 95 }, { 29 - 2 * toleft, 89 }, { 85 - 2 * toleft, 89 }, { 92 - 2 * toleft, 95 }, { 88 - 2 * toleft, 98 }, { 26 - 2 * toleft, 98 } });
      segs.push_back({ { 20 - toleft, 51 }, { 26 - toleft, 57 }, { 27 - 2 * toleft, 88 }, { 21 - 2 * toleft, 93 }, { 17 - 2 * toleft, 90 }, { 17 - toleft, 54 } });
      segs.push_back({ { 21, 7 }, { 27, 12 }, { 27 - toleft, 43 }, { 21 - toleft, 49 }, { 18 - toleft, 46 }, { 18, 10 } });
      segs.push_back({ { 23 - toleft, 50 }, { 27 - toleft, 46 }, { 52 - toleft, 46 }, { 55 - toleft, 50 }, { 52 - toleft, 54 }, { 27 - toleft, 54 } });
      segs.push_back({ { 104 - 2 * toleft, 87 }, { 109 - 2 * toleft, 90 }, { 109 - 2 * toleft, 95 }, { 104 - 2 * toleft, 99 }, { 100 - 2 * toleft, 95 }, { 100 - 2 * toleft, 90 } });
      segs.push_back({ { 30, 13 }, { 34, 17 }, { 54 - toleft, 38 }, { 51 - toleft, 43 }, { 48 - toleft, 40 }, { 27, 16 } });
      segs.push_back({ { 57, 13 }, { 61, 13 }, { 61 - toleft, 46 }, { 57 - toleft, 48 }, { 53 - toleft, 46 }, { 53, 13 } });
      segs.push_back({ { 82, 13 }, { 85, 16 }, { 68 - toleft, 42 }, { 65 - toleft, 44 }, { 63 - toleft, 39 }, { 77, 17 } });
      segs.push_back({ { 58 - toleft, 50 }, { 62 - toleft, 46 }, { 86 - toleft, 46 }, { 90 - toleft, 50 }, { 86 - toleft, 54 }, { 62 - toleft, 54 } });
      segs.push_back({ { 82 - 2 * toleft, 85 }, { 87 - 2 * toleft, 86 }, { 67 - toleft, 57 }, { 62 - toleft, 57 }, { 62 - toleft, 60 }, { 79 - 2 * toleft, 86 } });
      segs.push_back({ { 57 - toleft, 52 }, { 61 - toleft, 54 }, { 61 - 2 * toleft, 88 }, { 57 - 2 * toleft, 88 }, { 53 - 2 * toleft, 88 }, { 53 - toleft, 54 } });
      segs.push_back({ { 30 - 2 * toleft, 83 }, { 33 - 2 * toleft, 86 }, { 50 - toleft, 60 }, { 47 - toleft, 57 }, { 42 - toleft, 61 }, { 27 - 2 * toleft, 86 } });
      segs.push_back({ { 102 - 2 * toleft, 97 }, { 107 - 2 * toleft, 100 }, { 107 - 2 * toleft, 105 }, { 102 - 2 * toleft, 109 }, { 98 - 2 * toleft, 105 }, { 98 - 2 * toleft, 100 } });
      return segs;
   }();
   static const vector<vector<FPoint>> led16Seg = []()
   {
      vector<vector<FPoint>> segs;
      segs.push_back({ { 22, 5 }, { 26, 2 }, { 88, 2 }, { 92, 5 }, { 85, 11 }, { 29, 11 } });
      return segs;
   }();
   switch (ledType)
   {
   case eLEDType_LED10: return led10Seg;
   case eLEDType_LED14: return led14Seg;
   case eLEDType_LED16: return led16Seg;
   case eLEDType_LED8:
   default: return led8Seg;
   }
}

LEDBox::LEDBox(const VPXPluginAPI* vpxApi)
   : SegDisplay(vpxApi)
{
   SetBackColor(RGB(5, 5, 5));
   SetVisible(true);
}

void LEDBox::OnResize()
{
   m_pGraphics.reset();
   if (GetWidth() > 0 && GetHeight() > 0)
      m_pGraphics = std::make_unique<SegGraphics>(m_vpxApi, GetWidth(), GetHeight());

   // resize the led segments and load it into the current segment array
   float width = (float)GetWidth() / (float)c_ledCoordMax;
   float height = (float)GetHeight() / (float)c_ledCoordMax;
   m_currentSeg.clear();

   const vector<vector<FPoint>>& pLedData = GetLEDSegments(m_ledType);
   for (const auto& entry : pLedData)
   {
      vector<FPoint> scaled(entry.size());
      for (size_t i = 0; i < entry.size(); ++i)
         scaled[i] = { entry[i].x * width, entry[i].y * height };
      m_currentSeg.push_back(std::move(scaled));
   }

   Invalidate();
}

void LEDBox::Render(VPXRenderContext2D* const ctx)
{
   if (IsVisible())
   {
      if (!m_pGraphics && GetWidth() > 0 && GetHeight() > 0)
      {
         m_pGraphics = std::make_unique<SegGraphics>(m_vpxApi, GetWidth(), GetHeight());
         Invalidate();
      }

      if (m_pGraphics)
      {
         if (IsInvalidated())
         {
            m_pGraphics->Clear();
            OnPaintBackground(m_pGraphics.get());
            for (int i = 0; i < (int)m_currentSeg.size(); i++)
            {
               GraphicsPath path;
               path.AddPolygon(&m_currentSeg[i]);
               if ((m_value & (1 << i)) != 0)
                  m_pGraphics->SetColor(m_litLEDSegmentColor);
               else
                  m_pGraphics->SetColor(m_darkLEDSegmentColor);

               m_pGraphics->FillPath(path);
            }
            ClearInvalidated();
         }

         m_pGraphics->DrawToContext(ctx, GetLeft(), GetTop());
      }
   }
}

void LEDBox::SetLEDType(eLEDType ledType)
{
   if (m_ledType != ledType)
   {
      m_ledType = ledType;
      OnResize(); // rebuild the segment shapes for the new type (also invalidates)
   }
}

void LEDBox::SetValue(int value, bool refresh)
{
   if (m_value != value || refresh)
   {
      m_value = value;
      Invalidate();
   }
}

void LEDBox::SetText(const string& text)
{
   if (m_text != text && !text.empty())
   {
      m_text = text;
      switch (m_ledType)
      {
      case eLEDType_Undefined:
      case eLEDType_LED8:
         switch (text[0])
         {
         case ' ': m_value = 0; break;
         case '0': m_value = 63; break;
         case '1': m_value = 6; break;
         case '2': m_value = 91; break;
         case '3': m_value = 79; break;
         case '4': m_value = 102; break;
         case '5': m_value = 109; break;
         case '6': m_value = 125; break;
         case '7': m_value = 7; break;
         case '8': m_value = 127; break;
         case '9': m_value = 111; break;
         }
         break;
      case eLEDType_LED10:
         switch (text[0])
         {
         case ' ': m_value = 0; break;
         case '0': m_value = 63; break;
         case '1': m_value = 768; break;
         case '2': m_value = 91; break;
         case '3': m_value = 79; break;
         case '4': m_value = 102; break;
         case '5': m_value = 109; break;
         case '6': m_value = 124; break;
         case '7': m_value = 7; break;
         case '8': m_value = 127; break;
         case '9': m_value = 103; break;
         }
         break;
      case eLEDType_LED14:
         // TODO
         break;
      case eLEDType_LED16:
         // not implemented right now
         break;
      default: break;
      }
      Invalidate();
   }
}

}
