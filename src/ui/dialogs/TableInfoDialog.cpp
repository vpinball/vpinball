// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
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
   CCO(PinTable) * const pt = g_pvp->GetActiveTable();

/*
   HWND hwndParent = GetParent().GetHwnd();
   CRect rcDlg;
   RECT rcMain;
   ::GetWindowRect(hwndParent, &rcMain);
   rcDlg = GetWindowRect();

   SetWindowPos( nullptr,
      (rcMain.right + rcMain.left) / 2 - (rcDlg.right - rcDlg.left) / 2,
      (rcMain.bottom + rcMain.top) / 2 - (rcDlg.bottom - rcDlg.top) / 2,
      0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/ * | SWP_NOMOVE* /);
*/
   AttachItem(IDC_TABLENAME, m_tableNameEdit);
   AttachItem(IDC_TABLEAUTHOR, m_authorEdit);
   AttachItem(IDC_VERSION, m_versionEdit);
   AttachItem(IDC_RELEASE, m_releaseEdit);
   AttachItem(IDC_EMAIL, m_emailEdit);
   AttachItem(IDC_WEBSITE, m_websiteEdit);
   AttachItem(IDC_BLURB, m_blurbEdit);
   AttachItem(IDC_DESCRIPTION, m_descriptionEdit);
   AttachItem(IDC_RULES, m_rulesEdits);
   AttachItem(IDC_DATE_SAVED, m_dateSavedEdit);
   AttachItem(IDC_CUSTOMNAME, m_customNameEdit);
   AttachItem(IDC_CUSTOMVALUE, m_customValueEdit);
   AttachItem(IDC_CUSTOMLIST, m_customListView);
   AttachItem(IDC_SCREENSHOT, m_screenshotCombo);

   m_blurbEdit.LimitText(100);
   m_tableNameEdit.SetWindowText(pt->m_szTableName.c_str());
   m_authorEdit.SetWindowText(pt->m_szAuthor.c_str());
   m_versionEdit.SetWindowText(pt->m_szVersion.c_str());
   m_releaseEdit.SetWindowText(pt->m_szReleaseDate.c_str());
   m_emailEdit.SetWindowText(pt->m_szAuthorEMail.c_str());
   m_websiteEdit.SetWindowText(pt->m_szWebSite.c_str());
   m_blurbEdit.SetWindowText(pt->m_szBlurb.c_str());
   m_descriptionEdit.SetWindowText(pt->m_szDescription.c_str());
   m_rulesEdits.SetWindowText(pt->m_szRules.c_str());

   char buffer[256];
   sprintf_s(buffer, sizeof(buffer), "%s Revision %u", !pt->m_szDateSaved.empty() ? pt->m_szDateSaved.c_str() : "N.A.", pt->m_numTimesSaved);
   m_dateSavedEdit.SetWindowText(buffer);

   // Init list of images
   const LocalString ls(IDS_NONE);
   m_screenshotCombo.AddString(ls.m_szbuffer);

   for (size_t i = 0; i < pt->m_vimage.size(); ++i)
   {
      const Texture * const pin = pt->m_vimage[i];
      if (pin->m_ppb)
         m_screenshotCombo.AddString(pin->m_szName.c_str());
   }

   m_screenshotCombo.SelectString(0, pt->m_szScreenShot.c_str());

   m_customListView.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

   // Set up custom info list
   {
      LVCOLUMN lvcol = {};
      lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
      const LocalString ls3(IDS_NAME);
      lvcol.pszText = (LPSTR)ls3.m_szbuffer; // = "Name";
      lvcol.cx = 90;
      m_customListView.InsertColumn(0,lvcol);

      const LocalString ls2(IDS_VALUE);
      lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "Value";
      lvcol.cx = 100;
      m_customListView.InsertColumn(1, lvcol);

      pt->ListCustomInfo(m_customListView.GetHwnd());
   }

   m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 650, 500));
   m_resizer.AddChild(GetDlgItem(IDC_STATIC2).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC3).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC4).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC5).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC6).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC7).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC8).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC9).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC10).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC11).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC12).GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC13).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC14).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(m_tableNameEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_authorEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_versionEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_releaseEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_emailEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_websiteEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_blurbEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_descriptionEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(m_rulesEdits.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(m_dateSavedEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_customNameEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_customValueEdit.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_customListView.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(m_screenshotCombo.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_ADD).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_DELETE).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_SENDMAIL).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDC_GOWEBSITE).GetHwnd(), CResizer::center, 0);
   m_resizer.AddChild(GetDlgItem(IDOK).GetHwnd(), CResizer::bottomright, 0);
   m_resizer.AddChild(GetDlgItem(IDCANCEL).GetHwnd(), CResizer::bottomright, 0);

   return TRUE;
}

