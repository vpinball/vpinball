#pragma once

#include "BitmapFilter.h"

namespace Flex {

class AdditiveFilter final : public BitmapFilter
{
public:
   AdditiveFilter() = default;
   ~AdditiveFilter() override = default;

   void Filter(Bitmap* pBitmap) override;
};

}