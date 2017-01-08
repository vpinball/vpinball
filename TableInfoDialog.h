#ifndef H_TABLE_INFO_DIALOG
#define H_TABLE_INFO_DIALOG

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
   void VPGetDialogItemText(int nIDDlgItem, char **psztext);

};
#endif // !H_TABLE_INFO_DIALOG
