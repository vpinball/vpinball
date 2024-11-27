#include "SaveTableWin32Visitor.h"


SaveTableWin32Visitor::SaveTableWin32Visitor(HINSTANCE app, HWND statusBar, CMDIChild* mdiTable)
   : m_app{app},
     m_statusBar{statusBar},
     m_mdiTable{mdiTable}
{
}


SaveTableWin32Visitor::~SaveTableWin32Visitor()
{
   if (m_progressBar != nullptr) {
      ::DestroyWindow(m_progressBar);
   }
}


void SaveTableWin32Visitor::SavingStarted()
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


void SaveTableWin32Visitor::AboutToSaveItems(int itemCount)
{
   ::SendMessage(m_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, itemCount));
}


void SaveTableWin32Visitor::ItemHasBeenSaved(int totalItemSaved)
{
   ::SendMessage(m_progressBar, PBM_SETPOS, totalItemSaved, 0);
}


void SaveTableWin32Visitor::ErrorOccured(const char* error)
{
   m_mdiTable->MessageBox(error, "Visual Pinball", MB_ICONERROR);
}

void SaveTableWin32Visitor::DoneSaving()
{
   ::DestroyWindow(m_progressBar);
   m_progressBar = nullptr;
}
