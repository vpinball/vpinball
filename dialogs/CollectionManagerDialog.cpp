#include "StdAfx.h"
#include "resource.h"
#include "CollectionManagerDialog.h"

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int columnSortOrder[4];
extern int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);

static CollectionDialogStruct cds;

CollectionManagerDialog::CollectionManagerDialog() : CDialog(IDD_COLLECTDIALOG)
{
    hListHwnd = NULL;
}

BOOL CollectionManagerDialog::OnInitDialog()
{
    CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();

    hListHwnd = GetDlgItem(IDC_SOUNDLIST).GetHwnd();

    LoadPosition();

    ListView_SetExtendedListViewStyle(hListHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMN lvcol;
    lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
    LocalString ls(IDS_NAME);
    lvcol.pszText = ls.m_szbuffer;// = "Name";
    lvcol.cx = 330;
    ListView_InsertColumn(hListHwnd, 0, &lvcol);
    
    pt->ListCollections(hListHwnd);

    return TRUE;
}


void CollectionManagerDialog::EditCollection()
{
    CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();

    const int sel = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
    if(sel != -1)
    {
        LVITEM lvitem;
        lvitem.mask = LVIF_PARAM;
        lvitem.iItem = sel;
        lvitem.iSubItem = 0;
        ListView_GetItem(hListHwnd, &lvitem);
        CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem.lParam;

        cds.pcol = pcol;
        cds.ppt = pt;

        CollectionDialog *colDlg = new CollectionDialog(cds);
        if(colDlg->DoModal() >= 0)
            pt->SetNonUndoableDirty(eSaveDirty);

        char szT[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
        ListView_SetItemText(hListHwnd, sel, 0, szT);
    }
}

INT_PTR CollectionManagerDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();

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
                    SortData.hwndList = hListHwnd;
                    SortData.subItemIndex = columnNumber;
                    SortData.sortUpDown = columnSortOrder[columnNumber];
                    ListView_SortItems(SortData.hwndList, MyCompProc, &SortData);
                }
            }
            if(pnmhdr->code == LVN_ENDLABELEDIT)
            {
                NMLVDISPINFO *pinfo = (NMLVDISPINFO *)lParam;
                if(pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
                    return FALSE;
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = pinfo->item.iItem;
                lvitem.iSubItem = 0;
                ListView_GetItem(hListHwnd, &lvitem);
                Collection * const pcol = (Collection *)lvitem.lParam;
                pt->SetCollectionName(pcol, pinfo->item.pszText, hListHwnd, pinfo->item.iItem);
                pt->SetNonUndoableDirty(eSaveDirty);
                return TRUE;
            }
            else if(pnmhdr->code == NM_DBLCLK)
            {
                EditCollection();
                return TRUE;
            }
            break;
        }
    }

    return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL CollectionManagerDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
    UNREFERENCED_PARAMETER(lParam);

    switch(LOWORD(wParam))
    {
        case IDCLOSE:
        {
            OnClose();
            break;
        }
        case IDC_NEW:
        {
            pt->NewCollection(hListHwnd, fFalse);
            pt->SetNonUndoableDirty(eSaveDirty);
            break;
        }
        case IDC_CREATEFROMSELECTION:
        {
            pt->NewCollection(hListHwnd, fTrue);
            pt->SetNonUndoableDirty(eSaveDirty);
            break;
        }
        case IDC_EDIT:
        {
            EditCollection();
            break;
        }
        case IDC_RENAME:
        {
            const int sel = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
            if(sel != -1)
            {
                ::SetFocus(hListHwnd);
                ListView_EditLabel(hListHwnd, sel);
            }
            break;
        }
        case IDC_COL_UP_BUTTON:
        {
            const int idx = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
            if(idx != -1 && idx > 0)
            {
                ::SetFocus(hListHwnd);
                LVITEM lvitem1;
                lvitem1.mask = LVCF_TEXT | LVIF_PARAM;
                lvitem1.iItem = idx;
                lvitem1.iSubItem = 0;
                ListView_GetItem(hListHwnd, &lvitem1);
                CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem1.lParam;
                pt->MoveCollectionUp(pcol);
                ListView_DeleteItem(hListHwnd, idx);
                lvitem1.mask = LVIF_PARAM;
                lvitem1.iItem = idx - 1;
                ListView_InsertItem(hListHwnd, &lvitem1);
                char szT[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
                ListView_SetItemText(hListHwnd, idx - 1, 0, szT);
                ListView_SetItemState(hListHwnd, -1, 0, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx - 1, LVIS_SELECTED, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx - 1, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            break;
        }
        case IDC_COL_DOWN_BUTTON:
        {
            const int idx = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
            if(idx != -1 && (idx < pt->m_vcollection.Size() - 1))
            {
                ::SetFocus(hListHwnd);
                LVITEM lvitem1;
                lvitem1.mask = LVCF_TEXT | LVIF_PARAM;
                lvitem1.iItem = idx;
                lvitem1.iSubItem = 0;
                ListView_GetItem(hListHwnd, &lvitem1);
                CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem1.lParam;
                pt->MoveCollectionDown(pcol);
                ListView_DeleteItem(hListHwnd, idx);
                lvitem1.mask = LVIF_PARAM;
                lvitem1.iItem = idx + 1;
                ListView_InsertItem(hListHwnd, &lvitem1);
                char szT[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
                ListView_SetItemText(hListHwnd, idx + 1, 0, szT);
                ListView_SetItemState(hListHwnd, -1, 0, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx + 1, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            break;
        }
        case IDC_DELETE_COLLECTION:
        {
            const int sel = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
            if(sel != -1)
            {
                // TEXT
                const int ans = MessageBox("Are you sure you want to remove this collection?", "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
                if(ans == IDYES)
                {
                    LVITEM lvitem;
                    lvitem.mask = LVIF_PARAM;
                    lvitem.iItem = sel;
                    lvitem.iSubItem = 0;
                    ListView_GetItem(hListHwnd, &lvitem);
                    CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem.lParam;
                    pt->RemoveCollection(pcol);
                    ListView_DeleteItem(hListHwnd, sel);
                    pt->SetNonUndoableDirty(eSaveDirty);
                }
            }
            break;
        }

        default:
            return FALSE;
    }
    return TRUE;
}

void CollectionManagerDialog::OnOK()
{
    // do not call CDialog::OnOk() here because if you rename sounds keys like backspace or escape in rename mode cause an IDOK message and this function is called
}

void CollectionManagerDialog::OnClose()
{
    SavePosition();
    CDialog::OnClose();
}

void CollectionManagerDialog::OnCancel()
{
    SavePosition();
    CDialog::OnCancel();
}

void CollectionManagerDialog::LoadPosition()
{
    int x, y;
    HRESULT hr;

    hr = GetRegInt("Editor", "CollectionMngPosX", &x);
    if(hr != S_OK)
        x=0;
    hr = GetRegInt("Editor", "CollectionMngPosY", &y);
    if(hr != S_OK)
        y=0;

    SetWindowPos(NULL, x, y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CollectionManagerDialog::SavePosition()
{
    CRect rect = GetWindowRect();
    (void)SetRegValue("Editor", "CollectionMngPosX", REG_DWORD, &rect.left, 4);
    (void)SetRegValue("Editor", "CollectionMngPosY", REG_DWORD, &rect.top, 4);
}

//######################################## Collection Dialog ########################################

CollectionDialog::CollectionDialog(CollectionDialogStruct &pcds) : CDialog(IDD_COLLECTION), pCurCollection(pcds)
{
}

BOOL CollectionDialog::OnInitDialog()
{
    Collection * const pcol = pCurCollection.pcol;

    HWND hwndName = GetDlgItem(IDC_NAME).GetHwnd();

    char szT[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);

    ::SetWindowText(hwndName, szT);

    HWND hwndFireEvents = GetDlgItem(IDC_FIRE).GetHwnd();
    ::SendMessage(hwndFireEvents, BM_SETCHECK, pcol->m_fFireEvents ? BST_CHECKED : BST_UNCHECKED, 0);

    HWND hwndStopSingle = GetDlgItem(IDC_SUPPRESS).GetHwnd();
    ::SendMessage(hwndStopSingle, BM_SETCHECK, pcol->m_fStopSingleEvents ? BST_CHECKED : BST_UNCHECKED, 0);

    HWND hwndGroupElements = GetDlgItem(IDC_GROUP_CHECK).GetHwnd();
    ::SendMessage(hwndGroupElements, BM_SETCHECK, pcol->m_fGroupElements ? BST_CHECKED : BST_UNCHECKED, 0);

    HWND hwndOut = GetDlgItem(IDC_OUTLIST).GetHwnd();
    HWND hwndIn = GetDlgItem(IDC_INLIST).GetHwnd();

    for(int i = 0; i < pcol->m_visel.Size(); i++)
    {
        ISelect * const pisel = pcol->m_visel.ElementAt(i);
        IEditable * const piedit = pisel->GetIEditable();
        IScriptable * const piscript = piedit->GetScriptable();
        if(piscript)
        {
            WideCharToMultiByte(CP_ACP, 0, piscript->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
            const size_t index = ::SendMessage(hwndIn, LB_ADDSTRING, 0, (size_t)szT);
            ::SendMessage(hwndIn, LB_SETITEMDATA, index, (size_t)piscript);
        }
    }

    PinTable * const ppt = pCurCollection.ppt;

    for(int i = 0; i < ppt->m_vedit.Size(); i++)
    {
        IEditable * const piedit = ppt->m_vedit.ElementAt(i);
        IScriptable * const piscript = piedit->GetScriptable();
        ISelect * const pisel = piedit->GetISelect();

        // Only process objects not in this collection
        int l;
        for(l = 0; l < pcol->m_visel.Size(); l++)
        {
            if(pisel == pcol->m_visel.ElementAt(l))
            {
                break;
            }
        }

        if((l == pcol->m_visel.Size()) && piscript)
            //if (!piedit->m_pcollection)
        {
            WideCharToMultiByte(CP_ACP, 0, piscript->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
            const size_t index = ::SendMessage(hwndOut, LB_ADDSTRING, 0, (size_t)szT);
            ::SendMessage(hwndOut, LB_SETITEMDATA, index, (size_t)piscript);
        }
    }

    return TRUE;
}

BOOL CollectionDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
    UNREFERENCED_PARAMETER(lParam);

    switch(LOWORD(wParam))
    {
        case IDC_UP:
        case IDC_DOWN:
        {
            // Mode items up or down in the collection list
            HWND hwndList = GetDlgItem(IDC_INLIST).GetHwnd();
            const size_t listsize = ::SendMessage(hwndList, LB_GETCOUNT, 0, 0);
            const size_t count = ::SendMessage(hwndList, LB_GETSELCOUNT, 0, 0);
            int * const rgsel = new int[count]; //!! size_t?
            ::SendMessage(hwndList, LB_GETSELITEMS, count, (LPARAM)rgsel);

            for(size_t loop = 0; loop < count; loop++)
                //for (i=count-1;i>=0;i--)
            {
                const size_t i = (LOWORD(wParam) == IDC_UP) ? loop : (count - loop - 1);

                const size_t len = ::SendMessage(hwndList, LB_GETTEXTLEN, rgsel[i], 0);
                char * const szT = new char[len + 1]; // include null terminator
                ::SendMessage(hwndList, LB_GETTEXT, rgsel[i], (LPARAM)szT);
                const size_t data = ::SendMessage(hwndList, LB_GETITEMDATA, rgsel[i], 0);

                const int newindex = (LOWORD(wParam) == IDC_UP) ? max(rgsel[i] - 1, (int)i) : min(rgsel[i] + 2, (int)(listsize - (count - 1) + i)); //!! see above
                int oldindex = rgsel[i];

                if(oldindex > newindex)
                    oldindex++; // old item will be one lower when we try to delete it

                const size_t index = ::SendMessage(hwndList, LB_INSERTSTRING, newindex, (LPARAM)szT);
                ::SendMessage(hwndList, LB_SETITEMDATA, index, data);
                // Set the new value to be selected, like the old one was
                ::SendMessage(hwndList, LB_SETSEL, TRUE, index);
                // Delete the old value
                ::SendMessage(hwndList, LB_DELETESTRING, oldindex, 0);
                delete[] szT;
            }
            delete[] rgsel;
            break;
        }

        case IDC_IN:
        case IDC_OUT:
        {
            HWND hwndOut;
            HWND hwndIn;

            if(LOWORD(wParam) == IDC_IN)
            {
                hwndOut = GetDlgItem(IDC_OUTLIST).GetHwnd();
                hwndIn = GetDlgItem(IDC_INLIST).GetHwnd();
            }
            else
            {
                hwndOut = GetDlgItem(IDC_INLIST).GetHwnd();
                hwndIn = GetDlgItem(IDC_OUTLIST).GetHwnd();
            }

            const size_t count = ::SendMessage(hwndOut, LB_GETSELCOUNT, 0, 0);
            int * const rgsel = new int[count];
            ::SendMessage(hwndOut, LB_GETSELITEMS, count, (LPARAM)rgsel);
            for(size_t i = 0; i < count; i++)
            {
                const size_t len = ::SendMessage(hwndOut, LB_GETTEXTLEN, rgsel[i], 0);
                char * const szT = new char[len + 1]; // include null terminator
                ::SendMessage(hwndOut, LB_GETTEXT, rgsel[i], (LPARAM)szT);
                const size_t data = ::SendMessage(hwndOut, LB_GETITEMDATA, rgsel[i], 0);

                const size_t index = ::SendMessage(hwndIn, LB_ADDSTRING, 0, (LPARAM)szT);
                ::SendMessage(hwndIn, LB_SETITEMDATA, index, data);
                delete[] szT;
            }

            // Remove the old strings after everything else, to avoid messing up indices
            // Remove things in reverse order, so we don't get messed up inside this loop
            for(size_t i = 0; i < count; i++)
                ::SendMessage(hwndOut, LB_DELETESTRING, rgsel[count - i - 1], 0);

            delete[] rgsel;
        }
        break;
    }
    return TRUE;
}

void CollectionDialog::OnOK()
{
    Collection * const pcol = pCurCollection.pcol;

    for(int i = 0; i < pcol->m_visel.Size(); i++)
    {
        const int index = pcol->m_visel.ElementAt(i)->GetIEditable()->m_vCollection.IndexOf(pcol);
        if(index != -1)
        {
            pcol->m_visel.ElementAt(i)->GetIEditable()->m_vCollection.RemoveElementAt(index);
            pcol->m_visel.ElementAt(i)->GetIEditable()->m_viCollection.RemoveElementAt(index);
        }
    }

    pcol->m_visel.RemoveAllElements();

    HWND hwndIn = GetDlgItem(IDC_INLIST).GetHwnd();

    const size_t count = ::SendMessage(hwndIn, LB_GETCOUNT, 0, 0);

    for(size_t i = 0; i < count; i++)
    {
        IScriptable * const piscript = (IScriptable *)::SendMessage(hwndIn, LB_GETITEMDATA, i, 0);
        ISelect * const pisel = piscript->GetISelect();
        if(pisel) // Not sure how we could possibly get an iscript here that was never an iselect
        {
            pcol->m_visel.AddElement(pisel);
            pisel->GetIEditable()->m_vCollection.AddElement(pcol);
            pisel->GetIEditable()->m_viCollection.AddElement((void *)i);
        }
    }

    HWND hwndFireEvents = GetDlgItem(IDC_FIRE).GetHwnd();
    const size_t fEvents = ::SendMessage(hwndFireEvents, BM_GETCHECK, 0, 0);
    pcol->m_fFireEvents = (BOOL)fEvents;

    HWND hwndStopSingle = GetDlgItem(IDC_SUPPRESS).GetHwnd();
    const size_t fStopSingle = ::SendMessage(hwndStopSingle, BM_GETCHECK, 0, 0);
    pcol->m_fStopSingleEvents = (BOOL)fStopSingle;

    int groupElementsCollection = GetRegIntWithDefault("Editor", "GroupElementsInCollection", 1);
    if (groupElementsCollection)
    {
       HWND hwndGroupElements = GetDlgItem(IDC_GROUP_CHECK).GetHwnd();
       const size_t fGroupElements = ::SendMessage(hwndGroupElements, BM_GETCHECK, 0, 0);
       pcol->m_fGroupElements = (BOOL)fGroupElements;
    }

    char szT[1024];
    HWND hwndName = GetDlgItem(IDC_NAME).GetHwnd();
    ::GetWindowText(hwndName, szT, 1024);

    pCurCollection.ppt->SetCollectionName(pcol, szT, NULL, 0);

    CDialog::OnOK();
}
