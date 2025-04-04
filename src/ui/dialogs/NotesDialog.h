// license:GPLv3+

#pragma once

#include <wxx_appcore.h>		// Add CCriticalSection, CObject, CWinThread, CWinApp
#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 

class NotesEdit final : public CEdit
{
public:
   NotesEdit() = default;
   ~NotesEdit() override = default;

protected:
   LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
};

class NotesDialog final : public CDialog
{
public:
   NotesDialog();
   ~NotesDialog() override = default;

   BOOL PreTranslateMessage(MSG& msg) override;
   CString GetText() const
   {
      return m_notesEdit.GetWindowText();
   }
   void SetText();
   void UpdateText();
   void CleanText()
   {
      m_notesEdit.SetWindowText("");
   }
   void Enable()
   {
      m_notesEdit.EnableWindow();
   }
   void Disable()
   {
      m_notesEdit.EnableWindow(FALSE);
   }

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   CResizer  m_resizer;
   NotesEdit m_notesEdit;
};

class CContainNotes final : public CDockContainer
{
public:
   CContainNotes();
   ~CContainNotes() override = default;
   NotesDialog* GetNotesDialog()
   {
      return &m_notesDialog;
   }

private:
   NotesDialog m_notesDialog;
};

class CDockNotes final : public CDocker
{
public:
   CDockNotes();
   ~CDockNotes() override = default;

   CContainNotes* GetContainNotes()
   {
      return &m_notesContainer;
   }
   void UpdateText();
   void CleanText()
   {
      GetContainNotes()->GetNotesDialog()->CleanText();
   }
   void Refresh()
   {
      GetContainNotes()->GetNotesDialog()->SetText();
   }
   void Enable()
   {
      GetContainNotes()->GetNotesDialog()->Enable();
   }
   void Disable()
   {
      GetContainNotes()->GetNotesDialog()->Disable();
   }

protected:
   void OnClose() override;

private:
   CContainNotes m_notesContainer;
};
