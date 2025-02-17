#pragma once

#include "BitmapFilter.h"

class RegionFilter final : public BitmapFilter
{
public:
   RegionFilter() = default;
   ~RegionFilter() override = default;

   void Filter(Bitmap* pBitmap) override;

   int GetX() const { return m_x; }
   void SetX(int x) { m_x = x; }
   int GetY() const { return m_y; }
   void SetY(int y) { m_y = y; }
   int GetWidth() const { return m_width; }
   void SetWidth(int width) { m_width = width; }
   int GetHeight() const { return m_height; }
   void SetHeight(int height) { m_height = height; }

private:
   int m_x = 0;
   int m_y = 0;
   int m_width = 0;
   int m_height = 0;
};
