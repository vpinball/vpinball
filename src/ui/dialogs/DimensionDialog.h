// license:GPLv3+

#pragma once

#include "core/TableDB.h"

class DimensionDialog : public CDialog
{
public:
   DimensionDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);

private:
   void UpdateApplyState();

   CListView m_listView;
   TableDB m_db;
   bool m_discardChangeNotification = false;
};
