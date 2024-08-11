// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
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
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Collection");
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

   m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 650, 400));
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

bool SearchSelectDialog::IsValidString(const string &name)
{
   return (!name.empty() && name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ._-") == string::npos);
}

void SearchSelectDialog::AddSearchItemToList(IEditable * const piedit, int idx)
{
   constexpr char usedStringYes[] = "X";
   constexpr char usedStringNo[] = " ";

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
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Wall");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

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

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(sur->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(sur->m_d.m_sideVisible ? (sur->m_d.m_topBottomVisible ? usedStringYes : "S") : (sur->m_d.m_topBottomVisible ? "T" : usedStringNo)));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(sur->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*sur->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, (LPSTR)(sur->StaticRendering() ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 12, /*sur->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)"N/A");
      break;
   }
   case eItemRamp:
   {
      const Ramp *const ramp = (Ramp*)piedit;
      if (IsValidString(ramp->m_d.m_szImage))
         textBuf = ramp->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Ramp");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(ramp->m_d.m_szMaterial))
         textBuf = ramp->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(ramp->m_d.m_szPhysicsMaterial))
         textBuf = ramp->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(ramp->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(ramp->m_d.m_visible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(ramp->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      char textBufc[MAXNAMEBUFFER];
      sprintf_s(textBufc, sizeof(textBufc), "%.03f", ramp->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBufc);

      const Material *const mat = m_curTable->GetMaterial(ramp->m_d.m_szMaterial);
      ListView_SetItemText(m_hElementList, idx, 11, (LPSTR)(mat == nullptr || !mat->m_bOpacityActive ? usedStringYes : usedStringNo)); //!!
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(ramp->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)"N/A");
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
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Flasher");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)"");

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, /*flasher->m_d.m_collidable ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(flasher->m_d.m_isVisible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(flasher->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      char textBufc[MAXNAMEBUFFER];
      sprintf_s(textBufc, sizeof(textBufc), "%.03f", flasher->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBufc);
      ListView_SetItemText(m_hElementList, idx, 11, /*flasher->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, /*flasher->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)"N/A");
      break;
   }
   case eItemRubber:
   {
      const Rubber *const rubber = (Rubber*)piedit;
      if (IsValidString(rubber->m_d.m_szImage))
         textBuf = rubber->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Rubber");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(rubber->m_d.m_szMaterial))
         textBuf = rubber->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(rubber->m_d.m_szPhysicsMaterial))
         textBuf = rubber->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(rubber->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(rubber->m_d.m_visible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(rubber->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*rubber->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, (LPSTR)(rubber->m_d.m_staticRendering ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(rubber->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)"N/A");
      break;
   }
   case eItemSpinner:
   {
      const Spinner *const spinner = (Spinner*)piedit;
      if (IsValidString(spinner->m_d.m_szImage))
         textBuf = spinner->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Spinner");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(spinner->m_d.m_szMaterial))
         textBuf = spinner->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(spinner->m_d.m_szPhysicsMaterial))
         textBuf = spinner->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(spinner->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(spinner->m_d.m_visible ? (spinner->m_d.m_showBracket ? usedStringYes : "S") : (spinner->m_d.m_showBracket ? "B" : usedStringNo)));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(spinner->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*spinner->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*spinner->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(spinner->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)spinner->m_d.m_szSurface.c_str());
      break;
   }
   case eItemKicker:
   {
      const Kicker *const kicker = (Kicker*)piedit;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Kicker");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");

      if (IsValidString(kicker->m_d.m_szMaterial))
         textBuf = kicker->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(kicker->m_d.m_szPhysicsMaterial))
         textBuf = kicker->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(kicker->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(kicker->m_d.m_kickertype != KickerInvisible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(kicker->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*kicker->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*kicker->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(kicker->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)kicker->m_d.m_szSurface.c_str());
      break;
   }
   case eItemLight:
   {
      const Light *const light = (Light*)piedit;
      if (IsValidString(light->m_d.m_szImage))
         textBuf = light->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Light");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)"");

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(light->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(light->m_d.m_visible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(light->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      char textBufc[MAXNAMEBUFFER];
      sprintf_s(textBufc, sizeof(textBufc), "%.03f", light->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBufc);
      ListView_SetItemText(m_hElementList, idx, 11, /*light->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(light->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)light->m_d.m_szSurface.c_str());
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
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Bumper");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(bumper->m_d.m_szPhysicsMaterial))
         textBuf = bumper->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(bumper->m_d.m_collidable ? usedStringYes : usedStringNo));
      bool fully_visible = bumper->m_d.m_capVisible && bumper->m_d.m_baseVisible && bumper->m_d.m_ringVisible && bumper->m_d.m_skirtVisible;
      bool partly_visible = bumper->m_d.m_capVisible || bumper->m_d.m_baseVisible || bumper->m_d.m_ringVisible || bumper->m_d.m_skirtVisible;
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(fully_visible ? usedStringYes : (partly_visible ? "/" : usedStringNo)));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(bumper->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*bumper->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*bumper->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(bumper->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)bumper->m_d.m_szSurface.c_str());
      break;
   }
   case eItemFlipper:
   {
      const Flipper* const flipper = (Flipper*)piedit;
      if (IsValidString(flipper->m_d.m_szImage))
         textBuf = flipper->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Flipper");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");

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
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)"");

      textBuf.clear();
      if (IsValidString(flipper->m_d.m_szPhysicsMaterial))
         textBuf = flipper->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(flipper->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(flipper->m_d.m_visible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(flipper->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*flipper->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*flipper->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(flipper->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)flipper->m_d.m_szSurface.c_str());
      break;
   }
   case eItemGate:
   {
      const Gate *const gate = (Gate*)piedit;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Gate");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");

      if (IsValidString(gate->m_d.m_szMaterial))
         textBuf = gate->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(gate->m_d.m_szPhysicsMaterial))
         textBuf = gate->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(gate->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(gate->m_d.m_visible ? (gate->m_d.m_showBracket ? usedStringYes : "W") : (gate->m_d.m_showBracket ? "B" : usedStringNo)));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(gate->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*gate->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*gate->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(gate->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)gate->m_d.m_szSurface.c_str());
      break;
   }
   case eItemTrigger:
   {
      const Trigger *const trigger = (Trigger*)piedit;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Trigger");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");

      if (IsValidString(trigger->m_d.m_szMaterial))
         textBuf = trigger->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(trigger->m_d.m_szPhysicsMaterial))
         textBuf = trigger->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(trigger->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(trigger->m_d.m_visible && trigger->m_d.m_shape != TriggerNone ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(trigger->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*trigger->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*trigger->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(trigger->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)trigger->m_d.m_szSurface.c_str());
      break;
   }
   case eItemTimer:
   {
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Timer");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)"");

      break;
   }
   case eItemTextbox:
   {
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Textbox");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)"");

      break;
   }
   case eItemPlunger:
   {
      const Plunger *const plunger = (Plunger*)piedit;
      if (IsValidString(plunger->m_d.m_szImage))
         textBuf = plunger->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Plunger");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(plunger->m_d.m_szMaterial))
         textBuf = plunger->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(plunger->m_d.m_szPhysicsMaterial))
         textBuf = plunger->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(plunger->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(plunger->m_d.m_visible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(plunger->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 10, /*plunger->m_d.m_depthBias*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 11, /*plunger->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(plunger->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)plunger->m_d.m_szSurface.c_str());
      break;
   }
   case eItemDispReel:
   {
      const DispReel *const dispReel = (DispReel*)piedit;
      if (IsValidString(dispReel->m_d.m_szImage))
         textBuf = dispReel->m_d.m_szImage;

      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"EMReel");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)"");

      break;
   }
   case eItemPrimitive:
   {
      const Primitive *const primitive = (Primitive*)piedit;
      if (IsValidString(primitive->m_d.m_szImage))
         textBuf = primitive->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Primitive");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(primitive->m_d.m_szMaterial))
         textBuf = primitive->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(primitive->m_d.m_szPhysicsMaterial))
         textBuf = primitive->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(primitive->m_d.m_collidable && (!primitive->m_d.m_toy || primitive->IsPlayfield()) ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(primitive->m_d.m_visible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(primitive->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      char textBufc[MAXNAMEBUFFER];
      sprintf_s(textBufc, sizeof(textBufc), "%.03f", primitive->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBufc);
      ListView_SetItemText(m_hElementList, idx, 11, (LPSTR)(primitive->m_d.m_staticRendering ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(primitive->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)"N/A");
      break;
   }
   case eItemHitTarget:
   {
      const HitTarget *const hitTarget = (HitTarget*)piedit;
      if (IsValidString(hitTarget->m_d.m_szImage))
         textBuf = hitTarget->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Target");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(hitTarget->m_d.m_szMaterial))
         textBuf = hitTarget->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(hitTarget->m_d.m_szPhysicsMaterial))
         textBuf = hitTarget->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 6, (LPSTR)"N/A"); //!!
      ListView_SetItemText(m_hElementList, idx, 7, (LPSTR)(hitTarget->m_d.m_collidable ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 8, (LPSTR)(hitTarget->m_d.m_visible ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 9, (LPSTR)(hitTarget->m_d.m_tdr.m_TimerEnabled ? usedStringYes : usedStringNo));
      char textBufc[MAXNAMEBUFFER];
      sprintf_s(textBufc, sizeof(textBufc), "%.03f", hitTarget->m_d.m_depthBias);
      ListView_SetItemText(m_hElementList, idx, 10, textBufc);
      ListView_SetItemText(m_hElementList, idx, 11, /*hitTarget->StaticRendering() ? usedStringYes : usedStringNo*/ (LPSTR)"N/A");
      ListView_SetItemText(m_hElementList, idx, 12, (LPSTR)(hitTarget->m_d.m_reflectionEnabled ? usedStringYes : usedStringNo));
      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)"N/A");
      break;
   }
   case eItemDecal:
   {
      const Decal *const decal = (Decal*)piedit;
      if (IsValidString(decal->m_d.m_szImage))
         textBuf = decal->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"Decal");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(decal->m_d.m_szMaterial))
         textBuf = decal->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      textBuf.clear();
      if (IsValidString(decal->m_d.m_szPhysicsMaterial))
         textBuf = decal->m_d.m_szPhysicsMaterial;
      ListView_SetItemText(m_hElementList, idx, 5, (LPSTR)textBuf.c_str());

      ListView_SetItemText(m_hElementList, idx, 13, (LPSTR)decal->m_d.m_szSurface.c_str());
      break;
   }
   case eItemLightSeq:
   {
       //const LightSeq* const lightSeq = (LightSeq*)piedit;
       ListView_SetItemText(m_hElementList, idx, 1, (LPSTR)"LightSeq");
       ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)"");
       ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)"");

       break;
   }
   default:
      assert(!"AddSearchItemToList unhandled case");
      break;
   }
}

void SearchSelectDialog::LoadPosition()
{
   const int x = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SearchSelectPosX"s, 0);
   const int y = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SearchSelectPosY"s, 0);
   const int w = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SearchSelectWidth"s, 650);
   const int h = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SearchSelectHeight"s, 400);
   POINT p { x, y };
   if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
      SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SearchSelectDialog::SavePosition()
{
   const CRect rect = GetWindowRect();
   g_pvp->m_settings.SaveValue(Settings::Editor, "SearchSelectPosX"s, (int)rect.left);
   g_pvp->m_settings.SaveValue(Settings::Editor, "SearchSelectPosY"s, (int)rect.top);
   const int w = rect.right - rect.left;
   g_pvp->m_settings.SaveValue(Settings::Editor, "SearchSelectWidth"s, w);
   const int h = rect.bottom - rect.top;
   g_pvp->m_settings.SaveValue(Settings::Editor, "SearchSelectHeight"s, h);
}
