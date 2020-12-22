#include "stdafx.h"
#include "resource.h"
#include "CollectionManagerDialog.h"

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);
extern int CALLBACK MyCompProcIntValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption);

static CollectionDialogStruct cds;
int CollectionManagerDialog::m_columnSortOrder;

CollectionManagerDialog::CollectionManagerDialog() : CDialog(IDD_COLLECTDIALOG)
{
    hListHwnd = NULL;
}

BOOL CollectionManagerDialog::OnInitDialog()
{
    CCO(PinTable) * const pt = g_pvp->GetActiveTable();

    hListHwnd = GetDlgItem(IDC_SOUNDLIST).GetHwnd();

    m_columnSortOrder = 1;
    LoadPosition();

    ListView_SetExtendedListViewStyle(hListHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMN lvcol;
    lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
    const LocalString ls(IDS_NAME);
    lvcol.pszText = (LPSTR)ls.m_szbuffer; // = "Name";
    lvcol.cx = 280;
    ListView_InsertColumn(hListHwnd, 0, &lvcol);

    lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
    lvcol.fmt = LVCFMT_CENTER;
    const LocalString ls2(IDS_SIZE);
    lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "Size";
    lvcol.cx = 50;
    ListView_InsertColumn(hListHwnd, 1, &lvcol);

    pt->ListCollections(hListHwnd);
    ListView_SetItemState(hListHwnd, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    GotoDlgCtrl(hListHwnd);
    return FALSE;
}


void CollectionManagerDialog::EditCollection()
{
    CCO(PinTable) * const pt = g_pvp->GetActiveTable();

    const int sel = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
    if (sel != -1)
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
        if (colDlg->DoModal() >= 0)
            pt->SetNonUndoableDirty(eSaveDirty);

        char szT[sizeof(pcol->m_wzName)/sizeof(pcol->m_wzName[0])];
        WideCharToMultiByteNull(CP_ACP, 0, pcol->m_wzName, -1, szT, sizeof(szT), NULL, NULL);
        ListView_SetItemText(hListHwnd, sel, 0, szT);
    }
}

INT_PTR CollectionManagerDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCO(PinTable) * const pt = g_pvp->GetActiveTable();

    switch(uMsg)
    {
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
                    SortData.hwndList = hListHwnd;
                    SortData.subItemIndex = columnNumber;
                    SortData.sortUpDown = m_columnSortOrder;
                    if (columnNumber == 0)
                        ListView_SortItems(SortData.hwndList, MyCompProc, &SortData);
                    else
                        ListView_SortItems(SortData.hwndList, MyCompProcIntValues, &SortData);
                    int count = ListView_GetItemCount(hListHwnd);
                    for (int i = 0; i < count; i++)
                    {
                       LVITEM lvitem;
                       lvitem.mask = LVIF_PARAM;
                       lvitem.iItem = i;
                       lvitem.iSubItem = 0;
                       ListView_GetItem(hListHwnd, &lvitem);
                       Collection * const pcol = (Collection *)lvitem.lParam;
                       char buf[16] = { 0 };
                       sprintf_s(buf, "%i", pcol->m_visel.Size());
                       ListView_SetItemText(hListHwnd, i, 1, buf);
                    }
                }
            }
            if (pnmhdr->code == LVN_ENDLABELEDIT)
            {
                NMLVDISPINFO *pinfo = (NMLVDISPINFO *)lParam;
                if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
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
            else if (pnmhdr->code == NM_DBLCLK)
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
    CCO(PinTable) * const pt = g_pvp->GetActiveTable();
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
            pt->NewCollection(hListHwnd, false);
            pt->SetNonUndoableDirty(eSaveDirty);
            break;
        }
        case IDC_CREATEFROMSELECTION:
        {
            pt->NewCollection(hListHwnd, true);
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
            if (sel != -1)
            {
                ::SetFocus(hListHwnd);
                ListView_EditLabel(hListHwnd, sel);
            }
            break;
        }
        case IDC_COL_UP_BUTTON:
        {
            const int idx = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
            if (idx > 0)
            {
                ::SetFocus(hListHwnd);
                LVITEM lvitem1;
                memset(&lvitem1, 0, sizeof(LVITEM));
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
                char szT[sizeof(pcol->m_wzName)/sizeof(pcol->m_wzName[0])];
                WideCharToMultiByteNull(CP_ACP, 0, pcol->m_wzName, -1, szT, sizeof(szT), NULL, NULL);
                ListView_SetItemText(hListHwnd, idx - 1, 0, szT);

                char buf[16] = { 0 };
                sprintf_s(buf, "%i", pcol->m_visel.Size());
                ListView_SetItemText(hListHwnd, idx - 1, 1, buf);

                ListView_SetItemState(hListHwnd, -1, 0, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx - 1, LVIS_SELECTED, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx - 1, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            break;
        }
        case IDC_COL_DOWN_BUTTON:
        {
            const int idx = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
            if (idx != -1 && (idx < pt->m_vcollection.Size() - 1))
            {
                ::SetFocus(hListHwnd);
                LVITEM lvitem1;
                memset(&lvitem1, 0, sizeof(LVITEM));
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
                char szT[sizeof(pcol->m_wzName)/sizeof(pcol->m_wzName[0])];
                WideCharToMultiByteNull(CP_ACP, 0, pcol->m_wzName, -1, szT, sizeof(szT), NULL, NULL);
                ListView_SetItemText(hListHwnd, idx + 1, 0, szT);

                char buf[16] = { 0 };
                sprintf_s(buf, "%i", pcol->m_visel.Size());
                ListView_SetItemText(hListHwnd, idx + 1, 1, buf);

                ListView_SetItemState(hListHwnd, -1, 0, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
                ListView_SetItemState(hListHwnd, idx + 1, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            break;
        }
        case IDC_DELETE_COLLECTION:
        {
            const int sel = ListView_GetNextItem(hListHwnd, -1, LVNI_SELECTED);
            if (sel != -1)
            {
                // TEXT
                const int ans = MessageBox("Are you sure you want to remove this collection?", "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
                if (ans == IDYES)
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
    const int x = LoadValueIntWithDefault("Editor", "CollectionMngPosX", 0);
    const int y = LoadValueIntWithDefault("Editor", "CollectionMngPosY", 0);

    SetWindowPos(NULL, x, y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CollectionManagerDialog::SavePosition()
{
    const CRect rect = GetWindowRect();
    SaveValueInt("Editor", "CollectionMngPosX", rect.left);
    SaveValueInt("Editor", "CollectionMngPosY", rect.top);
}

//######################################## Collection Dialog ########################################

CollectionDialog::CollectionDialog(CollectionDialogStruct &pcds) : CDialog(IDD_COLLECTION), pCurCollection(pcds)
{
}

BOOL CollectionDialog::OnInitDialog()
{
    Collection * const pcol = pCurCollection.pcol;

    const HWND hwndName = GetDlgItem(IDC_NAME).GetHwnd();

    char szT[sizeof(pcol->m_wzName)/sizeof(pcol->m_wzName[0])];
    WideCharToMultiByteNull(CP_ACP, 0, pcol->m_wzName, -1, szT, sizeof(szT), NULL, NULL);

    ::SetWindowText(hwndName, szT);

    const HWND hwndFireEvents = GetDlgItem(IDC_FIRE).GetHwnd();
    ::SendMessage(hwndFireEvents, BM_SETCHECK, pcol->m_fireEvents ? BST_CHECKED : BST_UNCHECKED, 0);

    const HWND hwndStopSingle = GetDlgItem(IDC_SUPPRESS).GetHwnd();
    ::SendMessage(hwndStopSingle, BM_SETCHECK, pcol->m_stopSingleEvents ? BST_CHECKED : BST_UNCHECKED, 0);

    const HWND hwndGroupElements = GetDlgItem(IDC_GROUP_CHECK).GetHwnd();
    ::SendMessage(hwndGroupElements, BM_SETCHECK, pcol->m_groupElements ? BST_CHECKED : BST_UNCHECKED, 0);

    const HWND hwndOut = GetDlgItem(IDC_OUTLIST).GetHwnd();
    const HWND hwndIn = GetDlgItem(IDC_INLIST).GetHwnd();

    for (int i = 0; i < pcol->m_visel.Size(); i++)
    {
        ISelect * const pisel = pcol->m_visel.ElementAt(i);
        IEditable * const piedit = pisel->GetIEditable();
        IScriptable * const piscript = piedit->GetScriptable();
        if (piscript)
        {
            WideCharToMultiByteNull(CP_ACP, 0, piscript->m_wzName, -1, szT, sizeof(szT), NULL, NULL);
            const size_t index = ::SendMessage(hwndIn, LB_ADDSTRING, 0, (size_t)szT);
            ::SendMessage(hwndIn, LB_SETITEMDATA, index, (size_t)piscript);
        }
    }

    PinTable * const ppt = pCurCollection.ppt;

    for (size_t i = 0; i < ppt->m_vedit.size(); i++)
    {
        IEditable * const piedit = ppt->m_vedit[i];
        IScriptable * const piscript = piedit->GetScriptable();
        ISelect * const pisel = piedit->GetISelect();

        // Only process objects not in this collection
        int l;
        for (l = 0; l < pcol->m_visel.Size(); l++)
            if (pisel == pcol->m_visel.ElementAt(l))
                break;

        if ((l == pcol->m_visel.Size()) && piscript)
            //if (!piedit->m_pcollection)
        {
            WideCharToMultiByteNull(CP_ACP, 0, piscript->m_wzName, -1, szT, sizeof(szT), NULL, NULL);
            const size_t index = ::SendMessage(hwndOut, LB_ADDSTRING, 0, (size_t)szT);
            ::SendMessage(hwndOut, LB_SETITEMDATA, index, (size_t)piscript);
        }
    }

    return TRUE;
}

BOOL CollectionDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
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

            for (size_t loop = 0; loop < count; loop++)
                //for (i=count-1;i>=0;i--)
            {
                const size_t i = (LOWORD(wParam) == IDC_UP) ? loop : (count - loop - 1);

                const size_t len = ::SendMessage(hwndList, LB_GETTEXTLEN, rgsel[i], 0);
                char * const szT = new char[len + 1]; // include null terminator
                ::SendMessage(hwndList, LB_GETTEXT, rgsel[i], (LPARAM)szT);
                const size_t data = ::SendMessage(hwndList, LB_GETITEMDATA, rgsel[i], 0);

                const size_t newindex = (LOWORD(wParam) == IDC_UP) ? max(rgsel[i] - 1, (int)i) : min(rgsel[i] + 2, (int)(listsize - (count - 1) + i)); //!! see above
                size_t oldindex = rgsel[i];

                if (oldindex > newindex)
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
            const HWND hwndOut = GetDlgItem((LOWORD(wParam) == IDC_IN) ? IDC_OUTLIST : IDC_INLIST).GetHwnd();
            const HWND hwndIn = GetDlgItem((LOWORD(wParam) == IDC_IN) ? IDC_INLIST : IDC_OUTLIST).GetHwnd();

            const size_t count = ::SendMessage(hwndOut, LB_GETSELCOUNT, 0, 0);
            int * const rgsel = new int[count];
            ::SendMessage(hwndOut, LB_GETSELITEMS, count, (LPARAM)rgsel);
            for (size_t i = 0; i < count; i++)
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
            for (size_t i = 0; i < count; i++)
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

    for (int i = 0; i < pcol->m_visel.Size(); i++)
    {
        IEditable * const ie = pcol->m_visel.ElementAt(i)->GetIEditable();
        const int index = FindIndexOf(ie->m_vCollection, pcol);
        if (index != -1)
        {
            ie->m_vCollection.erase (ie->m_vCollection.begin()  + index);
            ie->m_viCollection.erase(ie->m_viCollection.begin() + index);
        }
    }

    pcol->m_visel.RemoveAllElements();

    const HWND hwndIn = GetDlgItem(IDC_INLIST).GetHwnd();

    const size_t count = ::SendMessage(hwndIn, LB_GETCOUNT, 0, 0);

    for (size_t i = 0; i < count; i++)
    {
        IScriptable * const piscript = (IScriptable *)::SendMessage(hwndIn, LB_GETITEMDATA, i, 0);
        ISelect * const pisel = piscript->GetISelect();
        if (pisel) // Not sure how we could possibly get an iscript here that was never an iselect
        {
            pcol->m_visel.AddElement(pisel);
            pisel->GetIEditable()->m_vCollection.push_back(pcol);
            pisel->GetIEditable()->m_viCollection.push_back((int)i);
        }
    }

    const size_t fireEvents = GetDlgItem(IDC_FIRE).SendMessage(BM_GETCHECK, 0, 0);
    pcol->m_fireEvents = !!fireEvents;

    const size_t stopSingleEvents = GetDlgItem(IDC_SUPPRESS).SendMessage(BM_GETCHECK, 0, 0);
    pcol->m_stopSingleEvents = !!stopSingleEvents;

    const size_t groupElements = GetDlgItem(IDC_GROUP_CHECK).SendMessage(BM_GETCHECK, 0, 0);
    pcol->m_groupElements = !!groupElements;

    pCurCollection.ppt->SetCollectionName(pcol, GetDlgItem(IDC_NAME).GetWindowText().c_str(), NULL, 0);

    CDialog::OnOK();
}
