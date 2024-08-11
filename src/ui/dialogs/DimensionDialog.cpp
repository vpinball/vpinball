// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "DimensionDialog.h"
#include <iostream>
#include <fstream>

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
      char textBuf[MAXNAMEBUFFER];
      const float width = pt->GetTableWidth();
      const float height = pt->GetHeight();
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", width);
      SetDlgItemText(IDC_VP_WIDTH, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", width);
      SetDlgItemText(IDC_VP_HEIGHT, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", VPUTOINCHES(width));
      SetDlgItemText(IDC_SIZE_WIDTH, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", VPUTOINCHES(height));
      SetDlgItemText(IDC_SIZE_HEIGHT, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", VPUTOINCHES(pt->m_glassTopHeight));
      SetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", VPUTOINCHES(pt->m_glassBottomHeight));
      SetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT, textBuf);
      float ratio = (float)height / width;
      sprintf_s(textBuf, sizeof(textBuf), "Aspect ratio: %.04f", ratio);
      SetDlgItemText(IDC_AR_LABEL, textBuf);

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
      char textBuf[MAXNAMEBUFFER];
      TableDB::Entry& dim = m_db.m_data[i];
      m_listView.InsertItem(i, dim.name.c_str());
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.width);
      m_listView.SetItemText(i, 1, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.height);
      m_listView.SetItemText(i, 2, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.glassBottom);
      m_listView.SetItemText(i, 3, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.glassTop);
      m_listView.SetItemText(i, 4, textBuf);
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
      float w, h, t, b;
      const int rw = sscanf_s(GetDlgItemText(IDC_VP_WIDTH).c_str(), "%f", &w);
      const int rh = sscanf_s(GetDlgItemText(IDC_VP_HEIGHT).c_str(), "%f", &h);
      const int rt = sscanf_s(GetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT).c_str(), "%f", &t);
      const int rb = sscanf_s(GetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT).c_str(), "%f", &b);
      if ((rw == 1 && w > 0.f && pt->GetTableWidth() != w)
       || (rh == 1 && h > 0.f && pt->GetHeight() != h)
       || (rt == 1 && t > 0.f && pt->m_glassTopHeight != INCHESTOVPU(t))
       || (rb == 1 && b > 0.f && pt->m_glassBottomHeight != INCHESTOVPU(b)))
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
         if (idx >= (int) m_db.m_data.size() || idx < 0)
            break;
         char textBuf[MAXNAMEBUFFER];
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_db.m_data[idx].width);
         SetDlgItemText(IDC_SIZE_WIDTH2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_db.m_data[idx].height);
         SetDlgItemText(IDC_SIZE_HEIGHT2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", INCHESTOVPU(m_db.m_data[idx].width));
         SetDlgItemText(IDC_VP_WIDTH2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", INCHESTOVPU(m_db.m_data[idx].height));
         SetDlgItemText(IDC_VP_HEIGHT2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_db.m_data[idx].glassTop);
         SetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_db.m_data[idx].glassBottom);
         SetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT2, textBuf);
         float ratio = m_db.m_data[idx].height / m_db.m_data[idx].width;
         sprintf_s(textBuf, sizeof(textBuf), "Aspect Ratio: %.04f", ratio);
         SetDlgItemText(IDC_AR_LABEL2, textBuf);
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
            char textBuf[MAXNAMEBUFFER];
            m_discardChangeNotification = true;
            float sizeWidth, sizeHeight;
            float vpWidth, vpHeight;
            int ret = 0;
            if (LOWORD(wParam) == IDC_SIZE_WIDTH)
            {
               ret = sscanf_s(GetDlgItemText(IDC_SIZE_WIDTH).c_str(), "%f", &sizeWidth);
               if (ret != 1 || sizeWidth < 0.0f)
                  sizeWidth = 0;
               sprintf_s(textBuf, sizeof(textBuf), "%.3f", INCHESTOVPU(sizeWidth));
               CString textStr2(textBuf);
               SetDlgItemText(IDC_VP_WIDTH, textStr2);
            }
            else if (LOWORD(wParam) == IDC_SIZE_HEIGHT)
            {
               ret = sscanf_s(GetDlgItemText(IDC_SIZE_HEIGHT).c_str(), "%f", &sizeHeight);
               if (ret != 1 || sizeHeight < 0.0f)
                  sizeHeight = 0;
               sprintf_s(textBuf, sizeof(textBuf), "%.3f", INCHESTOVPU(sizeHeight));
               CString textStr2(textBuf);
               SetDlgItemText(IDC_VP_HEIGHT, textStr2);
            }
            else if (LOWORD(wParam) == IDC_VP_WIDTH)
            {
               ret = sscanf_s(GetDlgItemText(IDC_VP_WIDTH).c_str(), "%f", &vpWidth);
               if (ret != 1 || vpWidth < 0.0f)
                  vpWidth = 0;
               const float width = (float)VPUTOINCHES(vpWidth);
               sprintf_s(textBuf, sizeof(textBuf), "%.3f", width);
               CString textStr2(textBuf);
               SetDlgItemText(IDC_SIZE_WIDTH, textStr2);
            }
            else if (LOWORD(wParam) == IDC_VP_HEIGHT)
            {
               ret = sscanf_s(GetDlgItemText(IDC_VP_HEIGHT).c_str(), "%f", &vpHeight);
               if (ret != 1 || vpHeight < 0.0f)
                  vpHeight = 0;
               const float height = (float)VPUTOINCHES(vpHeight);
               sprintf_s(textBuf, sizeof(textBuf), "%.03f", height);
               CString textStr2(textBuf);
               SetDlgItemText(IDC_SIZE_HEIGHT, textStr2);
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
         float value;
         int ret;
         ret = sscanf_s(GetDlgItemText(IDC_VP_WIDTH).c_str(), "%f", &value);
         if (ret == 1 && value > 0.f)
            pt->put_Width(value);
         ret = sscanf_s(GetDlgItemText(IDC_VP_HEIGHT).c_str(), "%f", &value);
         if (ret == 1 && value > 0.f)
            pt->put_Height(value);
         ret = sscanf_s(GetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT).c_str(), "%f", &value);
         if (ret == 1 && value > 0.f)
            pt->m_glassTopHeight = INCHESTOVPU(value);
         ret = sscanf_s(GetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT).c_str(), "%f", &value);
         if (ret == 1 && value > 0.f)
            pt->m_glassBottomHeight = INCHESTOVPU(value);
      }
      UpdateApplyState();
      return TRUE;
   }
   return FALSE;
}
