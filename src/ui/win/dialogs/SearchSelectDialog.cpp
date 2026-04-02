// license:GPLv3+

#include "core/stdafx.h"
#include "ui/win/resource.h"
#include "SearchSelectDialog.h"

#include "parts/plunger.h"
#include "parts/flipper.h"
#include "parts/timer.h"
#include "parts/textbox.h"
#include "parts/surface.h"
#include "parts/dispreel.h"
#include "parts/lightseq.h"
#include "parts/bumper.h"
#include "parts/trigger.h"
#include "parts/light.h"
#include "parts/kicker.h"
#include "parts/decal.h"
#include "parts/primitive.h"
#include "parts/hittarget.h"
#include "parts/gate.h"
#include "parts/spinner.h"
#include "parts/ramp.h"
#include "parts/flasher.h"
#include "parts/rubber.h"
#include "parts/PartGroup.h"

typedef struct _tagSORTDATA
{
   HWND hwndList;
   int subItemIndex;
   int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);

bool SearchSelectDialog::m_switchSortOrder;
bool SearchSelectDialog::m_columnSortOrder;
int SearchSelectDialog::m_lastSortColumn;

SearchSelectDialog::SearchSelectDialog(PinTableWnd *table)
   : CDialog(IDD_SEARCH_SELECT_ELEMENT)
   , m_hElementList(nullptr)
   , m_curTable(table)
{
   m_switchSortOrder = false;
   m_columnSortOrder = true;
   m_lastSortColumn = 0;
}

