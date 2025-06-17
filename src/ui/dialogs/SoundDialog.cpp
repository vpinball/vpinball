// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "SoundDialog.h"

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
} SORTDATA;

extern SORTDATA SortData;
extern int CALLBACK MyCompProc( LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption );
int SoundDialog::m_columnSortOrder;

SoundDialog::SoundDialog()
   : CDialog(IDD_SOUNDDIALOG)
{
   hSoundList = nullptr;
   m_columnSortOrder = 1;
}

SoundDialog::~SoundDialog()
{
}

void SoundDialog::OnDestroy()
{
    CDialog::OnDestroy();
}

void SoundDialog::OnClose()
{
   m_audioPlayer = nullptr;
    SavePosition();
    CDialog::OnClose();
}

static long GetSystemDPI()
{
	const CClientDC clientDC(nullptr);
	const SIZE ret = { clientDC.GetDeviceCaps(LOGPIXELSX), clientDC.GetDeviceCaps(LOGPIXELSY) };
	return ret.cx;
}

static long GetDPI()
{
	static const long dpi = GetSystemDPI();
	return dpi;
}

static int DPIValue(int value)
{
	return MulDiv(value, GetDPI(), 96);
}

BOOL SoundDialog::OnInitDialog()
{
   m_audioPlayer = std::make_unique<VPX::AudioPlayer>(g_pvp->m_settings);
    CCO( PinTable ) * const pt = g_pvp->GetActiveTable();
    const HWND toolTipHwnd = CreateWindowEx(
      0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), nullptr, g_pvp->theInstance, nullptr);
    hSoundList = GetDlgItem( IDC_SOUNDLIST ).GetHwnd();
    AddToolTip("Import a new sound from a file.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_IMPORT).GetHwnd());
    AddToolTip("ReImport selected sound(s) using the existing file path(s).", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_REIMPORT).GetHwnd());
    AddToolTip("ReImport selected sound (using a different file path).", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_REIMPORTFROM).GetHwnd());
    AddToolTip("Delete the selected sound(s).", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_DELETE_SOUND).GetHwnd());
    AddToolTip("Rename the selected sound.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_RENAME).GetHwnd());
    AddToolTip("Play the selected sound to preview it.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_PLAY).GetHwnd());
    AddToolTip("Stop playing the selected sound.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_STOP).GetHwnd());
    AddToolTip("Export the selected sound(s).", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_SNDEXPORT).GetHwnd());
    AddToolTip("Toggle the sound to play through Table vs Backglass speakers for the selected sound(s).", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_SNDTOBG).GetHwnd());
    AddToolTip("Set the volume, left/right (pan) and front/back (fade) sound position for the selected sound(s).", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_SNDPOSITION).GetHwnd());
    AddToolTip("The 'Name' value from the list will be used when exporting instead of the file name from the 'Import Path'.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_CHECK_RENAME_ON_EXPORT).GetHwnd());
    AddToolTip("Click 'OK' to close this window.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_OK).GetHwnd());
    m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 514, 231));
    m_resizer.AddChild(hSoundList, CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(GetDlgItem(IDC_IMPORT).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_REIMPORT).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_REIMPORTFROM).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_DELETE_SOUND).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_RENAME).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_PLAY).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STOP).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_SNDEXPORT).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_SNDTOBG).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_SNDPOSITION).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_CHECK_RENAME_ON_EXPORT).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC).GetHwnd(), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_OK).GetHwnd(), CResizer::topright, 0);

    LoadPosition();

    m_columnSortOrder = 1;

    ListView_SetExtendedListViewStyle( hSoundList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
    LVCOLUMN lvcol = {};
    lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT; 
    const LocalString ls( IDS_NAME );
    lvcol.pszText = (LPSTR)ls.m_szbuffer; // = "Name";
    lvcol.cx = DPIValue(150);
    ListView_InsertColumn(hSoundList, 0, &lvcol);

    const LocalString ls2( IDS_IMPORTPATH );
    lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "Import Path";
    lvcol.cx = DPIValue(200);
    ListView_InsertColumn(hSoundList, 1, &lvcol);

    lvcol.pszText = (LPSTR)"Output"; //!! use LocalString
    lvcol.cx = DPIValue(80);
    ListView_InsertColumn(hSoundList, 2, &lvcol);

    lvcol.pszText = (LPSTR)"Pan"; //!! use LocalString
    lvcol.cx = DPIValue(50);
    ListView_InsertColumn(hSoundList, 3, &lvcol);

    lvcol.pszText = (LPSTR)"Fade"; //!! use LocalString
    lvcol.cx = DPIValue(50);
    ListView_InsertColumn(hSoundList, 4, &lvcol);

    lvcol.pszText = (LPSTR)"Vol"; //!! use LocalString
    lvcol.cx = DPIValue(50);
    ListView_InsertColumn(hSoundList, 5, &lvcol);

    if (pt)
        pt->ListSounds(hSoundList);

    ListView_SetItemState(hSoundList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    GotoDlgCtrl(hSoundList);

    return FALSE;
}

INT_PTR SoundDialog::DialogProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    m_resizer.HandleMessage(uMsg, wParam, lParam);
    CCO( PinTable ) * const pt = g_pvp->GetActiveTable();

    switch(uMsg)
    {
        case WM_NOTIFY:
        {
            const LPNMHDR pnmhdr = (LPNMHDR)lParam;
            if (wParam == IDC_SOUNDLIST)
            {
                const LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
                if (lpnmListView->hdr.code == LVN_COLUMNCLICK)
                {
                    const int columnNumber = lpnmListView->iSubItem;
                    if (m_columnSortOrder == 1)
                       m_columnSortOrder = 0;
                    else
                       m_columnSortOrder = 1;
                    SortData.hwndList = hSoundList;
                    SortData.subItemIndex = columnNumber;
                    SortData.sortUpDown = m_columnSortOrder;
                    ListView_SortItems( SortData.hwndList, MyCompProc, &SortData );
                }
            }
            switch(pnmhdr->code)
            {
                case LVN_ENDLABELEDIT:
                {
                    NMLVDISPINFO *const pinfo = (NMLVDISPINFO *)lParam;
                    if (pinfo->item.pszText == nullptr || pinfo->item.pszText[0] == '\0')
                        return FALSE;
                    ListView_SetItemText( hSoundList, pinfo->item.iItem, 0, pinfo->item.pszText );
                    LVITEM lvitem;
                    lvitem.mask = LVIF_PARAM;
                    lvitem.iItem = pinfo->item.iItem;
                    lvitem.iSubItem = 0;
                    ListView_GetItem( hSoundList, &lvitem );
                    VPX::Sound *const pps = (VPX::Sound *)lvitem.lParam;
                    pps->m_name = pinfo->item.pszText;
                    if (pt)
                        pt->SetNonUndoableDirty( eSaveDirty );
                    return TRUE;
                }
                break;

                case LVN_ITEMCHANGING:
                {
                   NMLVDISPINFO *const pinfo = (NMLVDISPINFO *)lParam;
                   LVITEM lvitem;
                   lvitem.mask = LVIF_PARAM;
                   lvitem.iItem = pinfo->item.iItem;
                   lvitem.iSubItem = 0;
                   ListView_GetItem(hSoundList, &lvitem);
                   m_audioPlayer->StopSound((VPX::Sound *)lvitem.lParam);
                   return TRUE;
                }

                case LVN_ITEMCHANGED:
                {
                    const int count = ListView_GetSelectedCount( hSoundList );
                    const BOOL enable = !(count > 1);
                    GetDlgItem(IDC_REIMPORTFROM).EnableWindow( enable );
                    GetDlgItem(IDC_RENAME).EnableWindow( enable );
                    GetDlgItem(IDC_PLAY).EnableWindow( enable );
                    GetDlgItem(IDC_STOP).EnableWindow( fFalse );
                }
                break;
            }
        }
        break;
    }
    return DialogProcDefault( uMsg, wParam, lParam );
}

