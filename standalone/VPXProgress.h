#pragma once

#include "core/stdafx.h"
#include "parts/VPXFileFeedback.h"


class VPXProgress: public VPXFileFeedback
{
   public:
      void ItemHasBeenProcessed(int itemsCount, int totalItems) override;
      void SoundHasBeenProcessed(int soundCount, int totalSounds) override;
      void ImageHasBeenProcessed(int imageCount, int totalImages) override;
      void FontHasBeenProcessed(int fontCount, int totalFonts) override;
      void CollectionHasBeenProcessed(int collectionCount, int totalCollections) override;
};
