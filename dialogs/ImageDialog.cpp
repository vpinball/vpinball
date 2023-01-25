#include "stdafx.h"
#include "resource.h"
#include "ImageDialog.h"
#include "vpversion.h"

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int CALLBACK MyCompProc( LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption );
extern int CALLBACK MyCompProcMemValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);
int ImageDialog::m_columnSortOrder;
bool ImageDialog::m_doNotChange;

ImageDialog::ImageDialog() : CDialog(IDD_IMAGEDIALOG)
{
   m_columnSortOrder = 1;
   m_doNotChange = false;
}

ImageDialog::~ImageDialog()
{

}

void ImageDialog::OnDestroy()
{
   CDialog::OnDestroy();
}

void ImageDialog::OnClose()
{
   SavePosition();
   CDialog::OnClose();
}

BOOL ImageDialog::OnInitDialog()
{   
   m_columnSortOrder = 1;
   return TRUE;
}

INT_PTR ImageDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);

   switch (uMsg)
   {
      case WM_INITDIALOG:
      {
         const HWND hListView = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
         m_resizer.Initialize(*this, CRect(0, 0, 720, 450));
         m_resizer.AddChild(hListView, CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
         m_resizer.AddChild(GetDlgItem(IDC_PICTUREPREVIEW).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_IMPORT).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_REIMPORT).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_REIMPORTFROM).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_UPDATE_ALL_BUTTON).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_DELETE_IMAGE).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_RENAME).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_EXPORT).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_OK).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_ALPHA_MASK_EDIT).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_STATIC_ALPHA).GetHwnd(), CResizer::topright, 0);
         m_resizer.AddChild(GetDlgItem(IDC_CHECK_RENAME_ON_EXPORT).GetHwnd(), CResizer::topright, 0);

         LoadPosition();

         ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

         LVCOLUMN lvcol;
         lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
         const LocalString ls(IDS_NAME);
         lvcol.pszText = (LPSTR)ls.m_szbuffer; // = "Name";
         lvcol.cx = 150;
         lvcol.fmt = LVCFMT_LEFT;
         ListView_InsertColumn(hListView, 0, &lvcol);

         const LocalString ls2(IDS_IMPORTPATH);
         lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "Import Path";
         lvcol.cx = 200;
         lvcol.fmt = LVCFMT_LEFT;
         ListView_InsertColumn(hListView, 1, &lvcol);

         const LocalString ls3(IDS_IMAGESIZE);
         lvcol.pszText = (LPSTR)ls3.m_szbuffer; // = "Image Size";
         lvcol.cx = 100;
         lvcol.fmt = LVCFMT_CENTER;
         ListView_InsertColumn(hListView, 2, &lvcol);

         const LocalString ls4(IDS_USED_IN_TABLE);
         lvcol.pszText = (LPSTR)ls4.m_szbuffer; // = "In use";
         lvcol.cx = 45;
         lvcol.fmt = LVCFMT_CENTER;
         ListView_InsertColumn(hListView, 3, &lvcol);

         const LocalString ls5(IDS_IMAGE_RAW_SIZE);
         lvcol.pszText = (LPSTR)ls5.m_szbuffer; // = "Raw Size";
         lvcol.cx = 60;
         lvcol.fmt = LVCFMT_RIGHT;
         ListView_InsertColumn(hListView, 4, &lvcol);

         const LocalString ls6(IDS_FORMAT);
         lvcol.pszText = (LPSTR)ls6.m_szbuffer; // = "Format";
         lvcol.cx = 70;
         lvcol.fmt = LVCFMT_CENTER;
         ListView_InsertColumn(hListView, 5, &lvcol);

         CCO(PinTable) *const pt = g_pvp->GetActiveTable();
         if (pt)
            pt->ListImages(hListView);

         char textBuf[16];
         strncpy_s(textBuf, "128", sizeof(textBuf)-1);
         SetDlgItemText(IDC_ALPHA_MASK_EDIT, textBuf);
         ListView_SetItemState(hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
         GotoDlgCtrl(hListView);
         return FALSE;
      }
      case WM_NOTIFY:
      {
         LPNMHDR pnmhdr = (LPNMHDR)lParam;
         if (wParam == IDC_SOUNDLIST)
         {
            LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
            if (lpnmListView->hdr.code == LVN_COLUMNCLICK)
            {
               const int columnNumber = lpnmListView->iSubItem;
               if (m_columnSortOrder == 1)
                  m_columnSortOrder = 0;
               else
                  m_columnSortOrder = 1;
               SortData.hwndList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
               SortData.subItemIndex = columnNumber;
               SortData.sortUpDown = m_columnSortOrder;
               if (columnNumber==4)
                   ListView_SortItems(SortData.hwndList, MyCompProcMemValues, &SortData);
               else
                   ListView_SortItems(SortData.hwndList, MyCompProc, &SortData);
            }
         }

         switch (pnmhdr->code)
         {
            case LVN_ENDLABELEDIT:
            {
               NMLVDISPINFO * const pinfo = (NMLVDISPINFO *)lParam;
               const HWND hSoundlist = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
               if (pinfo->item.pszText == nullptr || pinfo->item.pszText[0] == '\0')
               {
                  return FALSE;
               }
               ListView_SetItemText(hSoundlist, pinfo->item.iItem, 0, pinfo->item.pszText);
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = pinfo->item.iItem;
               lvitem.iSubItem = 0;
               ListView_GetItem(hSoundlist, &lvitem);
               Texture * const ppi = (Texture *)lvitem.lParam;
               if (ppi != nullptr)
               {
                  ppi->m_szName = pinfo->item.pszText;
                  CCO(PinTable) * const pt = g_pvp->GetActiveTable();
                  if (pt)
                  {
                     pt->SetNonUndoableDirty(eSaveDirty);
                     pt->UpdatePropertyImageList();
                  }
               }
               return TRUE;
         }
         break;

         case LVN_ITEMCHANGING:
         {
            if (m_doNotChange)
               break;
            NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
            if ((plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED))
            {
               if (plistview->uNewState & LVIS_SELECTED)
               {
                  const int sel = plistview->iItem;
                  LVITEM lvitem;
                  lvitem.mask = LVIF_PARAM;
                  lvitem.iItem = sel;
                  lvitem.iSubItem = 0;
                  ListView_GetItem(GetDlgItem(IDC_SOUNDLIST).GetHwnd(), &lvitem);
                  Texture * const ppi = (Texture *)lvitem.lParam;
                  if (ppi != nullptr)
                     SetDlgItemText(IDC_ALPHA_MASK_EDIT, std::to_string((int)ppi->m_alphaTestValue).c_str());
               }
               ::InvalidateRect(GetDlgItem(IDC_PICTUREPREVIEW).GetHwnd(), nullptr, fTrue);
            }
         }
         break;

         case LVN_ITEMCHANGED:
         {
            if (m_doNotChange)
               break;
            NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
            const int sel = plistview->iItem;
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(GetDlgItem(IDC_SOUNDLIST).GetHwnd(), &lvitem);
            Texture * const ppi = (Texture *)lvitem.lParam;
            if (ppi != nullptr)
            {
               const float v = sz2f(GetDlgItemText(IDC_ALPHA_MASK_EDIT).c_str());
               if (ppi->m_alphaTestValue != v)
               {
                  ppi->m_alphaTestValue = v;
                  CCO(PinTable) * const pt = g_pvp->GetActiveTable();
                  pt->SetNonUndoableDirty(eSaveDirty);
               }

               const int count = ListView_GetSelectedCount(GetDlgItem(IDC_SOUNDLIST).GetHwnd());
               const BOOL enable = !(count > 1);
               ::EnableWindow(GetDlgItem(IDC_REIMPORTFROM).GetHwnd(), enable);
               ::EnableWindow(GetDlgItem(IDC_RENAME).GetHwnd(), enable);
            }
            //EnableWindow(GetDlgItem(hwndDlg, IDC_EXPORT), enable);
         }
         break;

         }
      }
      break;

      case WM_DRAWITEM:
      {
         const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
         DRAWITEMSTRUCT * const pdis = (DRAWITEMSTRUCT *)lParam;
         const int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
         if (sel != -1)
         {
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(hSoundList, &lvitem);
            Texture * const ppi = (Texture *)lvitem.lParam;
            if (ppi != nullptr)
            {
               RECT rcClient;
               ::GetWindowRect(pdis->hwndItem, &rcClient);

               const int xsize = rcClient.right - rcClient.left;
               const int ysize = rcClient.bottom - rcClient.top;

               const float controlaspect = (float)xsize / (float)ysize;
               const float aspect = (float)ppi->m_width / (float)ppi->m_height;

               int width, height;
               if (aspect > controlaspect)
               {
                  width = xsize;
                  height = (int)(xsize / aspect + 0.5f);
               }
               else
               {
                  height = ysize;
                  width = (int)(ysize*aspect + 0.5f);
               }

               const int x = (xsize - width) / 2;
               const int y = (ysize - height) / 2;

               HDC hdcDD;
               ppi->GetTextureDC(&hdcDD);
               SetStretchBltMode(pdis->hDC, HALFTONE); // somehow enables filtering
               StretchBlt(pdis->hDC, x, y, width, height, hdcDD, 0, 0, ppi->m_width, ppi->m_height, SRCCOPY);
               ppi->ReleaseTextureDC(hdcDD);
            }
         }
         else
         {
            // Nothing currently selected
            RECT rcClient;
            ::GetClientRect(pdis->hwndItem, &rcClient);
            SelectObject(pdis->hDC, GetStockObject(NULL_BRUSH));
            SelectObject(pdis->hDC, GetStockObject(BLACK_PEN));
            Rectangle(pdis->hDC, rcClient.left + 5, rcClient.top + 5, rcClient.right - 5, rcClient.bottom - 5);

            SetBkMode(pdis->hDC, TRANSPARENT);

            RECT rcText = rcClient;

            //ExtTextOut(pdis->hDC, 0, 20, 0, nullptr, "Image\nPreview", 13, nullptr);
            const LocalString ls(IDS_IMAGE_PREVIEW);
            const int len = lstrlen(ls.m_szbuffer);
            DrawText(pdis->hDC, ls.m_szbuffer/*"Image\n\nPreview"*/, len, &rcText, DT_CALCRECT);

            const int halfheight = (rcClient.bottom - rcClient.top) / 2;
            const int halffont = (rcText.bottom - rcText.top) / 2;

            rcText.left = rcClient.left;
            rcText.right = rcClient.right;
            rcText.top = halfheight - halffont;
            rcText.bottom = halfheight + halffont;

            DrawText(pdis->hDC, ls.m_szbuffer/*"Image\n\nPreview"*/, len, &rcText, DT_CENTER);
         }
         return TRUE;
      }
      break;

   }
   return DialogProcDefault(uMsg, wParam, lParam);
}

