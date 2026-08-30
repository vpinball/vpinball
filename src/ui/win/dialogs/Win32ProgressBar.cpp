// license:GPLv3+

#include "core/stdafx.h"
#include "Win32ProgressBar.h"

#include "ui/win/resource.h"


Win32ProgressBar::Win32ProgressBar(HINSTANCE app, HWND statusBar)
   : m_app { app }
   , m_statusBar { statusBar }
{
   ::SendMessage(m_statusBar, SB_SETTEXT, 3 | 0, (LPARAM)LocalString(IDS_LOADING).m_szbuffer);

   const HCURSOR cursor = ::LoadCursor(nullptr, IDC_WAIT);
   ::SetCursor(cursor);

   RECT rc;
   ::SendMessage(m_statusBar, SB_GETRECT, 2, (LPARAM)&rc);

   m_progressBar
      = ::CreateWindowEx(0, PROGRESS_CLASS, (LPSTR) nullptr, WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, m_statusBar, (HMENU) nullptr, m_app, nullptr);

   ::SendMessage(m_progressBar, PBM_SETPOS, 1, 0);
}

Win32ProgressBar::~Win32ProgressBar()
{
   ::SendMessage(m_statusBar, SB_SETTEXT, 3 | 0, (LPARAM)L"");

   const HCURSOR cursor = ::LoadCursor(nullptr, IDC_ARROW);
   ::SetCursor(cursor);

   if (m_progressBar != nullptr)
      ::DestroyWindow(m_progressBar);
}

void Win32ProgressBar::SetProgress(unsigned int progress)
{
   m_progress = progress;
   Update();
}

void Win32ProgressBar::SetLength(unsigned int length)
{
   m_length = length;
   Update();
}

void Win32ProgressBar::Update()
{
   if (!m_progressBar)
      return;
   ::SendMessage(m_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, m_length));
   ::SendMessage(m_progressBar, PBM_SETPOS, min(m_progress, m_length), 0);
}
