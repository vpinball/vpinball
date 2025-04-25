// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "DimensionDialog.h"

DimensionDialog::DimensionDialog() : CDialog(IDD_DIMENSION_CALCULATOR)
{
}

BOOL DimensionDialog::OnInitDialog()
{
   m_db.Load();

   AttachItem(IDC_TABLE_DIM_LIST, m_listView);
   m_listView.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   m_listView.InsertColumn(0, TEXT("Manufacturer"), LVCFMT_LEFT, 250);
   m_listView.InsertColumn(1, TEXT("Width"), LVCFMT_RIGHT, 80);
   m_listView.InsertColumn(2, TEXT("Height"), LVCFMT_RIGHT, 80);
   m_listView.InsertColumn(3, TEXT("Bot. Glass"), LVCFMT_RIGHT, 90);
   m_listView.InsertColumn(4, TEXT("Top Glass"), LVCFMT_RIGHT, 90);
   m_listView.InsertColumn(5, TEXT("Comments"), LVCFMT_LEFT, 250);

   int selectedItem = 0;

   CComObject<PinTable> * const pt = g_pvp->GetActiveTable();
   if (pt)
   {
      const float width = pt->GetTableWidth();
      const float height = pt->GetHeight();
      SetDlgItemText(IDC_VP_WIDTH, f2sz(width).c_str());
      SetDlgItemText(IDC_VP_HEIGHT, f2sz(height).c_str());
      SetDlgItemText(IDC_SIZE_WIDTH, f2sz(VPUTOINCHES(width)).c_str());
      SetDlgItemText(IDC_SIZE_HEIGHT, f2sz(VPUTOINCHES(height)).c_str());
      SetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT, f2sz(VPUTOINCHES(pt->m_glassTopHeight)).c_str());
      SetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT, f2sz(VPUTOINCHES(pt->m_glassBottomHeight)).c_str());
      const float ratio = height / width;
      SetDlgItemText(IDC_AR_LABEL, ("Aspect ratio: "+f2sz(VPUTOINCHES(ratio)).substr(0,5)).c_str());

      selectedItem = m_db.GetBestSizeMatch(width, height, pt->m_glassTopHeight, pt->m_glassBottomHeight);
   }
   GetDlgItem(IDC_SIZE_WIDTH).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_SIZE_HEIGHT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_VP_WIDTH).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_VP_HEIGHT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_COPY).ShowWindow(pt != nullptr);
   GetDlgItem(IDC_APPLY_TO_TABLE).ShowWindow(pt != nullptr);

   for (unsigned int i = 0; i < (unsigned int)m_db.m_data.size(); i++)
   {
      const TableDB::Entry& dim = m_db.m_data[i];
      m_listView.InsertItem(i, dim.name.c_str());
      m_listView.SetItemText(i, 1, f2sz(dim.width).c_str());
      m_listView.SetItemText(i, 2, f2sz(dim.height).c_str());
      m_listView.SetItemText(i, 3, f2sz(dim.glassBottom).c_str());
      m_listView.SetItemText(i, 4, f2sz(dim.glassTop).c_str());
      m_listView.SetItemText(i, 5, dim.comment.c_str());
      m_listView.SetItemState(i, i == selectedItem ? LVIS_SELECTED : 0, LVIS_SELECTED);
   }

   return TRUE;
}

void DimensionDialog::UpdateApplyState()
{
   GetDlgItem(IDC_APPLY_TO_TABLE).EnableWindow(false);
   CComObject<PinTable>* const pt = g_pvp->GetActiveTable();
   if (pt)
   {
      const float w = sz2f(GetDlgItemText(IDC_VP_WIDTH).GetString());
      const float h = sz2f(GetDlgItemText(IDC_VP_HEIGHT).GetString());
      const float t = sz2f(GetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT).GetString());
      const float b = sz2f(GetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT).GetString());
      if ((w > 0.f && pt->GetTableWidth() != w)
       || (h > 0.f && pt->GetHeight() != h)
       || (t > 0.f && pt->m_glassTopHeight != INCHESTOVPU(t))
       || (b > 0.f && pt->m_glassBottomHeight != INCHESTOVPU(b)))
         GetDlgItem(IDC_APPLY_TO_TABLE).EnableWindow(true);
   }
}