INT_PTR TableInfoDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   switch (uMsg)
   {
      case WM_NOTIFY:
      {
         const LPNMHDR pnmhdr = (LPNMHDR)lParam;
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

                     const CString name = m_customListView.GetItemText(sel, 0, MAXSTRING);
                     m_customNameEdit.SetWindowText(name.c_str());

                     const CString value = m_customListView.GetItemText(sel, 1, MAXSTRING);
                     m_customValueEdit.SetWindowText(value.c_str());
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

void TableInfoDialog::VPGetDialogItemText(const CEdit &edit, string &psztext)
{
   psztext = edit.GetWindowText();
}


BOOL TableInfoDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_ADD:
      {
         CCO(PinTable) * const pt = g_pvp->GetActiveTable();
         string szCustomName;
         VPGetDialogItemText(m_customNameEdit, szCustomName);
         if (!szCustomName.empty())
         {
            LVFINDINFO lvfi;
            lvfi.flags = LVFI_STRING;
            lvfi.psz = szCustomName.c_str();
            const int found = m_customListView.FindItem(lvfi, -1);

            if (found != -1)
               m_customListView.DeleteItem(found);

            string szCustomValue;
            VPGetDialogItemText(m_customValueEdit, szCustomValue);
            pt->AddListItem(m_customListView.GetHwnd(), szCustomName, szCustomValue, 0);
         }
         break;
      }
      case IDC_DELETE:
      {
         const int sel = m_customListView.GetNextItem(-1, LVNI_SELECTED);
         m_customListView.DeleteItem(sel);
         break;
      }
      case IDC_GOWEBSITE:
      {
         OpenURL(m_websiteEdit.GetWindowText().c_str());
         break;
      }
      case IDC_SENDMAIL:
      {
         const string url = "mailto:"s + m_emailEdit.GetWindowText().c_str() + "?subject=" + m_tableNameEdit.GetWindowText().c_str();
         OpenURL(url);
         break;
      }
      default: 
         return FALSE;
   }
   return TRUE;
}

void TableInfoDialog::OnOK()
{
   CCO(PinTable) * const pt = g_pvp->GetActiveTable();

   pt->m_szTableName = m_tableNameEdit.GetWindowText();
   pt->m_szAuthor = m_authorEdit.GetWindowText();
   pt->m_szVersion = m_versionEdit.GetWindowText();
   pt->m_szReleaseDate = m_releaseEdit.GetWindowText();
   pt->m_szAuthorEMail = m_emailEdit.GetWindowText();
   pt->m_szWebSite = m_websiteEdit.GetWindowText();
   pt->m_szBlurb = m_blurbEdit.GetWindowText();
   pt->m_szDescription = m_descriptionEdit.GetWindowText();
   pt->m_szRules = m_rulesEdits.GetWindowTextA();

   const CString sshot = GetDlgItem(IDC_SCREENSHOT).GetWindowText();

   const LocalString ls(IDS_NONE);
   if (!lstrcmp(sshot.c_str(), ls.m_szbuffer))
      pt->m_szScreenShot.clear();
   else
      pt->m_szScreenShot = sshot.c_str();

   // Clear old custom values, read back new ones
   pt->m_vCustomInfoTag.clear();
   pt->m_vCustomInfoContent.clear();

   const int customcount = m_customListView.GetItemCount();
   for (int i = 0; i < customcount; i++)
   {
      const CString name = m_customListView.GetItemText(i, 0, MAXSTRING);
      pt->m_vCustomInfoTag.push_back(name.c_str());

      const CString value = m_customListView.GetItemText(i, 1, MAXSTRING);
      pt->m_vCustomInfoContent.push_back(value.c_str());
   }

   pt->SetNonUndoableDirty(eSaveDirty);
   CDialog::OnOK();
}

void TableInfoDialog::OnCancel()
{
   CDialog::OnCancel();
}
