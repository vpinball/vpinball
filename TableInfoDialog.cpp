#include "StdAfx.h"
#include "resource.h"
#include "TableInfoDialog.h"


TableInfoDialog::TableInfoDialog() : CDialog(IDD_TABLEINFO)
{
}

void TableInfoDialog::OnClose()
{
   CDialog::OnClose();
}

BOOL TableInfoDialog::OnInitDialog()
{
   CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();

   HWND hwndParent = GetParent().GetHwnd();
/*
   CRect rcDlg;
   RECT rcMain;
   ::GetWindowRect(hwndParent, &rcMain);
   rcDlg = GetWindowRect();

   SetWindowPos( NULL,
      (rcMain.right + rcMain.left) / 2 - (rcDlg.right - rcDlg.left) / 2,
      (rcMain.bottom + rcMain.top) / 2 - (rcDlg.bottom - rcDlg.top) / 2,
      0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/ * | SWP_NOMOVE* /);
*/

   SendDlgItemMessage(IDC_BLURB, EM_LIMITTEXT, 100, 0);

   SetDlgItemText(IDC_TABLENAME, pt->m_szTableName);
   SetDlgItemText(IDC_TABLEAUTHOR, pt->m_szAuthor);
   SetDlgItemText(IDC_VERSION, pt->m_szVersion);
   SetDlgItemText(IDC_RELEASE, pt->m_szReleaseDate);
   SetDlgItemText(IDC_EMAIL, pt->m_szAuthorEMail);
   SetDlgItemText(IDC_WEBSITE, pt->m_szWebSite);
   SetDlgItemText(IDC_BLURB, pt->m_szBlurb);
   SetDlgItemText(IDC_DESCRIPTION, pt->m_szDescription);
   SetDlgItemText(IDC_RULES, pt->m_szRules);

   // Init list of images

   HWND hwndList = GetDlgItem(IDC_SCREENSHOT).GetHwnd();

   LocalString ls(IDS_NONE);
   ::SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)ls.m_szbuffer);

   for (unsigned i = 0; i < pt->m_vimage.size(); ++i)
   {
      Texture * const pin = pt->m_vimage[i];
      if (pin->m_ppb)
         ::SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)pin->m_szName);
   }

   ::SendMessage(hwndList, CB_SELECTSTRING, ~0u, (LPARAM)pt->m_szScreenShot);

   ListView_SetExtendedListViewStyle(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

   // Set up custom info list
   {
      LVCOLUMN lvcol;
      lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
      LocalString ls3(IDS_NAME);
      lvcol.pszText = ls3.m_szbuffer;// = "Name";
      lvcol.cx = 90;
      ListView_InsertColumn(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), 0, &lvcol);

      LocalString ls2(IDS_VALUE);
      lvcol.pszText = ls2.m_szbuffer; // = "Value";
      lvcol.cx = 100;
      ListView_InsertColumn(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), 1, &lvcol);

      pt->ListCustomInfo(GetDlgItem(IDC_CUSTOMLIST).GetHwnd());
   }

   return TRUE;
}

INT_PTR TableInfoDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();

   switch (uMsg)
   {
      case WM_NOTIFY:
      {
         LPNMHDR pnmhdr = (LPNMHDR)lParam;
         switch (pnmhdr->code)
         {
            case LVN_ITEMCHANGING:
            {
               NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
               if ((plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED))
               {
                  if (plistview->uNewState & LVIS_SELECTED)
                  {
                     const int sel = plistview->iItem;
                     char szT[1024];

                     ListView_GetItemText(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), sel, 0, szT, 1024);
                     ::SetWindowText(GetDlgItem(IDC_CUSTOMNAME).GetHwnd(), szT);

                     ListView_GetItemText(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), sel, 1, szT, 1024);
                     ::SetWindowText(GetDlgItem(IDC_CUSTOMVALUE).GetHwnd(), szT);
                  }
               }
               break;
            }
         }
         break;
      }
   }
   return DialogProcDefault(uMsg, wParam, lParam);

}

void TableInfoDialog::VPGetDialogItemText(int nIDDlgItem, char **psztext)
{
   HWND hwndItem = GetDlgItem(nIDDlgItem);

   const int length = ::GetWindowTextLength(hwndItem);
   *psztext = new char[length + 1];

   ::GetWindowText(hwndItem, *psztext, length + 1);
}


