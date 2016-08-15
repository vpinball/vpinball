#include "StdAfx.h"
#include "resource.h"
#include "vpversion.h"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <fstream>
#include <sstream>
#include "svn_version.h"

using namespace rapidxml;

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int columnSortOrder[4];
extern int CALLBACK MyCompProc( LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption );

void DisableAllMaterialDialogItems( HWND hwndDlg )
{
    EnableWindow( GetDlgItem( hwndDlg, IDC_DIFFUSE_CHECK ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_DIFFUSE_EDIT ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_GLOSSY_EDIT ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_SPECULAR_EDIT ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_OPACITY_EDIT ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_OPACITY_CHECK ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_EDGEALPHA_EDIT ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_CLONE_BUTTON ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RENAME ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_IMPORT ), FALSE );
}

void EnableAllMaterialDialogItems( HWND hwndDlg )
{
    EnableWindow( GetDlgItem( hwndDlg, IDC_DIFFUSE_CHECK ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_DIFFUSE_EDIT ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_GLOSSY_EDIT ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_SPECULAR_EDIT ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_OPACITY_EDIT ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_OPACITY_CHECK ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_EDGEALPHA_EDIT ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_CLONE_BUTTON ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RENAME ), TRUE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_IMPORT ), TRUE );
}

INT_PTR CALLBACK MaterialManagerProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CCO( PinTable ) *pt = (CCO( PinTable ) *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
            LVCOLUMN lvcol;
            ListView_SetExtendedListViewStyle( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
            lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
            LocalString ls( IDS_NAME );
            lvcol.pszText = ls.m_szbuffer;// = "Name";
            lvcol.cx = 280;
            ListView_InsertColumn( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), 0, &lvcol );

            pt = (CCO( PinTable ) *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
            pt->ListMaterials( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
            ListView_SetItemState( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), 0, LVIS_SELECTED, LVIS_SELECTED )
                return TRUE;
        }
        case WM_CLOSE:
        {
            EndDialog( hwndDlg, FALSE );
            //       DestroyWindow(pt->m_hMaterialManager);
            //       pt->m_hMaterialManager = NULL;
            //       SetForegroundWindow(g_pvp->m_hwnd);
            break;
        }

        case GET_COLOR_TABLE:
        {
            *((unsigned long **)lParam) = pt->m_rgcolorcustom;
            return TRUE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            if ( wParam == IDC_MATERIAL_LIST )
            {
                LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
                if ( lpnmListView->hdr.code == LVN_COLUMNCLICK )
                {
                    const int columnNumber = lpnmListView->iSubItem;
                    if ( columnSortOrder[columnNumber] == 1 )
                        columnSortOrder[columnNumber] = 0;
                    else
                        columnSortOrder[columnNumber] = 1;
                    SortData.hwndList = GetDlgItem( hwndDlg, IDC_MATERIAL_LIST );
                    SortData.subItemIndex = columnNumber;
                    SortData.sortUpDown = columnSortOrder[columnNumber];
                    ListView_SortItems( SortData.hwndList, MyCompProc, &SortData );
                }
            }
            switch ( pnmhdr->code )
            {
                case LVN_ENDLABELEDIT:
                {
                    NMLVDISPINFO * const pinfo = (NMLVDISPINFO *)lParam;
                    if ( pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0' )
                    {
                        return FALSE;
                    }
                    LVITEM lvitem;
                    lvitem.mask = LVIF_PARAM;
                    lvitem.iItem = pinfo->item.iItem;
                    lvitem.iSubItem = 0;
                    ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                    Material *pmat = (Material*)lvitem.lParam;
                    if ( pt->IsMaterialNameUnique( pinfo->item.pszText ) )
                    {
                        strncpy_s( pmat->m_szName, pinfo->item.pszText, 31 );
                        ListView_SetItemText( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), pinfo->item.iItem, 0, pinfo->item.pszText );
                    }
                    else
                    {
                        char textBuf[32];
                        int suffix = 1;
                        do
                        {
                            sprintf_s( textBuf, "%s%i", pinfo->item.pszText, suffix );
                            suffix++;
                        }
                        while ( !pt->IsMaterialNameUnique( textBuf ) );
                        strncpy_s( pmat->m_szName, textBuf, 31 );
                        ListView_SetItemText( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), pinfo->item.iItem, 0, pmat->m_szName );
                    }
                    g_pvp->m_sb.PopulateDropdowns();
                    g_pvp->m_sb.RefreshProperties();
                    pt->SetNonUndoableDirty( eSaveDirty );
                    return TRUE;
                }
                case LVN_ITEMCHANGING:
                {
                    const int count = ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
                    if ( count > 1 )
                    {
                        DisableAllMaterialDialogItems( hwndDlg );
                        break;
                    }
                    EnableAllMaterialDialogItems( hwndDlg );

                    NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
                    if ( (plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED) )
                    {
                        if ( (plistview->uNewState & LVIS_SELECTED) == LVIS_SELECTED )
                        {
                            const int sel = plistview->iItem;
                            LVITEM lvitem;
                            lvitem.mask = LVIF_PARAM;
                            lvitem.iItem = sel;
                            lvitem.iSubItem = 0;
                            ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                            Material * const pmat = (Material*)lvitem.lParam;
                            HWND hwndColor = GetDlgItem( hwndDlg, IDC_COLOR );
                            SendMessage( hwndColor, CHANGE_COLOR, 0, pmat->m_cBase );
                            hwndColor = GetDlgItem( hwndDlg, IDC_COLOR2 );
                            SendMessage( hwndColor, CHANGE_COLOR, 0, pmat->m_cGlossy );
                            hwndColor = GetDlgItem( hwndDlg, IDC_COLOR3 );
                            SendMessage( hwndColor, CHANGE_COLOR, 0, pmat->m_cClearcoat );
                            char textBuf[256] = { 0 };
                            f2sz( pmat->m_fWrapLighting, textBuf );
                            SetDlgItemText( hwndDlg, IDC_DIFFUSE_EDIT, textBuf );
                            f2sz( pmat->m_fRoughness, textBuf );
                            SetDlgItemText( hwndDlg, IDC_GLOSSY_EDIT, textBuf );
                            f2sz( pmat->m_fEdge, textBuf );
                            SetDlgItemText( hwndDlg, IDC_SPECULAR_EDIT, textBuf );
                            f2sz( pmat->m_fOpacity, textBuf );
                            SetDlgItemText( hwndDlg, IDC_OPACITY_EDIT, textBuf );
                            f2sz( pmat->m_fEdgeAlpha, textBuf );
                            SetDlgItemText( hwndDlg, IDC_EDGEALPHA_EDIT, textBuf );

                            HWND checkboxHwnd = GetDlgItem( hwndDlg, IDC_DIFFUSE_CHECK );
                            SendMessage( checkboxHwnd, BM_SETCHECK, pmat->m_bIsMetal ? BST_CHECKED : BST_UNCHECKED, 0 );
                            checkboxHwnd = GetDlgItem( hwndDlg, IDC_OPACITY_CHECK );
                            SendMessage( checkboxHwnd, BM_SETCHECK, pmat->m_bOpacityActive ? BST_CHECKED : BST_UNCHECKED, 0 );

                            f2sz( pmat->m_fElasticity, textBuf );
                            SetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY, textBuf );
                            f2sz( pmat->m_fElasticityFalloff, textBuf );
                            SetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY_FALLOFF, textBuf );
                            f2sz( pmat->m_fFriction, textBuf );
                            SetDlgItemText( hwndDlg, IDC_MAT_FRICTION, textBuf );
                            f2sz( pmat->m_fScatterAngle, textBuf );
                            SetDlgItemText( hwndDlg, IDC_MAT_SCATTER_ANGLE, textBuf );

                            InvalidateRect( hwndColor, NULL, FALSE );
                        }
                    }
                }
                break;
                case LVN_ITEMCHANGED:
                {
                    const int count = ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );

                    if ( count > 1 )
                    {
                        DisableAllMaterialDialogItems( hwndDlg );
                        break;
                    }
                    EnableAllMaterialDialogItems( hwndDlg );
                    NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
                    const int sel = plistview->iItem;
                    LVITEM lvitem;
                    lvitem.mask = LVIF_PARAM;
                    lvitem.iItem = sel;
                    lvitem.iSubItem = 0;
                    ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                    Material * const pmat = (Material*)lvitem.lParam;
                    if ( (plistview->uNewState & LVIS_SELECTED) == 0 )
                    {
                        char textBuf[256] = { 0 };
                        float fv;
                        GetDlgItemText( hwndDlg, IDC_DIFFUSE_EDIT, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fWrapLighting != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fWrapLighting = fv;
                        GetDlgItemText( hwndDlg, IDC_GLOSSY_EDIT, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fRoughness != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fRoughness = fv;
                        GetDlgItemText( hwndDlg, IDC_SPECULAR_EDIT, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fEdge != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fEdge = fv;
                        GetDlgItemText( hwndDlg, IDC_OPACITY_EDIT, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fOpacity != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fOpacity = fv;
                        size_t checked = SendDlgItemMessage( hwndDlg, IDC_DIFFUSE_CHECK, BM_GETCHECK, 0, 0 );
                        if ( pmat->m_bIsMetal != (checked == 1) )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_bIsMetal = checked == 1;
                        checked = SendDlgItemMessage( hwndDlg, IDC_OPACITY_CHECK, BM_GETCHECK, 0, 0 );
                        if ( pmat->m_bOpacityActive != (checked == 1) )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_bOpacityActive = checked == 1;
                        GetDlgItemText( hwndDlg, IDC_EDGEALPHA_EDIT, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fEdgeAlpha != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fEdgeAlpha = fv;

                        GetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fElasticity != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fElasticity = fv;
                        GetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY_FALLOFF, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fElasticityFalloff != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fElasticityFalloff = fv;
                        GetDlgItemText( hwndDlg, IDC_MAT_FRICTION, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fFriction != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fFriction = fv;
                        GetDlgItemText( hwndDlg, IDC_MAT_SCATTER_ANGLE, textBuf, 31 );
                        fv = sz2f( textBuf );
                        if ( pmat->m_fScatterAngle != fv )
                            pt->SetNonUndoableDirty( eSaveDirty );
                        pmat->m_fScatterAngle = fv;
                    }
                    else if ( (plistview->uOldState & LVIS_SELECTED) == 0 )
                    {
                        HWND hwndColor = GetDlgItem( hwndDlg, IDC_COLOR );
                        SendMessage( hwndColor, CHANGE_COLOR, 0, pmat->m_cBase );
                        hwndColor = GetDlgItem( hwndDlg, IDC_COLOR2 );
                        SendMessage( hwndColor, CHANGE_COLOR, 0, pmat->m_cGlossy );
                        hwndColor = GetDlgItem( hwndDlg, IDC_COLOR3 );
                        SendMessage( hwndColor, CHANGE_COLOR, 0, pmat->m_cClearcoat );
                        char textBuf[256] = { 0 };
                        f2sz( pmat->m_fWrapLighting, textBuf );
                        SetDlgItemText( hwndDlg, IDC_DIFFUSE_EDIT, textBuf );
                        f2sz( pmat->m_fRoughness, textBuf );
                        SetDlgItemText( hwndDlg, IDC_GLOSSY_EDIT, textBuf );
                        f2sz( pmat->m_fEdge, textBuf );
                        SetDlgItemText( hwndDlg, IDC_SPECULAR_EDIT, textBuf );
                        f2sz( pmat->m_fOpacity, textBuf );
                        SetDlgItemText( hwndDlg, IDC_OPACITY_EDIT, textBuf );
                        HWND checkboxHwnd = GetDlgItem( hwndDlg, IDC_DIFFUSE_CHECK );
                        SendMessage( checkboxHwnd, BM_SETCHECK, pmat->m_bIsMetal ? BST_CHECKED : BST_UNCHECKED, 0 );
                        checkboxHwnd = GetDlgItem( hwndDlg, IDC_OPACITY_CHECK );
                        SendMessage( checkboxHwnd, BM_SETCHECK, pmat->m_bOpacityActive ? BST_CHECKED : BST_UNCHECKED, 0 );
                        f2sz( pmat->m_fEdgeAlpha, textBuf );
                        SetDlgItemText( hwndDlg, IDC_EDGEALPHA_EDIT, textBuf );

                        f2sz( pmat->m_fElasticity, textBuf );
                        SetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY, textBuf );
                        f2sz( pmat->m_fElasticityFalloff, textBuf );
                        SetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY_FALLOFF, textBuf );
                        f2sz( pmat->m_fFriction, textBuf );
                        SetDlgItemText( hwndDlg, IDC_MAT_FRICTION, textBuf );
                        f2sz( pmat->m_fScatterAngle, textBuf );
                        SetDlgItemText( hwndDlg, IDC_MAT_SCATTER_ANGLE, textBuf );
                    }
                    //EnableWindow(GetDlgItem(hwndDlg, IDC_EXPORT), fEnable);
                }
                break;
            }
        }
        break;

        case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT * const pdis = (DRAWITEMSTRUCT *)lParam;
            const int sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
            if ( sel != -1 )
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
            }
            else
            {
                // Nothing currently selected
            }
            return TRUE;
        }
        case WM_COMMAND:
        {
            switch ( HIWORD( wParam ) )
            {
                case COLOR_CHANGED:
                {
                    const int count = ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
                    if ( count > 0 )
                    {
                        const size_t color = GetWindowLongPtr( (HWND)lParam, GWLP_USERDATA );
                        HWND hwndcolor1 = GetDlgItem( hwndDlg, IDC_COLOR );
                        HWND hwndcolor2 = GetDlgItem( hwndDlg, IDC_COLOR2 );
                        HWND hwndcolor3 = GetDlgItem( hwndDlg, IDC_COLOR3 );
                        int sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
                        while ( sel != -1 )
                        {
                            LVITEM lvitem;
                            lvitem.mask = LVIF_PARAM;
                            lvitem.iItem = sel;
                            lvitem.iSubItem = 0;
                            ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                            Material * const pmat = (Material*)lvitem.lParam;
                            if ( hwndcolor1 == (HWND)lParam )
                                pmat->m_cBase = (COLORREF)color;
                            else if ( hwndcolor2 == (HWND)lParam )
                                pmat->m_cGlossy = (COLORREF)color;
                            else if ( hwndcolor3 == (HWND)lParam )
                                pmat->m_cClearcoat = (COLORREF)color;

                            // The previous selection is now deleted, so look again from the top of the list
                            sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), sel, LVNI_SELECTED );
                        }

                        pt->SetNonUndoableDirty( eSaveDirty );
                    }
                    break;
                }

                case BN_CLICKED:
                {
                    switch ( LOWORD( wParam ) )
                    {
                        case IDOK:
                        {
                            const int count = ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
                            if ( count > 0 )
                            {
                                int sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
                                while ( sel != -1 )
                                {
                                    LVITEM lvitem;
                                    lvitem.mask = LVIF_PARAM;
                                    lvitem.iItem = sel;
                                    lvitem.iSubItem = 0;
                                    ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                                    Material * const pmat = (Material*)lvitem.lParam;
                                    char textBuf[256];
                                    float fv;
                                    GetDlgItemText( hwndDlg, IDC_DIFFUSE_EDIT, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fWrapLighting != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fWrapLighting = fv;
                                    GetDlgItemText( hwndDlg, IDC_GLOSSY_EDIT, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fRoughness != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fRoughness = fv;
                                    GetDlgItemText( hwndDlg, IDC_SPECULAR_EDIT, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fEdge != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fEdge = fv;
                                    GetDlgItemText( hwndDlg, IDC_OPACITY_EDIT, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fOpacity != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fOpacity = fv;
                                    size_t checked = SendDlgItemMessage( hwndDlg, IDC_DIFFUSE_CHECK, BM_GETCHECK, 0, 0 );
                                    if ( pmat->m_bIsMetal != (checked == 1) )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_bIsMetal = checked == 1;
                                    checked = SendDlgItemMessage( hwndDlg, IDC_OPACITY_CHECK, BM_GETCHECK, 0, 0 );
                                    if ( pmat->m_bOpacityActive != (checked == 1) )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_bOpacityActive = checked == 1;
                                    GetDlgItemText( hwndDlg, IDC_EDGEALPHA_EDIT, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fEdgeAlpha != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fEdgeAlpha = fv;

                                    GetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fElasticity != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fElasticity = fv;
                                    GetDlgItemText( hwndDlg, IDC_MAT_ELASTICITY_FALLOFF, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fElasticityFalloff != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fElasticityFalloff = fv;
                                    GetDlgItemText( hwndDlg, IDC_MAT_FRICTION, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fFriction != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fFriction = fv;
                                    GetDlgItemText( hwndDlg, IDC_MAT_SCATTER_ANGLE, textBuf, 31 );
                                    fv = sz2f( textBuf );
                                    if ( pmat->m_fScatterAngle != fv )
                                        pt->SetNonUndoableDirty( eSaveDirty );
                                    pmat->m_fScatterAngle = fv;

                                    // The previous selection is now deleted, so look again from the top of the list
                                    sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), sel, LVNI_SELECTED );
                                }
                            }
                            SendMessage( hwndDlg, WM_CLOSE, 0, 0 );
                            break;
                        }
                        case IDCANCEL:
                        {
                            SendMessage( hwndDlg, WM_CLOSE, 0, 0 );
                            return TRUE;
                        }
                        case IDC_CLONE_BUTTON:
                        {
                            if ( ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) ) )	// if some items are selected???
                            {
                                int sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
                                int selCount = ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
                                if ( sel == -1 )
                                    break;

                                while ( sel != -1 )
                                {
                                    LVITEM lvitem;
                                    lvitem.mask = LVIF_PARAM;
                                    lvitem.iItem = sel;
                                    lvitem.iSubItem = 0;
                                    ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                                    Material *pNewMat = new Material();
                                    Material * const pmat = (Material*)lvitem.lParam;
                                    pNewMat->m_bIsMetal = pmat->m_bIsMetal;
                                    pNewMat->m_bOpacityActive = pmat->m_bOpacityActive;
                                    pNewMat->m_cBase = pmat->m_cBase;
                                    pNewMat->m_cClearcoat = pmat->m_cClearcoat;
                                    pNewMat->m_cGlossy = pmat->m_cGlossy;
                                    pNewMat->m_fEdge = pmat->m_fEdge;
                                    pNewMat->m_fEdgeAlpha = pmat->m_fEdgeAlpha;
                                    pNewMat->m_fOpacity = pmat->m_fOpacity;
                                    pNewMat->m_fRoughness = pmat->m_fRoughness;
                                    pNewMat->m_fWrapLighting = pmat->m_fWrapLighting;
                                    memcpy( pNewMat->m_szName, pmat->m_szName, 32 );

                                    pNewMat->m_fElasticity = pmat->m_fElasticity;
                                    pNewMat->m_fElasticityFalloff = pmat->m_fElasticityFalloff;
                                    pNewMat->m_fFriction = pmat->m_fFriction;
                                    pNewMat->m_fScatterAngle = pmat->m_fScatterAngle;
                                    pt->AddMaterial( pNewMat );
                                    pt->AddListMaterial( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), pNewMat );

                                    sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), sel, LVNI_SELECTED );
                                }
                                g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
                                g_pvp->m_sb.RefreshProperties();
                                pt->SetNonUndoableDirty( eSaveDirty );
                            }
                            break;
                        }
                        case IDC_ADD_BUTTON:
                        {
                            Material *pmat = new Material();
                            pt = (CCO( PinTable ) *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

                            pt->AddMaterial( pmat );
                            pt->AddListMaterial( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), pmat );
                            g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
                            g_pvp->m_sb.RefreshProperties();
                            pt->SetNonUndoableDirty( eSaveDirty );

                            break;
                        }
                        case IDC_IMPORT:
                        {
                            char szFileName[10240];
                            char szInitialDir[10240];
                            szFileName[0] = '\0';

                            OPENFILENAME ofn;
                            ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
                            ofn.lStructSize = sizeof( OPENFILENAME );
                            ofn.hInstance = g_hinst;
                            ofn.hwndOwner = g_pvp->m_hwnd;

                            ofn.lpstrFilter = "Material Files (.mat)\0*.mat\0";
                            ofn.lpstrFile = szFileName;
                            ofn.nMaxFile = 10240;
                            ofn.lpstrDefExt = "mat";
                            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER;

                            HRESULT hr = GetRegString( "RecentDir", "MaterialDir", szInitialDir, 1024 );
                            ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

                            const int ret = GetOpenFileName( &ofn );

                            if ( ret )
                            {
                                int materialCount = 0;
                                int versionNumber = 0;
                                FILE *f;
                                fopen_s( &f, ofn.lpstrFile, "rb" );

                                fread( &versionNumber, 1, 4, f );
                                if ( versionNumber != 1 )
                                {
                                    ShowError( "Materials are not compatible with this version!" );
                                    fclose( f );
                                    break;
                                }
                                fread( &materialCount, 1, 4, f );
                                for ( int i = 0; i < materialCount; i++ )
                                {
                                    Material *pmat = new Material();
                                    SaveMaterial mat;
                                    float physicsValue;

                                    fread( &mat, 1, sizeof( SaveMaterial ), f );
                                    pmat->m_cBase = mat.cBase;
                                    pmat->m_cGlossy = mat.cGlossy;
                                    pmat->m_cClearcoat = mat.cClearcoat;
                                    pmat->m_fWrapLighting = mat.fWrapLighting;
                                    pmat->m_fRoughness = mat.fRoughness;
                                    pmat->m_fEdge = mat.fEdge;
                                    pmat->m_bIsMetal = mat.bIsMetal;
                                    pmat->m_fOpacity = mat.fOpacity;
                                    pmat->m_bOpacityActive = !!(mat.bOpacityActive_fEdgeAlpha & 1);
                                    pmat->m_fEdgeAlpha = (float)(mat.bOpacityActive_fEdgeAlpha >> 1)*(float)(1.0 / 127.0); //!! + rounding offset?
                                    memcpy( pmat->m_szName, mat.szName, 32 );
                                    
                                    fread( &physicsValue, 1, sizeof(float), f);
                                    pmat->m_fElasticity = physicsValue;
                                    fread( &physicsValue, 1, sizeof( float ), f );
                                    pmat->m_fElasticityFalloff = physicsValue;
                                    fread( &physicsValue, 1, sizeof( float ), f );
                                    pmat->m_fFriction = physicsValue;
                                    fread( &physicsValue, 1, sizeof( float ), f );
                                    pmat->m_fScatterAngle = physicsValue;

                                    pt = (CCO( PinTable ) *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

                                    pt->AddMaterial( pmat );
                                    pt->AddListMaterial( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), pmat );
                                }
                                fclose( f );
                                SetRegValue( "RecentDir", "MaterialDir", REG_SZ, szInitialDir, lstrlen( szInitialDir ) );
                                pt->SetNonUndoableDirty( eSaveDirty );
                                g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
                                g_pvp->m_sb.RefreshProperties();
                            }
                            break;
                        }

                        case IDC_RENAME:
                        {
                            const int sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
                            if ( sel != -1 )
                            {
                                SetFocus( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
                                ListView_EditLabel( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), sel );
                            }
                        }
                        break;

                        case IDC_EXPORT:
                        {
                            if ( ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) ) )	// if some items are selected???
                            {
                                char szFileName[10240];
                                char szInitialDir[2096];
                                int sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
                                int selCount = ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
                                if ( sel == -1 )
                                    break;

                                strcpy_s( szFileName, "Materials.mat" );
                                OPENFILENAME ofn;
                                ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
                                ofn.lStructSize = sizeof( OPENFILENAME );
                                ofn.hInstance = g_hinst;
                                ofn.hwndOwner = g_pvp->m_hwnd;
                                ofn.lpstrFile = szFileName;
                                //TEXT
                                ofn.lpstrFilter = "Material Files (.mat)\0*.mat\0";
                                ofn.nMaxFile = 2096;
                                ofn.lpstrDefExt = "mat";

                                const HRESULT hr = GetRegString( "RecentDir", "MaterialDir", szInitialDir, 2096 );

                                if ( hr == S_OK )ofn.lpstrInitialDir = szInitialDir;
                                else ofn.lpstrInitialDir = NULL;

                                ofn.lpstrTitle = "Export materials";
                                ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

                                szInitialDir[ofn.nFileOffset] = 0;

                                if ( GetSaveFileName( &ofn ) )	//Get filename from user
                                {
                                    FILE *f;
                                    fopen_s( &f, ofn.lpstrFile, "wb" );
                                    const int MATERIAL_VERSION = 1;
                                    fwrite( &MATERIAL_VERSION, 1, 4, f );
                                    fwrite( &selCount, 1, 4, f );
                                    while ( sel != -1 )
                                    {
                                        LVITEM lvitem;
                                        lvitem.mask = LVIF_PARAM;
                                        lvitem.iItem = sel;
                                        lvitem.iSubItem = 0;
                                        ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                                        Material * const pmat = (Material*)lvitem.lParam;
                                        SaveMaterial mat;
                                        mat.cBase = pmat->m_cBase;
                                        mat.cGlossy = pmat->m_cGlossy;
                                        mat.cClearcoat = pmat->m_cClearcoat;
                                        mat.fRoughness = pmat->m_fRoughness;
                                        mat.fEdge = pmat->m_fEdge;
                                        mat.fWrapLighting = pmat->m_fWrapLighting;
                                        mat.bIsMetal = pmat->m_bIsMetal;
                                        mat.fOpacity = pmat->m_fOpacity;
                                        mat.bOpacityActive_fEdgeAlpha = pmat->m_bOpacityActive ? 1 : 0;
                                        mat.bOpacityActive_fEdgeAlpha |= ((unsigned char)(clamp( pmat->m_fEdgeAlpha, 0.f, 1.f )*127.f)) << 1;
                                        memcpy( mat.szName, pmat->m_szName, 32 );
                                        fwrite( &mat, 1, sizeof( SaveMaterial ), f );
                                        fwrite( &pmat->m_fElasticity, 1, sizeof( float ), f );
                                        fwrite( &pmat->m_fElasticityFalloff, 1, sizeof( float ), f );
                                        fwrite( &pmat->m_fFriction, 1, sizeof( float ), f );
                                        fwrite( &pmat->m_fScatterAngle, 1, sizeof( float ), f );

                                        sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), sel, LVNI_SELECTED );
                                    }
                                    fclose( f );
                                }
                                SetRegValue( "RecentDir", "MaterialDir", REG_SZ, szInitialDir, lstrlen( szInitialDir ) );
                            }
                        }
                        break;

                        case IDC_DELETE_MATERIAL:
                        {
                            const int count = ListView_GetSelectedCount( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ) );
                            if ( count > 0 )
                            {
                                LocalString ls( IDS_REMOVEMATERIAL );
                                const int ans = MessageBox( hwndDlg, ls.m_szbuffer/*"Are you sure you want to remove this material?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2 );
                                if ( ans == IDYES )
                                {
                                    pt = (CCO( PinTable ) *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
                                    int sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
                                    while ( sel != -1 )
                                    {
                                        LVITEM lvitem;
                                        lvitem.mask = LVIF_PARAM;
                                        lvitem.iItem = sel;
                                        lvitem.iSubItem = 0;
                                        ListView_GetItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), &lvitem );
                                        ListView_DeleteItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), sel );
                                        Material * const pmat = (Material*)lvitem.lParam;
                                        pt->RemoveMaterial( pmat );
                                        // The previous selection is now deleted, so look again from the top of the list
                                        sel = ListView_GetNextItem( GetDlgItem( hwndDlg, IDC_MATERIAL_LIST ), -1, LVNI_SELECTED );
                                    }
                                }
                                pt->SetNonUndoableDirty( eSaveDirty );
                                g_pvp->m_sb.PopulateDropdowns();
                                g_pvp->m_sb.RefreshProperties();
                            }
                        }
                        break;
                    }
                    break;
                }
            }
            break;
        }
    }

    return FALSE;
}
