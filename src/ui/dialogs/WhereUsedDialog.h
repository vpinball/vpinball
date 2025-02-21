// license:GPLv3+

#pragma once

class WhereUsedDialog final : public CDialog
{
public:
   WhereUsedDialog();
   virtual ~WhereUsedDialog();
   EnumWhereUsedSource m_whereUsedSource;

protected:
   virtual void OnDestroy();
   virtual void OnClose();
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnCancel();

private:
   CResizer m_resizer;

   void LoadPosition();
   void SavePosition();
   void RefreshList();
   void EditObject(HWND hWhereListView);
   void AddToolTip(const char* const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
   static int m_columnSortOrder;
   static bool m_doNotChange; // to avoid triggering LVN_ITEMCHANGING or LVN_ITEMCHANGED code on deletion
};
