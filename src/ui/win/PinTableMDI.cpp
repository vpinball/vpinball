// license:GPLv3+

#include "core/stdafx.h"

#include "PinTableMDI.h"


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
#ifndef __STANDALONE__
   SetView(*m_tableWnd);
   //m_menu.LoadMenu(IDR_APPMENU);
   SetHandles(m_vpxEditor->GetMenu(), nullptr);
#endif
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
#ifndef __STANDALONE__
        const int result = MessageBox(szText.c_str(), "Visual Pinball", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONWARNING);

        if (result == IDCANCEL)
            return false;

        if ((result == IDYES) && (m_tableWnd->m_table->Save() != S_OK))
            MessageBox(LocalString(IDS_SAVEERROR).m_szbuffer, "Visual Pinball", MB_ICONERROR);
#endif
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
#ifndef __STANDALONE__
    SetWindowText(m_tableWnd->m_table->m_title.c_str());
    SetIconLarge(IDI_TABLE);
    SetIconSmall(IDI_TABLE);
    return CMDIChild::OnCreate(cs);
#else
    return 0;
#endif
}

void PinTableMDI::OnClose()
{
#ifndef __STANDALONE__
   if (m_vpxEditor->IsClosing() || CanClose())
   {
      if(m_vpxEditor->GetNotesDocker() != nullptr)
      {
         m_vpxEditor->GetNotesDocker()->UpdateText();
         m_vpxEditor->GetNotesDocker()->CleanText();
      }
      m_tableWnd->KillTimer(WinEditor::TIMER_ID_AUTOSAVE);
      CMDIChild::OnClose();
   }
#endif
}

LRESULT PinTableMDI::OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef __STANDALONE__
   return 0L;
#else
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
         m_vpxEditor->SetPropSel(m_tableWnd->m_table->m_vmultisel);
      }
   }
   return CMDIChild::OnMDIActivate(msg, wparam, lparam);
#endif
}

BOOL PinTableMDI::OnEraseBkgnd(CDC& dc)
{
   return TRUE;
}
