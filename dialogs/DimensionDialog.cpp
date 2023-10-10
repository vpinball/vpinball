#include "stdafx.h"
#include "resource.h"
#include "DimensionDialog.h"

struct ManufacturerDimensions
{
   string name;
   float width;
   float height;
};

#define DIM_TABLE_SIZE 40
static const ManufacturerDimensions dimTable[DIM_TABLE_SIZE] =
{
   { "Atari (widebody)"s, 27.0f, 45.0f },
   { "Bally EM (standard)"s, 20.25f, 41.0f },
   { "Bally (standard)"s, 20.25f, 42.0f },
   { "Bally (widebody)"s, 26.75f, 42.0f }, //!! OPA claims 40.5
   { "Capcom"s, 20.25f, 46.0f },
   { "Data East (up to Hook)"s, 20.25f, 42.0f }, //!! cyberpez: A KLOV member measured his BTTF for me: "42.5 x 20.25. I expect the extra half inch to be an anomaly on my machine, or I'm measuring at the wrong spot." (speculation: at least Batman, Monday Night Football, King Kong and Phantom of the Opera, are this size too)
   { "Data East/Sega (standard)"s, 20.25f, 46.0f }, // verified by Sliderpoint on RaB (speculation: at least TMNT and LW3 are this size, too)
   { "Data East/Sega (widebody)"s, 23.25f, 46.0f },
   { "Game Plan"s, 20.25f, 42.0f },
   { "Gottlieb EM (through 76)"s, 20.25f, 41.0f },
   { "Gottlieb EM (76-79)"s, 20.25f, 42.0f },
   { "Gottlieb System 1 (standard)"s, 20.25f, 42.0f },
   { "Gottlieb System 1 (widebody)"s, 26.75f, 47.0f },
   { "Gottlieb System 80 (standard)"s, 20.25f, 42.0f },
   { "Gottlieb System 80 (widebody)"s, 23.875f, 47.875f }, // verified on at least two actual machines (via BorgDog)
   { "Gottlieb System 80 (extrawide)"s, 26.75f, 46.5f },
   { "Gottlieb System 3"s, 20.25f, 46.0f },
   { "Stern (widebody)"s, 23.875f, 45.0f },
   { "Stern (standard)"s, 20.25f, 42.0f },
   { "Stern Modern (standard)"s, 20.25f, 45.0f },
   { "Williams EM (standard)"s, 20.25f, 42.0f }, // measured by Bord via a Williams 1965 EM
   { "Williams System 1-11 (standard)"s, 20.25f, 42.0f },
   { "Williams System 1-11 (widebody)"s, 27.0f, 42.0f },
   { "WMS WPC (standard)"s, 20.5f, 46.0f },
   { "WMS WPC (superpin)"s, 23.25f, 46.0f },
   { "WMS Pinball 2000"s, 20.5f, 43.0f },
   { "Zaccaria (standard)"s, 20.25f, 42.0f },
   { "Black Knight 2000 (1991)"s, 20.25f, 46.0f }, //!! OPA: 20.2756 x 42.0
   { "Bride Of Pinbot (1991)"s, 20.25f, 45.25f }, //!! OPA: 20.2362 x 45.27559
   { "BSD Dracula (1993)"s, 20.5f, 45.0f }, //!! chepas claims its 20.5 (by pics/compared to TAF), OPA 20.25
   { "Doctor Who (1992)"s, 20.25f, 45.0625f },
   { "Future Spa (1979)"s, 26.7717f, 40.55118f },
   { "Guns N' Roses (1994)"s, 23.0f, 46.0f }, // Niwak: changed to 23"x46" after checking from multiple sources and hirez playfield scan (was 25"x51"75)
   { "Hercules (1979 Atari)"s, 36.0f, 72.0f },
   { "Mystery Castle (1993 Alvin G)"s, 20.25f, 46.0f },
   // Playboy does not have standard size, so around 20.25f, 42.0f ???
   { "Safe Cracker (1996)"s, 16.5f, 41.5f }, //!! OPA: "propably" 18.35 x 41.5
   { "Secret Service (1988)"s, 20.275f, 42.126f }, // measured by Baron Shadow
   { "Star Trek 25th (1991)"s, 20.25f, 42.625f }, //!! via cyberpez, with an error of +/- 1/4 of an inch (bracket was in the way), so 42.375-42.875
   { "Varkon (1982)"s, 24.0f, 21.0f },
   { "World Cup Soccer (1994)"s, 20.25f, 45.75f } //!! OPA: 20.2756 x 45.7874
};

DimensionDialog::DimensionDialog() : CDialog(IDD_DIMENSION_CALCULATOR)
{
}

