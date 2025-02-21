// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "WhereUsedDialog.h"
#include "core/vpversion.h"
#include "atlconv.h"

typedef struct _tagSORTDATA
{
   HWND hwndList;
   int subItemIndex;
   int sortUpDown;
} SORTDATA;

extern SORTDATA SortData;
extern int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);
extern int CALLBACK MyCompProcMemValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);
int WhereUsedDialog::m_columnSortOrder;
bool WhereUsedDialog::m_doNotChange;


WhereUsedDialog::WhereUsedDialog()
   : CDialog(IDD_WHEREUSEDDIALOG)
{
   m_columnSortOrder = 1;
   m_doNotChange = false;
}

WhereUsedDialog::~WhereUsedDialog() { }

void WhereUsedDialog::OnDestroy() { CDialog::OnDestroy(); }

void WhereUsedDialog::OnClose()
{
   SavePosition();
   CDialog::OnClose();
}

BOOL WhereUsedDialog::OnInitDialog()
{
   m_columnSortOrder = 1;
   return TRUE;
}

INT_PTR WhereUsedDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      const HWND toolTipHwnd = CreateWindowEx(
         0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), nullptr, g_pvp->theInstance, nullptr);

              //Get active pinball table (one currently selected...you can edit more than one at a time)
      CCO(PinTable) *const pt = g_pvp->GetActiveTable();
      if (pt)
      {
         vector<WhereUsedInfo> vWhereUsed; //vector storing a list of the names of objects using this image.

         //There is a 'Where Used' button on the 'Image Manager' as well as the 'Materials Manager' window.  Since we are using the WhereUsedDialog for both we need a switch here
         switch (m_whereUsedSource)
         {
            case IMAGES:
            {
               SetWindowText("Where Used (Images)");
               pt->ShowWhereImagesUsed(vWhereUsed); //Fill a vector with a list of all images used and the table objects that reference them
            }
            break;
            case MATERIALS:
            {
               SetWindowText("Where Used (Materials)");
               pt->ShowWhereMaterialsUsed(vWhereUsed); //Fill a vector with a list of all materials used and the table objects that reference them
            }
            break;
            default:
            {
            }
            break;
         }

         AddToolTip("Edit the selected 'Used By Object' in the table properties editor", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_EDIT_OBJECT).GetHwnd());
         AddToolTip("Refresh this list (recommended after editing a control)", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_REFRESH_LIST).GetHwnd());
         AddToolTip("Clicking 'OK' will close this window", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_OK).GetHwnd());


         //Create a listview for the Where Used dialog
         const HWND hListView = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
         m_resizer.Initialize(*this, CRect(0, 0, 720, 450));
         m_resizer.AddChild(GetDlgItem(IDC_WHERE_DESCR), CResizer::topleft, RD_STRETCH_WIDTH);
         m_resizer.AddChild(hListView, CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
         m_resizer.AddChild(GetDlgItem(IDC_EDIT_OBJECT).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_REFRESH_LIST).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_OK).GetHwnd(), CResizer::topright, 0);
         //Get the listview's position
         LoadPosition();

         ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
         // Create listvew columns then fill it with the items in the vector 'vWhereUsed'
         //
         //Create Listview Column Headings
         LVCOLUMN lvcol = {};
         lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
         const LocalString ls(IDS_WHERE_USED_SRC_OBJECT);
         lvcol.pszText = (LPSTR)ls.m_szbuffer; // = "Image Name";
         lvcol.cx = 150;
         lvcol.fmt = LVCFMT_LEFT;
         ListView_InsertColumn(hListView, 0, &lvcol);

         const LocalString ls2(IDS_WHERE_USED_OBJECT_NAME);
         lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "In Use By";
         lvcol.cx = 200;
         lvcol.fmt = LVCFMT_LEFT;
         ListView_InsertColumn(hListView, 1, &lvcol);

         const LocalString ls3(IDS_WHERE_USED_PROPERTY);
         lvcol.pszText = (LPSTR)ls3.m_szbuffer; // = "Image Property";
         lvcol.cx = 200;
         lvcol.fmt = LVCFMT_LEFT;
         ListView_InsertColumn(hListView, 2, &lvcol);

         //For each loop iterator to fill the Listview rows
         int i = 0;
         for (const WhereUsedInfo &where : vWhereUsed)
         {
            //Create Listview Item
            LPSTR srcImage;
            LPSTR usedByObject;
            LPSTR usedByPropertyName;
            srcImage = LPSTR(where.searchObjectName.c_str());

            // Convert the string into an ANSI string
            CW2A szUsedByObject(where.whereUsedObjectname);
            // Now Convert it to a LPSTR
            usedByObject = LPSTR(szUsedByObject);
            usedByPropertyName = LPSTR(where.whereUsedPropertyName.c_str());

            LVITEM lvitem;
            lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
            lvitem.iItem = i;
            lvitem.iSubItem = 0;
            lvitem.pszText = srcImage;
            lvitem.lParam = (LPARAM)i; //Need this LPARM in order to sort the listview by column.  Using the counter i for this as each loop will provide a unique number.
            const int index = ListView_InsertItem(hListView, &lvitem);
            //Create Listview SubItem for 'Where Used By Object Name' like 'Wall001' or 'Light047' etc
            ListView_SetItemText(hListView, index, 1, usedByObject);
            //Create Listview SubItem for 'Where Used By Object Property Name'...lime 'Image' or 'Side Image'
            ListView_SetItemText(hListView, index, 2, usedByPropertyName);
            i++;
         }
         ListView_SetItemState(hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
         GotoDlgCtrl(hListView);
         return FALSE;
      }
   }
   break;
   case WM_NOTIFY:
   {
      LPNMHDR pnmhdr = (LPNMHDR)lParam;
      if (wParam == IDC_SOUNDLIST)
      {
         LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
         switch (lpnmListView->hdr.code)
         {
         case LVN_KEYDOWN:
         {
            //Key was pressed.  Char will be in pnkd.  Certain chars like <return> and other navigation keys won't be detected here though.
            LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)lParam;
         }
         break;
         case LVN_COLUMNCLICK:
         {
            {
               //Add code here if you want to support resorting the listview when a column heading is clicked
               const int columnNumber = lpnmListView->iSubItem;
               if (m_columnSortOrder == 1)
                  m_columnSortOrder = 0;
               else
                  m_columnSortOrder = 1;
               SortData.hwndList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
               SortData.subItemIndex = columnNumber;
               SortData.sortUpDown = m_columnSortOrder;
               if (columnNumber == 4)
                  ListView_SortItems(SortData.hwndList, MyCompProcMemValues, &SortData);
               else
                  ListView_SortItems(SortData.hwndList, MyCompProc, &SortData);
            }
         }
         break;
         case NM_DBLCLK:
         {
            const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
            EditObject(hSoundList);
         }
         break;
         default:
         {
         }
         break;
         }
      }
      switch (pnmhdr->code)
      {
      case LVN_ENDLABELEDIT:
      {
      }
      break;
      case LVN_ITEMCHANGING:
      {
      }
      break;
      case LVN_ITEMCHANGED:
      {
      }
      break;
      default:
      {
      }
      break;
      }
   }
   break;
   case WM_DRAWITEM:
   {
   }
   break;
   default:
   {
   }
   break;
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL WhereUsedDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();

   switch (LOWORD(wParam))
   {
      case IDC_OK:
      {
         const int count = ListView_GetSelectedCount(hSoundList);
         if (count > 0)
         {
            SetFocus();
         }
         SavePosition();
         CDialog::OnOK();
      }
      break;
      case IDC_EDIT_OBJECT:
      {
         EditObject(hSoundList);
      }
      break;
      case IDC_REFRESH_LIST:
      {
         RefreshList();
      }
      break;
   }
   return FALSE;
}

