#pragma once

#include "BitmapFilter.h"

class PadFilter : public BitmapFilter
{
public:
   PadFilter();
   ~PadFilter();

   void Filter(Bitmap* pBitmap) override;

   int GetLeft() { return m_left; }
   void SetLeft(int left) { m_left = left; }
   int GetTop() { return m_top; }
   void SetTop(int top) { m_top = top; }
   int GetRight() { return m_right; }
   void SetRight(int right) { m_right = right; }
   int GetBottom() { return m_bottom; }
   void SetBottom(int bottom) { m_bottom = bottom; }

private:
   int m_left;
   int m_top;
   int m_right;
   int m_bottom;
};
