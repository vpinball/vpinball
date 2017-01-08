#include "StdAfx.h"
#include "resource.h"
#include "DimensionDialog.h"
struct ManufacturerDimensions
{
   char name[32];
   float width;
   float height;
};

#define DIM_TABLE_SIZE 36
ManufacturerDimensions dimTable[DIM_TABLE_SIZE] =
{
   { "Atari (widebody)", 27.0f, 45.0f },
   { "Bally 70s EM (standard)", 20.25f, 41.0f },
   { "Bally (standard)", 20.25f, 42.0f },
   { "Bally (widebody)", 26.75f, 42.0f }, //!! OPA claims 40.5
   { "Capcom", 20.25f, 46.0f },
   { "Data East/Sega (standard)", 20.25f, 46.0f },
   { "Data East/Sega (widebody)", 23.25f, 46.0f },
   { "Game Plan", 20.25f, 42.0f },
   { "Gottlieb 70s EM (standard)", 20.25f, 41.0f },
   { "Gottlieb System 1 (standard)", 20.25f, 42.0f },
   { "Gottlieb System 3", 20.25f, 46.0f },
   { "Gottlieb System 80 (standard)", 20.25f, 42.0f },
   { "Gottlieb System 80 (widebody)", 23.75f, 46.5f },
   { "Gottlieb System 80 (extrawide)", 26.75f, 46.5f },
   { "Stern (widebody)", 23.875f, 45.0f },
   { "Stern (standard)", 20.25f, 42.0f },
   { "Stern Modern (standard)", 20.25f, 45.0f },
   { "WMS Pinball 2000", 20.5f, 43.0f },
   { "WMS System 1-11 (standard)", 20.25f, 42.0f },
   { "WMS System 1-11 (widebody)", 27.0f, 42.0f },
   { "WPC (through 1987)", 20.5f, 42.0f },
   { "WPC (1987 on)", 20.5f, 46.0f },
   { "WPC (superpin)", 23.25f, 46.0f },
   { "Zaccaria (standard)", 20.25f, 42.0f },
   { "Black Knight 2000 (1991)", 20.25f, 46.0f }, //!! OPA: 20.2756 x 42.0
   { "Bride Of Pinbot (1991)", 20.25f, 45.25f }, //!! OPA: 20.2362 x 45.27559
   { "BSD Dracula (1993)", 20.5f, 45.0f }, //!! chepas claims its 20.5 (by pics/compared to TAF), OPA 20.25
   { "Doctor Who (1992)", 20.25f, 45.0625f },
   { "Future Spa (1979)", 26.7717f, 40.55118f },
   { "Genie (1979)", 16.75f, 47.0f },
   { "Guns N' Roses (1994)", 25.0f, 51.75f },
   { "Hercules (1979 Atari)", 36.0f, 72.0f },
   { "Mystery Castle (Alvin G)", 20.25f, 46.0f },
   { "Safecracker", 16.5f, 41.5f }, //!! OPA: "propably" 18.35 x 41.5
   { "Varkon (1982)", 24.0f, 21.0f },
   { "World Cup Soccer (1994)", 20.25f, 45.75f } //!! OPA: 20.2756 x 45.7874
};

DimensionDialog::DimensionDialog() : CDialog(IDD_DIMENSION_CALCULATOR)
{
}

