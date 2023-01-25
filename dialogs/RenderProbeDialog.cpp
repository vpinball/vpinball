#include "stdafx.h"
#include "resource.h"
#include "RenderProbeDialog.h"

RenderProbeDialog::RenderProbeDialog() : CDialog(IDD_RENDERPROBE)
{
   hListHwnd = nullptr;
}

BOOL RenderProbeDialog::OnInitDialog()
{
   CCO(PinTable)* const pt = g_pvp->GetActiveTable();

   hListHwnd = GetDlgItem(IDC_RENDERPROBE_LIST).GetHwnd();

   ListView_SetExtendedListViewStyle(hListHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

   LVCOLUMN lvcol;
   lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
   const LocalString ls(IDS_NAME);
   lvcol.pszText = (LPSTR)ls.m_szbuffer; // = "Name";
   lvcol.cx = 200;
   ListView_InsertColumn(hListHwnd, 0, &lvcol);

   /* lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
   lvcol.fmt = LVCFMT_CENTER;
   const LocalString ls2(IDS_TYPE);
   lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "Type";
   lvcol.cx = 200;
   ListView_InsertColumn(hListHwnd, 1, &lvcol); */

   HWND hwnd = GetDlgItem(IDC_REFLECTION_MAX_LEVEL).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Balls Only");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static Only");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static & Balls");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static & Unsynced Dynamic");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static & Synced Dynamic");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Dynamic");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   UpdateList();

   ListView_SetItemState(hListHwnd, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
   GotoDlgCtrl(hListHwnd);
   return FALSE;
}

void RenderProbeDialog::UpdateList()
{
   CCO(PinTable) *const pt = g_pvp->GetActiveTable();
   ListView_DeleteAllItems(hListHwnd);
   for (size_t i = 0; i < pt->GetRenderProbeList().size(); i++)
   {
      RenderProbe *const prp = pt->GetRenderProbeList()[i];

      LVITEM lvitem;
      lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
      lvitem.iItem = 0;
      lvitem.iSubItem = 0;
      lvitem.pszText = (LPSTR)prp->GetName().c_str();
      lvitem.lParam = (size_t)prp;

      const int index = ListView_InsertItem(hListHwnd, &lvitem);
      /* switch (prp->GetType())
      {
      case RenderProbe::PLANE_REFLECTION: ListView_SetItemText(hListHwnd, index, 1, (LPSTR) "Plane Reflection"); break;
      case RenderProbe::SCREEN_SPACE_TRANSPARENCY: ListView_SetItemText(hListHwnd, index, 1, (LPSTR) "ScreenSpace Refraction"); break;
      }*/
   }
}

INT_PTR RenderProbeDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *const pt = g_pvp->GetActiveTable();

   switch (uMsg)
   {
   case WM_NOTIFY:
   {
      const LPNMHDR pnmhdr = (LPNMHDR)lParam;
      switch (pnmhdr->code)
      {
      case LVN_ENDLABELEDIT:
      {
         NMLVDISPINFO *pinfo = (NMLVDISPINFO *)lParam;
         if (pinfo->item.pszText == nullptr || pinfo->item.pszText[0] == '\0')
            return FALSE;
         auto new_name = string(pinfo->item.pszText);
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = pinfo->item.iItem;
         lvitem.iSubItem = 0;
         ListView_GetItem(hListHwnd, &lvitem);
         RenderProbe *const pb = (RenderProbe *)lvitem.lParam;
         // prohibit editing core playfield reflection probe name (or creating a duplicate)
         if (pb->GetName() == PLAYFIELD_REFLECTION_RENDERPROBE_NAME || new_name == PLAYFIELD_REFLECTION_RENDERPROBE_NAME)
            return FALSE;
         pb->SetName(new_name);
         lvitem.mask = LVIF_TEXT;
         lvitem.pszText = pinfo->item.pszText;
         ListView_SetItem(hListHwnd, &lvitem);
         SaveProbeFromUI(pb);
         LoadProbeToUI(pb);
         return TRUE;
      }
      break;

      case LVN_ITEMCHANGING:
      {
         NMLISTVIEW *const plistview = (LPNMLISTVIEW)lParam;
         if ((plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED))
         {
            if (plistview->uNewState & LVIS_SELECTED)
            {
               const int sel = plistview->iItem;
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(hListHwnd, &lvitem);
               RenderProbe *const pb = (RenderProbe *)lvitem.lParam;
               LoadProbeToUI(pb);
            }
         }
      }
      break;

      case LVN_ITEMCHANGED:
      {
         NMLISTVIEW *const plistview = (LPNMLISTVIEW)lParam;
         const int sel = plistview->iItem;
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = sel;
         lvitem.iSubItem = 0;
         ListView_GetItem(hListHwnd, &lvitem);
         RenderProbe *const pb = (RenderProbe *)lvitem.lParam;
         SaveProbeFromUI(pb);
         const int count = ListView_GetSelectedCount(hListHwnd);
         const BOOL enable = !(count > 1);
         ::EnableWindow(GetDlgItem(IDC_RENAME).GetHwnd(), enable);
      }
      break;
      
      }
   }
   break;

   case WM_HSCROLL:
   {
      if ((HWND)lParam == GetDlgItem(IDC_ROUGHNESS_BASE).GetHwnd() || (HWND)lParam == GetDlgItem(IDC_ROUGHNESS_CLEAR_LABEL).GetHwnd())
      {
         auto sel = ListView_GetSelectionMark(hListHwnd);
         if (sel >= 0)
         {
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(hListHwnd, &lvitem);
            RenderProbe *const pb = (RenderProbe *)lvitem.lParam;
            if (pb != nullptr)
            {
               SaveProbeFromUI(pb);
               LoadProbeToUI(pb);
            }
         }
      }
   }
   break;

   }

   return DialogProcDefault(uMsg, wParam, lParam);
}