BOOL SoundDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );

    switch(LOWORD( wParam ))
    {
        case IDC_IMPORT: Import(); break;
        case IDC_REIMPORT: ReImport(); break;
        case IDC_REIMPORTFROM: ReImportFrom(); break;
        case IDC_SNDEXPORT: Export(); break;
        case IDC_SNDTOBG: SoundToBG(); break;
        case IDC_SNDPOSITION: SoundPosition(); break;
        case IDC_DELETE_SOUND: DeleteSound(); break;
        case IDC_OK: SavePosition(); CDialog::OnOK(); break;
        case IDC_STOP:
        {
            const int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
            if (sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                m_audioPlayer->StopSound((VPX::Sound *)lvitem.lParam);
                GetDlgItem(IDC_STOP).EnableWindow(fFalse);
            }
            break;
        }
        case IDC_PLAY:
        {
            const int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
            if (sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                m_audioPlayer->PlaySound((VPX::Sound *)lvitem.lParam, 1.0f, 0.0f, 0, 0.f, 0.f, 0, false, false);
                GetDlgItem(IDC_STOP).EnableWindow(TRUE);
            }
            break;
        }
        case IDC_RENAME:
        {
            const int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
            if (sel != -1)
            {
                ::SetFocus( hSoundList );
                ListView_EditLabel( hSoundList, sel );
            }
            break;
        }
        default: return FALSE;
    }

    return TRUE;
}

