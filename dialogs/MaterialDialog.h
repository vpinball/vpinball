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
   virtual void OnClose();

private:
   void DisableAllMaterialDialogItems();
   void EnableAllMaterialDialogItems();
   float getItemText(int id);
   void  setItemText(int id, float value);
   void LoadPosition();
   void SavePosition();
   HWND m_hMaterialList;
   CResizer m_resizer;
   static int m_columnSortOrder;
   static bool m_deletingItem;
};

#endif // !H_MATERIAL_DIALOG