void RenderProbeDialog::LoadProbeToUI(RenderProbe *const pb)
{
   GetDlgItem(IDC_RENDER_PROBE_NAME_LABEL).SetWindowText(pb->GetName().c_str());
   SendMessage(GetDlgItem(IDC_REFLECTION_MAX_LEVEL).GetHwnd(), CB_SETCURSEL, pb->GetReflectionMode(), 0);
   RenderProbe::ProbeType type = pb->GetType();
   CheckRadioButton(IDC_REFLECTION_PROBE, IDC_REFRACTION_PROBE, type == RenderProbe::PLANE_REFLECTION ? IDC_REFLECTION_PROBE : IDC_REFRACTION_PROBE);
   vec4 plane;
   pb->GetReflectionPlane(plane);
   GetDlgItem(IDC_REFLECTION_PLANE_NX).SetWindowText(f2sz(plane.x).c_str());
   GetDlgItem(IDC_REFLECTION_PLANE_NY).SetWindowText(f2sz(plane.y).c_str());
   GetDlgItem(IDC_REFLECTION_PLANE_NZ).SetWindowText(f2sz(plane.z).c_str());
   GetDlgItem(IDC_REFLECTION_PLANE_DIST).SetWindowText(f2sz(plane.w).c_str());
   GetDlgItem(IDC_REFLECTION_PLANE_NX).EnableWindow(type == RenderProbe::PLANE_REFLECTION);
   GetDlgItem(IDC_REFLECTION_PLANE_NY).EnableWindow(type == RenderProbe::PLANE_REFLECTION);
   GetDlgItem(IDC_REFLECTION_PLANE_NZ).EnableWindow(type == RenderProbe::PLANE_REFLECTION);
   GetDlgItem(IDC_REFLECTION_PLANE_DIST).EnableWindow(type == RenderProbe::PLANE_REFLECTION);
   GetDlgItem(IDC_REFLECTION_MAX_LEVEL).EnableWindow(type == RenderProbe::PLANE_REFLECTION);
   HWND hwnd = GetDlgItem(IDC_ROUGHNESS_BASE).GetHwnd();
   SendMessage(hwnd, TBM_SETRANGE, fTrue, MAKELONG(0, 13 - 1));
   SendMessage(hwnd, TBM_SETTICFREQ, 1, 0);
   SendMessage(hwnd, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwnd, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwnd, TBM_SETTHUMBLENGTH, 5, 0);
   SendMessage(hwnd, TBM_SETPOS, TRUE, pb->GetBaseRoughness());
   GetDlgItem(IDC_ROUGHNESS_BASE_LABEL).SetWindowText("Base: "s.append(std::to_string(pb->GetBaseRoughness())).c_str());
   hwnd = GetDlgItem(IDC_ROUGHNESS_CLEAR).GetHwnd();
   SendMessage(hwnd, TBM_SETRANGE, fTrue, MAKELONG(0, 13 - 1));
   SendMessage(hwnd, TBM_SETTICFREQ, 1, 0);
   SendMessage(hwnd, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwnd, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwnd, TBM_SETTHUMBLENGTH, 5, 0);
   SendMessage(hwnd, TBM_SETPOS, TRUE, pb->GetClearRoughness());
   GetDlgItem(IDC_ROUGHNESS_CLEAR_LABEL).SetWindowText("Clear: "s.append(std::to_string(pb->GetClearRoughness())).c_str());
}