void SoundDialog::OnOK()
{
    // do not call CDialog::OnOk() here because if you rename sounds keys like backspace or escape in rename mode cause an IDOK message and this function is called
}

void SoundDialog::OnCancel()
{
    SavePosition();
    CDialog::OnCancel();
}

void SoundDialog::Import()
{
   CCO(PinTable)* const pt = g_pvp->GetActiveTable();

   if (pt == nullptr)
      return;

   string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "SoundDir"s, PATH_TABLES);

   vector<string> szFileName;
   if (g_pvp->OpenFileDialog(szInitialDir, szFileName, "Sound Files (.wav/.ogg/.mp3)\0*.wav;*.ogg;*.mp3\0", "mp3", OFN_EXPLORER | OFN_ALLOWMULTISELECT))
   {
      const size_t index = szFileName[0].find_last_of(PATH_SEPARATOR_CHAR);
      if (index != string::npos)
         g_pvp->m_settings.SaveValue(Settings::RecentDir, "SoundDir"s, szFileName[0].substr(0, index));

      for (const string &file : szFileName)
         pt->ImportSound(hSoundList, file);

      pt->SetNonUndoableDirty(eSaveDirty);
   }

   SetFocus();
}

void SoundDialog::ReImport()
{
    CCO( PinTable ) * const pt = g_pvp->GetActiveTable();
    const int count = ListView_GetSelectedCount( hSoundList );
    if (count > 0)
    {
        const LocalString ls( IDS_REPLACESOUND );
        const int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2 );
        if (ans == IDYES)
        {
            int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
            while(sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                VPX::Sound *const pps = (VPX::Sound *)lvitem.lParam;

                const HANDLE hFile = CreateFile( pps->m_path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                                 nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    CloseHandle( hFile );

                    pt->ReImportSound( hSoundList, pps, pps->m_path );
                    pt->SetNonUndoableDirty( eSaveDirty );
                }
                else
                    MessageBox( pps->m_path.c_str(), "FILE NOT FOUND!", MB_OK );

                sel = ListView_GetNextItem( hSoundList, sel, LVNI_SELECTED );
            }
        }
        //pt->SetNonUndoableDirty(eSaveDirty);
    }
    SetFocus();
}

