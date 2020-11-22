#include "stdafx.h"
#include "resource.h"
#include "NotesDialog.h"

NotesDialog::NotesDialog() : CDialog(IDD_NOTES_DIALOG)
{
}


BOOL NotesDialog::OnInitDialog()
{
   m_resizer.Initialize(*this, CRect(0, 0, 400 , 300));
   AttachItem(IDC_NOTES_EDIT, m_notesEdit);
   m_resizer.AddChild(m_notesEdit.GetHwnd(), topright, RD_STRETCH_HEIGHT|RD_STRETCH_WIDTH);
   return TRUE;
}

BOOL NotesDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   return FALSE;
}

INT_PTR NotesDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}

void NotesDialog::OnOK()
{
}

bool NotesDialog::PreTranslateMessage(MSG* msg)
{
   if (!IsWindow())
      return false;

   // only pre-translate mouse and keyboard input events
//    if (((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST)))
//    {
//       const int keyPressed = LOWORD(msg->wParam);
//       // only pass F1-F12 to the main VPinball class to open subdialogs from everywhere
//       if (keyPressed >= VK_F1 && keyPressed <= VK_F12 && TranslateAccelerator(g_pvp->GetHwnd(), g_haccel, msg))
//          return true;
//    }

   return !!IsDialogMessage(*msg);
}

CContainNotes::CContainNotes()
{
   SetView(m_notesDialog);
   SetTabText(_T("Notes"));
   //SetTabIcon(IDI_TOOLBAR);
   SetDockCaption(_T("Notes"));

}

CDockNotes::CDockNotes()
{
   SetView(m_notesContainer);
   SetBarWidth(4);
}

LRESULT NotesEdit::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
      case WM_KEYUP:
         if (wparam == VK_RETURN)
         {
            // stupid win32 edit control. add a CR/LF to the end of each line manually
            AppendText("\r\n");
            return FALSE;
         }
   }
   return WndProcDefault(msg, wparam, lparam);

}
