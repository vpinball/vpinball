#pragma once

#include "BitmapFilter.h"

class RegionFilter : public BitmapFilter
{
public:
   RegionFilter();
   ~RegionFilter();

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
   int m_x;
   int m_y;
   int m_width;
   int m_height;
};