void SoundDialog::ReImportFrom()
{
    CCO( PinTable ) * const pt = g_pvp->GetActiveTable();
    const int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
    if (sel != -1)
    {
        const LocalString ls( IDS_REPLACESOUND );
        const int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to replace this sound with a new one?"*/, "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2 );
        if (ans == IDYES)
        {
            string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "SoundDir"s, PATH_TABLES);

            vector<string> szFileName;
            if (g_pvp->OpenFileDialog(szInitialDir, szFileName, "Sound Files (.wav/.ogg/.mp3)\0*.wav;*.ogg;*.mp3\0", "mp3", 0))
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                VPX::Sound *const pps = (VPX::Sound *)lvitem.lParam;

                pt->ReImportSound( hSoundList, pps, szFileName[0] );
                ListView_SetItemText( hSoundList, sel, 1, (LPSTR)szFileName[0].c_str() );

                const size_t index = szFileName[0].find_last_of(PATH_SEPARATOR_CHAR);
                if (index != string::npos)
                   g_pvp->m_settings.SaveValue(Settings::RecentDir, "SoundDir"s, szFileName[0].substr(0, index));

                pt->SetNonUndoableDirty( eSaveDirty );
            }
        }
    }
    SetFocus();
}

void SoundDialog::Export()
{
    CCO( PinTable ) * const pt = g_pvp->GetActiveTable();
    const int selectedItemsCount = ListView_GetSelectedCount(hSoundList);
    const size_t renameOnExport = IsDlgButtonChecked(IDC_CHECK_RENAME_ON_EXPORT);

    if (selectedItemsCount)
    {
        LVITEM lvitem;
        int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED ); //next selected item 	
        if (sel != -1)
        {
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem( hSoundList, &lvitem );
            VPX::Sound *pps = (VPX::Sound *)lvitem.lParam;

            OPENFILENAME ofn = {};
            ofn.lStructSize = sizeof( OPENFILENAME );
            ofn.hInstance = g_pvp->theInstance;
            ofn.hwndOwner = g_pvp->GetHwnd();
            ofn.lpstrFilter = "Sound Files (.wav/.ogg/.mp3)\0*.wav;*.ogg;*.mp3\0";

            char filename[MAXSTRING] = {};

            if (!renameOnExport)
            {
               const int len0 = (int)pps->m_path.length();
               int begin; //select only file name from pathfilename
               for (begin = len0; begin >= 0; begin--)
               {
                  if (pps->m_path[begin] == PATH_SEPARATOR_CHAR)
                  {
                     begin++;
                     break;
                  }
               }
               memcpy(filename, pps->m_path.c_str() + begin, len0 - begin);
            }
            else
            {
               strncat_s(filename, pps->m_name.c_str(), sizeof(filename)-strnlen_s(filename, sizeof(filename))-1);
               const size_t idx = pps->m_path.find_last_of('.');
               strncat_s(filename, pps->m_path.c_str() + idx, sizeof(filename)-strnlen_s(filename, sizeof(filename))-1);
            }
            ofn.lpstrFile = filename;
            ofn.nMaxFile = sizeof(filename);
            ofn.lpstrDefExt = "mp3";

            string initDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "SoundDir"s, PATH_TABLES);

            ofn.lpstrInitialDir = initDir.c_str();
            //ofn.lpstrTitle = "SAVE AS";
            ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

            if (GetSaveFileName( &ofn ))	//Get filename from user
            {
                int begin;
                for (begin = (int)strlen(ofn.lpstrFile); begin >= 0; begin--)
                {
                    if (ofn.lpstrFile[begin] == PATH_SEPARATOR_CHAR)
                    {
                        begin++;
                        break;
                    }
                }

                if (begin >= MAXSTRING)
                    begin = MAXSTRING-1;

                char pathName[MAXSTRING];
                if(begin > 0)
                    memcpy( pathName, ofn.lpstrFile, begin );
                pathName[begin] = '\0';

                while(sel != -1)
                {
                    if (selectedItemsCount > 1)
                    {
                       strncpy_s(filename, pathName, sizeof(filename)-1);
                       if (!renameOnExport)
                       {
                          for (begin = (int)pps->m_path.length(); begin >= 0; begin--)
                          {
                             if (pps->m_path[begin] == PATH_SEPARATOR_CHAR)
                             {
                                begin++;
                                break;
                             }
                          }
                          strncat_s(filename, pps->m_path.c_str()+begin, sizeof(filename)-strnlen_s(filename, sizeof(filename))-1);
                       }
                       else
                       {
                          strncat_s(filename, pps->m_name.c_str(), sizeof(filename)-strnlen_s(filename, sizeof(filename))-1);
                          const size_t idx = pps->m_path.find_last_of('.');
                          strncat_s(filename, pps->m_path.c_str() + idx, sizeof(filename)-strnlen_s(filename, sizeof(filename))-1);
                       }
                    }

                    pt->ExportSound(pps, filename);
                    sel = ListView_GetNextItem( hSoundList, sel, LVNI_SELECTED ); //next selected item
                    lvitem.iItem = sel;
                    lvitem.iSubItem = 0;
                    ListView_GetItem( hSoundList, &lvitem );
                    pps = (VPX::Sound *)lvitem.lParam;
                }

                g_pvp->m_settings.SaveValue(Settings::RecentDir, "SoundDir"s, string(pathName));
            }
        }
    }
    SetFocus();
}

