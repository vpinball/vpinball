#include "StdAfx.h"
#include "resource.h"
#include "SoundDialog.h"

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int columnSortOrder[4];
extern int CALLBACK MyCompProc( LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption );

SoundDialog::SoundDialog() : CDialog( IDD_SOUNDDIALOG )
{
    hSoundList = NULL;
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
    SavePosition();
    CDialog::OnClose();
}

BOOL SoundDialog::OnInitDialog()
{
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();
    hSoundList = GetDlgItem( IDC_SOUNDLIST ).GetHwnd();

    LoadPosition();

    LVCOLUMN lvcol;

    ListView_SetExtendedListViewStyle( hSoundList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
    lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
    LocalString ls( IDS_NAME );
    lvcol.pszText = ls.m_szbuffer;// = "Name";
    lvcol.cx = 100;
    ListView_InsertColumn( hSoundList, 0, &lvcol );

    LocalString ls2( IDS_IMPORTPATH );
    lvcol.pszText = ls2.m_szbuffer; // = "Import Path";
    lvcol.cx = 200;
    ListView_InsertColumn( hSoundList, 1, &lvcol );
    if ( pt )
      pt->ListSounds( hSoundList );
    return TRUE;
}

INT_PTR SoundDialog::DialogProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    HWND hwndDlg = GetHwnd();
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();

    switch(uMsg)
    {
        case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            if(wParam == IDC_SOUNDLIST)
            {
                LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
                if(lpnmListView->hdr.code == LVN_COLUMNCLICK)
                {
                    const int columnNumber = lpnmListView->iSubItem;
                    if(columnSortOrder[columnNumber] == 1)
                        columnSortOrder[columnNumber] = 0;
                    else
                        columnSortOrder[columnNumber] = 1;
                    SortData.hwndList = hSoundList;
                    SortData.subItemIndex = columnNumber;
                    SortData.sortUpDown = columnSortOrder[columnNumber];
                    ListView_SortItems( SortData.hwndList, MyCompProc, &SortData );
                }
            }
            switch(pnmhdr->code)
            {
                case LVN_ENDLABELEDIT:
                {
                    NMLVDISPINFO *pinfo = (NMLVDISPINFO *)lParam;
                    if(pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
                        return FALSE;
                    ListView_SetItemText( hSoundList, pinfo->item.iItem, 0, pinfo->item.pszText );
                    LVITEM lvitem;
                    lvitem.mask = LVIF_PARAM;
                    lvitem.iItem = pinfo->item.iItem;
                    lvitem.iSubItem = 0;
                    ListView_GetItem( hSoundList, &lvitem );
                    PinSound *pps = (PinSound *)lvitem.lParam;
                    strncpy_s( pps->m_szName, pinfo->item.pszText, MAXTOKEN );
                    strncpy_s( pps->m_szInternalName, pinfo->item.pszText, MAXTOKEN );
                    CharLowerBuff( pps->m_szInternalName, lstrlen( pps->m_szInternalName ) );
                    if (pt)
                        pt->SetNonUndoableDirty( eSaveDirty );
                    return TRUE;
                }
                break;

                case LVN_ITEMCHANGED:
                {
                    const int count = ListView_GetSelectedCount( hSoundList );
                    const int fEnable = !(count > 1);
                    ::EnableWindow( GetDlgItem(IDC_REIMPORTFROM).GetHwnd(), fEnable );
                    ::EnableWindow( GetDlgItem(IDC_RENAME).GetHwnd(), fEnable );
                    ::EnableWindow( GetDlgItem(IDC_PLAY).GetHwnd(), fEnable );
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
        case IDC_DELETE_SOUND: DeleteSound(); break;
        case IDC_OK: SavePosition(); CDialog::OnOK(); break;
        case IDC_PLAY:
        {
           CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
           const int sel = ListView_GetNextItem(hSoundList, -1, LVNI_SELECTED);
            if(sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                PinSound * const pps = (PinSound *)lvitem.lParam;
                pps->m_pDSBuffer->Play( 0, 0, 0 );
            }
            break;
        }
        case IDC_RENAME:
        {
            const int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
            if(sel != -1)
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
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();
    char szFileName[4096];
    char szInitialDir[4096];
    char szT[4096];

    szFileName[0] = '\0';

    OPENFILENAME ofn;
    ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
    ofn.lStructSize = sizeof( OPENFILENAME );
    ofn.hInstance = g_hinst;
    ofn.hwndOwner = g_pvp->m_hwnd;
    // TEXT
    ofn.lpstrFilter = "Sound Files (*.wav)\0*.wav\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = 4096;
    ofn.lpstrDefExt = "wav";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

    HRESULT hr = GetRegString( "RecentDir", "SoundDir", szInitialDir, 4096 );
    ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

    const int ret = GetOpenFileName( &ofn );

    if(ret)
    {
        strcpy_s( szInitialDir, sizeof( szInitialDir ), szFileName );

        int len = lstrlen( szFileName );
        if(len < ofn.nFileOffset)
        {
            // Multi-file select
            lstrcpy( szT, szFileName );
            lstrcat( szT, "\\" );
            len++;
            int filenamestart = ofn.nFileOffset;
            int filenamelen = lstrlen( &szFileName[filenamestart] );
            while(filenamelen > 0)
            {
                lstrcpy( &szT[len], &szFileName[filenamestart] );
                pt->ImportSound( hSoundList, szT, fFalse );
                filenamestart += filenamelen + 1;
                filenamelen = lstrlen( &szFileName[filenamestart] );
            }
        }
        else
        {
            szInitialDir[ofn.nFileOffset] = 0;
            if(pt)
               pt->ImportSound( hSoundList, szFileName, fTrue );
        }
        SetRegValue( "RecentDir", "SoundDir", REG_SZ, szInitialDir, lstrlen( szInitialDir ) );
        if (pt)
            pt->SetNonUndoableDirty( eSaveDirty );
    }
    SetFocus();
}

void SoundDialog::ReImport()
{
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();
    const int count = ListView_GetSelectedCount( hSoundList );
    if(count > 0)
    {
        LocalString ls( IDS_REPLACESOUND );
        const int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2 );
        if(ans == IDYES)
        {
            int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
            while(sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                PinSound * const pps = (PinSound *)lvitem.lParam;

                HANDLE hFile = CreateFile( pps->m_szPath, GENERIC_READ, FILE_SHARE_READ,
                                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

                if(hFile != INVALID_HANDLE_VALUE)
                {
                    CloseHandle( hFile );

                    pt->ReImportSound( hSoundList, pps, pps->m_szPath, count == 1 );
                    pt->SetNonUndoableDirty( eSaveDirty );
                }
                else MessageBox( pps->m_szPath, "  FILE NOT FOUND!  ", MB_OK );
                sel = ListView_GetNextItem( hSoundList, sel, LVNI_SELECTED );
            }
        }
        //pt->SetNonUndoableDirty(eSaveDirty);
    }
    SetFocus();
}

void SoundDialog::ReImportFrom()
{
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();
    const int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
    if(sel != -1)
    {
        char szFileName[1024];
        char szInitialDir[1024];

        LocalString ls( IDS_REPLACESOUND );
        int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to replace this sound with a new one?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2 );
        if(ans == IDYES)
        {
            szFileName[0] = '\0';

            OPENFILENAME ofn;
            ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
            ofn.lStructSize = sizeof( OPENFILENAME );
            ofn.hInstance = g_hinst;
            ofn.hwndOwner = g_pvp->m_hwnd;
            // TEXT
            ofn.lpstrFilter = "Sound Files (*.wav)\0*.wav\0";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = _MAX_PATH;
            ofn.lpstrDefExt = "wav";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

            HRESULT hr = GetRegString( "RecentDir", "SoundDir", szInitialDir, 1024 );
            ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

            const int ret = GetOpenFileName( &ofn );

            if(ret)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                PinSound *pps = (PinSound *)lvitem.lParam;

                pt->ReImportSound( hSoundList, pps, ofn.lpstrFile, fTrue );
                ListView_SetItemText( hSoundList, sel, 1, ofn.lpstrFile );
                pt->SetNonUndoableDirty( eSaveDirty );
            }
        }
    }
    SetFocus();
}

void SoundDialog::Export()
{
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();

    if(ListView_GetSelectedCount( hSoundList ))
    {
        OPENFILENAME ofn;
        LVITEM lvitem;
        int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED ); //next selected item 	
        if(sel != -1)
        {
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem( hSoundList, &lvitem );
            PinSound *pps = (PinSound *)lvitem.lParam;

            ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
            ofn.lStructSize = sizeof( OPENFILENAME );
            ofn.hInstance = g_hinst;
            ofn.hwndOwner = g_pvp->m_hwnd;
            //TEXT
            ofn.lpstrFilter = "Sound Files (*.wav)\0*.wav\0";
            char pathName[MAX_PATH] ={ 0 };

            int begin;		//select only file name from pathfilename
            int len = lstrlen( pps->m_szPath );
            memset( m_filename, 0, MAX_PATH );
            memset( m_initDir, 0, MAX_PATH );

            for(begin = len; begin >= 0; begin--)
            {
                if(pps->m_szPath[begin] == '\\')
                {
                    begin++;
                    break;
                }
            }
            memcpy( m_filename, &pps->m_szPath[begin], len - begin );
            ofn.lpstrFile = m_filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrDefExt = "wav";
            const HRESULT hr = GetRegString( "RecentDir", "SoundDir", m_initDir, MAX_PATH );

            if(hr == S_OK)ofn.lpstrInitialDir = m_initDir;
            else ofn.lpstrInitialDir = NULL;

            ofn.lpstrTitle = "SAVE AS";
            ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

            m_initDir[ofn.nFileOffset] = 0;
            if(GetSaveFileName( &ofn ))	//Get filename from user
            {
                len = lstrlen( ofn.lpstrFile );
                for(begin = len; begin >= 0; begin--)
                {
                    if(ofn.lpstrFile[begin] == '\\')
                    {
                        begin++;
                        break;
                    }
                }

                if(begin >= MAX_PATH)
                    begin=MAX_PATH - 1;

                memcpy( pathName, ofn.lpstrFile, begin );
                pathName[begin] = 0;
                while(sel != -1)
                {
                    len = lstrlen( pps->m_szPath );
                    for(begin = len; begin >= 0; begin--)
                    {
                        if(pps->m_szPath[begin] == '\\')
                        {
                            begin++;
                            break;
                        }
                    }
                    memset( m_filename, 0, MAX_PATH );
                    strcpy_s( m_filename, MAX_PATH, pathName );
                    memcpy( &m_filename[lstrlen( pathName )], &pps->m_szPath[begin], (len - begin) + 1 );
                    if(pt->ExportSound( hSoundList, pps, m_filename ))
                    {
                        //pt->ReImportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pps, ofn.lpstrFile, fTrue);
                        //pt->SetNonUndoableDirty(eSaveDirty);
                    }
                    sel = ListView_GetNextItem( hSoundList, sel, LVNI_SELECTED ); //next selected item
                    lvitem.iItem = sel;
                    lvitem.iSubItem = 0;
                    ListView_GetItem( hSoundList, &lvitem );
                    pps = (PinSound *)lvitem.lParam;

                }
                SetRegValue( "RecentDir", "SoundDir", REG_SZ, pathName, lstrlen( pathName ) );
            }
        }
    }
    SetFocus();
}

void SoundDialog::SoundToBG()
{
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();

    if(ListView_GetSelectedCount( hSoundList ))
    {
        LVITEM lvitem;
        int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED ); //next selected item 	
        while(sel != -1)
        {
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem( hSoundList, &lvitem );
            PinSound *pps = (PinSound *)lvitem.lParam;

            //strcpy_s(pps->m_szPath, "* Backglass Output *");
            pps->m_bToBackglassOutput ^= true;

            char pathName[MAX_PATH];
            memset( pathName, 0, MAX_PATH );
            if(pps->m_bToBackglassOutput)
            {
                strcpy_s( pathName, "*BG* " );
            }
            strcat_s( pathName, pps->m_szPath );
            ListView_SetItemText( hSoundList, sel, 1, pathName );
            pt->SetNonUndoableDirty( eSaveDirty );

            sel = ListView_GetNextItem(hSoundList, sel, LVNI_SELECTED ); //next selected item
        }
    }
    SetFocus();
}

void SoundDialog::DeleteSound()
{
    CCO( PinTable ) *pt = (CCO( PinTable ) *)g_pvp->GetActiveTable();

    const int count = ListView_GetSelectedCount( hSoundList );
    if(count > 0)
    {
        LocalString ls( IDS_REMOVESOUND );
        const int ans = MessageBox( ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2 );
        if(ans == IDYES)
        {
            int sel = ListView_GetNextItem( hSoundList, -1, LVNI_SELECTED );
            while(sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( hSoundList, &lvitem );
                PinSound *pps = (PinSound *)lvitem.lParam;
                pt->RemoveSound( pps );
                ListView_DeleteItem( hSoundList, sel );

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
    int x, y;
    HRESULT hr;

    hr = GetRegInt( "Editor", "SoundMngPosX", &x );
    if(hr != S_OK)
        x=0;
    hr = GetRegInt( "Editor", "SoundMngPosY", &y );
    if(hr != S_OK)
        y=0;

    SetWindowPos( NULL, x, y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
}

void SoundDialog::SavePosition()
{
    CRect rect = GetWindowRect();
    (void)SetRegValue( "Editor", "SoundMngPosX", REG_DWORD, &rect.left, 4 );
    (void)SetRegValue( "Editor", "SoundMngPosY", REG_DWORD, &rect.top, 4 );
}