void WhereUsedDialog::OnOK()
{
   // do not call CDialog::OnOk() here because if you rename images keys like backspace or escape in rename mode cause an IDOK message and this function is called
}

void WhereUsedDialog::OnCancel() { }

void WhereUsedDialog::LoadPosition()
{
   const int x = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "WhereUsedPosX"s, 0);
   const int y = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "WhereUsedPosY"s, 0);
   const int w = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "WhereUsedWidth"s, 1000);
   const int h = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "WhereUsedHeight"s, 800);
   POINT p { x, y };
   if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
      SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void WhereUsedDialog::SavePosition()
{
   const CRect rect = GetWindowRect();

   g_pvp->m_settings.SaveValue(Settings::Editor, "WhereUsedPosX"s, (int)rect.left);
   g_pvp->m_settings.SaveValue(Settings::Editor, "WhereUsedPosY"s, (int)rect.top);
   const int w = rect.right - rect.left;
   g_pvp->m_settings.SaveValue(Settings::Editor, "WhereUsedWidth"s, w);
   const int h = rect.bottom - rect.top;
   g_pvp->m_settings.SaveValue(Settings::Editor, "WhereUsedHeight"s, h);
}

void WhereUsedDialog::RefreshList()
{
   //This method is very similar to the WhereUsedDialog WM_INITDIALOG code.  We don't need to build the dialog from scratch through.  Just need to delete existing items and re-add them.
   //Get active pinball table (one currently selected...you can edit more than one at a time)
   CCO(PinTable) *const pt = g_pvp->GetActiveTable();
   if (pt)
   {
      vector<WhereUsedInfo> vWhereUsed; //vector storing a list of the names of objects using this image.
      //There is a 'Where Used' button on the 'Image Manager' as well as the 'Materials Manager' window.  Since we are using the WhereUsedDialog for both we need a switch here
      switch (m_whereUsedSource)
      {
         case IMAGES:
         {
            pt->ShowWhereImagesUsed(vWhereUsed); //Fill a vector with a list of all images and the table objects that reference them
         }
         break;
         case MATERIALS:
         {
            pt->ShowWhereMaterialsUsed(vWhereUsed); //Fill a vector with a list of all materials and the table objects that reference them
         }
         break;
         case Default:
         {
         }
         break;
      }

      //Get the handle to the listview for the Where Used dialog
      const HWND hListView = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
      //Delete all the current items in the 'Where Used' listview because we are going to re-add them, and we don't want duplicates
      ListView_DeleteAllItems(hListView);

      //For each loop iterator to build the Listview items
      int i = 0;
      for (const WhereUsedInfo &where : vWhereUsed)
      {
         //Create Listview Item
         LPSTR srcImage = LPSTR(where.searchObjectName.c_str());

         // Convert the string into an ANSI string
         CW2A szUsedByObject(where.whereUsedObjectname);
         // Now Convert it to a LPSTR
         LPSTR usedByObject = LPSTR(szUsedByObject);
         LPSTR usedByPropertyName = LPSTR(where.whereUsedPropertyName.c_str());

         LVITEM lvitem;
         lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
         lvitem.iItem = i;
         lvitem.iSubItem = 0;
         lvitem.pszText = srcImage;
         lvitem.lParam = (LPARAM)i; //Need this LPARM in order to sort the listview by column.  Using counter i as it will be a unique number for each loop.
         const int index = ListView_InsertItem(hListView, &lvitem);
         //Create Listview SubItem for 'Where Used By Object Name' like 'Wall001' or 'Light047' etc
         ListView_SetItemText(hListView, index, 1, usedByObject);
         //Create Listview SubItem for 'Where Used By Object Property Name'...lime 'Image' or 'Side Image'
         ListView_SetItemText(hListView, index, 2, usedByPropertyName);
         i++;
      }
   }
}

