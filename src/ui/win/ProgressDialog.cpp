// license:GPLv3+

#include "core/stdafx.h"
#include "ProgressDialog.h"

#include "ui/win/resource.h"


#define ID_SETTEXT 0x100

ProgressDialog::ProgressDialog()
   : CDialog(IDD_PROGRESS)
{
}

BOOL ProgressDialog::OnInitDialog()
{
   AttachItem(IDC_PROGRESS2, m_progressBar);
   AttachItem(IDC_STATUSNAME, m_progressName);
   return TRUE;
}

BOOL ProgressDialog::OnCommand(WPARAM wparam, LPARAM lparam)
{
   if (wparam == ID_SETTEXT)
   {
      std::unique_ptr<CString> wtext(reinterpret_cast<CString*>(lparam));
      m_progressName.SetWindowText(*wtext);
   }
   return FALSE;
}

void ProgressDialog::SetProgress(const string &text, const float value)
{
   if (IsWindow())
   {
      auto* wtext = new CString(text);
      if (!PostMessage(WM_COMMAND, ID_SETTEXT, (LPARAM)wtext))
         delete wtext;
      if (value >= 0.f && m_progress != value)
         PostMessage(m_progressBar.GetHwnd(), PBM_SETPOS, static_cast<WPARAM>((int)value), 0);
   }
   if (value >= 0.f)
      m_progress = value;
}
