// license:GPLv3+

#include "core/stdafx.h"
#include "NotesDialog.h"

#include "parts/pintable.h"
#include "ui/win/resource.h"
#include "ui/win/WinEditor.h"


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

NotesDialog::NotesDialog(WinEditor* vpxEditor)
   : CDialog(IDD_NOTES_DIALOG)
   , m_vpxEditor(vpxEditor)
{
}

BOOL NotesDialog::OnInitDialog()
{
   m_resizer.Initialize(GetHwnd(), CRect(0, 0, 400 , 300));
   AttachItem(IDC_NOTES_EDIT, m_notesEdit);
   m_resizer.AddChild(m_notesEdit.GetHwnd(), CResizer::topright, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);

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

void NotesDialog::SetText()
{
   CCO(PinTable)* const pt = m_vpxEditor->GetActiveTable();
   if (pt != nullptr)
   {
      m_notesEdit.SetWindowText(pt->GetNotesText().c_str());
   }
}

void NotesDialog::UpdateText()
{
   CCO(PinTable)* const pt = m_vpxEditor->GetActiveTable();
   if (pt != nullptr)
      pt->SetNotesText(GetText());
}

CContainNotes::CContainNotes(WinEditor* vpxEditor)
   : m_notesDialog(vpxEditor)
{
   SetView(m_notesDialog);
   SetTabText(_T("Notes"));
   SetDockCaption(_T("Notes"));
}

CDockNotes::CDockNotes(WinEditor* vpxEditor)
   : m_vpxEditor(vpxEditor)
   , m_notesContainer(vpxEditor)
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
   m_vpxEditor->DestroyNotesDocker();
}