BOOL DimensionDialog::OnInitDialog()
{
   const HWND listHwnd = GetDlgItem(IDC_TABLE_DIM_LIST).GetHwnd();
   LVITEM lv;
   ListView_SetExtendedListViewStyle(listHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   LVCOLUMN lvc = {};
   lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
   lvc.cx = 150;
   lvc.pszText = (LPSTR)TEXT("Manufacturer");
   ListView_InsertColumn(listHwnd, 0, &lvc);
   lvc.cx = 60;
   lvc.pszText = (LPSTR)TEXT("Width");
   ListView_InsertColumn(listHwnd, 1, &lvc);
   lvc.cx = 60;
   lvc.pszText = (LPSTR)TEXT("Height");
   ListView_InsertColumn(listHwnd, 2, &lvc);

   if (listHwnd != nullptr)
      ListView_DeleteAllItems(listHwnd);
   lv.mask = LVIF_TEXT;
   for (int i = 0; i < DIM_TABLE_SIZE; i++)
   {
      lv.iItem = i;
      lv.iSubItem = 0;
      lv.pszText = (LPSTR)dimTable[i].name.c_str();
      ListView_InsertItem(listHwnd, &lv);

      char textBuf[MAXNAMEBUFFER];
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", dimTable[i].width);
      ListView_SetItemText(listHwnd, i, 1, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", dimTable[i].height);
      ListView_SetItemText(listHwnd, i, 2, textBuf);
   }
   
   CComObject<PinTable> * const pt = g_pvp->GetActiveTable();
   if (pt)
   {
      char textBuf[MAXNAMEBUFFER];
      const int width = (int)(pt->m_right - pt->m_left + 0.5f);
      const int height = (int)(pt->m_bottom - pt->m_top + 0.5f);
      SetDlgItemText(IDC_VP_WIDTH, std::to_string(width).c_str());
      SetDlgItemText(IDC_VP_HEIGHT, std::to_string(height).c_str());
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", VPUTOINCHES(width));
      SetDlgItemText(IDC_SIZE_WIDTH, textBuf);
      sprintf_s(textBuf, sizeof(textBuf), "%.03f", VPUTOINCHES(height));
      SetDlgItemText(IDC_SIZE_HEIGHT, textBuf);
      float ratio = (float)height / width;
      sprintf_s(textBuf, sizeof(textBuf), "%.04f", ratio);
      SetDlgItemText(IDC_ASPECT_RATIO_EDIT, textBuf);
   }

   return TRUE;
}

INT_PTR DimensionDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_NOTIFY:
      {
         const LPNMHDR pnmhdr = (LPNMHDR)lParam;
         switch (pnmhdr->code)
         {
            case LVN_ITEMCHANGED:
            {
               NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
               const int idx = plistview->iItem;
               if (idx >= DIM_TABLE_SIZE || idx < 0)
                  break;

               const int width = (int)(INCHESTOVPU(dimTable[idx].width) + 0.5f);
               const int height = (int)(INCHESTOVPU(dimTable[idx].height) + 0.5f);
               SetDlgItemText(IDC_VP_WIDTH, std::to_string(width).c_str());
               SetDlgItemText(IDC_VP_HEIGHT, std::to_string(height).c_str());
               char textBuf[MAXNAMEBUFFER];
               sprintf_s(textBuf, sizeof(textBuf), "%.03f", dimTable[idx].width);
               SetDlgItemText(IDC_SIZE_WIDTH, textBuf);
               sprintf_s(textBuf, sizeof(textBuf), "%.03f", dimTable[idx].height);
               SetDlgItemText(IDC_SIZE_HEIGHT, textBuf);
               float ratio = (float)height / width;
               sprintf_s(textBuf, sizeof(textBuf), "%.04f", ratio);
               SetDlgItemText(IDC_ASPECT_RATIO_EDIT, textBuf);
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
                  ret = sscanf_s(GetDlgItemText(IDC_SIZE_WIDTH).c_str(), "%f", &sizeWidth);
                  if (ret != 1 || sizeWidth < 0.0f)
                     sizeWidth = 0;
                  const int width = (int)(INCHESTOVPU(sizeWidth) + 0.5f);
                  SetDlgItemText(IDC_VP_WIDTH, std::to_string(width).c_str());
               }
               if (LOWORD(wParam) == IDC_SIZE_HEIGHT)
               {
                  ret = sscanf_s(GetDlgItemText(IDC_SIZE_HEIGHT).c_str(), "%f", &sizeHeight);
                  if (ret != 1 || sizeHeight < 0.0f)
                     sizeHeight = 0;
                  const int height = (int)(INCHESTOVPU(sizeHeight) + 0.5f);
                  SetDlgItemText(IDC_VP_HEIGHT, std::to_string(height).c_str());
               }
               if (LOWORD(wParam) == IDC_VP_WIDTH)
               {
                  ret = sscanf_s(GetDlgItemText(IDC_VP_WIDTH).c_str(), "%i", &vpWidth);
                  if (ret != 1 || vpWidth < 0)
                     vpWidth = 0;
                  const float width = (float)VPUTOINCHES(vpWidth);
                  char textBuf[MAXNAMEBUFFER];
                  sprintf_s(textBuf, sizeof(textBuf), "%.3f", width);
                  CString textStr2(textBuf);
                  SetDlgItemText(IDC_SIZE_WIDTH, textStr2);
               }
               if (LOWORD(wParam) == IDC_VP_HEIGHT)
               {
                  ret = sscanf_s(GetDlgItemText(IDC_VP_HEIGHT).c_str(), "%i", &vpHeight);
                  if (ret != 1 || vpHeight < 0)
                     vpHeight = 0;
                  const float height = (float)VPUTOINCHES(vpHeight);
                  char textBuf[MAXNAMEBUFFER];
                  sprintf_s(textBuf, sizeof(textBuf), "%.03f", height);
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
      case IDC_APPLY_TO_TABLE:
      {
         CComObject<PinTable> * const pt = g_pvp->GetActiveTable();
         if (pt == nullptr)
         {
            ShowError("No table loaded!");
            break;
         }

         int vpWidth, vpHeight;
         int ret;

         ret = sscanf_s(GetDlgItemText(IDC_VP_WIDTH).c_str(), "%i", &vpWidth);
         if (ret != 1 || vpWidth < 0)
            vpWidth = 0;

         ret = sscanf_s(GetDlgItemText(IDC_VP_HEIGHT).c_str(), "%i", &vpHeight);
         if (ret != 1 || vpHeight < 0)
            vpHeight = 0;
         pt->put_Width((float)vpWidth);
         pt->put_Height((float)vpHeight);
         break;
      }
      case IDC_OK:
         CDialog::OnOK();
      default:
         return FALSE;
   }
   return TRUE;
}
