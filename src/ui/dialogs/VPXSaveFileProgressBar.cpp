#include "core/stdafx.h"
#include "VPXSaveFileProgressBar.h"


VPXSaveFileProgressBar::VPXSaveFileProgressBar(HINSTANCE app, HWND statusBar, CMDIChild* mdiTable)
   : m_app{app},
     m_statusBar{statusBar},
     m_mdiTable{mdiTable}
{
}


VPXSaveFileProgressBar::~VPXSaveFileProgressBar()
{
   if (m_progressBar != nullptr) {
      ::DestroyWindow(m_progressBar);
   }
}


void VPXSaveFileProgressBar::OperationStarted()
{
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
}


void VPXSaveFileProgressBar::AboutToProcessTable(int elementCount)
{
   m_loadedElementCount = 0;

   if (m_progressBar != nullptr) {
      ::SendMessage(m_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, elementCount));
   }
}


void VPXSaveFileProgressBar::StepProgress()
{
   m_loadedElementCount++;

   if (m_progressBar != nullptr) {
      ::SendMessage(m_progressBar, PBM_SETPOS, m_loadedElementCount, 0);
   }
}


void VPXSaveFileProgressBar::ItemHasBeenProcessed(int itemsCount, int totalItems)
{
   StepProgress();
}


void VPXSaveFileProgressBar::SoundHasBeenProcessed(int soundCount, int totalSounds)
{
   StepProgress();
}


void VPXSaveFileProgressBar::ImageHasBeenProcessed(int imageCount, int totalImages)
{
   StepProgress();
}


void VPXSaveFileProgressBar::FontHasBeenProcessed(int fontCount, int totalFonts)
{
   StepProgress();
}


void VPXSaveFileProgressBar::CollectionHasBeenProcessed(int collectionCount, int totalCollections)
{
   StepProgress();
}


void VPXSaveFileProgressBar::ErrorOccured(const char* error)
{
   m_mdiTable->MessageBox(error, "Visual Pinball", MB_ICONERROR);
}

void VPXSaveFileProgressBar::Done()
{
   if (m_progressBar == nullptr) {
      return;
   }

   ::DestroyWindow(m_progressBar);
   m_progressBar = nullptr;
}
