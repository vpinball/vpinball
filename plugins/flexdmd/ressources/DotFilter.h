#pragma once

#include "BitmapFilter.h"

class DotFilter : public BitmapFilter
{
public:
   DotFilter() = default;
   ~DotFilter() = default;

   void Filter(Bitmap* pBitmap) override;

   int GetDotSize() { return m_dotSize; }
   void SetDotSize(int dotSize) { m_dotSize = dotSize; }
   int GetOffset() { return m_offset; }
   void SetOffset(int m_offset) { m_offset = m_offset; }

private:
   int m_dotSize = 2;
   int m_offset = 0;
};
