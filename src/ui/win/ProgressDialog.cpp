// license:GPLv3+

#include "core/stdafx.h"
#include "ProgressDialog.h"

#include "ui/win/resource.h"


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

void ProgressDialog::SetProgress(const string &text, const int value)
{
#ifndef __STANDALONE__
   if (IsWindow())
   {
      m_progressName.SetWindowText(text.c_str());
      if (value != -1)
         m_progressBar.SetPos(value);
   }
#else
   if (value != -1 && m_progress != value)
   {
      PLOGI.printf("%s %d%%", text.c_str(), value);
      m_progress = value;
   }
#endif
}
