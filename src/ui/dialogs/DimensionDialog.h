// license:GPLv3+

#pragma once

#include "core/TableDB.h"

class DimensionDialog final : public CDialog
{
public:
   DimensionDialog();

protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;

private:
   void UpdateApplyState();

   CListView m_listView;
   TableDB m_db;
   bool m_discardChangeNotification = false;
};
