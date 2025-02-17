#pragma once

#include "BitmapFilter.h"

class DotFilter final : public BitmapFilter
{
public:
   DotFilter() = default;
   ~DotFilter() override = default;

   void Filter(Bitmap* pBitmap) override;

   int GetDotSize() const { return m_dotSize; }
   void SetDotSize(int dotSize) { m_dotSize = dotSize; }
   int GetOffset() const { return m_offset; }
   void SetOffset(int offset) { m_offset = offset; }

private:
   int m_dotSize = 2;
   int m_offset = 0;
};