void SoundDialog::SoundToBG()
{
    CCO( PinTable ) * const pt = g_pvp->GetActiveTable();

    if (ListView_GetSelectedCount( hSoundList ))
    {
        LVITEM lvitem;
        int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED ); //next selected item 	
        while(sel != -1)
        {
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem( hSoundList, &lvitem );
            VPX::Sound *const pps = (VPX::Sound *)lvitem.lParam;
            pps->SetOutputTarget((pps->GetOutputTarget() != VPX::SNDOUT_BACKGLASS) ? VPX::SNDOUT_BACKGLASS : VPX::SNDOUT_TABLE);
            switch (pps->GetOutputTarget())
            {
               case VPX::SNDOUT_BACKGLASS:
                  ListView_SetItemText(hSoundList, sel, 2, (LPSTR)"Backglass");
                  break;
               case VPX::SNDOUT_TABLE:
               default:
                  ListView_SetItemText(hSoundList, sel, 2, (LPSTR)"Table");
                  break;
            }
            pt->SetNonUndoableDirty(eSaveDirty);
            sel = ListView_GetNextItem(hSoundList, sel, LVNI_SELECTED ); //next selected item
        }
    }
    SetFocus();
}

void SoundDialog::SoundPosition()
{
	CCO(PinTable) * const pt = g_pvp->GetActiveTable();

	if (ListView_GetSelectedCount(hSoundList))
	{
		LVITEM lvitem;
		int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED); //next selected item 	

		lvitem.mask = LVIF_PARAM;
		lvitem.iItem = sel;
		lvitem.iSubItem = 0;
		ListView_GetItem(hSoundList, &lvitem);
      VPX::Sound *pps = (VPX::Sound *)lvitem.lParam;
		SoundPositionDialog spd(m_audioPlayer.get(), pps);

		if (spd.DoModal() == IDOK)
		{
			while (sel != -1)
			{
				lvitem.mask = LVIF_PARAM;
				lvitem.iItem = sel;
				lvitem.iSubItem = 0;
				ListView_GetItem(hSoundList, &lvitem);
				pps = (VPX::Sound *)lvitem.lParam;
				pps->SetOutputTarget(spd.m_cOutputTarget);
				pps->SetPan(spd.m_balance);
				pps->SetFrontRearFade(spd.m_fade);
				pps->SetVolume(spd.m_volume);

				pt->SetNonUndoableDirty(eSaveDirty);

				sel = ListView_GetNextItem(hSoundList, sel, LVNI_SELECTED); //next selected item
			}
			pt->ListSounds(hSoundList);
			SetFocus();
		}
	}
}


