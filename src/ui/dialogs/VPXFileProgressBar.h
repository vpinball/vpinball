#pragma once

#include "core/stdafx.h"

#include "parts/VPXFileFeedback.h"


/**
 * This visitor displays a progress bar using Win32 API to track to progress
 * of a table being saved.
 */
class VPXFileProgressBar: public VPXFileFeedback
{
   public:
      VPXFileProgressBar(HINSTANCE app, HWND statusBar, CMDIChild* mdiTable);
      ~VPXFileProgressBar() override;

      void OperationStarted() override;
      void AboutToProcessItems(int itemCount) override;
      void ItemHasBeenProcessed(int totalItemsProcessed) override;
      void ErrorOccured(const char* error) override;
      void Done() override;

   private:
      HINSTANCE m_app;
      HWND m_statusBar;
      CMDIChild* m_mdiTable;

      HWND m_progressBar = nullptr;
};
