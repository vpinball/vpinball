#pragma once

#include "core/stdafx.h"

#include "parts/SaveTableVisitor.h"


/**
 * This visitor displays a progress bar using Win32 API to track to progress
 * of a table being saved.
 */
class SaveTableWin32Visitor: public SaveTableVisitor
{
   public:
      SaveTableWin32Visitor(HINSTANCE app, HWND statusBar, CMDIChild* mdiTable);
      ~SaveTableWin32Visitor() override;

      void SavingStarted() override;
      void AboutToSaveItems(int itemCount) override;
      void ItemHasBeenSaved(int totalItemSaved) override;
      void ErrorOccured(const char* error) override;
      void DoneSaving() override;

   private:
      HINSTANCE m_app;
      HWND m_statusBar;
      CMDIChild* m_mdiTable;

      HWND m_progressBar = nullptr;
};
