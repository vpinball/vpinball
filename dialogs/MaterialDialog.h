#ifndef H_MATERIAL_DIALOG
#define H_MATERIAL_DIALOG

class MaterialDialog : public CDialog
{
public:
   MaterialDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual void OnOK();

private:
   void DisableAllMaterialDialogItems();
   void EnableAllMaterialDialogItems();
   float getItemText(int id);
   void  setItemText(int id, float value);
   HWND m_hMaterialList;
};
#endif // !H_MATERIAL_DIALOG