void ImageDialog::UpdateImages()
{
    const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
    const int count = ListView_GetSelectedCount(hSoundList);

    if (count > 0)
    {
        int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
        while (sel != -1)
        {
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(hSoundList, &lvitem);
            Texture * const ppi = (Texture *)lvitem.lParam;
            if (ppi != nullptr)
            {
                const float v = sz2f(GetDlgItemText(IDC_ALPHA_MASK_EDIT).c_str());
                if (ppi->m_alphaTestValue != v)
                {
                    ppi->m_alphaTestValue = v;
                    CCO(PinTable) * const pt = g_pvp->GetActiveTable();
                    pt->SetNonUndoableDirty(eSaveDirty);
                }

                sel = ListView_GetNextItem(hSoundList, sel, LVNI_SELECTED);
            }
        }
        SetFocus();
    }

}

BOOL ImageDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();

   switch (LOWORD(wParam))
   {
      case IDC_IMPORT: Import(); break;
      case IDC_EXPORT: Export(); break;
      case IDC_DELETE_IMAGE: DeleteImage(); break;
      case IDC_REIMPORT: Reimport(); break;
      case IDC_UPDATE_ALL_BUTTON: UpdateAll(); break;
      case IDC_REIMPORTFROM: ReimportFrom(); break;
      case IDC_RENAME:
      {
         const int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
         if (sel != -1)
         {
            ::SetFocus(hSoundList);
            ListView_EditLabel(hSoundList, sel);
            CCO(PinTable) * const pt = g_pvp->GetActiveTable();
            pt->SetNonUndoableDirty(eSaveDirty);
         }
         break;
      }
      case IDC_OK:
      {
         const int count = ListView_GetSelectedCount(hSoundList);
         if (count > 0)
         {
            UpdateImages();
            SetFocus();
         }

         SavePosition();
         CDialog::OnOK();
         break;
      }
   }
   return FALSE;
}