BOOL TableInfoDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_ADD:
      {
         CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
         char *szCustomName;
         VPGetDialogItemText(IDC_CUSTOMNAME, &szCustomName);
         if (szCustomName[0] != '\0')
         {
            LVFINDINFO lvfi;
            lvfi.flags = LVFI_STRING;
            lvfi.psz = szCustomName;

            const int found = ListView_FindItem(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), -1, &lvfi);

            if (found != -1)
               ListView_DeleteItem(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), found);

            char *szCustomValue;
            VPGetDialogItemText(IDC_CUSTOMVALUE, &szCustomValue);
            pt->AddListItem(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), szCustomName, szCustomValue, NULL);
            delete[] szCustomValue;
         }
         delete[] szCustomName;
         break;
      }
      case IDC_DELETE:
      {
         const int sel = ListView_GetNextItem(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), -1, LVNI_SELECTED);
         ListView_DeleteItem(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), sel);
         break;
      }
      case IDC_GOWEBSITE:
      {
         char *szT;
         VPGetDialogItemText(IDC_WEBSITE, &szT);
         OpenURL(szT);
         delete[] szT;
         break;
      }
      case IDC_SENDMAIL:
      {
         char *szEMail;
         char *szTableName;
         char szMail[] = "mailto:";
         char szHeaders[] = "?subject=";
         VPGetDialogItemText(IDC_EMAIL, &szEMail);
         VPGetDialogItemText(IDC_TABLENAME, &szTableName);
         char * const szLong = new char[lstrlen(szMail) + lstrlen(szEMail) + lstrlen(szHeaders) + lstrlen(szTableName) + 1];
         lstrcpy(szLong, szMail);
         lstrcat(szLong, szEMail);
         lstrcat(szLong, szHeaders);
         lstrcat(szLong, szTableName);
         OpenURL(szLong);
         delete[] szLong;
         delete[] szEMail;
         delete[] szTableName;
         break;
      }
      default: 
         return FALSE;
   }
   return TRUE;
}

void TableInfoDialog::OnOK()
{
   CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
   HWND hwndDlg = GetHwnd();
   SAFE_VECTOR_DELETE(pt->m_szTableName);
   SAFE_VECTOR_DELETE(pt->m_szAuthor);
   SAFE_VECTOR_DELETE(pt->m_szVersion);
   SAFE_VECTOR_DELETE(pt->m_szReleaseDate);
   SAFE_VECTOR_DELETE(pt->m_szAuthorEMail);
   SAFE_VECTOR_DELETE(pt->m_szWebSite);
   SAFE_VECTOR_DELETE(pt->m_szBlurb);
   SAFE_VECTOR_DELETE(pt->m_szDescription);
   SAFE_VECTOR_DELETE(pt->m_szRules);

   VPGetDialogItemText(IDC_TABLENAME, &pt->m_szTableName);
   VPGetDialogItemText(IDC_TABLEAUTHOR, &pt->m_szAuthor);
   VPGetDialogItemText(IDC_VERSION, &pt->m_szVersion);
   VPGetDialogItemText(IDC_RELEASE, &pt->m_szReleaseDate);
   VPGetDialogItemText(IDC_EMAIL, &pt->m_szAuthorEMail);
   VPGetDialogItemText(IDC_WEBSITE, &pt->m_szWebSite);
   VPGetDialogItemText(IDC_BLURB, &pt->m_szBlurb);
   VPGetDialogItemText(IDC_DESCRIPTION, &pt->m_szDescription);
   VPGetDialogItemText(IDC_RULES, &pt->m_szRules);

   HWND hwndList = GetDlgItem(IDC_SCREENSHOT).GetHwnd();

   ::GetWindowText(hwndList, pt->m_szScreenShot, MAXTOKEN);

   LocalString ls(IDS_NONE);
   if (!lstrcmp(pt->m_szScreenShot, ls.m_szbuffer))
   {
      // <None> is selected
      pt->m_szScreenShot[0] = '\0';
   }

   // Clear old custom values, read back new ones
   for (int i = 0; i < pt->m_vCustomInfoTag.Size(); i++)
   {
      delete pt->m_vCustomInfoTag.ElementAt(i);
      delete pt->m_vCustomInfoContent.ElementAt(i);
   }
   pt->m_vCustomInfoTag.RemoveAllElements();
   pt->m_vCustomInfoContent.RemoveAllElements();

   const int customcount = ListView_GetItemCount(GetDlgItem(IDC_CUSTOMLIST).GetHwnd());
   for (int i = 0; i < customcount; i++)
   {
      char szT[1024];
      ListView_GetItemText(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), i, 0, szT, 1024);

      char * const szName = new char[lstrlen(szT) + 1];
      lstrcpy(szName, szT);
      pt->m_vCustomInfoTag.AddElement(szName);

      ListView_GetItemText(GetDlgItem(IDC_CUSTOMLIST).GetHwnd(), i, 1, szT, 1024);
      char * const szValue = new char[lstrlen(szT) + 1];
      lstrcpy(szValue, szT);
      pt->m_vCustomInfoContent.AddElement(szValue);
   }

   pt->SetNonUndoableDirty(eSaveDirty);
   CDialog::OnOK();
}

void TableInfoDialog::OnCancel()
{
   CDialog::OnCancel();
}
