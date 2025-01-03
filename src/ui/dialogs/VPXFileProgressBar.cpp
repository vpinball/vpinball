#include "VPXFileProgressBar.h"


VPXFileProgressBar::VPXFileProgressBar(HINSTANCE app, HWND statusBar, CMDIChild* mdiTable)
   : m_app{app},
     m_statusBar{statusBar},
     m_mdiTable{mdiTable}
{
}


VPXFileProgressBar::~VPXFileProgressBar()
{
   if (m_progressBar != nullptr) {
      ::DestroyWindow(m_progressBar);
   }
}


void VPXFileProgressBar::OperationStarted()
{
   if (m_progressBar != nullptr) {
      return;
   }

   RECT rc;
   ::SendMessage(m_statusBar, SB_GETRECT, 2, (size_t)&rc);
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
      (HMENU)0,
      m_app,
      nullptr
   );

   ::SendMessage(m_progressBar, PBM_SETPOS, 1, 0);
}


void VPXFileProgressBar::AboutToProcessItems(int itemCount)
{
   ::SendMessage(m_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, itemCount));
}


void VPXFileProgressBar::ItemHasBeenProcessed(int totalItemsProcessed)
{
   ::SendMessage(m_progressBar, PBM_SETPOS, totalItemsProcessed, 0);
}


void VPXFileProgressBar::ErrorOccured(const char* error)
{
   m_mdiTable->MessageBox(error, "Visual Pinball", MB_ICONERROR);
}

void VPXFileProgressBar::Done()
{
   ::DestroyWindow(m_progressBar);
   m_progressBar = nullptr;
}
