#pragma once

#include "BitmapFilter.h"

class AdditiveFilter : public BitmapFilter
{
public:
   AdditiveFilter();
   ~AdditiveFilter();

   virtual void Filter(Bitmap* pBitmap);

private:
};