LRESULT DimensionDialog::OnNotify(WPARAM wparam, LPARAM lparam)
{
   const LPNMHDR pnmhdr = (LPNMHDR)lparam;
   switch (pnmhdr->code)
   {
      case LVN_ITEMCHANGED:
      {
         NMLISTVIEW* const plistview = (LPNMLISTVIEW)lparam;
         const int idx = plistview->iItem;
         if (idx >= (int)m_db.m_data.size() || idx < 0)
            break;
         SetDlgItemText(IDC_SIZE_WIDTH2, f2sz(m_db.m_data[idx].width).c_str());
         SetDlgItemText(IDC_SIZE_HEIGHT2, f2sz(m_db.m_data[idx].height).c_str());
         SetDlgItemText(IDC_VP_WIDTH2, f2sz(INCHESTOVPU(m_db.m_data[idx].width)).c_str());
         SetDlgItemText(IDC_VP_HEIGHT2, f2sz(INCHESTOVPU(m_db.m_data[idx].height)).c_str());
         SetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT2, f2sz(m_db.m_data[idx].glassTop).c_str());
         SetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT2, f2sz(m_db.m_data[idx].glassBottom).c_str());
         const float ratio = m_db.m_data[idx].height / m_db.m_data[idx].width;
         SetDlgItemText(IDC_AR_LABEL2, f2sz(ratio).c_str());
         UpdateApplyState();
         break;
      }
   }
   return CDialog::OnNotify(wparam, lparam); 
}


INT_PTR DimensionDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_COMMAND:
      {
         if (!m_discardChangeNotification && HIWORD(wParam) == EN_CHANGE)
         {
            m_discardChangeNotification = true;
            if (LOWORD(wParam) == IDC_SIZE_WIDTH)
            {
               float sizeWidth = fmaxf(sz2f(GetDlgItemText(IDC_SIZE_WIDTH).GetString()), 0.0f);
               SetDlgItemText(IDC_VP_WIDTH, f2sz(INCHESTOVPU(sizeWidth)).c_str());
            }
            else if (LOWORD(wParam) == IDC_SIZE_HEIGHT)
            {
               float sizeHeight = fmaxf(sz2f(GetDlgItemText(IDC_SIZE_HEIGHT).GetString()), 0.0f);
               SetDlgItemText(IDC_VP_HEIGHT, f2sz(INCHESTOVPU(sizeHeight)).c_str());
            }
            else if (LOWORD(wParam) == IDC_VP_WIDTH)
            {
               float vpWidth = fmaxf(sz2f(GetDlgItemText(IDC_VP_WIDTH).GetString()), 0.0f);
               SetDlgItemText(IDC_SIZE_WIDTH, f2sz((float)VPUTOINCHES(vpWidth)).c_str());
            }
            else if (LOWORD(wParam) == IDC_VP_HEIGHT)
            {
               float vpHeight = fmaxf(sz2f(GetDlgItemText(IDC_VP_HEIGHT).GetString()), 0.0f);
               SetDlgItemText(IDC_SIZE_HEIGHT, f2sz((float)VPUTOINCHES(vpHeight)).c_str());
            }
            m_discardChangeNotification = false;
            UpdateApplyState();
         }
         break;
      }
   }

   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL DimensionDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   CComObject<PinTable>* const pt = g_pvp->GetActiveTable();
   UNREFERENCED_PARAMETER(lParam);
   switch (LOWORD(wParam))
   {
   case IDC_OK:
      Close();
      return TRUE;
   case IDC_COPY:
      // TODO this will assert if fields are empty (no consequence but not clean)
      SetDlgItemText(IDC_SIZE_WIDTH, GetDlgItemText(IDC_SIZE_WIDTH2));
      SetDlgItemText(IDC_SIZE_HEIGHT, GetDlgItemText(IDC_SIZE_HEIGHT2));
      SetDlgItemText(IDC_VP_WIDTH, GetDlgItemText(IDC_VP_WIDTH2));
      SetDlgItemText(IDC_VP_HEIGHT, GetDlgItemText(IDC_VP_HEIGHT2));
      SetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT, GetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT2));
      SetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT, GetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT2));
      SetDlgItemText(IDC_AR_LABEL, GetDlgItemText(IDC_AR_LABEL2));
      UpdateApplyState();
      return TRUE;
   case IDC_APPLY_TO_TABLE:
      if (pt != nullptr)
      {
         float value = sz2f(GetDlgItemText(IDC_VP_WIDTH).GetString());
         if (value > 0.f)
            pt->put_Width(value);
         value = sz2f(GetDlgItemText(IDC_VP_HEIGHT).GetString());
         if (value > 0.f)
            pt->put_Height(value);
         value = sz2f(GetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT).GetString());
         if (value > 0.f)
            pt->m_glassTopHeight = INCHESTOVPU(value);
         value = sz2f(GetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT).GetString());
         if (value > 0.f)
            pt->m_glassBottomHeight = INCHESTOVPU(value);
      }
      UpdateApplyState();
      return TRUE;
   }
   return FALSE;
}
