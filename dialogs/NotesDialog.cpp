#include "stdafx.h"
#include "resource.h"
#include "NotesDialog.h"

NotesDialog::NotesDialog() : CDialog(IDD_NOTES_DIALOG)
{
}


BOOL NotesDialog::OnInitDialog()
{
   m_resizer.Initialize(*this, CRect(0, 0, 400 , 300));
   AttachItem(IDC_NOTES_RICHEDIT, m_richEdit);
   m_resizer.AddChild(m_richEdit.GetHwnd(), topright, RD_STRETCH_HEIGHT|RD_STRETCH_WIDTH);
   return TRUE;
}

BOOL NotesDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   UINT nID = LOWORD(wParam);
   switch (nID)
   {
      case IDC_COPY: m_richEdit.Copy(); return TRUE;
      case IDC_PASTE: m_richEdit.PasteSpecial(CF_TEXT); return TRUE;
   }
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
   if (((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST)))
   {
      const int keyPressed = LOWORD(msg->wParam);
      // only pass F1-F12 to the main VPinball class to open subdialogs from everywhere
      if (keyPressed >= VK_F1 && keyPressed <= VK_F12 && TranslateAccelerator(g_pvp->GetHwnd(), g_haccel, msg))
         return true;
   }

   return !!IsDialogMessage(*msg);
}


void RichEdit::SetFontDefaults()
{
   //Set font
   if (m_Font.GetHandle() == 0)
      m_Font.CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("Courier New"));
   SetFont(m_Font, FALSE);

   // Required for Dev-C++
#ifndef IMF_AUTOFONT
#define IMF_AUTOFONT          0x0002
#endif

    // Prevent Unicode characters from changing the font
   LRESULT lres = SendMessage(EM_GETLANGOPTIONS, 0, 0);
   lres &= ~IMF_AUTOFONT;
   SendMessage(EM_SETLANGOPTIONS, 0, lres);
}

void RichEdit::OnAttach()
{
   //increase the text limit of the rich edit window
   LimitText(-1);

   //Determine which messages will be passed to the parent
   DWORD dwMask = ENM_KEYEVENTS | ENM_DROPFILES;
   SetEventMask(dwMask);

   SetFontDefaults();
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

