// license:GPLv3+

#include "core/stdafx.h"

#include "PinTableMDI.h"

#include "ui/win/WinEditor.h"

#include "core/VPApp.h"
#include "ui/win/dialogs/Win32ProgressBar.h"

static CComObject<PinTable>* CreatePinTable()
{
   CComObject<PinTable>* table;
   CComObject<PinTable>::CreateInstance(&table);
   return table; // Note that the ref count is zero so far
}

PinTableMDI::PinTableMDI(WinEditor* vpinball)
   : m_tableWnd(std::make_unique<PinTableWnd>(vpinball, CreatePinTable()))
   , m_vpxEditor(vpinball)
{
   m_tableWnd->SetMDITable(this);
   SetView(*m_tableWnd);
   //m_menu.LoadMenu(IDR_APPMENU);
   SetHandles(m_vpxEditor->GetMenu(), nullptr);
}

PinTableMDI::~PinTableMDI()
{
   m_vpxEditor->CloseAllDialogs();
   m_tableWnd->FVerifySaveToClose();
   RemoveFromVectorSingle(m_vpxEditor->m_vtable, m_tableWnd.get());
}

bool PinTableMDI::CanClose() const
{
    if (m_tableWnd->m_table != nullptr && m_tableWnd->m_table->FDirty())
    {
        const string szText = LocalString(IDS_SAVE_CHANGES1).m_szbuffer /*"Do you want to save the changes you made to '"*/ + m_tableWnd->m_table->m_title + LocalString(IDS_SAVE_CHANGES2).m_szbuffer;
        const int result = MessageBox(szText.c_str(), "Visual Pinball", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONWARNING);

        if (result == IDCANCEL)
            return false;

        Win32ProgressBar feedback(g_app->GetInstanceHandle(), m_vpxEditor->m_hwndStatusBar);
        if (result == IDYES)
        {
           m_vpxEditor->SetActionCur(LocalString(IDS_SAVING).m_szbuffer);
           m_vpxEditor->SetCursorCur(IDC_WAIT);
           const bool failed = m_tableWnd->m_table->Save(feedback) != S_OK;
           m_vpxEditor->SetActionCur(string());
           m_vpxEditor->SetCursorCur(IDC_ARROW);
           if (failed)
              MessageBox(LocalString(IDS_SAVEERROR).m_szbuffer, "Visual Pinball", MB_ICONERROR);
        }
    }
    return true;
}

void PinTableMDI::PreCreate(CREATESTRUCT &cs)
{
    cs.x = 20;
    cs.y = 20;
    cs.cx = 400;
    cs.cy = 400;
    cs.style = WS_MAXIMIZE;
    cs.hwndParent = m_vpxEditor->GetHwnd();
    cs.lpszClass = _T("PinTable");
    cs.lpszName = _T("");
}

int PinTableMDI::OnCreate(CREATESTRUCT &cs)
{
    SetWindowText(m_tableWnd->m_table->m_title.c_str());
    SetIconLarge(IDI_TABLE);
    SetIconSmall(IDI_TABLE);
    return CMDIChild::OnCreate(cs);
}

void PinTableMDI::OnClose()
{
   if (m_vpxEditor->IsClosing() || m_vpxEditor->IsUnloadingTable() || CanClose())
   {
      if(m_vpxEditor->GetNotesDocker() != nullptr)
      {
         m_vpxEditor->GetNotesDocker()->UpdateText();
         m_vpxEditor->GetNotesDocker()->CleanText();
      }
      m_tableWnd->KillTimer(WinEditor::TIMER_ID_AUTOSAVE);
      CMDIChild::OnClose();
   }
}

LRESULT PinTableMDI::OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
   //wparam holds HWND of the MDI frame that is about to be deactivated
   //lparam holds HWND of the MDI frame that is about to be activated
   if (GetHwnd() == (HWND)wparam)
   {
      if (m_vpxEditor->m_ptableActive == m_tableWnd->m_table)
         m_vpxEditor->m_ptableActive = nullptr;
   }
   if(GetHwnd() == (HWND)lparam)
   {
      m_vpxEditor->m_ptableActive = m_tableWnd->m_table;
      if (m_vpxEditor->GetLayersDocker() != nullptr)
      {
         m_vpxEditor->GetLayersListDialog()->SetActiveTable(m_tableWnd->m_table);
         m_tableWnd->RefreshProperties();
      }
   }
   return CMDIChild::OnMDIActivate(msg, wparam, lparam);
}

BOOL PinTableMDI::OnEraseBkgnd(CDC& dc)
{
   return TRUE;
}
