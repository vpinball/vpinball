#ifndef H_DIMENSION_MANAGER_DIALOG
#define H_DIMENSION_MANAGER_DIALOG

class DimensionDialog : public CDialog
{
public:
   DimensionDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};

#endif