void ImageDialog::OnOK()
{
   // do not call CDialog::OnOk() here because if you rename images keys like backspace or escape in rename mode cause an IDOK message and this function is called
}

void ImageDialog::OnCancel()
{
    SavePosition();
    CDialog::OnCancel();
}

void ImageDialog::Import()
{
   string szInitialDir;
   HRESULT hr = LoadValue(regKey[RegName::RecentDir], "ImageDir"s, szInitialDir);
   if (hr != S_OK)
      szInitialDir = "c:\\Visual Pinball\\tables\\";

   vector<string> szFileName;
   if (g_pvp->OpenFileDialog(szInitialDir, szFileName, "Bitmap, JPEG, PNG, TGA, WEBP, EXR, HDR Files (.bmp/.jpg/.png/.tga/.webp/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.tga;*.webp;*.exr;*.hdr\0", "png", OFN_EXPLORER | OFN_ALLOWMULTISELECT))
   {
      CCO(PinTable) * const pt = g_pvp->GetActiveTable();
      const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();

      for (const string &file : szFileName)
         pt->ImportImage(hSoundList, file);

      const size_t index = szFileName[0].find_last_of('\\');
      if (index != string::npos)
         hr = SaveValue(regKey[RegName::RecentDir], "ImageDir"s, szFileName[0].substr(0, index));

      pt->SetNonUndoableDirty(eSaveDirty);
      pt->UpdatePropertyImageList();
      SetFocus();
   }
}

