// license:GPLv3+

#pragma once

#include "ui/VPXFileFeedback.h"

/**
 * Displays a progress bar using Win32 API to track progress of a table being loaded.
 */
class VPXLoadFileProgressBar final : public VPXFileFeedback
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
      void Update();

      int m_itemsCount = 0;
      int m_totalItems = 0;
      int m_soundCount = 0;
      int m_totalSounds = 0;
      int m_imageCount = 0;
      int m_totalImages = 0;
      int m_fontCount = 0;
      int m_totalFonts = 0;
      int m_collectionCount = 0;
      int m_totalCollections = 0;

      HINSTANCE m_app;
      HWND m_statusBar;

      HWND m_progressBar = nullptr;
};
