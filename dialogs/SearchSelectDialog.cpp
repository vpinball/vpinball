#include "StdAfx.h"
#include "resource.h"
#include "SearchSelectDialog.h"

typedef struct _tagSORTDATA
{
   HWND hwndList;
   int subItemIndex;
   int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int columnSortOrder[4];
extern int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);

SearchSelectDialog::SearchSelectDialog() : CDialog(IDD_SEARCH_SELECT_ELEMENT)
{
    hElementList = NULL;
}

void SearchSelectDialog::Update()
{
   LVCOLUMN lvc;
   LVITEM lv;

   ListView_SetExtendedListViewStyle(hElementList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   memset(&lvc, 0, sizeof(LVCOLUMN));
   lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
   lvc.cx = 200;
   lvc.pszText = TEXT("Name");
   int index = -1;
   index = ListView_InsertColumn(hElementList, 0, &lvc);
   lvc.cx = 70;
   lvc.pszText = TEXT("Type");
   index = ListView_InsertColumn(hElementList, 1, &lvc);
   lvc.cx = 50;
   lvc.pszText = TEXT("Layer");
   index = ListView_InsertColumn(hElementList, 2, &lvc);
   lvc.cx = 200;
   lvc.pszText = TEXT("Image");
   index = ListView_InsertColumn(hElementList, 3, &lvc);
   lvc.cx = 250;
   lvc.pszText = TEXT("Material");
   index = ListView_InsertColumn(hElementList, 4, &lvc);

   if (hElementList != NULL)
      ListView_DeleteAllItems(hElementList);

   lv.mask = LVIF_TEXT | LVIF_PARAM;
   int idx = 0;
   for (int i = 0; i < curTable->m_vedit.Size(); i++)
   {
      IEditable * const piedit = curTable->m_vedit.ElementAt(i);
      IScriptable * const piscript = piedit->GetScriptable();
      if (piscript)
      {
         lv.iItem = idx;
         lv.iSubItem = 0;
         lv.lParam = (LPARAM)piscript;
         lv.pszText = curTable->GetElementName(piedit);
         ListView_InsertItem(hElementList, &lv);
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
   hElementList = GetDlgItem(IDC_ELEMENT_LIST).GetHwnd();
   curTable = (CCO(PinTable) *)g_pvp->GetActiveTable();

   m_resizer.Initialize(*this, CRect(0, 0, 650, 400));
   m_resizer.AddChild(GetDlgItem(IDC_ELEMENT_LIST).GetHwnd(), topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(GetDlgItem(IDOK).GetHwnd(), bottomleft, 0);
   m_resizer.AddChild(GetDlgItem(IDCANCEL).GetHwnd(), bottomleft, 0);
   Update();
   LoadPosition();
   return TRUE;

}

void SearchSelectDialog::SelectElement()
{
    const size_t count = ListView_GetSelectedCount(hElementList);

    curTable->ClearMultiSel();
    int iItem = -1;
    LVITEM lv;
    for (size_t i = 0; i < count; i++)
    {
        iItem = ListView_GetNextItem(hElementList, iItem, LVNI_SELECTED);
        lv.iItem = iItem;
        lv.mask = LVIF_PARAM;
        if (ListView_GetItem(hElementList, &lv) == TRUE)
        {
            IScriptable *pscript = (IScriptable*)lv.lParam;
            ISelect *const pisel = pscript->GetISelect();
            if (pisel)
                curTable->AddMultiSel(pisel, true);
        }
    }
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
               if (columnSortOrder[columnNumber] == 1)
                  columnSortOrder[columnNumber] = 0;
               else
                  columnSortOrder[columnNumber] = 1;
               SortData.hwndList = hElementList;
               SortData.subItemIndex = columnNumber;
               SortData.sortUpDown = columnSortOrder[columnNumber];
               ListView_SortItems(SortData.hwndList, MyCompProc, &SortData);
            }
         }
         break;
      }
      case WM_SIZE:
      {
         CRect rc;
         RECT buttonRc;

         rc = GetWindowRect();
         int windowHeight = rc.bottom - rc.top;
         int windowWidth = rc.right - rc.left;

         HWND hOkButton = GetDlgItem(IDOK).GetHwnd();
         HWND hCancelButton = GetDlgItem(IDCANCEL).GetHwnd();
         int buttonY = (windowHeight - 80) + 5;
         ::GetClientRect(hOkButton, &buttonRc);
         int buttonWidth = buttonRc.right - buttonRc.left;
         int buttonHeight = buttonRc.bottom - buttonRc.top;
         
         ::SetWindowPos(hElementList, NULL, 6, 5, windowWidth - 28, windowHeight - 90, 0);
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

bool SearchSelectDialog::IsValidString(const char *name)
{
   if (name[0] == 0)
      return false;
   std::string str(name);
   if (str.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ._") != std::string::npos)
      return false;

   return true;
}


void SearchSelectDialog::AddSearchItemToList(IEditable * const piedit, int idx)
{
   char textBuf[518];
   char firstImage[256];
   char secondImage[256];
   char layerBuf[16];
   IScriptable * const piscript = piedit->GetScriptable();
   memset(layerBuf, 0, 16);
   if (piscript)
   {
      sprintf_s(layerBuf, "%i", piscript->GetISelect()->layerIndex + 1);
   }
   ListView_SetItemText(hElementList, idx, 2, layerBuf);
   if (piedit->GetItemType() == eItemSurface)
   {
      Surface *sur = (Surface*)piedit;
      firstImage[0] = 0;
      secondImage[0] = 0;
      if (IsValidString(sur->m_d.m_szImage))
         sprintf_s(firstImage, "%s", sur->m_d.m_szImage);
      if (IsValidString(sur->m_d.m_szSideImage))
         sprintf_s(secondImage, "%s", sur->m_d.m_szSideImage);
      textBuf[0] = 0;
      if (firstImage[0] != 0)
         strncat_s(textBuf, firstImage, 511);
      if (secondImage[0] != 0)
      {
         if (firstImage[0] != 0)
            strncat_s(textBuf, " -- ", 511);
         strncat_s(textBuf, secondImage, 511);
      }
      ListView_SetItemText(hElementList, idx, 1, "Wall");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      firstImage[0] = 0;
      secondImage[0] = 0;
      textBuf[0] = 0;
      if (IsValidString(sur->m_d.m_szTopMaterial))
         sprintf_s(firstImage, "%s", sur->m_d.m_szTopMaterial);
      if (IsValidString(sur->m_d.m_szSideMaterial))
         sprintf_s(secondImage, "%s", sur->m_d.m_szSideMaterial);
      textBuf[0] = 0;
      if (firstImage[0] != 0)
         strncat_s(textBuf, firstImage, 511);
      if (secondImage[0] != 0)
      {
         if (firstImage[0] != 0)
            strncat_s(textBuf, " -- ", 511);
         strncat_s(textBuf, secondImage, 511);
      }
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemRamp)
   {
      Ramp *ramp = (Ramp*)piedit;
      if (IsValidString(ramp->m_d.m_szImage))
         sprintf_s(textBuf, "%s", ramp->m_d.m_szImage);

      ListView_SetItemText(hElementList, idx, 1, "Ramp");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      if (IsValidString(ramp->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", ramp->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemFlasher)
   {
      Flasher *flasher = (Flasher*)piedit;
      firstImage[0] = 0;
      secondImage[0] = 0;
      if (IsValidString(flasher->m_d.m_szImageA))
         sprintf_s(firstImage, "%s", flasher->m_d.m_szImageA);
      if (IsValidString(flasher->m_d.m_szImageB))
         sprintf_s(secondImage, "%s", flasher->m_d.m_szImageB);

      textBuf[0] = 0;
      if (firstImage[0] != 0)
         strncat_s(textBuf, firstImage, 511);
      if (secondImage[0] != 0)
      {
         if (firstImage[0] != 0)
            strncat_s(textBuf, " -- ", 511);
         strncat_s(textBuf, secondImage, 511);
      }
      ListView_SetItemText(hElementList, idx, 1, "Flasher");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
   }
   else if (piedit->GetItemType() == eItemRubber)
   {
      Rubber *rubber = (Rubber*)piedit;
      if (IsValidString(rubber->m_d.m_szImage))
         sprintf_s(textBuf, "%s", rubber->m_d.m_szImage);

      ListView_SetItemText(hElementList, idx, 1, "Rubber");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      if (IsValidString(rubber->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", rubber->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemSpinner)
   {
      Spinner *spinner = (Spinner*)piedit;
      if (IsValidString(spinner->m_d.m_szImage))
         sprintf_s(textBuf, "%s", spinner->m_d.m_szImage);

      ListView_SetItemText(hElementList, idx, 1, "Spinner");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      if (IsValidString(spinner->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", spinner->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemKicker)
   {
      Kicker *kicker = (Kicker*)piedit;
      ListView_SetItemText(hElementList, idx, 1, "Kicker");
      ListView_SetItemText(hElementList, idx, 3, "");
      if (IsValidString(kicker->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", kicker->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemLight)
   {
      Light *light = (Light*)piedit;
      if (IsValidString(light->m_d.m_szOffImage))
         sprintf_s(textBuf, "%s", light->m_d.m_szOffImage);

      ListView_SetItemText(hElementList, idx, 1, "Light");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      ListView_SetItemText(hElementList, idx, 4, "");
   }
   else if (piedit->GetItemType() == eItemBumper)
   {
      Bumper *bumper = (Bumper*)piedit;
      char thirdImage[256];
      firstImage[0] = 0;
      secondImage[0] = 0;
      thirdImage[0] = 0;
      textBuf[0] = 0;
      if (IsValidString(bumper->m_d.m_szBaseMaterial))
         sprintf_s(firstImage, "%s", bumper->m_d.m_szBaseMaterial);
      if (firstImage[0] != 0)
         strcat_s(textBuf, firstImage);
      if (IsValidString(bumper->m_d.m_szCapMaterial))
         sprintf_s(secondImage, "%s", bumper->m_d.m_szCapMaterial);
      if (IsValidString(bumper->m_d.m_szSkirtMaterial))
         sprintf_s(thirdImage, "%s", bumper->m_d.m_szSkirtMaterial);
      if (secondImage[0] != 0)
      {
         if (firstImage[0] != 0)
            strcat_s(textBuf, " -- ");
         strcat_s(textBuf, secondImage);
      }
      if (thirdImage[0] != 0)
      {
         if (firstImage[0] != 0 || secondImage[0] != 0)
            strncat_s(textBuf, " -- ", 511);
         strncat_s(textBuf, thirdImage, 511);
      }
      ListView_SetItemText(hElementList, idx, 1, "Bumper");
      ListView_SetItemText(hElementList, idx, 3, "");
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemFlipper)
   {
      ListView_SetItemText(hElementList, idx, 1, "Flipper");
      ListView_SetItemText(hElementList, idx, 3, "");
   }
   else if (piedit->GetItemType() == eItemGate)
   {
      Gate *gate = (Gate*)piedit;
      ListView_SetItemText(hElementList, idx, 1, "Gate");
      ListView_SetItemText(hElementList, idx, 3, "");
      if (IsValidString(gate->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", gate->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemTrigger)
   {
      Trigger *trigger = (Trigger*)piedit;
      ListView_SetItemText(hElementList, idx, 1, "Trigger");
      ListView_SetItemText(hElementList, idx, 3, "");
      if (IsValidString(trigger->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", trigger->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemTimer)
   {
      ListView_SetItemText(hElementList, idx, 1, "Timer");
      ListView_SetItemText(hElementList, idx, 3, "");
   }
   else if (piedit->GetItemType() == eItemTextbox)
   {
      ListView_SetItemText(hElementList, idx, 1, "Textbox");
      ListView_SetItemText(hElementList, idx, 3, "");
   }
   else if (piedit->GetItemType() == eItemPlunger)
   {
      Plunger *plunger = (Plunger*)piedit;
      if (IsValidString(plunger->m_d.m_szImage))
         sprintf_s(textBuf, "%s", plunger->m_d.m_szImage);
      ListView_SetItemText(hElementList, idx, 1, "Plunger");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      if (IsValidString(plunger->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", plunger->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemDispReel)
   {
      DispReel *dispReel = (DispReel*)piedit;
      if (IsValidString(dispReel->m_d.m_szImage))
         sprintf_s(textBuf, "%s", dispReel->m_d.m_szImage);
      ListView_SetItemText(hElementList, idx, 1, "EMReel");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
   }
   else if (piedit->GetItemType() == eItemPrimitive)
   {
      Primitive *primitive = (Primitive*)piedit;
      if (IsValidString(primitive->m_d.m_szImage))
         sprintf_s(textBuf, "%s", primitive->m_d.m_szImage);
      ListView_SetItemText(hElementList, idx, 1, "Primitive");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      if (IsValidString(primitive->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", primitive->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }
   else if (piedit->GetItemType() == eItemHitTarget)
   {
      HitTarget *hitTraget = (HitTarget*)piedit;
      if (IsValidString(hitTraget->m_d.m_szImage))
         sprintf_s(textBuf, "%s", hitTraget->m_d.m_szImage);
      ListView_SetItemText(hElementList, idx, 1, "Target");
      ListView_SetItemText(hElementList, idx, 3, textBuf);
      if (IsValidString(hitTraget->m_d.m_szMaterial))
         sprintf_s(textBuf, "%s", hitTraget->m_d.m_szMaterial);
      ListView_SetItemText(hElementList, idx, 4, textBuf);
   }

}
void SearchSelectDialog::LoadPosition()
{
   int x, y,w,h;

   x = GetRegIntWithDefault("Editor", "SearchSelectPosX", 0);
   y = GetRegIntWithDefault("Editor", "SearchSelectPosY", 0);
   w = GetRegIntWithDefault("Editor", "SearchSelectWidth", 650);
   h = GetRegIntWithDefault("Editor", "SearchSelectHeight", 400);

   SetWindowPos(NULL, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SearchSelectDialog::SavePosition()
{
   int w, h;
   CRect rect = GetWindowRect();
   (void)SetRegValue("Editor", "SearchSelectPosX", REG_DWORD, &rect.left, 4);
   (void)SetRegValue("Editor", "SearchSelectPosY", REG_DWORD, &rect.top, 4);
   w = rect.right - rect.left;
   (void)SetRegValue("Editor", "SearchSelectWidth", REG_DWORD, &w, 4);
   h = rect.bottom - rect.top;
   (void)SetRegValue("Editor", "SearchSelectHeight", REG_DWORD, &h, 4);
}