void RenderProbeDialog::SaveProbeFromUI(RenderProbe *const pb)
{
   const size_t isReflection = SendMessage(GetDlgItem(IDC_REFLECTION_PROBE).GetHwnd(), BM_GETCHECK, 0, 0);
   RenderProbe::ProbeType type = isReflection ? RenderProbe::PLANE_REFLECTION : RenderProbe::SCREEN_SPACE_TRANSPARENCY;
   vec4 plane;
   pb->GetReflectionPlane(plane);
   const float vx = sz2f(GetDlgItemText(IDC_REFLECTION_PLANE_NX).c_str());
   const float vy = sz2f(GetDlgItemText(IDC_REFLECTION_PLANE_NY).c_str());
   const float vz = sz2f(GetDlgItemText(IDC_REFLECTION_PLANE_NZ).c_str());
   const float vw = sz2f(GetDlgItemText(IDC_REFLECTION_PLANE_DIST).c_str());
   LRESULT reflectionMode = SendMessage(GetDlgItem(IDC_REFLECTION_MAX_LEVEL).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (reflectionMode == LB_ERR)
      reflectionMode = RenderProbe::REFL_STATIC;
   const size_t roughness_base = SendMessage(GetDlgItem(IDC_ROUGHNESS_BASE).GetHwnd(), TBM_GETPOS, 0, 0);
   const size_t roughness_clear = SendMessage(GetDlgItem(IDC_ROUGHNESS_CLEAR).GetHwnd(), TBM_GETPOS, 0, 0);

   if (pb->GetType() != type || pb->GetReflectionMode() != reflectionMode || plane.x != vx || plane.y != vy || plane.z != vz || plane.w != vw || roughness_base != pb->GetBaseRoughness() || roughness_clear != pb->GetClearRoughness())
   {
      plane.x = vx;
      plane.y = vy;
      plane.z = vz;
      plane.w = vw;
      pb->SetType(type);
      pb->SetBaseRoughness(roughness_base);
      pb->SetClearRoughness(roughness_clear);
      pb->SetReflectionPlane(plane);
      pb->SetReflectionMode((RenderProbe::ReflectionMode)reflectionMode);
      CCO(PinTable) *const pt = g_pvp->GetActiveTable();
      pt->SetNonUndoableDirty(eSaveDirty);
   }
}

BOOL RenderProbeDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *const pt = g_pvp->GetActiveTable();
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
   case IDC_NEW:
   {
      RenderProbe* rp = pt->NewRenderProbe();
      rp->SetName("New Render Probe"s);
      pt->SetNonUndoableDirty(eSaveDirty);
      UpdateList();
      break;
   }
   case IDC_RENAME:
   {
      const int sel = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
      if (sel != -1)
      {
         ::SetFocus(hListHwnd);
         ListView_EditLabel(hListHwnd, sel);
      }
      break;
   }
   case IDC_DELETE:
   {
      const int sel = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
      if (sel != -1)
      {
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = sel;
         lvitem.iSubItem = 0;
         ListView_GetItem(hListHwnd, &lvitem);
         RenderProbe *const pcol = (RenderProbe *)lvitem.lParam;
         // prohibit deleting core playfield reflection probe
         if (pcol->GetName() != PLAYFIELD_REFLECTION_RENDERPROBE_NAME)
         {
            // TEXT
            const int ans = MessageBox("Are you sure you want to remove this render probe?", "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
            if (ans == IDYES)
            {
               pt->RemoveRenderProbe(pcol);
               ListView_DeleteItem(hListHwnd, sel);
               pt->SetNonUndoableDirty(eSaveDirty);
               UpdateList();
            }
         }
      }
      break;
   }
   case IDC_REFLECTION_PROBE:
   case IDC_REFRACTION_PROBE:
   {
      auto sel = ListView_GetSelectionMark(hListHwnd);
      if (sel >= 0)
      {
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = sel;
         lvitem.iSubItem = 0;
         ListView_GetItem(hListHwnd, &lvitem);
         RenderProbe *const pb = (RenderProbe *)lvitem.lParam;
         if (pb != nullptr)
         {
            SaveProbeFromUI(pb);
            LoadProbeToUI(pb);
         }
      }
      break;
   }

   default: return FALSE;
   }
   return TRUE;
}

void RenderProbeDialog::OnOK()
{
   auto sel = ListView_GetSelectionMark(hListHwnd);
   if (sel >= 0)
   {
      LVITEM lvitem;
      lvitem.mask = LVIF_PARAM;
      lvitem.iItem = sel;
      lvitem.iSubItem = 0;
      ListView_GetItem(hListHwnd, &lvitem);
      RenderProbe *const pb = (RenderProbe *)lvitem.lParam;
      if (pb != nullptr)
      {
         SaveProbeFromUI(pb);
      }
   }
   SavePosition();
   CDialog::OnOK();
}

void RenderProbeDialog::OnClose()
{
   SavePosition();
   CDialog::OnCancel();
}

void RenderProbeDialog::LoadPosition()
{
   const int x = LoadValueIntWithDefault(regKey[RegName::Editor], "RenderProbeMngPosX"s, 0);
   const int y = LoadValueIntWithDefault(regKey[RegName::Editor], "RenderProbeMngPosY"s, 0);

   const int w = LoadValueIntWithDefault(regKey[RegName::Editor], "RenderProbeMngWidth"s, 1000);
   const int h = LoadValueIntWithDefault(regKey[RegName::Editor], "RenderProbeMngHeight"s, 800);
   SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void RenderProbeDialog::SavePosition()
{
   const CRect rect = GetWindowRect();

   SaveValueInt(regKey[RegName::Editor], "RenderProbeMngPosX"s, rect.left);
   SaveValueInt(regKey[RegName::Editor], "RenderProbeMngPosY"s, rect.top);
   const int w = rect.right - rect.left;
   SaveValueInt(regKey[RegName::Editor], "RenderProbeMngWidth"s, w);
   const int h = rect.bottom - rect.top;
   SaveValueInt(regKey[RegName::Editor], "RenderProbeMngHeight"s, h);
}
