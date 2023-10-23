#include "stdafx.h"
#include "resource.h"
#include "DimensionDialog.h"
#include <iostream>
#include <fstream>

DimensionDialog::DimensionDialog() : CDialog(IDD_DIMENSION_CALCULATOR)
{
}

BOOL DimensionDialog::OnInitDialog()
{
   AttachItem(IDC_TABLE_DIM_LIST, m_listView);
   m_listView.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   m_listView.InsertColumn(0, TEXT("Manufacturer"), LVCFMT_LEFT, 250);
   m_listView.InsertColumn(1, TEXT("Width"), LVCFMT_RIGHT, 80);
   m_listView.InsertColumn(2, TEXT("Height"), LVCFMT_RIGHT, 80);
   m_listView.InsertColumn(3, TEXT("Bot. Glass"), LVCFMT_RIGHT, 100);
   m_listView.InsertColumn(4, TEXT("Top Glass"), LVCFMT_RIGHT, 100);
   m_listView.InsertColumn(5, TEXT("Comments"), LVCFMT_LEFT, 250);

   m_dimensions.clear();
   std::fstream dbFile;
   dbFile.open(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "TableSizes.csv", std::ios::in); 
   if (dbFile.is_open())
   {
      int lineIndex = 0;
      string line;
      char textBuf[MAXNAMEBUFFER];
      while (std::getline(dbFile, line))
      {
         if (lineIndex != 0) // Skip header line
         {
            int fieldIndex = 0;
            size_t prevPos = 0, pos = 0;
            string fields[6];
            while (fieldIndex < 6 && (pos = line.find(';', prevPos)) != std::string::npos)
            {
               fields[fieldIndex] = line.substr(prevPos, pos);
               prevPos = pos + 1;
               fieldIndex++;
            }
            fields[fieldIndex] = line.substr(prevPos, line.length());

            ManufacturerDimensions dim;
            dim.name = fields[0];
            try { dim.width = std::stof(fields[1]); } catch (const std::exception&) { dim.width = 20.25f; }
            try { dim.height = std::stof(fields[2]); } catch (const std::exception&) { dim.height = 46.f; }
            try { dim.glassBottom = std::stof(fields[3]); } catch (const std::exception&) { dim.glassBottom = 3.f; }
            try { dim.glassTop = std::stof(fields[4]); } catch (const std::exception&) { dim.glassTop = 8.5f; }
            dim.comment = fields[5];
            m_dimensions.push_back(dim);

            m_listView.InsertItem(lineIndex - 1, dim.name.c_str());
            sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.width);
            m_listView.SetItemText(lineIndex - 1, 1, textBuf);
            sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.height);
            m_listView.SetItemText(lineIndex - 1, 2, textBuf);
            sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.glassBottom);
            m_listView.SetItemText(lineIndex - 1, 3, textBuf);
            sprintf_s(textBuf, sizeof(textBuf), "%.03f", dim.glassTop);
            m_listView.SetItemText(lineIndex - 1, 4, textBuf);
            m_listView.SetItemText(lineIndex - 1, 5, dim.comment.c_str());
         }
         lineIndex++;
      }
      dbFile.close();
   }

   CComObject<PinTable> * const pt = g_pvp->GetActiveTable();
   if (pt)
   {
      char textBuf[MAXNAMEBUFFER];
      const float width = pt->m_right - pt->m_left;
      const float height = pt->m_bottom - pt->m_top;
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
   }
   GetDlgItem(IDC_SIZE_WIDTH).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_SIZE_HEIGHT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_VP_WIDTH).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_VP_HEIGHT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT).EnableWindow(pt != nullptr);
   GetDlgItem(IDC_COPY).ShowWindow(pt != nullptr);
   GetDlgItem(IDC_APPLY_TO_TABLE).ShowWindow(pt != nullptr);

   return TRUE;
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
         if (idx >= m_dimensions.size() || idx < 0)
            break;
         char textBuf[MAXNAMEBUFFER];
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_dimensions[idx].width);
         SetDlgItemText(IDC_SIZE_WIDTH2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_dimensions[idx].height);
         SetDlgItemText(IDC_SIZE_HEIGHT2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", INCHESTOVPU(m_dimensions[idx].width));
         SetDlgItemText(IDC_VP_WIDTH2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", INCHESTOVPU(m_dimensions[idx].height));
         SetDlgItemText(IDC_VP_HEIGHT2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_dimensions[idx].glassTop);
         SetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT2, textBuf);
         sprintf_s(textBuf, sizeof(textBuf), "%.03f", m_dimensions[idx].glassBottom);
         SetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT2, textBuf);
         float ratio = m_dimensions[idx].height / m_dimensions[idx].width;
         sprintf_s(textBuf, sizeof(textBuf), "Aspect Ratio: %.04f", ratio);
         SetDlgItemText(IDC_AR_LABEL2, textBuf);
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
         char textBuf[MAXNAMEBUFFER];
         if (!m_discardChangeNotification && HIWORD(wParam) == EN_CHANGE)
         {
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
               if (ret != 1 || vpWidth < 0)
                  vpWidth = 0;
               const float width = (float)VPUTOINCHES(vpWidth);
               sprintf_s(textBuf, sizeof(textBuf), "%.3f", width);
               CString textStr2(textBuf);
               SetDlgItemText(IDC_SIZE_WIDTH, textStr2);
            }
            else if (LOWORD(wParam) == IDC_VP_HEIGHT)
            {
               ret = sscanf_s(GetDlgItemText(IDC_VP_HEIGHT).c_str(), "%f", &vpHeight);
               if (ret != 1 || vpHeight < 0)
                  vpHeight = 0;
               const float height = (float)VPUTOINCHES(vpHeight);
               sprintf_s(textBuf, sizeof(textBuf), "%.03f", height);
               CString textStr2(textBuf);
               SetDlgItemText(IDC_SIZE_HEIGHT, textStr2);
            }
            m_discardChangeNotification = false;
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
      return TRUE;
   case IDC_APPLY_TO_TABLE:
      if (pt != nullptr)
      {
         float value;
         int ret;
         ret = sscanf_s(GetDlgItemText(IDC_VP_WIDTH).c_str(), "%f", &value);
         if (ret == 1 && value > 0)
            pt->put_Width(value);
         ret = sscanf_s(GetDlgItemText(IDC_VP_HEIGHT).c_str(), "%f", &value);
         if (ret == 1 && value > 0)
            pt->put_Height(value);
         ret = sscanf_s(GetDlgItemText(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT).c_str(), "%f", &value);
         if (ret == 1 && value > 0)
            pt->m_glassTopHeight = INCHESTOVPU(value);
         ret = sscanf_s(GetDlgItemText(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT).c_str(), "%f", &value);
         if (ret == 1 && value > 0)
            pt->m_glassBottomHeight = INCHESTOVPU(value);
      }
      return TRUE;
   }
   return FALSE;
}
