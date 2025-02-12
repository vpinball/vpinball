#pragma once

#include "parts/VPXFileFeedback.h"

/**
 * Displays a progress bar using Win32 API to track progress of a table being loaded.
 */
class VPXLoadFileProgressBar: public VPXFileFeedback
{
   public:
      VPXLoadFileProgressBar(HINSTANCE app, HWND statusBar);
      ~VPXLoadFileProgressBar() override;

      void OperationStarted() override;
      void AboutToProcessTable(int elementCount) override;
      void ItemHasBeenProcessed(int itemsCount, int totalItems) override;
      void SoundHasBeenProcessed(int soundCount, int totalSounds) override;
      void ImageHasBeenProcessed(int imageCount, int totalImages) override;
      void FontHasBeenProcessed(int fontCount, int totalFonts) override;
      void CollectionHasBeenProcessed(int collectionCount, int totalCollections) override;
      void Done() override;

   private:
      void StepProgress(bool updateBar = true);

      HINSTANCE m_app;
      HWND m_statusBar;

      HWND m_progressBar = nullptr;
      int m_loadedElementCount = 0;
};
