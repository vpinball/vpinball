// license:GPLv3+

#include "core/stdafx.h"
#include "ProgressDialog.h"

#include "ui/win/resource.h"


#ifndef __STANDALONE__
#define ID_SETTEXT 0x100
#endif

ProgressDialog::ProgressDialog()
   : CDialog(IDD_PROGRESS)
{
}

BOOL ProgressDialog::OnInitDialog()
{
#ifndef __STANDALONE__
   AttachItem(IDC_PROGRESS2, m_progressBar);
   AttachItem(IDC_STATUSNAME, m_progressName);
#endif
   return TRUE;
}

BOOL ProgressDialog::OnCommand(WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   if (wparam == ID_SETTEXT)
   {
      std::unique_ptr<CString> wtext(reinterpret_cast<CString*>(lparam));
      m_progressName.SetWindowText(*wtext);
   }
#endif
   return FALSE;
}

void ProgressDialog::SetProgress(const string &text, const float value)
{
#ifndef __STANDALONE__
   if (IsWindow())
   {
      auto* wtext = new CString(text.c_str());
      if (!PostMessage(WM_COMMAND, ID_SETTEXT, (LPARAM)wtext))
         delete wtext;
      if (value >= 0.f && m_progress != value)
         PostMessage(m_progressBar.GetHwnd(), PBM_SETPOS, static_cast<WPARAM>((int)value), 0);
   }
#else
   if (value >= 0.f && m_progress != value)
   {
      PLOGI.printf("%s %d%%", text.c_str(), (int)value);
   }
#endif
   m_progress = value;
}
