// license:GPLv3+

#pragma once

class TableInfoDialog : public CDialog
{
public:
   TableInfoDialog();

protected:
   virtual void OnClose();
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnCancel();

private:
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
