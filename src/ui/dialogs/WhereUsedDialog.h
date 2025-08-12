// license:GPLv3+

#pragma once

enum EnumWhereUsedSource // The 'Where Used' button appears on both the 'Image Manager' and the 'Materials Manager'.  The 'WhereUsed' dialog has a property of this enum type to 'remember' whether it's to display a list of images or materials.
{
   IMAGES,
   MATERIALS
};

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