void SoundDialog::DeleteSound()
{
    CCO( PinTable ) * const pt = g_pvp->GetActiveTable();

    const int count = ListView_GetSelectedCount( hSoundList );
    if (count > 0)
    {
        const LocalString ls( IDS_REMOVESOUND );
        const int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2 );
        if (ans == IDYES)
        {
            int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
            while(sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                VPX::Sound *const pps = (VPX::Sound *)lvitem.lParam;
                ListView_DeleteItem( hSoundList, sel );
                pt->RemoveSound(pps);

                // The previous selection is now deleted, so look again from the top of the list
                sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
            }
        }
        pt->SetNonUndoableDirty( eSaveDirty );
    }
    SetFocus();
}

void SoundDialog::LoadPosition()
{
   const int x = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SoundMngPosX"s, 0);
   const int y = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SoundMngPosY"s, 0);
   const int w = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SoundMngWidth"s, 1000);
   const int h = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SoundMngHeight"s, 800);
   POINT p { x, y };
   if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
      SetWindowPos( nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE );
}

void SoundDialog::SavePosition()
{
    const CRect rect = GetWindowRect();
    g_pvp->m_settings.SaveValue(Settings::Editor, "SoundMngPosX"s, (int)rect.left);
    g_pvp->m_settings.SaveValue(Settings::Editor, "SoundMngPosY"s, (int)rect.top);
    const int w = rect.right - rect.left;
    g_pvp->m_settings.SaveValue(Settings::Editor, "SoundMngWidth"s, w);
    const int h = rect.bottom - rect.top;
    g_pvp->m_settings.SaveValue(Settings::Editor, "SoundMngHeight"s, h);
}

void SoundDialog::AddToolTip(const char *const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { 0 };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = (char *)text;
   ::SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

SoundPositionDialog::SoundPositionDialog(VPX::AudioPlayer *const AudioPlayer, VPX::Sound *const pps)
   : CDialog(IDD_SOUND_POSITION_DIALOG)
   , m_audioPlayer(AudioPlayer)
   , m_volume(pps->GetVolume())
   , m_fade(pps->GetFrontRearFade())
   , m_balance(pps->GetPan())
   , m_cOutputTarget(pps->GetOutputTarget())
   , m_pSound(pps)
{
}

SoundPositionDialog::~SoundPositionDialog()
{
   m_audioPlayer->StopSound(m_pSound);
}

void SoundPositionDialog::OnDestroy()
{
	CDialog::OnDestroy();
}

void SoundPositionDialog::OnClose()
{
	CDialog::OnClose();
}

BOOL SoundPositionDialog::OnInitDialog()
{
	m_Volume.Attach(GetDlgItem(IDC_AUD_VOLUME).GetHwnd());
	m_Volume.SetRangeMin(-100);
	m_Volume.SetRangeMax(100);
	m_Volume.SetTicFreq(25);
	m_Balance.Attach(GetDlgItem(IDC_AUD_BALANCE).GetHwnd());
	m_Balance.SetRangeMin(-100);
	m_Balance.SetRangeMax(100);
	m_Balance.SetTicFreq(25);
	m_Fader.Attach(GetDlgItem(IDC_AUD_FADER).GetHwnd());
	m_Fader.SetRangeMin(-100);
	m_Fader.SetRangeMax(100);
	m_Fader.SetTicFreq(25);
	SetSliderValues();
	SetTextValues();

	switch (m_cOutputTarget)
	{
   case VPX::SNDOUT_BACKGLASS:
		SendDlgItemMessage(IDC_SPT_BACKGLASS, BM_SETCHECK, BST_CHECKED, 0);
		break;
	default:
		assert(false);
   case VPX::SNDOUT_TABLE:
		SendDlgItemMessage(IDC_SPT_TABLE, BM_SETCHECK, BST_CHECKED, 0);
		break;
	}

	return TRUE;
}

void SoundPositionDialog::SetSliderValues()
{
	m_Volume.SetPos(m_volume, 1);
	m_Balance.SetPos(m_balance, 1);
	m_Fader.SetPos(m_fade, 1);
}

INT_PTR SoundPositionDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//const HWND hwndDlg = GetHwnd();
	//CCO(PinTable) * const pt = g_pvp->GetActiveTable();

	switch (uMsg)
	{
	case WM_HSCROLL:
	case WM_VSCROLL:
		switch (LOWORD(wParam))
		{
		case TB_ENDTRACK:
			ReadValuesFromSliders();
			SetTextValues();
			break;
		default:
			break;
		}
		break;
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case EN_KILLFOCUS:
			ReadTextValue(IDC_EDIT_BALANCE, m_balance);
			ReadTextValue(IDC_EDIT_FADER, m_fade);
			ReadTextValue(IDC_EDIT_VOL, m_volume);
			SetSliderValues();
			SetTextValues();
			break;
		}
		break;
	}
	return DialogProcDefault(uMsg, wParam, lParam);
}