void SearchSelectDialog::Update()
{
   ListView_SetExtendedListViewStyle(m_hElementList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

   int index;
   LVCOLUMN lvc = {};
   lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
   lvc.cx = 200;
   lvc.pszText = (LPSTR)TEXT("Name");
   index = ListView_InsertColumn(m_hElementList, 0, &lvc);
   lvc.cx = 70;
   lvc.pszText = (LPSTR)TEXT("Type");
   index = ListView_InsertColumn(m_hElementList, 1, &lvc);
   lvc.cx = 100;
   lvc.pszText = (LPSTR)TEXT("Layer");
   index = ListView_InsertColumn(m_hElementList, 2, &lvc);
   lvc.cx = 200;
   lvc.pszText = (LPSTR)TEXT("Image");
   index = ListView_InsertColumn(m_hElementList, 3, &lvc);
   lvc.cx = 250;
   lvc.pszText = (LPSTR)TEXT("Material");
   index = ListView_InsertColumn(m_hElementList, 4, &lvc);
   lvc.cx = 250;
   lvc.pszText = (LPSTR)TEXT("Physics Material");
   index = ListView_InsertColumn(m_hElementList, 5, &lvc);
   lvc.cx = 100;
   lvc.pszText = (LPSTR)TEXT("Collection");
   index = ListView_InsertColumn(m_hElementList, 6, &lvc);
   lvc.cx = 70;
   lvc.pszText = (LPSTR)TEXT("Collidable");
   index = ListView_InsertColumn(m_hElementList, 7, &lvc);
   lvc.cx = 70;
   lvc.pszText = (LPSTR)TEXT("Visible");
   index = ListView_InsertColumn(m_hElementList, 8, &lvc);
   lvc.cx = 70;
   lvc.pszText = (LPSTR)TEXT("Timer enabled");
   index = ListView_InsertColumn(m_hElementList, 9, &lvc);
   lvc.cx = 70;
   lvc.pszText = (LPSTR)TEXT("Depth Bias");
   index = ListView_InsertColumn(m_hElementList, 10, &lvc);
   lvc.cx = 70;
   lvc.pszText = (LPSTR)TEXT("Static rendering");
   index = ListView_InsertColumn(m_hElementList, 11, &lvc);
   lvc.cx = 70;
   lvc.pszText = (LPSTR)TEXT("Reflections enabled");
   index = ListView_InsertColumn(m_hElementList, 12, &lvc);
   lvc.cx = 200;
   lvc.pszText = (LPSTR)TEXT("Surface");
   index = ListView_InsertColumn(m_hElementList, 13, &lvc);

   if (m_hElementList != nullptr)
      ListView_DeleteAllItems(m_hElementList);

   int idx = 0;
   for (int i = 0; i < m_curTable->m_table->m_vcollection.size(); i++)
   {
      CComObject<Collection> *const pcol = m_curTable->m_table->m_vcollection.ElementAt(i);
      LVITEM lv;
      lv.mask = LVIF_TEXT | LVIF_PARAM;
      lv.iItem = idx;
      lv.iSubItem = 0;
      lv.lParam = (LPARAM)pcol;
      string name = MakeString(pcol->m_wzName);
      lv.pszText = name.data();
      ListView_InsertItem(m_hElementList, &lv);
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Collection");
      idx++;
   }
   for (IEditable *const piedit : m_curTable->m_table->GetParts())
   {
      if (IScriptable *const piscript = piedit->GetScriptable(); piscript)
      {
         LVITEM lv;
         lv.mask = LVIF_TEXT | LVIF_PARAM;
         lv.iItem = idx;
         lv.iSubItem = 0;
         lv.lParam = (LPARAM)piscript;
         const string szTemp = PinTable::GetElementName(piedit);
         lv.pszText = (char*)szTemp.c_str();
         ListView_InsertItem(m_hElementList, &lv);
         AddSearchItemToList(piedit, idx);
         idx++;
      }
   }
}

void SearchSelectDialog::OnClose()
{
   SavePosition();
   CDialog::OnClose();
}


BOOL SearchSelectDialog::OnInitDialog()
{
   m_hElementList = GetDlgItem(IDC_ELEMENT_LIST).GetHwnd();

   const string windowName = "Search/Select Element - " + m_curTable->m_table->m_filename.string();
   SetWindowText(windowName.c_str());

   m_switchSortOrder = false;

   m_resizer.Initialize(GetHwnd(), CRect(0, 0, 650, 400));
   m_resizer.AddChild(GetDlgItem(IDC_ELEMENT_LIST).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(GetDlgItem(IDOK).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDCANCEL).GetHwnd(), CResizer::topright, 0);
   Update();
   SortItems(0);
   LoadPosition();
   SetFocus();
   ListView_SetItemState(m_hElementList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
   ListView_EnsureVisible(m_hElementList, 0, false);
   GotoDlgCtrl(m_hElementList);

   return FALSE;
}

void SearchSelectDialog::SelectElement()
{
    const int count = ListView_GetSelectedCount(m_hElementList);

    m_curTable->m_table->ClearMultiSel();
    int iItem = -1;
    for (int i = 0; i < count; i++)
    {
        iItem = ListView_GetNextItem(m_hElementList, iItem, LVNI_SELECTED);
        LVITEM lv;
        lv.iItem = iItem;
        lv.mask = LVIF_PARAM;
        if (ListView_GetItem(m_hElementList, &lv) == TRUE)
        {
           char szType[MAXSTRING];
           ListView_GetItemText(m_hElementList, iItem, 1, szType, std::size(szType));
           if (szType == "Collection"sv)
           {
              CComObject<Collection> *const pcol = (CComObject<Collection>*)lv.lParam;
              if (!pcol->m_visel.empty())
              {
                 ISelect *const pisel = pcol->m_visel.ElementAt(0);
                 if (pisel)
                    m_curTable->m_table->AddMultiSel(pisel, false, true, false);
              }
           }
           else
           {
              IScriptable * const pscript = (IScriptable*)lv.lParam;
              for (const auto &pedit : m_curTable->m_table->GetParts())
              {
                 if (pscript == pedit->GetScriptable())
                 {
                    if (ISelect *const pisel = pedit->GetISelect(); pisel)
                       m_curTable->m_table->AddMultiSel(pisel, true, true, false);
                    break;
                 }
              }
           }
        }
    }
}

void SearchSelectDialog::SortItems(const int columnNumber)
{
   if (m_switchSortOrder)
      m_columnSortOrder = !m_columnSortOrder;

   SortData.hwndList = m_hElementList;
   SortData.subItemIndex = m_switchSortOrder ? columnNumber: m_lastSortColumn;
   m_lastSortColumn = columnNumber;
   SortData.sortUpDown = m_columnSortOrder ? 1 : 0;
   ListView_SortItems(SortData.hwndList, MyCompProc, &SortData);
   m_switchSortOrder = true;
}

INT_PTR SearchSelectDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);

   switch (uMsg)
   {
      case WM_NOTIFY:
      {
         if (((LPNMHDR)lParam)->code == NM_DBLCLK)
         {
             SelectElement();
         }
         else if (wParam == IDC_ELEMENT_LIST)
         {
            const LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
            if (lpnmListView->hdr.code == LVN_COLUMNCLICK)
            {
               const int columnNumber = lpnmListView->iSubItem;
               SortItems(columnNumber);
            }
         }
         break;
      }
      case WM_SIZE:
      {
         const CRect rc = GetWindowRect();
         const int windowHeight = rc.bottom - rc.top;
         const int windowWidth = rc.right - rc.left;

         const int buttonY = (windowHeight - 85) + 5;
         const CRect buttonRc = GetDlgItem(IDOK).GetClientRect();
         const int buttonWidth = buttonRc.right - buttonRc.left;
         const int buttonHeight = buttonRc.bottom - buttonRc.top;
         
         ::SetWindowPos(m_hElementList, nullptr, 6, 5, windowWidth - 28, windowHeight - 90, 0);
         GetDlgItem(IDOK).SetWindowPos(nullptr, 6, buttonY, buttonWidth, buttonHeight, 0);
         GetDlgItem(IDCANCEL).SetWindowPos(nullptr, 6 + buttonWidth + 50, buttonY, buttonWidth, buttonHeight, 0);
         break;
      }

   }
   return DialogProcDefault(uMsg, wParam, lParam);
}

