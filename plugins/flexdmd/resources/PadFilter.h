#pragma once

#include "BitmapFilter.h"

class PadFilter final : public BitmapFilter
{
public:
   PadFilter() = default;
   ~PadFilter() override = default;

   void Filter(Bitmap* pBitmap) override;

   int GetLeft() const { return m_left; }
   void SetLeft(int left) { m_left = left; }
   int GetTop() const { return m_top; }
   void SetTop(int top) { m_top = top; }
   int GetRight() const { return m_right; }
   void SetRight(int right) { m_right = right; }
   int GetBottom() const { return m_bottom; }
   void SetBottom(int bottom) { m_bottom = bottom; }

private:
   int m_left = 0;
   int m_top = 0;
   int m_right = 0;
   int m_bottom = 0;
};
