#pragma once

#include "BitmapFilter.h"

class DotFilter : public BitmapFilter
{
public:
   DotFilter();
   ~DotFilter();

   void Filter(Bitmap* pBitmap) override;

   int GetDotSize() { return m_dotSize; }
   void SetDotSize(int dotSize) { m_dotSize = dotSize; }
   int GetOffset() { return m_offset; }
   void SetOffset(int m_offset) { m_offset = m_offset; }

private:
   int m_dotSize;
   int m_offset;
};
