// license:GPLv3+

#pragma once

class SearchSelectDialog final : public CDialog
{
public:
   SearchSelectDialog();
   void Update();

protected:
   void OnClose() override;
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;
   void OnCancel() override;

private:
   static bool IsValidString(const string &name);
   void AddSearchItemToList(IEditable * const piedit, int idx);

   void LoadPosition();
   void SavePosition();
   void SelectElement();
   void SortItems(const int columnNumber);

   HWND m_hElementList;
   CResizer m_resizer;
   CCO(PinTable) *m_curTable;

   static int m_lastSortColumn;
   static bool m_columnSortOrder;
   static bool m_switchSortOrder;
};
