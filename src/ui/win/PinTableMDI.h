// license:GPLv3+

#pragma once

#include "ui/win/resource.h" // win32xx related IDW_*
#include <wxx_mdi.h>

#include "PinTableWnd.h"

// Multiple Document Interface child window that contains a PinTable view
class PinTableMDI final : public CMDIChild
{
public:
   PinTableMDI(WinEditor *vpinball);
   ~PinTableMDI() override;
   PinTableWnd* GetTableWnd() const { return m_tableWnd.get(); }
   CComObject<PinTable> *GetTable() const { return m_tableWnd->m_table; }
   bool CanClose() const;

protected:
   void PreCreate(CREATESTRUCT &cs) override;
   int OnCreate(CREATESTRUCT &cs) override;
   void OnClose() override;
   LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam) override;
   BOOL OnEraseBkgnd(CDC &dc) override;

private:
   std::unique_ptr<PinTableWnd> m_tableWnd;
   WinEditor *m_vpxEditor;
};
