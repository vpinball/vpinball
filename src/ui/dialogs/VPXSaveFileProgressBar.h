#pragma once

#include "parts/VPXFileFeedback.h"


/**
 * Displays a progress bar using Win32 API to track progress of a table being saved.
 */
class VPXSaveFileProgressBar: public VPXFileFeedback
{
   public:
      VPXSaveFileProgressBar(HINSTANCE app, HWND statusBar, CMDIChild* mdiTable);
      ~VPXSaveFileProgressBar() override;

      void OperationStarted() override;
      void AboutToProcessTable(int elementCount) override;
      void ItemHasBeenProcessed(int itemsCount, int totalItems) override;
      void SoundHasBeenProcessed(int soundCount, int totalSounds) override;
      void ImageHasBeenProcessed(int imageCount, int totalImages) override;
      void FontHasBeenProcessed(int fontCount, int totalFonts) override;
      void CollectionHasBeenProcessed(int collectionCount, int totalCollections) override;
      void ErrorOccured(const char* error) override;
      void Done() override;

   private:
      void StepProgress();

      HINSTANCE m_app;
      HWND m_statusBar;
      CMDIChild* m_mdiTable;

      HWND m_progressBar = nullptr;
      int m_loadedElementCount = 0;
};