void ImageDialog::Export()
{
   const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
   const int selectedItemsCount = ListView_GetSelectedCount(hSoundList);
      
   if (selectedItemsCount)	// if some items are selected???
   {
      int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
      if (sel != -1)
      {
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = sel;
         lvitem.iSubItem = 0;
         ListView_GetItem(hSoundList, &lvitem);
         Texture * ppi = (Texture*)lvitem.lParam;
         if (ppi != nullptr)
         {
            OPENFILENAME ofn = {};
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hInstance = g_pvp->theInstance;
            ofn.hwndOwner = g_pvp->GetHwnd();
            char g_filename[MAXSTRING];
            g_filename[0] = '\0';

            const size_t renameOnExport = SendMessage(GetDlgItem(IDC_CHECK_RENAME_ON_EXPORT).GetHwnd(), BM_GETCHECK, 0, 0);

            if (!renameOnExport)
            {
               const int len0 = (int)ppi->m_szPath.length();
               int begin; //select only file name from pathfilename
               for (begin = len0; begin >= 0; begin--)
               {
                  if (ppi->m_szPath[begin] == '\\' || ppi->m_szPath[begin] == '/')
                  {
                     begin++;
                     break;
                  }
               }
               if (begin > 0)
               {
                  memcpy(g_filename, ppi->m_szPath.c_str()+begin, len0 - begin);
                  g_filename[len0 - begin] = 0;
               }
            }
            else
            {
               strncat_s(g_filename, ppi->m_szName.c_str(), sizeof(g_filename)-strnlen_s(g_filename, sizeof(g_filename))-1);
               const size_t idx = ppi->m_szPath.find_last_of('.');
               strncat_s(g_filename, ppi->m_szPath.c_str() + idx, sizeof(g_filename)-strnlen_s(g_filename, sizeof(g_filename))-1);
            }
            ofn.lpstrFile = g_filename;
            ofn.nMaxFile = sizeof(g_filename);

            const string ext2(g_filename);
            const size_t idx2 = ext2.find_last_of('.');
            ofn.lpstrDefExt = ext2.c_str() + idx2 + 1;
            // check which default file extension should be selected
            ofn.lpstrFilter = "PNG (.png)\0*.png;\0Bitmap (.bmp)\0*.bmp;\0JPEG (.jpg/.jpeg)\0*.jpg;*.jpeg;\0IFF (.iff)\0*.IFF;\0PCX (.pcx)\0*.PCX;\0PICT (.pict)\0*.PICT;\0Photoshop (.psd)\0*.psd;\0TGA (.tga)\0*.tga;\0TIFF (.tiff/.tif)\0*.tiff;*.tif;\0WEBP (.webp)\0*.webp;\0EXR (.exr)\0*.exr;\0HDR (.hdr)\0*.hdr\0";
            if(!_stricmp(ofn.lpstrDefExt,"png"))
               ofn.nFilterIndex = 1;
            else if (!_stricmp(ofn.lpstrDefExt, "bmp"))
               ofn.nFilterIndex = 2;
            else if (!_stricmp(ofn.lpstrDefExt, "jpg") || !_stricmp(ofn.lpstrDefExt, "jpeg"))
               ofn.nFilterIndex = 3;
            else if (!_stricmp(ofn.lpstrDefExt, "iff"))
               ofn.nFilterIndex = 4;
            else if (!_stricmp(ofn.lpstrDefExt, "pcx"))
               ofn.nFilterIndex = 5;
            else if (!_stricmp(ofn.lpstrDefExt, "pict"))
               ofn.nFilterIndex = 6;
            else if (!_stricmp(ofn.lpstrDefExt, "psd"))
               ofn.nFilterIndex = 7;
            else if (!_stricmp(ofn.lpstrDefExt, "tga"))
               ofn.nFilterIndex = 8;
            else if (!_stricmp(ofn.lpstrDefExt, "tif") || !_stricmp(ofn.lpstrDefExt, "tiff"))
               ofn.nFilterIndex = 9;
            else if (!_stricmp(ofn.lpstrDefExt, "webp"))
               ofn.nFilterIndex = 10;
            else if (!_stricmp(ofn.lpstrDefExt, "exr"))
               ofn.nFilterIndex = 11;
            else if (!_stricmp(ofn.lpstrDefExt, "hdr"))
               ofn.nFilterIndex = 12;

            string g_initDir;
            const HRESULT hr = LoadValue(regKey[RegName::RecentDir], "ImageDir"s, g_initDir);
            if (hr != S_OK)
               g_initDir = "c:\\Visual Pinball\\tables\\";

            ofn.lpstrInitialDir = (hr == S_OK) ? g_initDir.c_str() : nullptr;
            //ofn.lpstrTitle = "SAVE AS";
            ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

            g_initDir[ofn.nFileOffset] = 0;

            if (GetSaveFileName(&ofn))	//Get filename from user
            {
               int begin; //select only file name from pathfilename
               for (begin = lstrlen(ofn.lpstrFile); begin >= 0; begin--)
               {
                  if (ofn.lpstrFile[begin] == '\\')
                  {
                     begin++;
                     break;
                  }
               }

               if (begin >= MAXSTRING)
                   begin = MAXSTRING - 1;

               char pathName[MAXSTRING];
               if (begin > 0)
                  memcpy(pathName, ofn.lpstrFile, begin);
               pathName[begin] = 0;

               while (sel != -1 && ppi != nullptr)
               {
                  if (selectedItemsCount>1)
                  {
                     strncpy_s(g_filename, pathName, sizeof(g_filename)-1);
                     if (!renameOnExport)
                     {
                        for (begin = (int)ppi->m_szPath.length(); begin >= 0; begin--)
                        {
                           if (ppi->m_szPath[begin] == '\\')
                           {
                              begin++;
                              break;
                           }
                        }
                        strncat_s(g_filename, ppi->m_szPath.c_str()+begin, sizeof(g_filename)-strnlen_s(g_filename, sizeof(g_filename))-1);
                     }
                     else
                     {
                        strncat_s(g_filename, ppi->m_szName.c_str(), sizeof(g_filename)-strnlen_s(g_filename, sizeof(g_filename))-1);
                        const size_t idx = ppi->m_szPath.find_last_of('.');
                        strncat_s(g_filename, ppi->m_szPath.c_str() + idx, sizeof(g_filename)-strnlen_s(g_filename, sizeof(g_filename))-1);
                     }
                  }

                  CCO(PinTable) * const pt = g_pvp->GetActiveTable();
                  if (!pt->ExportImage(ppi, g_filename)) //!! this will always export the image in its original format, no matter what was actually selected by the user
                     ShowError("Could not export Image");
                  sel = ListView_GetNextItem(hSoundList, sel, LVNI_SELECTED);
                  lvitem.iItem = sel;
                  lvitem.iSubItem = 0;
                  ListView_GetItem(hSoundList, &lvitem);
                  ppi = (Texture*)lvitem.lParam;
               }

               SaveValue(regKey[RegName::RecentDir], "ImageDir"s, pathName);
            } // finished all selected items
         }
      }
   }

   SetFocus();
}