void SearchSelectDialog::OnOK()
{
   SelectElement();
}

void SearchSelectDialog::OnCancel()
{
   SavePosition();
   CDialog::OnCancel();
}

bool SearchSelectDialog::IsValidString(const string &name)
{
   return (!name.empty() && name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ._-") == string::npos);
}

void SearchSelectDialog::AddSearchItemToList(IEditable * const piedit, int idx)
{
   constexpr char usedStringYes[] = "X";
   constexpr char usedStringNo[] = " ";

   ListView_SetItemText_Safe(m_hElementList, idx, 2, piedit->GetPathString(false).c_str());

   string textBuf;
   string firstImage;
   string secondImage;

   switch (piedit->GetItemType())
   {
   case eItemSurface:
   {
      const Surface *const sur = (Surface*)piedit;
      if (IsValidString(sur->m_d.m_szImage))
         firstImage = sur->m_d.m_szImage;
      textBuf = firstImage;
      if (IsValidString(sur->m_d.m_szSideImage))
         secondImage = sur->m_d.m_szSideImage;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--"sv;
         textBuf += secondImage;
      }
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Wall");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, textBuf.c_str());

      firstImage.clear();
      if (IsValidString(sur->m_d.m_szTopMaterial))
         firstImage = sur->m_d.m_szTopMaterial;
      textBuf = firstImage;
      secondImage.clear();
      if (IsValidString(sur->m_d.m_szSideMaterial))
         secondImage = sur->m_d.m_szSideMaterial;
      string thirdImage;
      if (IsValidString(sur->m_d.m_szSlingShotMaterial))
          thirdImage = sur->m_d.m_szSlingShotMaterial;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--"sv;
         textBuf += secondImage;
      }
      if (!thirdImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty())
            textBuf += "--"sv;
         textBuf += thirdImage;
      }
      ListView_SetItemText_Safe(m_hElementList, idx, 4, textBuf.c_str());
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(sur->m_d.m_szPhysicsMaterial) ? sur->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, sur->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, sur->m_d.m_sideVisible ? (sur->m_d.m_topBottomVisible ? usedStringYes : "S") : (sur->m_d.m_topBottomVisible ? "T" : usedStringNo));
      ListView_SetItemText_Safe(m_hElementList, idx, 9, sur->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*sur->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, sur->StaticRendering() ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 12, /*sur->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 13, "N/A");

      break;
   }
   case eItemRamp:
   {
      const Ramp *const ramp = (Ramp*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Ramp");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(ramp->m_d.m_szImage) ? ramp->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(ramp->m_d.m_szMaterial) ? ramp->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(ramp->m_d.m_szPhysicsMaterial) ? ramp->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, ramp->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, ramp->m_d.m_visible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, ramp->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, f2sz(ramp->m_d.m_depthBias).c_str());

      const Material *const mat = m_curTable->m_table->GetMaterial(ramp->m_d.m_szMaterial);
      ListView_SetItemText_Safe(m_hElementList, idx, 11, mat == nullptr || !mat->m_bOpacityActive ? usedStringYes : usedStringNo); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 12, ramp->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, "N/A");

      break;
   }
   case eItemFlasher:
   {
      const Flasher *const flasher = (Flasher*)piedit;
      if (IsValidString(flasher->m_d.m_szImageA))
         firstImage = flasher->m_d.m_szImageA;
      if (IsValidString(flasher->m_d.m_szImageB))
         secondImage = flasher->m_d.m_szImageB;
      textBuf = firstImage;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--"sv;
         textBuf += secondImage;
      }
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Flasher");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, textBuf.c_str());
      ListView_SetItemText_Safe(m_hElementList, idx, 4, "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, /*flasher->m_d.m_collidable ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 8, flasher->m_d.m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, flasher->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, f2sz(flasher->m_d.m_depthBias).c_str());
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*flasher->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, /*flasher->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 13, "N/A");

      break;
   }
   case eItemRubber:
   {
      const Rubber *const rubber = (Rubber*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Rubber");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(rubber->m_d.m_szImage) ? rubber->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(rubber->m_d.m_szMaterial) ? rubber->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(rubber->m_d.m_szPhysicsMaterial) ? rubber->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, rubber->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, rubber->m_d.m_visible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, rubber->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*rubber->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, rubber->m_d.m_staticRendering ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 12, rubber->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, "N/A");

      break;
   }
   case eItemSpinner:
   {
      const Spinner *const spinner = (Spinner*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Spinner");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(spinner->m_d.m_szImage) ? spinner->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(spinner->m_d.m_szMaterial) ? spinner->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(spinner->m_d.m_szPhysicsMaterial) ? spinner->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, spinner->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, spinner->m_d.m_visible ? (spinner->m_d.m_showBracket ? usedStringYes : "S") : (spinner->m_d.m_showBracket ? "B" : usedStringNo));
      ListView_SetItemText_Safe(m_hElementList, idx, 9, spinner->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*spinner->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*spinner->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, spinner->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, spinner->m_d.m_szSurface.c_str());

      break;
   }
   case eItemKicker:
   {
      const Kicker *const kicker = (Kicker*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Kicker");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(kicker->m_d.m_szMaterial) ? kicker->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(kicker->m_d.m_szPhysicsMaterial) ? kicker->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, kicker->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, kicker->m_d.m_kickertype != KickerInvisible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, kicker->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*kicker->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*kicker->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, kicker->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, kicker->m_d.m_szSurface.c_str());

      break;
   }
   case eItemLight:
   {
      const Light *const light = (Light*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Light");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(light->m_d.m_szImage) ? light->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, light->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, light->m_d.m_visible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, light->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, f2sz(light->m_d.m_depthBias).c_str());
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*light->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, light->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, light->m_d.m_szSurface.c_str());

      break;
   }
   case eItemBumper:
   {
      const Bumper *const bumper = (Bumper*)piedit;
      if (IsValidString(bumper->m_d.m_szBaseMaterial))
         firstImage = bumper->m_d.m_szBaseMaterial;
      textBuf = firstImage;
      if (IsValidString(bumper->m_d.m_szCapMaterial))
         secondImage = bumper->m_d.m_szCapMaterial;
      string thirdImage;
      if (IsValidString(bumper->m_d.m_szSkirtMaterial))
         thirdImage = bumper->m_d.m_szSkirtMaterial;
      string fourthImage;
      if (IsValidString(bumper->m_d.m_szRingMaterial))
          fourthImage = bumper->m_d.m_szRingMaterial;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--"sv;
         textBuf += secondImage;
      }
      if (!thirdImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty())
            textBuf += "--"sv;
         textBuf += thirdImage;
      }
      if (!fourthImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty() || !thirdImage.empty())
            textBuf += "--"sv;
         textBuf += fourthImage;
      }
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Bumper");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, textBuf.c_str());
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(bumper->m_d.m_szPhysicsMaterial) ? bumper->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, bumper->m_d.m_collidable ? usedStringYes : usedStringNo);
      const bool fully_visible = bumper->m_d.m_capVisible && bumper->m_d.m_baseVisible && bumper->m_d.m_ringVisible && bumper->m_d.m_skirtVisible;
      const bool partly_visible = bumper->m_d.m_capVisible || bumper->m_d.m_baseVisible || bumper->m_d.m_ringVisible || bumper->m_d.m_skirtVisible;
      ListView_SetItemText_Safe(m_hElementList, idx, 8, fully_visible ? usedStringYes : (partly_visible ? "/" : usedStringNo));
      ListView_SetItemText_Safe(m_hElementList, idx, 9, bumper->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*bumper->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*bumper->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, bumper->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, bumper->m_d.m_szSurface.c_str());

      break;
   }
   case eItemFlipper:
   {
      const Flipper* const flipper = (Flipper*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Flipper");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(flipper->m_d.m_szImage) ? flipper->m_d.m_szImage.c_str() : "");

      if (IsValidString(flipper->m_d.m_szMaterial))
         firstImage = flipper->m_d.m_szMaterial;
      textBuf = firstImage;
      if (IsValidString(flipper->m_d.m_szRubberMaterial))
         secondImage = flipper->m_d.m_szRubberMaterial;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--"sv;
         textBuf += secondImage;
      }
      ListView_SetItemText_Safe(m_hElementList, idx, 4, textBuf.c_str());

      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(flipper->m_d.m_szPhysicsMaterial) ? flipper->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, flipper->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, flipper->m_d.m_visible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, flipper->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*flipper->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*flipper->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, flipper->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, flipper->m_d.m_szSurface.c_str());

      break;
   }
   case eItemGate:
   {
      const Gate *const gate = (Gate*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Gate");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(gate->m_d.m_szMaterial) ? gate->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(gate->m_d.m_szPhysicsMaterial) ? gate->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, gate->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, gate->m_d.m_visible ? (gate->m_d.m_showBracket ? usedStringYes : "W") : (gate->m_d.m_showBracket ? "B" : usedStringNo));
      ListView_SetItemText_Safe(m_hElementList, idx, 9, gate->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*gate->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*gate->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, gate->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, gate->m_d.m_szSurface.c_str());

      break;
   }
   case eItemTrigger:
   {
      const Trigger *const trigger = (Trigger*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Trigger");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(trigger->m_d.m_szMaterial) ? trigger->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(trigger->m_d.m_szPhysicsMaterial) ? trigger->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, trigger->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, trigger->m_d.m_visible && trigger->m_d.m_shape != TriggerNone ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, trigger->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*trigger->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*trigger->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, trigger->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, trigger->m_d.m_szSurface.c_str());

      break;
   }
   case eItemTimer:
   {
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Timer");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, "");

      break;
   }
   case eItemTextbox:
   {
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "TextBox");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, "");

      break;
   }
   case eItemPlunger:
   {
      const Plunger *const plunger = (Plunger*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Plunger");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(plunger->m_d.m_szImage) ? plunger->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(plunger->m_d.m_szMaterial) ? plunger->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(plunger->m_d.m_szPhysicsMaterial) ? plunger->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, plunger->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, plunger->m_d.m_visible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, plunger->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, /*plunger->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*plunger->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, plunger->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, plunger->m_d.m_szSurface.c_str());

      break;
   }
   case eItemDispReel:
   {
      const DispReel *const dispReel = (DispReel*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "EMReel");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(dispReel->m_d.m_szImage) ? dispReel->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, "");

      break;
   }
   case eItemPrimitive:
   {
      const Primitive *const primitive = (Primitive*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Primitive");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(primitive->m_d.m_szImage) ? primitive->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(primitive->m_d.m_szMaterial) ? primitive->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(primitive->m_d.m_szPhysicsMaterial) ? primitive->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, primitive->m_d.m_collidable && (!primitive->m_d.m_toy || primitive->IsPlayfield()) ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, primitive->m_d.m_visible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, primitive->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, f2sz(primitive->m_d.m_depthBias).c_str());
      ListView_SetItemText_Safe(m_hElementList, idx, 11, primitive->m_d.m_staticRendering ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 12, primitive->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, "N/A");

      break;
   }
   case eItemHitTarget:
   {
      const HitTarget *const hitTarget = (HitTarget*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Target");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(hitTarget->m_d.m_szImage) ? hitTarget->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(hitTarget->m_d.m_szMaterial) ? hitTarget->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(hitTarget->m_d.m_szPhysicsMaterial) ? hitTarget->m_d.m_szPhysicsMaterial.c_str() : "");

      ListView_SetItemText_Safe(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText_Safe(m_hElementList, idx, 7, hitTarget->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 8, hitTarget->m_d.m_visible ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 9, hitTarget->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 10, f2sz(hitTarget->m_d.m_depthBias).c_str());
      ListView_SetItemText_Safe(m_hElementList, idx, 11, /*hitTarget->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText_Safe(m_hElementList, idx, 12, hitTarget->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText_Safe(m_hElementList, idx, 13, "N/A");

      break;
   }
   case eItemDecal:
   {
      const Decal *const decal = (Decal*)piedit;
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "Decal");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, IsValidString(decal->m_d.m_szImage) ? decal->m_d.m_szImage.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, IsValidString(decal->m_d.m_szMaterial) ? decal->m_d.m_szMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 5, IsValidString(decal->m_d.m_szPhysicsMaterial) ? decal->m_d.m_szPhysicsMaterial.c_str() : "");
      ListView_SetItemText_Safe(m_hElementList, idx, 13, decal->m_d.m_szSurface.c_str());

      break;
   }
   case eItemLightSeq:
   {
       //const LightSeq* const lightSeq = (LightSeq*)piedit;
       ListView_SetItemText_Safe(m_hElementList, idx, 1, "LightSeq");
       ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
       ListView_SetItemText_Safe(m_hElementList, idx, 4, "");

       break;
   }
   case eItemPartGroup:
   {
      ListView_SetItemText_Safe(m_hElementList, idx, 1, "PartGroup");
      ListView_SetItemText_Safe(m_hElementList, idx, 3, "");
      ListView_SetItemText_Safe(m_hElementList, idx, 4, "");

      break;
   }
   default:
      assert(!"AddSearchItemToList unhandled case");
      break;
   }
}

void SearchSelectDialog::LoadPosition()
{
   const int x = g_app->m_settings.GetEditor_SearchSelectPosX();
   const int y = g_app->m_settings.GetEditor_SearchSelectPosY();
   const int w = g_app->m_settings.GetEditor_SearchSelectWidth();
   const int h = g_app->m_settings.GetEditor_SearchSelectHeight();
   POINT p { x, y };
   if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
      SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SearchSelectDialog::SavePosition()
{
   const CRect rect = GetWindowRect();
   g_app->m_settings.SetEditor_SearchSelectPosX((int)rect.left, false);
   g_app->m_settings.SetEditor_SearchSelectPosY((int)rect.top, false);
   g_app->m_settings.SetEditor_SearchSelectWidth(rect.right - rect.left, false);
   g_app->m_settings.SetEditor_SearchSelectHeight(rect.bottom - rect.top, false);
}