BOOL DimensionDialog::OnInitDialog()
{
   HWND listHwnd = GetDlgItem(IDC_TABLE_DIM_LIST).GetHwnd();
   LVCOLUMN lvc;
   LVITEM lv;
   ListView_SetExtendedListViewStyle(listHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   memset(&lvc, 0, sizeof(LVCOLUMN));
   lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
   lvc.cx = 150;
   lvc.pszText = TEXT("Manufacturer");
   ListView_InsertColumn(listHwnd, 0, &lvc);
   lvc.cx = 60;
   lvc.pszText = TEXT("Width");
   ListView_InsertColumn(listHwnd, 1, &lvc);
   lvc.cx = 60;
   lvc.pszText = TEXT("Height");
   ListView_InsertColumn(listHwnd, 2, &lvc);

   if (listHwnd != NULL)
      ListView_DeleteAllItems(listHwnd);
   lv.mask = LVIF_TEXT;
   char textBuf[32];
   for (int i = 0; i < DIM_TABLE_SIZE; i++)
   {
      lv.iItem = i;
      lv.iSubItem = 0;
      lv.pszText = dimTable[i].name;
      ListView_InsertItem(listHwnd, &lv);
      sprintf_s(textBuf, "%.03f", dimTable[i].width);
      ListView_SetItemText(listHwnd, i, 1, textBuf);
      sprintf_s(textBuf, "%.03f", dimTable[i].height);
      ListView_SetItemText(listHwnd, i, 2, textBuf);
   }
   return TRUE;
}

INT_PTR DimensionDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_NOTIFY:
   {
      LPNMHDR pnmhdr = (LPNMHDR)lParam;
      switch (pnmhdr->code)
      {
      case LVN_ITEMCHANGED:
      {
         NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
         int idx = plistview->iItem;
         if (idx >= DIM_TABLE_SIZE || idx < 0)
            break;

         int width = (int)floorf(dimTable[idx].width*47.0f + 0.5f);
         int height = (int)floorf(dimTable[idx].height*47.0f + 0.5f);
         char textBuf[32];
         sprintf_s(textBuf, "%i", width);
         CString textStr(textBuf);
         SetDlgItemText(IDC_VP_WIDTH, textStr);
         sprintf_s(textBuf, "%i", height);
         textStr = CString(textBuf);
         SetDlgItemText(IDC_VP_HEIGHT, textStr);
         sprintf_s(textBuf, "%.03f", dimTable[idx].width);
         textStr = CString(textBuf);
         SetDlgItemText(IDC_SIZE_WIDTH, textStr);
         sprintf_s(textBuf, "%.03f", dimTable[idx].height);
         textStr = CString(textBuf);
         SetDlgItemText(IDC_SIZE_HEIGHT, textStr);
         float ratio = (float)height / width;
         sprintf_s(textBuf, "%.04f", ratio);
         textStr = CString(textBuf);
         SetDlgItemText(IDC_ASPECT_RATIO_EDIT, textStr);
         break;
      }
      }
      break;
   }
   case WM_COMMAND:
   {
      switch (HIWORD(wParam))
      {
      case EN_KILLFOCUS:
      {
         float sizeWidth, sizeHeight;
         int vpWidth, vpHeight;
         int ret = 0;
         if (LOWORD(wParam) == IDC_SIZE_WIDTH)
         {
            char textBuf[32];
            CString textStr;
            textStr = GetDlgItemText(IDC_SIZE_WIDTH);
            ret = sscanf_s(textStr.c_str(), "%f", &sizeWidth);
            if (ret != 1 || sizeWidth < 0.0f)
               sizeWidth = 0;
            int width = (int)floorf(sizeWidth*47.0f + 0.5f);
            sprintf_s(textBuf, "%i", width);
            CString textStr2(textBuf);
            SetDlgItemText(IDC_VP_WIDTH, textStr2);
         }
         if (LOWORD(wParam) == IDC_SIZE_HEIGHT)
         {
            char textBuf[32];
            CString textStr;
            textStr = GetDlgItemText(IDC_SIZE_HEIGHT);
            ret = sscanf_s(textStr.c_str(), "%f", &sizeHeight);
            if (ret != 1 || sizeHeight < 0.0f)
               sizeHeight = 0;
            int height = (int)floorf(sizeHeight*47.0f + 0.5f);
            sprintf_s(textBuf, "%i", height);
            CString textStr2(textBuf);
            SetDlgItemText(IDC_VP_HEIGHT, textStr2);
         }
         if (LOWORD(wParam) == IDC_VP_WIDTH)
         {
            char textBuf[32];
            CString textStr;
            textStr = GetDlgItemText(IDC_VP_WIDTH);
            ret = sscanf_s(textStr.c_str(), "%i", &vpWidth);
            if (ret != 1 || vpWidth < 0)
               vpWidth = 0;
            float width = (float)vpWidth / 47.0f;
            sprintf_s(textBuf, "%.3f", width);
            CString textStr2(textBuf);
            SetDlgItemText(IDC_SIZE_WIDTH, textStr2);
         }
         if (LOWORD(wParam) == IDC_VP_HEIGHT)
         {
            char textBuf[32];
            CString textStr;
            textStr = GetDlgItemText(IDC_VP_HEIGHT);
            ret = sscanf_s(textStr.c_str(), "%i", &vpHeight);
            if (ret != 1 || vpHeight < 0)
               vpHeight = 0;
            float height = (float)vpHeight / 47.0f;
            sprintf_s(textBuf, "%.03f", height);
            CString textStr2(textBuf);
            SetDlgItemText(IDC_SIZE_HEIGHT, textStr2);
         }
         break;
      }
      }
   }
   }
   return DialogProcDefault(uMsg, wParam, lParam);

}

BOOL DimensionDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_OK:
         CDialog::OnOK();
      default:
         return FALSE;
   }
   return TRUE;
}

