#include "core/stdafx.h"
#include "VPXLoadFileProgressBar.h"


VPXLoadFileProgressBar::VPXLoadFileProgressBar(HINSTANCE app, HWND statusBar)
   : m_app{app},
     m_statusBar{statusBar}
{
}


VPXLoadFileProgressBar::~VPXLoadFileProgressBar()
{
   if (m_progressBar != nullptr) {
      ::DestroyWindow(m_progressBar);
   }
}


void VPXLoadFileProgressBar::OperationStarted()
{
   const LocalString loadingText(IDS_LOADING);
   ::SendMessage(m_statusBar, SB_SETTEXT, 3 | 0, (LPARAM)loadingText.m_szbuffer);

   const HCURSOR cursor = ::LoadCursor(nullptr, IDC_WAIT);
   ::SetCursor(cursor);

   if (m_progressBar != nullptr) {
      return;
   }

   RECT rc;
   ::SendMessage(m_statusBar, SB_GETRECT, 2, (LPARAM)&rc);

   m_progressBar = ::CreateWindowEx(
      0,
      PROGRESS_CLASS,
      (LPSTR)nullptr,
      WS_CHILD | WS_VISIBLE,
      rc.left,
      rc.top,
      rc.right - rc.left,
      rc.bottom - rc.top,
      m_statusBar,
      (HMENU)nullptr,
      m_app, 
      nullptr
   );

   ::SendMessage(m_progressBar, PBM_SETPOS, 1, 0);
};


void VPXLoadFileProgressBar::AboutToProcessTable(int elementCount)
{
   m_loadedElementCount = 0;
   ::SendMessage(m_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, elementCount));
}


void VPXLoadFileProgressBar::StepProgress(bool updateBar)
{
   m_loadedElementCount++;

   if (updateBar && m_progressBar != nullptr) {
      ::SendMessage(m_progressBar, PBM_SETPOS, m_loadedElementCount, 0);
   }
}


void VPXLoadFileProgressBar::ItemHasBeenProcessed(int itemsCount, int totalItems)
{
   StepProgress();
}


void VPXLoadFileProgressBar::SoundHasBeenProcessed(int soundCount, int totalSounds)
{
   StepProgress();
}


void VPXLoadFileProgressBar::ImageHasBeenProcessed(int imageCount, int totalImages)
{
   // Only update item count. Since we know that mage are loaded in a multi-threaded
   // environment, we prefere not to update the GUI progress bar to avoid any threading issues.
   StepProgress(false);
}


void VPXLoadFileProgressBar::FontHasBeenProcessed(int fontCount, int totalFonts)
{
   StepProgress();
}


void VPXLoadFileProgressBar::CollectionHasBeenProcessed(int collectionCount, int totalCollection)
{
   StepProgress();
}


void VPXLoadFileProgressBar::Done()
{
   ::SendMessage(m_statusBar, SB_SETTEXT, 3 | 0, (LPARAM)L"");

   const HCURSOR cursor = ::LoadCursor(nullptr, IDC_ARROW);
   ::SetCursor(cursor);

   if (m_progressBar == nullptr) {
      return;
   }

   ::DestroyWindow(m_progressBar);
   m_progressBar = nullptr;
}
