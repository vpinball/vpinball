#pragma once

#include "BitmapFilter.h"

class RegionFilter : public BitmapFilter
{
public:
   RegionFilter() = default;
   ~RegionFilter() = default;

   void Filter(Bitmap* pBitmap) override;

   int GetX() { return m_x; }
   void SetX(int x) { m_x = x; }
   int GetY() { return m_y; }
   void SetY(int y) { m_y = y; }
   int GetWidth() { return m_width; }
   void SetWidth(int width) { m_width = width; }
   int GetHeight() { return m_height; }
   void SetHeight(int height) { m_height = height; }

private:
   int m_x = 0;
   int m_y = 0;
   int m_width = 0;
   int m_height = 0;
};
