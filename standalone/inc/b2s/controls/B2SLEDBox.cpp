#include "stdafx.h"

#include "B2SLEDBox.h"
#include "../classes/B2SData.h"

B2SLEDBox::B2SLEDBox() : B2SBaseBox()
{
    SetBackColor(RGB(0, 0, 0));
    SetVisible(true);
}

B2SLEDBox::~B2SLEDBox()
{
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
       for (size_t i = 0; i < entry.size(); ++i) {
          scaled[i] = { entry[i].x * width, entry[i].y * height };
       }
       m_currentSeg.push_back(scaled);
    }
}

void B2SLEDBox::OnPaint(SDL_Surface* pSurface)
{
   B2SData* pB2SData = B2SData::GetInstance();

   // Create or recreate brushes
   static SDL_Color glowingbrush = {
      (UINT8)SDL_min(GetRValue(m_litLEDSegmentColor) + 20, 255),
      (UINT8)SDL_min(GetGValue(m_litLEDSegmentColor) + 20, 255),
      (UINT8)SDL_min(GetBValue(m_litLEDSegmentColor) + 20, 255), 255 };

   static SDL_Color litbrush = {
      GetRValue(m_litLEDSegmentColor),
      GetGValue(m_litLEDSegmentColor),
      GetBValue(m_litLEDSegmentColor), 255 };

   static SDL_Color litpen = {
      GetRValue(m_litLEDSegmentColor),
      GetGValue(m_litLEDSegmentColor),
      GetBValue(m_litLEDSegmentColor), 255 };

   static SDL_Color darkbrush = {
      GetRValue(m_darkLEDSegmentColor),
      GetGValue(m_darkLEDSegmentColor),
      GetBValue(m_darkLEDSegmentColor), 255 };

   if (litbrush.r != GetRValue(m_litLEDSegmentColor) ||
      litbrush.g != GetGValue(m_litLEDSegmentColor) ||
      litbrush.b != GetBValue(m_litLEDSegmentColor)) {
      litbrush = {
         GetRValue(m_litLEDSegmentColor),
         GetGValue(m_litLEDSegmentColor),
         GetBValue(m_litLEDSegmentColor), 255 };
      litpen = {
         GetRValue(m_litLEDSegmentColor),
         GetGValue(m_litLEDSegmentColor),
         GetBValue(m_litLEDSegmentColor), 255 };
   }

   if (darkbrush.r != GetRValue(m_darkLEDSegmentColor) ||
      darkbrush.g != GetGValue(m_darkLEDSegmentColor) ||
      darkbrush.b != GetBValue(m_darkLEDSegmentColor)) {
      darkbrush = {
         GetRValue(m_darkLEDSegmentColor),
         GetGValue(m_darkLEDSegmentColor),
         GetBValue(m_darkLEDSegmentColor), 255 };
   }

   // draw LED segments
   float width = (float)GetWidth() / (float)pB2SData->GetLEDCoordMax();
   float height = (float)GetHeight() / (float)pB2SData->GetLEDCoordMax();

   // draw LED segments
   SDL_Rect rect = GetRect();
   for (int i = 0; i < m_currentSeg.size(); i++) {
      if ((m_value & (1 << i)) != 0) {
         SDL_Surface* pSegmentSurface = CreatePolygonSurface(m_currentSeg[i]);
         SDL_BlitScaled(pSegmentSurface, NULL, pSurface, &rect);
         SDL_FreeSurface(pSegmentSurface);
         //e.Graphics.FillPolygon(litbrush, currentSeg[i]);
      }
      else {
         SDL_Surface* pSegmentSurface = CreatePolygonSurface(m_currentSeg[i]);
         SDL_BlitScaled(pSegmentSurface, NULL, pSurface, &rect);
         SDL_FreeSurface(pSegmentSurface);
         //e.Graphics.FillPolygon(darkbrush, currentSeg[i]);
      }
   }
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
   if (!text.empty()) {
      m_text = text;
        switch (m_ledType) {
            case eLEDType_Undefined:
            case eLEDType_LED8:
                switch (text.at(0)) {
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
                switch (text.at(0)) {
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
   }
}

bool pointInPolygon(const std::vector<SDL_FPoint>& polygon, float x, float y) {
    size_t n = polygon.size();
    bool result = false;

    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        float xi = polygon[i].x, yi = polygon[i].y;
        float xj = polygon[j].x, yj = polygon[j].y;

        if ((yi > y) != (yj > y) &&
            (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
            result = !result;
        }
    }

    return result;
}

SDL_Surface* B2SLEDBox::CreatePolygonSurface(const std::vector<SDL_FPoint>& polygon)
{
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    for(const auto& point : polygon) {
        minX = std::min(minX, point.x);
        minY = std::min(minY, point.y);
        maxX = std::max(maxX, point.x);
        maxY = std::max(maxY, point.y);
    }

    int width = static_cast<int>(maxX - minX);
    int height = static_cast<int>(maxY - minY);

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA8888);

    SDL_LockSurface(surface);
    Uint32* pixels = (Uint32*)surface->pixels;
    Uint32 blue = SDL_MapRGB(surface->format, 0, 0, 255);

    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            if(pointInPolygon(polygon, x + minX, y + minY)) {
                pixels[y * surface->pitch/4 + x] = blue;
            }
        }
    }

    SDL_UnlockSurface(surface);

    return surface;
}