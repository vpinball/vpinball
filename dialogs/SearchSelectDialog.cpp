#include "stdafx.h"
#include "resource.h"
#include "SearchSelectDialog.h"

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

SearchSelectDialog::SearchSelectDialog() : CDialog(IDD_SEARCH_SELECT_ELEMENT)
{
    m_hElementList = nullptr;

    m_switchSortOrder = false;
    m_columnSortOrder = true;
    m_lastSortColumn = 0;
    m_curTable = nullptr;
}

void SearchSelectDialog::Update()
{
   ListView_SetExtendedListViewStyle(m_hElementList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

   int index;
   LVCOLUMN lvc = {};
   lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
   lvc.cx = 200;
   lvc.pszText = TEXT("Name");
   index = ListView_InsertColumn(m_hElementList, 0, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Type");
   index = ListView_InsertColumn(m_hElementList, 1, &lvc);
   lvc.cx = 100;
   lvc.pszText = TEXT("Layer");
   index = ListView_InsertColumn(m_hElementList, 2, &lvc);
   lvc.cx = 200;
   lvc.pszText = TEXT("Image");
   index = ListView_InsertColumn(m_hElementList, 3, &lvc);
   lvc.cx = 250;
   lvc.pszText = TEXT("Material");
   index = ListView_InsertColumn(m_hElementList, 4, &lvc);
   lvc.cx = 250;
   lvc.pszText = TEXT("Physics Material");
   index = ListView_InsertColumn(m_hElementList, 5, &lvc);
   lvc.cx = 100;
   lvc.pszText = TEXT("Collection");
   index = ListView_InsertColumn(m_hElementList, 6, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Collidable");
   index = ListView_InsertColumn(m_hElementList, 7, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Visible");
   index = ListView_InsertColumn(m_hElementList, 8, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Timer enabled");
   index = ListView_InsertColumn(m_hElementList, 9, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Depth Bias");
   index = ListView_InsertColumn(m_hElementList, 10, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Static rendering");
   index = ListView_InsertColumn(m_hElementList, 11, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Reflections enabled");
   index = ListView_InsertColumn(m_hElementList, 12, &lvc);

   if (m_hElementList != nullptr)
      ListView_DeleteAllItems(m_hElementList);

   int idx = 0;
   for (int i = 0; i < m_curTable->m_vcollection.size(); i++)
   {
      CComObject<Collection> *const pcol = m_curTable->m_vcollection.ElementAt(i);
      char szT[sizeof(pcol->m_wzName)/sizeof(pcol->m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, pcol->m_wzName, -1, szT, sizeof(szT), nullptr, nullptr);
      LVITEM lv;
      lv.mask = LVIF_TEXT | LVIF_PARAM;
      lv.iItem = idx;
      lv.iSubItem = 0;
      lv.lParam = (LPARAM)pcol;
      lv.pszText = szT;
      ListView_InsertItem(m_hElementList, &lv);
      ListView_SetItemText(m_hElementList, idx, 1, "Collection");
      idx++;
   }
   for (size_t i = 0; i < m_curTable->m_vedit.size(); i++)
   {
      IEditable * const piedit = m_curTable->m_vedit[i];
      IScriptable * const piscript = piedit->GetScriptable();
      if (piscript)
      {
         LVITEM lv;
         lv.mask = LVIF_TEXT | LVIF_PARAM;
         lv.iItem = idx;
         lv.iSubItem = 0;
         lv.lParam = (LPARAM)piscript;
         lv.pszText = (char*)m_curTable->GetElementName(piedit);
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
   m_curTable = g_pvp->GetActiveTable();

   m_switchSortOrder = false;

   m_resizer.Initialize(*this, CRect(0, 0, 650, 400));
   m_resizer.AddChild(GetDlgItem(IDC_ELEMENT_LIST).GetHwnd(), topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(GetDlgItem(IDOK).GetHwnd(), bottomleft, 0);
   m_resizer.AddChild(GetDlgItem(IDCANCEL).GetHwnd(), bottomleft, 0);
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

    m_curTable->ClearMultiSel();
    int iItem = -1;
    for (int i = 0; i < count; i++)
    {
        iItem = ListView_GetNextItem(m_hElementList, iItem, LVNI_SELECTED);
        LVITEM lv;
        lv.iItem = iItem;
        lv.mask = LVIF_PARAM;
        if (ListView_GetItem(m_hElementList, &lv) == TRUE)
        {
           char szType[MAXNAMEBUFFER*2];
           ListView_GetItemText(m_hElementList, iItem, 1, szType, MAXNAMEBUFFER);
           if (strcmp(szType, "Collection") == 0)
           {
              CComObject<Collection> *const pcol = (CComObject<Collection>*)lv.lParam;
              if (!pcol->m_visel.empty())
              {
                 ISelect *const pisel = pcol->m_visel.ElementAt(0);
                 if (pisel)
                    m_curTable->AddMultiSel(pisel, false, true, false);
              }
           }
           else
           {
              IScriptable * const pscript = (IScriptable*)lv.lParam;
              ISelect *const pisel = pscript->GetISelect();
              if (pisel)
                 m_curTable->AddMultiSel(pisel, true, true, false);
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

         const HWND hOkButton = GetDlgItem(IDOK).GetHwnd();
         const int buttonY = (windowHeight - 85) + 5;
         RECT buttonRc;
         ::GetClientRect(hOkButton, &buttonRc);
         const int buttonWidth = buttonRc.right - buttonRc.left;
         const int buttonHeight = buttonRc.bottom - buttonRc.top;
         
         ::SetWindowPos(m_hElementList, nullptr, 6, 5, windowWidth - 28, windowHeight - 90, 0);
         ::SetWindowPos(hOkButton, nullptr, 6, buttonY, buttonWidth, buttonHeight, 0);
         ::SetWindowPos(GetDlgItem(IDCANCEL).GetHwnd(), nullptr, 6 + buttonWidth + 50, buttonY, buttonWidth, buttonHeight, 0);
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

bool SearchSelectDialog::IsValidString(const std::string &name)
{
   return (!name.empty() && name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ._-") == std::string::npos);
}

void SearchSelectDialog::AddSearchItemToList(IEditable * const piedit, int idx)
{
   constexpr char *const usedStringYes = "X";
   constexpr char *const usedStringNo = " ";

   string layerBuf;

   const IScriptable * const piscript = piedit->GetScriptable();
   if (piscript)
       layerBuf = piscript->GetISelect()->m_layerName;

   ListView_SetItemText(m_hElementList, idx, 2, (LPSTR)layerBuf.c_str());

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
            textBuf += "--";
         textBuf += secondImage;
      }
      ListView_SetItemText(m_hElementList, idx, 1, "Wall");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      firstImage.clear();
      if (IsValidString(sur->m_d.m_szTopMaterial))
         firstImage = sur->m_d.m_szTopMaterial;
      textBuf = firstImage;
      secondImage.clear();
      if (IsValidString(sur->m_d.m_szSideMaterial))
         secondImage = sur->m_d.m_szSideMaterial;
      std::string thirdImage;
      if (IsValidString(sur->m_d.m_szSlingShotMaterial))
          thirdImage = sur->m_d.m_szSlingShotMaterial;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--";
         textBuf += secondImage;
      }
      if (!thirdImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty())
            textBuf += "--";
         textBuf += thirdImage;
      }
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(sur->m_d.m_szPhysicsMaterial))
         textBuf = sur->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, sur->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, sur->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, sur->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*sur->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, sur->StaticRendering() ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 12, /*sur->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo*/ "N/A");
      break;
   }
   case eItemRamp:
   {
      const Ramp *const ramp = (Ramp*)piedit;
      if (IsValidString(ramp->m_d.m_szImage))
         textBuf = ramp->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Ramp");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(ramp->m_d.m_szMaterial))
         textBuf = ramp->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(ramp->m_d.m_szPhysicsMaterial))
         textBuf = ramp->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, ramp->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, ramp->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, ramp->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      char textBuf[MAXNAMEBUFFER];
      sprintf_s(textBuf, "%.03f", ramp->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBuf);
      ListView_SetItemText(m_hElementList, idx, 11, ramp->isHabitrail() /*&& !ramp->m_ptable->GetMaterial(ramp->m_d.m_szMaterial)->m_bOpacityActive*/ ? usedStringYes : usedStringNo); //!!
      ListView_SetItemText(m_hElementList, idx, 12, ramp->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
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
            textBuf += "--";
         textBuf += secondImage;
      }
      ListView_SetItemText(m_hElementList, idx, 1, "Flasher");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
      ListView_SetItemText(m_hElementList, idx, 4, "");

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, /*flasher->m_d.m_collidable ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 8, flasher->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, flasher->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      char textBuf[MAXNAMEBUFFER];
      sprintf_s(textBuf, "%.03f", flasher->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBuf);
      ListView_SetItemText(m_hElementList, idx, 11, /*flasher->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, /*flasher->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo*/ "N/A");
      break;
   }
   case eItemRubber:
   {
      const Rubber *const rubber = (Rubber*)piedit;
      if (IsValidString(rubber->m_d.m_szImage))
         textBuf = rubber->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Rubber");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(rubber->m_d.m_szMaterial))
         textBuf = rubber->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(rubber->m_d.m_szPhysicsMaterial))
         textBuf = rubber->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, rubber->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, rubber->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, rubber->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*rubber->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, rubber->m_d.m_staticRendering ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 12, rubber->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemSpinner:
   {
      const Spinner *const spinner = (Spinner*)piedit;
      if (IsValidString(spinner->m_d.m_szImage))
         textBuf = spinner->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Spinner");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(spinner->m_d.m_szMaterial))
         textBuf = spinner->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(spinner->m_d.m_szPhysicsMaterial))
         textBuf = spinner->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, spinner->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, spinner->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, spinner->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*spinner->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*spinner->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, spinner->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemKicker:
   {
      const Kicker *const kicker = (Kicker*)piedit;
      ListView_SetItemText(m_hElementList, idx, 1, "Kicker");
      ListView_SetItemText(m_hElementList, idx, 3, "");

      if (IsValidString(kicker->m_d.m_szMaterial))
         textBuf = kicker->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(kicker->m_d.m_szPhysicsMaterial))
         textBuf = kicker->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, kicker->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, kicker->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, kicker->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*kicker->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*kicker->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, kicker->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemLight:
   {
      const Light *const light = (Light*)piedit;
      if (IsValidString(light->m_d.m_szImage))
         textBuf = light->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Light");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
      ListView_SetItemText(m_hElementList, idx, 4, "");

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, light->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, light->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, light->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      char textBuf[MAXNAMEBUFFER];
      sprintf_s(textBuf, "%.03f", light->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBuf);
      ListView_SetItemText(m_hElementList, idx, 11, /*light->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, light->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
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
      std::string thirdImage;
      if (IsValidString(bumper->m_d.m_szSkirtMaterial))
         thirdImage = bumper->m_d.m_szSkirtMaterial;
      std::string fourthImage;
      if (IsValidString(bumper->m_d.m_szRingMaterial))
          fourthImage = bumper->m_d.m_szRingMaterial;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--";
         textBuf += secondImage;
      }
      if (!thirdImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty())
            textBuf += "--";
         textBuf += thirdImage;
      }
      if (!fourthImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty() || !thirdImage.empty())
            textBuf += "--";
         textBuf += fourthImage;
      }
      ListView_SetItemText(m_hElementList, idx, 1, "Bumper");
      ListView_SetItemText(m_hElementList, idx, 3, "");
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(bumper->m_d.m_szPhysicsMaterial))
         textBuf = bumper->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, bumper->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, bumper->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, bumper->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*bumper->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*bumper->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, bumper->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemFlipper:
   {
      const Flipper* const flipper = (Flipper*)piedit;
      if (IsValidString(flipper->m_d.m_szImage))
         textBuf = flipper->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Flipper");
      ListView_SetItemText(m_hElementList, idx, 3, "");

      if (IsValidString(flipper->m_d.m_szMaterial))
         firstImage = flipper->m_d.m_szMaterial;
      textBuf = firstImage;
      if (IsValidString(flipper->m_d.m_szRubberMaterial))
         secondImage = flipper->m_d.m_szRubberMaterial;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf += "--";
         textBuf += secondImage;
      }
      ListView_SetItemText(m_hElementList, idx, 4, "");

      textBuf.clear();
      if (IsValidString(flipper->m_d.m_szPhysicsMaterial))
         textBuf = flipper->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, flipper->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, flipper->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, flipper->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*flipper->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*flipper->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, flipper->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemGate:
   {
      const Gate *const gate = (Gate*)piedit;
      ListView_SetItemText(m_hElementList, idx, 1, "Gate");
      ListView_SetItemText(m_hElementList, idx, 3, "");

      if (IsValidString(gate->m_d.m_szMaterial))
         textBuf = gate->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(gate->m_d.m_szPhysicsMaterial))
         textBuf = gate->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, gate->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, gate->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, gate->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*gate->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*gate->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, gate->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemTrigger:
   {
      const Trigger *const trigger = (Trigger*)piedit;
      ListView_SetItemText(m_hElementList, idx, 1, "Trigger");
      ListView_SetItemText(m_hElementList, idx, 3, "");

      if (IsValidString(trigger->m_d.m_szMaterial))
         textBuf = trigger->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(trigger->m_d.m_szPhysicsMaterial))
         textBuf = trigger->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, trigger->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, trigger->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, trigger->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*trigger->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*trigger->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, trigger->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemTimer:
   {
      ListView_SetItemText(m_hElementList, idx, 1, "Timer");
      ListView_SetItemText(m_hElementList, idx, 3, "");
      ListView_SetItemText(m_hElementList, idx, 4, "");

      break;
   }
   case eItemTextbox:
   {
      ListView_SetItemText(m_hElementList, idx, 1, "Textbox");
      ListView_SetItemText(m_hElementList, idx, 3, "");
      ListView_SetItemText(m_hElementList, idx, 4, "");

      break;
   }
   case eItemPlunger:
   {
      const Plunger *const plunger = (Plunger*)piedit;
      if (IsValidString(plunger->m_d.m_szImage))
         textBuf = plunger->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Plunger");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(plunger->m_d.m_szMaterial))
         textBuf = plunger->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(plunger->m_d.m_szPhysicsMaterial))
         textBuf = plunger->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, plunger->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, plunger->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, plunger->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 10, /*plunger->m_d.m_depthBias*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*plunger->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, plunger->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemDispReel:
   {
      const DispReel *const dispReel = (DispReel*)piedit;
      if (IsValidString(dispReel->m_d.m_szImage))
         textBuf = dispReel->m_d.m_szImage;

      ListView_SetItemText(m_hElementList, idx, 1, "EMReel");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
      ListView_SetItemText(m_hElementList, idx, 4, "");

      break;
   }
   case eItemPrimitive:
   {
      const Primitive *const primitive = (Primitive*)piedit;
      if (IsValidString(primitive->m_d.m_szImage))
         textBuf = primitive->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Primitive");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(primitive->m_d.m_szMaterial))
         textBuf = primitive->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(primitive->m_d.m_szPhysicsMaterial))
         textBuf = primitive->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, primitive->m_d.m_collidable && (!primitive->m_d.m_toy || primitive->m_d.m_useAsPlayfield) ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, primitive->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, primitive->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      char textBuf[MAXNAMEBUFFER];
      sprintf_s(textBuf, "%.03f", primitive->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBuf);
      ListView_SetItemText(m_hElementList, idx, 11, primitive->m_d.m_staticRendering ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 12, primitive->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemHitTarget:
   {
      const HitTarget *const hitTarget = (HitTarget*)piedit;
      if (IsValidString(hitTarget->m_d.m_szImage))
         textBuf = hitTarget->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Target");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(hitTarget->m_d.m_szMaterial))
         textBuf = hitTarget->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(hitTarget->m_d.m_szPhysicsMaterial))
         textBuf = hitTarget->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, "N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, hitTarget->m_d.m_collidable ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 8, hitTarget->m_isVisible ? usedStringYes : usedStringNo);
      ListView_SetItemText(m_hElementList, idx, 9, hitTarget->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo);
      char textBuf[MAXNAMEBUFFER];
      sprintf_s(textBuf, "%.03f", hitTarget->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBuf);
      ListView_SetItemText(m_hElementList, idx, 11, /*hitTarget->StaticRendering() ? usedStringYes : usedStringNo*/ "N/A");
      ListView_SetItemText(m_hElementList, idx, 12, hitTarget->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo);
      break;
   }
   case eItemDecal:
   {
      const Decal *const decal = (Decal*)piedit;
      if (IsValidString(decal->m_d.m_szImage))
         textBuf = decal->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Decal");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(decal->m_d.m_szMaterial))
         textBuf = decal->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(decal->m_d.m_szPhysicsMaterial))
         textBuf = decal->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      break;
   }
   case eItemLightSeq:
   {
       const LightSeq* const lightSeq = (LightSeq*)piedit;
       ListView_SetItemText(m_hElementList, idx, 1, "LightSeq");
       ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
       ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

       break;
   }
   default:
      assert(!"AddSearchItemToList unhandled case");
      break;
   }
}

void SearchSelectDialog::LoadPosition()
{
   const int x = LoadValueIntWithDefault("Editor", "SearchSelectPosX", 0);
   const int y = LoadValueIntWithDefault("Editor", "SearchSelectPosY", 0);
   const int w = LoadValueIntWithDefault("Editor", "SearchSelectWidth", 650);
   const int h = LoadValueIntWithDefault("Editor", "SearchSelectHeight", 400);

   SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SearchSelectDialog::SavePosition()
{
   const CRect rect = GetWindowRect();
   SaveValueInt("Editor", "SearchSelectPosX", rect.left);
   SaveValueInt("Editor", "SearchSelectPosY", rect.top);
   const int w = rect.right - rect.left;
   SaveValueInt("Editor", "SearchSelectWidth", w);
   const int h = rect.bottom - rect.top;
   SaveValueInt("Editor", "SearchSelectHeight", h);
}
