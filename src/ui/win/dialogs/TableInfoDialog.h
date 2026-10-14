// license:GPLv3+

#pragma once

#include <wxx_wincore.h> // Needed first: wxx_controls.h alone lacks the CString definition
#include <wxx_controls.h> // Add CComboBox
#include <wxx_dialog.h>
#include <wxx_listview.h> // Add CListView
#include <wxx_stdcontrols.h> // Add CEdit

class PinTableWnd;

class TableInfoDialog final : public CDialog
{
public:
   explicit TableInfoDialog(PinTableWnd *tableEditor);

protected:
   void OnClose() override;
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;
   void OnCancel() override;

private:
   PinTableWnd *const m_tableEditor;

   int AddListItem(HWND hwndListView, const string &szName, const string &szValue1, LPARAM lparam);
   void VPGetDialogItemText(const CEdit &edit, string &psztext);

   CEdit m_tableNameEdit;
   CEdit m_authorEdit;
   CEdit m_versionEdit;
   CEdit m_releaseEdit;
   CEdit m_emailEdit;
   CEdit m_websiteEdit;
   CEdit m_blurbEdit;
   CEdit m_descriptionEdit;
   CEdit m_rulesEdits;
   CEdit m_dateSavedEdit;
   CEdit m_customNameEdit;
   CEdit m_customValueEdit;
   CListView m_customListView;
   CComboBox m_screenshotCombo;
   CResizer m_resizer;
};
