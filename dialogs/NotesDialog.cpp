#include "stdafx.h"
#include "resource.h"
#include "NotesDialog.h"

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

NotesDialog::NotesDialog() : CDialog(IDD_NOTES_DIALOG)
{
}

BOOL NotesDialog::OnInitDialog()
{
   m_resizer.Initialize(*this, CRect(0, 0, 400 , 300));
   AttachItem(IDC_NOTES_EDIT, m_notesEdit);
   m_resizer.AddChild(m_notesEdit.GetHwnd(), topright, RD_STRETCH_HEIGHT|RD_STRETCH_WIDTH);

   SetText();
   return TRUE;
}

BOOL NotesDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (HIWORD(wParam))
   {
      case EN_KILLFOCUS:
      {
         UpdateText();
         return TRUE;
      }
   }
   return FALSE;
}

INT_PTR NotesDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}

bool NotesDialog::PreTranslateMessage(MSG* msg)
{
   if (!IsWindow())
      return false;

   return !!IsDialogMessage(*msg);
}

void NotesDialog::SetText()
{
   CCO(PinTable)* const pt = g_pvp->GetActiveTable();
   if (pt != nullptr)
   {
      m_notesEdit.SetWindowText(pt->GetNotesText().c_str());
   }
}

void NotesDialog::UpdateText()
{
   CCO(PinTable)* const pt = g_pvp->GetActiveTable();
   if (pt != nullptr)
      pt->SetNotesText(GetText());
}

CContainNotes::CContainNotes()
{
   SetView(m_notesDialog);
   SetTabText(_T("Notes"));
   SetDockCaption(_T("Notes"));
}

CDockNotes::CDockNotes()
{
   SetView(m_notesContainer);
   SetBarWidth(4);
}

void CDockNotes::UpdateText()
{
   m_notesContainer.GetNotesDialog()->UpdateText();
}

void CDockNotes::OnClose()
{
   UpdateText();
   CDocker::OnClose();
   g_pvp->DestroyNotesDocker();
}
