// license:GPLv3+

#pragma once

class SearchSelectDialog : public CDialog
{
public:
   SearchSelectDialog();
   void Update();

protected:
   virtual void OnClose();
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnCancel();

private:
   bool IsValidString(const string &name);
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
