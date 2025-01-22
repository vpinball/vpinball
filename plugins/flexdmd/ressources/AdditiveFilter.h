#pragma once

#include "BitmapFilter.h"

class AdditiveFilter : public BitmapFilter
{
public:
   AdditiveFilter() = default;
   ~AdditiveFilter() = default;

   void Filter(Bitmap* pBitmap) override;

private:
};