void WhereUsedDialog::AddToolTip(const char *const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { 0 };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = (char *)text;
   SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void WhereUsedDialog::EditObject(HWND hWhereListView)
{
   //Get the index of the selected item in the 'WhereUsed' listview
   const int sel = ListView_GetNextItem(hWhereListView, -1, LVNI_SELECTED);
   if (sel == -1)
   {
      MessageBox("Nothing is selected!", "No item selected", MB_OK);
   }
   else
   {
      //Create a buffer for the controlName to be returned
      char controlName[MAX_PATH];
      //Make the call to get the item text from the listview item at the 'sel' index and subitem 1.  Store the listview item text in 'controlName'.
      ListView_GetItemText(hWhereListView, sel, 1, controlName, sizeof(controlName));
      //Make sure the controlName returned isn't 'Decal' as Decal doesn't support a unique name so we can't issue the 'GetElementByName' method on it.
      if (strcmp(controlName, "decal") == 0)
      {
         MessageBox("Decal objects don't have unique names so these objects need to be selected/edited manually.", "DECALS", MB_OK);
      }
      else
      {
         IEditable *const pedit = g_pvp->GetActiveTable()->GetElementByName(controlName);
         if (pedit != nullptr)
         {
            ISelect *const psel = pedit->GetISelect();
            if (psel != nullptr)
            {
               CCO(PinTable) *const pt = g_pvp->GetActiveTable();
               pt->AddMultiSel(psel, false, false, false);
               pt->RefreshProperties();
            }
         }
      }
   }
}
