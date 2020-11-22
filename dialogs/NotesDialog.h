#pragma once

#include <dlgs.h>
#include <cderr.h>
#include <wxx_appcore.h>		// Add CCriticalSection, CObject, CWinThread, CWinApp
#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 
#include <wxx_richedit.h>
#include "Properties/PropertyDialog.h"
#include <WindowsX.h>

class NotesEdit : public CEdit
{

public:
   NotesEdit() = default;
   ~NotesEdit() = default;

protected:
   virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

};
class NotesDialog : public CDialog
{
public:
   NotesDialog();
   ~NotesDialog() = default;
   bool PreTranslateMessage(MSG* msg);
   CString GetText() const
   {
      return m_notesEdit.GetWindowText();
   }
   void SetText(const CString& text)
   {
      m_notesEdit.SetWindowText(text.c_str());
   }

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
   CResizer  m_resizer;
   NotesEdit m_notesEdit;
};

class CContainNotes : public CDockContainer
{
public:
   CContainNotes();
   ~CContainNotes() = default;
   NotesDialog* GetNotesDialog()
   {
      return &m_notesDialog;
   }

private:
   NotesDialog m_notesDialog;
};

class CDockNotes : public CDocker
{
public:
   CDockNotes();
   ~CDockNotes() = default;

   CContainNotes* GetContainNotes()
   {
      return &m_notesContainer;
   }
protected:
   virtual void OnClose();

private:
   CContainNotes m_notesContainer;
};