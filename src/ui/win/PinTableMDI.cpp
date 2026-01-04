// license:GPLv3+

#include "core/stdafx.h"

#include "PinTableMDI.h"


PinTableMDI::PinTableMDI(VPinball *vpinball)
{
    CComObject<PinTable>::CreateInstance(&m_table);
    m_vpinball = vpinball;

    m_table->AddRef();

    m_table->SetMDITable(this);
#ifndef __STANDALONE__
    SetView(*m_table);

    //m_menu.LoadMenu(IDR_APPMENU);
    SetHandles(m_vpinball->GetMenu(), nullptr);
#endif
}

PinTableMDI::~PinTableMDI()
{
    m_vpinball->CloseAllDialogs();

    if (m_table != nullptr)
    {
#ifndef __STANDALONE__
        if (m_table->m_searchSelectDlg.IsWindow())
           m_table->m_searchSelectDlg.Destroy();
#endif

        m_table->FVerifySaveToClose();

        RemoveFromVectorSingle(m_vpinball->m_vtable, (CComObject<PinTable>*)m_table);

        m_table->Release();
    }
}

bool PinTableMDI::CanClose() const
{
    if (m_table != nullptr && m_table->FDirty() && !g_pvp->m_povEdit)
    {
        const string szText = LocalString(IDS_SAVE_CHANGES1).m_szbuffer /*"Do you want to save the changes you made to '"*/ + m_table->m_title + LocalString(IDS_SAVE_CHANGES2).m_szbuffer;
#ifndef __STANDALONE__
        const int result = MessageBox(szText.c_str(), "Visual Pinball", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONWARNING);

        if (result == IDCANCEL)
            return false;

        if ((result == IDYES) && (m_table->TableSave() != S_OK))
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
    cs.hwndParent = m_vpinball->GetHwnd();
    cs.lpszClass = _T("PinTable");
    cs.lpszName = _T(m_table->m_filename.c_str());
}

int PinTableMDI::OnCreate(CREATESTRUCT &cs)
{
#ifndef __STANDALONE__
    SetWindowText(m_table->m_title.c_str());
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
   if (m_vpinball->IsClosing() || CanClose())
   {
      if(g_pvp->GetNotesDocker() != nullptr)
      {
         g_pvp->GetNotesDocker()->UpdateText();
         g_pvp->GetNotesDocker()->CleanText();
      }
      m_table->KillTimer(VPinball::TIMER_ID_AUTOSAVE);
      CMDIChild::OnClose();
   }
#endif
}

LRESULT PinTableMDI::OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   //wparam holds HWND of the MDI frame that is about to be deactivated
   //lparam holds HWND of the MDI frame that is about to be activated
   if (GetHwnd() == (HWND)wparam)
   {
      if (!m_table->m_filename.empty())
      {
         m_table->m_settings.SetIniPath(m_table->GetSettingsFileName());
         m_table->m_settings.Save();
      }
      if (g_pvp->m_ptableActive == m_table)
         g_pvp->m_ptableActive = nullptr;
   }
   if(GetHwnd()==(HWND)lparam)
   {
      g_pvp->m_ptableActive = m_table;
      if (g_pvp->GetLayersDocker() != nullptr)
      {
         g_pvp->GetLayersListDialog()->SetActiveTable(m_table);
         g_pvp->SetPropSel(m_table->m_vmultisel);
      }
   }
   return CMDIChild::OnMDIActivate(msg, wparam, lparam);
#else 
   return 0L;
#endif
}

BOOL PinTableMDI::OnEraseBkgnd(CDC& dc)
{
   return TRUE;
}
