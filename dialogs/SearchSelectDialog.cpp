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
int SearchSelectDialog::m_columnSortOrder;
int SearchSelectDialog::m_lastSortColumn;

SearchSelectDialog::SearchSelectDialog() : CDialog(IDD_SEARCH_SELECT_ELEMENT)
{
    m_hElementList = NULL;

    m_switchSortOrder = false;
    m_columnSortOrder = 1;
    m_lastSortColumn = 0;
    m_curTable = NULL;
}

void SearchSelectDialog::Update()
{
   ListView_SetExtendedListViewStyle(m_hElementList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

   LVCOLUMN lvc;
   memset(&lvc, 0, sizeof(LVCOLUMN));
   lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
   lvc.cx = 200;
   lvc.pszText = TEXT("Name");
   int index = -1;
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

   if (m_hElementList != NULL)
      ListView_DeleteAllItems(m_hElementList);

   int idx = 0;
   for (int i = 0; i < m_curTable->m_vcollection.Size(); i++)
   {
      CComObject<Collection> *const pcol = m_curTable->m_vcollection.ElementAt(i);
      char szT[sizeof(pcol->m_wzName)/sizeof(pcol->m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, pcol->m_wzName, -1, szT, sizeof(szT), NULL, NULL);
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
              if (pcol->m_visel.Size()>0)
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
   {
      if (m_columnSortOrder == 1)
         m_columnSortOrder = 0;
      else
         m_columnSortOrder = 1;
   }

   SortData.hwndList = m_hElementList;
   SortData.subItemIndex = (m_switchSortOrder==true) ? columnNumber: m_lastSortColumn;
   m_lastSortColumn = columnNumber;
   SortData.sortUpDown = m_columnSortOrder;
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
            LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
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
         RECT buttonRc;

         const CRect rc = GetWindowRect();
         const int windowHeight = rc.bottom - rc.top;
         const int windowWidth = rc.right - rc.left;

         const HWND hOkButton = GetDlgItem(IDOK).GetHwnd();
         const HWND hCancelButton = GetDlgItem(IDCANCEL).GetHwnd();
         const int buttonY = (windowHeight - 85) + 5;
         ::GetClientRect(hOkButton, &buttonRc);
         const int buttonWidth = buttonRc.right - buttonRc.left;
         const int buttonHeight = buttonRc.bottom - buttonRc.top;
         
         ::SetWindowPos(m_hElementList, NULL, 6, 5, windowWidth - 28, windowHeight - 90, 0);
         ::SetWindowPos(hOkButton, NULL, 6, buttonY, buttonWidth, buttonHeight, 0);
         ::SetWindowPos(hCancelButton, NULL, 6 + buttonWidth + 50, buttonY, buttonWidth, buttonHeight, 0);
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
   if (name.empty() || name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ._-") != std::string::npos)
      return false;

   return true;
}

void SearchSelectDialog::AddSearchItemToList(IEditable * const piedit, int idx)
{
   std::string textBuf;
   std::string firstImage;
   std::string secondImage;
   char layerBuf[16];

   const IScriptable * const piscript = piedit->GetScriptable();
   if (piscript)
       strncpy_s(layerBuf, piscript->GetISelect()->m_layerName.c_str(), sizeof(layerBuf)-1);
   else
       memset(layerBuf, 0, sizeof(layerBuf));

   ListView_SetItemText(m_hElementList, idx, 2, layerBuf);
   switch(piedit->GetItemType())
   {
   case eItemSurface:
   {
      const Surface *const sur = (Surface*)piedit;
      firstImage.clear();
      if (IsValidString(sur->m_d.m_szImage))
         firstImage = sur->m_d.m_szImage;
      textBuf = firstImage;
      secondImage.clear();
      if (IsValidString(sur->m_d.m_szSideImage))
         secondImage = sur->m_d.m_szSideImage;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf.append("--");
         textBuf.append(secondImage);
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
            textBuf.append("--");
         textBuf.append(secondImage);
      }
      if (!thirdImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty())
            textBuf.append("--");
         textBuf.append(thirdImage);
      }
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      break;
   }
   case eItemRamp:
   {
      const Ramp *const ramp = (Ramp*)piedit;
      if (IsValidString(ramp->m_d.m_szImage))
         textBuf = ramp->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Ramp");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      if (IsValidString(ramp->m_d.m_szMaterial))
         textBuf = ramp->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      break;
   }
   case eItemFlasher:
   {
      const Flasher *const flasher = (Flasher*)piedit;
      firstImage.clear();
      if (IsValidString(flasher->m_d.m_szImageA))
         firstImage = flasher->m_d.m_szImageA;
      secondImage.clear();
      if (IsValidString(flasher->m_d.m_szImageB))
         secondImage = flasher->m_d.m_szImageB;
      textBuf = firstImage;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf.append("--");
         textBuf.append(secondImage);
      }
      ListView_SetItemText(m_hElementList, idx, 1, "Flasher");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());
      ListView_SetItemText(m_hElementList, idx, 4, "");

      break;
   }
   case eItemRubber:
   {
      const Rubber *const rubber = (Rubber*)piedit;
      if (IsValidString(rubber->m_d.m_szImage))
         textBuf = rubber->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Rubber");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      if (IsValidString(rubber->m_d.m_szMaterial))
         textBuf = rubber->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      break;
   }
   case eItemSpinner:
   {
      const Spinner *const spinner = (Spinner*)piedit;
      if (IsValidString(spinner->m_d.m_szImage))
         textBuf = spinner->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Spinner");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      if (IsValidString(spinner->m_d.m_szMaterial))
         textBuf = spinner->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

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

      break;
   }
   case eItemBumper:
   {
      const Bumper *const bumper = (Bumper*)piedit;
      firstImage.clear();
      if (IsValidString(bumper->m_d.m_szBaseMaterial))
         firstImage = bumper->m_d.m_szBaseMaterial;
      textBuf = firstImage;
      secondImage.clear();
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
            textBuf.append("--");
         textBuf.append(secondImage);
      }
      if (!thirdImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty())
            textBuf.append("--");
         textBuf.append(thirdImage);
      }
      if (!fourthImage.empty())
      {
         if (!firstImage.empty() || !secondImage.empty() || !thirdImage.empty())
            textBuf.append("--");
         textBuf.append(fourthImage);
      }
      ListView_SetItemText(m_hElementList, idx, 1, "Bumper");
      ListView_SetItemText(m_hElementList, idx, 3, "");
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      break;
   }
   case eItemFlipper:
   {
      const Flipper* const flipper = (Flipper*)piedit;
      if (IsValidString(flipper->m_d.m_szImage))
         textBuf = flipper->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Flipper");
      ListView_SetItemText(m_hElementList, idx, 3, "");

      firstImage.clear();
      if (IsValidString(flipper->m_d.m_szMaterial))
         firstImage = flipper->m_d.m_szMaterial;
      textBuf = firstImage;
      secondImage.clear();
      if (IsValidString(flipper->m_d.m_szRubberMaterial))
         secondImage = flipper->m_d.m_szRubberMaterial;
      if (!secondImage.empty())
      {
         if (!firstImage.empty())
            textBuf.append("--");
         textBuf.append(secondImage);
      }
      ListView_SetItemText(m_hElementList, idx, 4, "");

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

      if (IsValidString(plunger->m_d.m_szMaterial))
         textBuf = plunger->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

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

      if (IsValidString(primitive->m_d.m_szMaterial))
         textBuf = primitive->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      break;
   }
   case eItemHitTarget:
   {
      const HitTarget *const hitTarget = (HitTarget*)piedit;
      if (IsValidString(hitTarget->m_d.m_szImage))
         textBuf = hitTarget->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Target");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      if (IsValidString(hitTarget->m_d.m_szMaterial))
         textBuf = hitTarget->m_d.m_szMaterial;
      ListView_SetItemText(m_hElementList, idx, 4, (LPSTR)textBuf.c_str());

      break;
   }
   case eItemDecal:
   {
      const Decal *const decal = (Decal*)piedit;
      if (IsValidString(decal->m_d.m_szImage))
         textBuf = decal->m_d.m_szImage;
      ListView_SetItemText(m_hElementList, idx, 1, "Decal");
      ListView_SetItemText(m_hElementList, idx, 3, (LPSTR)textBuf.c_str());

      if (IsValidString(decal->m_d.m_szMaterial))
         textBuf = decal->m_d.m_szMaterial;
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

   SetWindowPos(NULL, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
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