void ImageDialog::DeleteImage()
{
   const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
   const int count = ListView_GetSelectedCount(hSoundList);

   if (count > 0)
   {
      const LocalString ls(IDS_REMOVEIMAGE);
      const int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
      if (ans == IDYES)
      {
         CCO(PinTable) * const pt = g_pvp->GetActiveTable();
         int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
         int lastsel = -1;
         while (sel != -1)
         {
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(hSoundList, &lvitem);
            Texture * const ppi = (Texture*)lvitem.lParam;
            if (ppi != nullptr)
            {
               m_doNotChange = true; // do not trigger LVN_ITEMCHANGING or LVN_ITEMCHANGED code!
               ListView_DeleteItem(hSoundList, sel);
               m_doNotChange = false;
               pt->RemoveImage(ppi);

               // The previous selection is now deleted, so look again from the top of the list
               lastsel = sel;
               sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
            }
         }
         if (lastsel != -1)
            ListView_SetItemState(hSoundList, lastsel, LVNI_SELECTED | LVNI_FOCUSED, LVNI_SELECTED | LVNI_FOCUSED);
         pt->SetNonUndoableDirty(eSaveDirty);
      }
   }

   SetFocus();
}

void ImageDialog::Reimport()
{
   const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
   const int count = ListView_GetSelectedCount(hSoundList);

   if (count > 0)
   {
      const LocalString ls(IDS_REPLACEIMAGE);
      const int ans = MessageBox(ls.m_szbuffer/*"Are you sure you want to replace this image?"*/, "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2);
      if (ans == IDYES)
      {
         int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
         while (sel != -1)
         {
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(hSoundList, &lvitem);
            Texture * const ppi = (Texture*)lvitem.lParam;
            if (ppi != nullptr)
            {
               const HANDLE hFile = CreateFile(ppi->m_szPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                               nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

               if (hFile != INVALID_HANDLE_VALUE)
               {
                  CloseHandle(hFile);
                  CCO(PinTable) * const pt = g_pvp->GetActiveTable();
                  pt->ReImportImage(ppi, ppi->m_szPath);
                  pt->SetNonUndoableDirty(eSaveDirty);
               }
               else
                  MessageBox(ppi->m_szPath.c_str(), "FILE NOT FOUND!", MB_OK);

               sel = ListView_GetNextItem(hSoundList, sel, LVNI_SELECTED);
            }
         }
      }
      // Display new image
      ::InvalidateRect(GetDlgItem(IDC_PICTUREPREVIEW).GetHwnd(), nullptr, fTrue);
   }
   SetFocus();
}

void ImageDialog::UpdateAll()
{
   const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
   const int count = ListView_GetSelectedCount(hSoundList);
   bool errorOccurred = false;

   for (int sel = 0; sel < count; sel++)
   {
      LVITEM lvitem;
      lvitem.mask = LVIF_PARAM;
      lvitem.iItem = sel;
      lvitem.iSubItem = 0;
      ListView_GetItem(hSoundList, &lvitem);
      Texture * const ppi = (Texture*)lvitem.lParam;
      if (ppi != nullptr)
      {
         const HANDLE hFile = CreateFile(ppi->m_szPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
         if (hFile != INVALID_HANDLE_VALUE)
         {
            CloseHandle(hFile);
            CCO(PinTable) * const pt = g_pvp->GetActiveTable();
            pt->ReImportImage(ppi, ppi->m_szPath);
            pt->SetNonUndoableDirty(eSaveDirty);
         }
         else
            errorOccurred = true;
      }
      // Display new image
      ::InvalidateRect(GetDlgItem(IDC_PICTUREPREVIEW).GetHwnd(), nullptr, fTrue);
   }

   if (errorOccurred)
      MessageBox( "Not all images were updated!", "  FILES NOT FOUND!  ", MB_OK);
}

void ImageDialog::ReimportFrom()
{
   const HWND hSoundList = GetDlgItem(IDC_SOUNDLIST).GetHwnd();
   const int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);

   if (sel != -1)
   {
      const LocalString ls(IDS_REPLACEIMAGE);
      const int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to replace this image with a new one?"*/, "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2);
      if (ans == IDYES)
      {
         string szInitialDir;
         const HRESULT hr = LoadValue(regKey[RegName::RecentDir], "ImageDir"s, szInitialDir);
         if (hr != S_OK)
            szInitialDir = "c:\\Visual Pinball\\tables\\";

         vector<string> szFileName;
         if (g_pvp->OpenFileDialog(szInitialDir, szFileName, "Bitmap, JPEG, PNG, TGA, WEBP, EXR, HDR Files (.bmp/.jpg/.png/.tga/.webp/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.tga;*.webp;*.exr;*.hdr\0","png",0))
         {
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(hSoundList, &lvitem);
            Texture * const ppi = (Texture*)lvitem.lParam;
            if (ppi != nullptr)
            {
               const size_t index = szFileName[0].find_last_of('\\');
               if (index != string::npos)
                  SaveValue(regKey[RegName::RecentDir], "ImageDir"s, szFileName[0].substr(0, index));

               CCO(PinTable) * const pt = g_pvp->GetActiveTable();
               pt->ReImportImage(ppi, szFileName[0]);
               ListView_SetItemText(hSoundList, sel, 1, (LPSTR)ppi->m_szPath.c_str());
               pt->SetNonUndoableDirty(eSaveDirty);
               pt->UpdatePropertyImageList();
               // Display new image
               ::InvalidateRect(GetDlgItem(IDC_PICTUREPREVIEW).GetHwnd(), nullptr, fTrue);
            }
         }
      }
   }

   SetFocus();
}

void ImageDialog::LoadPosition()
{
    const int x = LoadValueIntWithDefault(regKey[RegName::Editor], "ImageMngPosX"s, 0 );
    const int y = LoadValueIntWithDefault(regKey[RegName::Editor], "ImageMngPosY"s, 0 );

    const int w = LoadValueIntWithDefault(regKey[RegName::Editor], "ImageMngWidth"s, 1000);
    const int h = LoadValueIntWithDefault(regKey[RegName::Editor], "ImageMngHeight"s, 800);
    SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void ImageDialog::SavePosition()
{
    const CRect rect = GetWindowRect();

    SaveValueInt(regKey[RegName::Editor], "ImageMngPosX"s, rect.left);
    SaveValueInt(regKey[RegName::Editor], "ImageMngPosY"s, rect.top);
    const int w = rect.right - rect.left;
    SaveValueInt(regKey[RegName::Editor], "ImageMngWidth"s, w);
    const int h = rect.bottom - rect.top;
    SaveValueInt(regKey[RegName::Editor], "ImageMngHeight"s, h);
}
