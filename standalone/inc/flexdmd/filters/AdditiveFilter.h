#pragma once

#include "BitmapFilter.h"

class AdditiveFilter : public BitmapFilter
{
public:
   AdditiveFilter();
   ~AdditiveFilter();

   void Filter(Bitmap* pBitmap) override;

private:
};