void SoundPositionDialog::ReadTextValue(int item, int &oValue)
{
	float fval = sz2f(GetDlgItemText(item).GetString());
	if (fval >= -1.0f && fval <= 1.0f)
		oValue = quantizeSignedPercent(fval);
}

void SoundPositionDialog::SetTextValues()
{
	SetTextValue(IDC_EDIT_BALANCE, m_balance);
	SetTextValue(IDC_EDIT_FADER, m_fade);
	SetTextValue(IDC_EDIT_VOL, m_volume);
}

void SoundPositionDialog::SetTextValue(int ctl, int val)
{
	SetDlgItemText(ctl, f2sz(dequantizeSignedPercent(val)).c_str());
}


void SoundPositionDialog::GetDialogValues()
{
	m_cOutputTarget = IsDlgButtonChecked(IDC_SPT_BACKGLASS) ? VPX::SNDOUT_BACKGLASS : VPX::SNDOUT_TABLE;
}

void SoundPositionDialog::ReadValuesFromSliders()
{
	m_volume = m_Volume.GetPos();
	m_fade = m_Fader.GetPos();
	m_balance = m_Balance.GetPos();
}

BOOL SoundPositionDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (LOWORD(wParam))
	{
	case IDC_TEST:
		TestSound();
		break;
	case IDC_OK:
		GetDialogValues();
		CDialog::OnOK(); 
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

void SoundPositionDialog::TestSound()
{
	// Hold the actual settings temporarily and reinitialize, as it could be reset if dialog is canceled.
   const VPX::SoundOutTypes iOutputTargetTmp = m_pSound->GetOutputTarget();
	const int iVolume = m_pSound->GetVolume();
	const int iPan = m_pSound->GetPan();
	const int iFrontRearFade = m_pSound->GetFrontRearFade();

	GetDialogValues();

	m_pSound->SetOutputTarget(m_cOutputTarget);
	m_pSound->SetVolume(m_volume);
	m_pSound->SetPan(m_balance);
	m_pSound->SetFrontRearFade(m_fade);

   m_audioPlayer->PlaySound(m_pSound, 1.0f, 0.0f, 0, 0.f, 0.f, 0, false, false);

	m_pSound->SetOutputTarget(iOutputTargetTmp);
	m_pSound->SetVolume(iVolume);
	m_pSound->SetPan(iPan);
	m_pSound->SetFrontRearFade(iFrontRearFade);
}

void SoundPositionDialog::OnOK()
{
	// do not call CDialog::OnOk() here because if you rename sounds keys like backspace or escape in rename mode cause an IDOK message and this function is called
}

void SoundPositionDialog::OnCancel()
{
	CDialog::OnCancel();
}

/*int SoundPositionDialog::SliderToValue(const int Slider)
{
	return quantizeSignedPercent(powf(dequantizeSignedPercent(Slider), 10.0f));
}

int SoundPositionDialog::ValueToSlider(const int Value)
{
	return quantizeSignedPercent(powf(dequantizeSignedPercent(Value), (float)(1.0/10.0)));
}*/
