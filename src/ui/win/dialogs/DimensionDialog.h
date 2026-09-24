// license:GPLv3+

#pragma once

#include <wxx_listview.h>		// Add CListView
#include "core/TableDB.h"

class PinTableWnd;

class DimensionDialog final : public CDialog
{
public:
   explicit DimensionDialog(PinTableWnd *tableEditor);

protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;

private:
   PinTableWnd *const m_tableEditor;

   void UpdateApplyState();

   CListView m_listView;
   TableDB m_db;
   bool m_discardChangeNotification = false;
};
