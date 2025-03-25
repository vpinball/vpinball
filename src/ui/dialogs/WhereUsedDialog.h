// license:GPLv3+

#pragma once

class WhereUsedDialog final : public CDialog
{
public:
   WhereUsedDialog();
   ~WhereUsedDialog() override;
   EnumWhereUsedSource m_whereUsedSource;

protected:
   void OnDestroy() override;
   void OnClose() override;
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;
   void OnCancel() override;

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
