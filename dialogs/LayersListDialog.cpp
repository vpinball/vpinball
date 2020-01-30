#include "stdafx.h"
#include "LayersListDialog.h"
#include <WindowsX.h>

LayersListDialog::LayersListDialog() : CDialog(IDD_LAYERS), m_layerCount(0)
{
}

LayersListDialog::~LayersListDialog()
{

}

LRESULT LayersListDialog::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Set window focus. The docker will now report this as active.
    if (!IsChild(::GetFocus()))
        SetFocus();

    return FinalWindowProc(msg, wparam, lparam);
}

bool LayersListDialog::AddLayer(const string &name, IEditable *piedit) 
{
    bool success = false;

    if (m_layerTreeView.GetItemCount() == 0)
    {
        success = m_layerTreeView.AddLayer(name);
    }
    else
    {
        if (!m_layerTreeView.ContainsLayer(name))
        {
            success = m_layerTreeView.AddLayer(name);
        }
        else
            m_layerTreeView.SetActiveLayer(name);
    }
    if (piedit != nullptr)
        success = m_layerTreeView.AddElement(piedit->GetName(), piedit);
    return success;
}

void LayersListDialog::DeleteLayer()
{
/*
    bool notEmptyMessage = true;
    std::vector<int> indexList;
    const int selectedCount = m_layerListView.GetSelectedCount();

    if (selectedCount == m_layerListView.GetItemCount())
    {
        ShowError("Can't delete all layers!");
        return;
    }
    CCO(PinTable)* const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return;

    for (int i = 0, curItem = -1; i < selectedCount; i++)
        indexList.push_back(m_layerListView.GetNextItem(curItem, LVNI_SELECTED));

    int nextLayerIndex = indexList[0];
    for (size_t i = 0; i < indexList.size(); i++)
    {
        nextLayerIndex = max(nextLayerIndex, indexList[i])+1;
    }
    if (nextLayerIndex >= m_layerListView.GetItemCount())
        nextLayerIndex = 0;

    string newLayerName = string(m_layerListView.GetItemText(nextLayerIndex, 0).c_str());

    for (size_t i = 0; i < indexList.size(); i++)
    {
        string layerName = string(m_layerListView.GetItemText(indexList[i], 0).c_str());
        for (size_t t = 0; t < pt->m_vedit.size(); t++)
        {
            ISelect* psel = pt->m_vedit[t]->GetISelect();
            if (layerName == psel->m_layerName)
            {
                if (notEmptyMessage)
                {
                    string msg = "Layer '" + layerName + "' contains elements! Move elements to layer '" + newLayerName + "' and delete layer?";
                    const int ans = g_pvp->MessageBox(msg.c_str(), "Warning", MB_YESNO | MB_DEFBUTTON2);
                    if (ans == IDNO)
                        return;
                    notEmptyMessage = false;
                }
                psel->m_layerName = newLayerName;
            }
        }
        m_layerListView.DeleteItem(indexList[i]);
    }

*/
}

void LayersListDialog::ClearList()
{
    m_layerTreeView.DeleteAll();
}

void LayersListDialog::UpdateLayerList()
{
    CCO(PinTable) *const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return;

    ClearList();
    for (size_t t = 0; t < pt->m_vedit.size(); t++)
    {
        ISelect *psel = pt->m_vedit[t]->GetISelect();
        AddLayer(psel->m_layerName, pt->m_vedit[t]);        
    }
    Expand();
}

string LayersListDialog::GetCurrentSelectedLayerName() const
{
    return m_layerTreeView.GetCurrentLayerName();
}

BOOL LayersListDialog::OnInitDialog()
{
    AttachItem(IDC_LAYER_TREEVIEW, m_layerTreeView);
    AttachItem(IDC_ADD_LAYER_BUTTON, m_addLayerButton);
    AttachItem(IDC_DELETE_LAYER_BUTTON, m_deleteLayerButton);
    AttachItem(IDC_ASSIGN_BUTTON, m_assignButton);

    m_resizer.Initialize(*this, CRect(0, 0, 61, 200));
    m_resizer.AddChild(m_layerTreeView, leftcenter, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_addLayerButton, topcenter, 0);
    m_resizer.AddChild(m_deleteLayerButton, topcenter, 0);
    m_resizer.AddChild(m_assignButton, topleft, 0);

    m_resizer.RecalcLayout();

    return TRUE;
}

INT_PTR LayersListDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    m_resizer.HandleMessage(msg, wparam, lparam);

    switch (msg)
    {
        case WM_MOUSEACTIVATE:
            return OnMouseActivate(msg, wparam, lparam);

        case WM_NOTIFY:
        {
            const LPNMHDR pnmhdr = (LPNMHDR)lparam;
            switch (pnmhdr->code)
            {
                case LVN_ENDLABELEDIT:
                {
                    return OnEditListItemLabel(lparam);
                }
                case LVN_ITEMCHANGED:
                {
                    return OnListItemChanged(lparam);
                }
            }
        }

    }

    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);
}

BOOL LayersListDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int id = LOWORD(wParam);
    switch (id)
    {
        case IDC_ADD_LAYER_BUTTON:
        {
            if (!AddLayer(string("New Layer 0"), nullptr))
            {
                int i = 0;
                while (!AddLayer(string("New Layer ") + std::to_string(i), nullptr)) i++;
            }
            return TRUE;
        }
        case IDC_DELETE_LAYER_BUTTON:
        {
            DeleteLayer();
            return TRUE;
        }
        case IDC_ASSIGN_BUTTON:
        {
            OnAssignButton();
            return TRUE;
        }
        default:
            break;
    }
    return FALSE;
}

BOOL LayersListDialog::OnEditListItemLabel(LPARAM lparam)
{
/*
    NMLVDISPINFO *const pinfo = (NMLVDISPINFO *)lparam;
    if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
    {
        return FALSE;
    }
    CCO(PinTable)* const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return FALSE;

    const string oldName = string(m_layerListView.GetItemText(pinfo->item.iItem, 0).c_str());
    const string newName = string(pinfo->item.pszText);
    m_layerListView.SetItemText(pinfo->item.iItem, 0, pinfo->item.pszText);

    for (size_t t = 0; t < pt->m_vedit.size(); t++)
    {
        ISelect* psel = pt->m_vedit[t]->GetISelect();
        if (psel->m_layerName == oldName)
            psel->m_layerName = newName;

    }
*/
    return TRUE;
}

BOOL LayersListDialog::OnListItemChanged(LPARAM lparam)
{
/*
    NMLISTVIEW *const plistview = (LPNMLISTVIEW)lparam;
    if (plistview->uChanged & LVIF_STATE)
    {
        CCO(PinTable) *const pt = g_pvp->GetActiveTable();
        if (pt == nullptr)
            return FALSE;

        const int sel = plistview->iItem;
        m_currentLayerName = string(m_layerListView.GetItemText(sel, 0).c_str());
        const bool itemChecked = !!m_layerListView.GetCheckState(sel);

        for (size_t t = 0; t < pt->m_vedit.size(); t++)
        {
            IEditable* pedit = pt->m_vedit[t];
            if (pedit->GetISelect()->m_layerName == m_currentLayerName)
            {
                pedit->m_isVisible = itemChecked;
            }
        }
        pt->SetDirtyDraw();
    }
*/
    return TRUE;
}

void LayersListDialog::OnAssignButton()
{
    std::string layerName = m_layerTreeView.GetCurrentLayerName();
    if (layerName == "")
    {
        ShowError("Please select a layer!");
        return;
    }
    CCO(PinTable)* const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return;
    for (int t=0;t<pt->m_vmultisel.size();t++)
    {
        ISelect* psel = pt->m_vmultisel.ElementAt(t);
        psel->m_layerName = layerName;
    }
    UpdateLayerList();
}

CContainLayers::CContainLayers()
{
    SetView(m_layersDialog);
    SetTabText(_T("Layers"));
    SetTabIcon(IDI_VPINBALL);
    SetDockCaption(_T("Layers"));
}

CDockLayers::CDockLayers()
{
    SetView(m_layersContainer);
    SetBarWidth(4);
}

void CDockLayers::OnDestroy()
{
//     const CRect rect = GetWindowRect();
//     SaveValueInt("Editor", "ToolbarPosX", rect.left);
//     SaveValueInt("Editor", "ToolbarPosY", rect.top);
//     SaveValueBool("Editor", "ToolbarDocked", IsDocked());
}

HTREEITEM LayerTreeView::AddItem(HTREEITEM hParent, LPCTSTR text, IEditable *pedit, int image)
{
    TVITEM tvi;
    ZeroMemory(&tvi, sizeof(TVITEM));
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvi.iImage = image;
    tvi.iSelectedImage = image;
    tvi.pszText = const_cast<LPTSTR>(text);
    tvi.lParam = (LPARAM)pedit;

    TVINSERTSTRUCT tvis;
    ZeroMemory(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent = hParent;
    tvis.item = tvi;

    HTREEITEM item = InsertItem(tvis);
    TreeView_SetCheckState(GetHwnd(), item, 1);
    return item;
}

bool LayerTreeView::AddLayer(const std::string name)
{
    hCurrentLayerItem = AddItem(hRootItem, name.c_str(), NULL, 1);
    return hCurrentLayerItem != NULL;
}

bool LayerTreeView::AddElement(const std::string name, IEditable *pedit)
{
    hCurrentElementItem = AddItem(hCurrentLayerItem, name.c_str(), pedit, 2);
    return hCurrentElementItem != NULL;
}

bool LayerTreeView::ContainsLayer(const std::string name) const
{
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        std::string itemName(GetItemText(item).c_str());
        if (itemName == name)
            return true;
        item = GetNextItem(item, TVGN_NEXT);
    }
    return false;
}

std::string LayerTreeView::GetCurrentLayerName() const
{
    return std::string(GetItemText(hCurrentLayerItem).c_str());
}

int LayerTreeView::GetItemCount() const
{
    std::vector<HTREEITEM> children;
    HTREEITEM item = GetChild(hRootItem);
    int count = 0;
    while (item)
    {
        children.push_back(item);
        item = GetNextItem(item, TVGN_NEXT);
    }
    count = children.size();
    for (HTREEITEM child : children)
    {
        HTREEITEM subItem = GetChild(child);
        while (subItem)
        {
            count++;
            subItem = GetNextItem(subItem, TVGN_NEXT);
        }
    }
    return count;
}

std::vector<HTREEITEM> LayerTreeView::GetSubItems(HTREEITEM hParent)
{
    std::vector<HTREEITEM> allSubItems;
    HTREEITEM item = GetChild(hParent);
    while (item)
    {
        allSubItems.push_back(item);
        item = GetNextItem(item, TVGN_NEXT);
    }
    return allSubItems;
}

bool LayerTreeView::IsItemChecked(HTREEITEM hItem) const
{
    TVITEM tvItem;
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;
    tvItem.hItem = hItem;
    GetItem(tvItem);
    return ((tvItem.state >> 12) - 1) == 0;
}

void LayerTreeView::SetAllItemStates(const bool checked)
{
    TVITEM tvItem;
    std::vector<HTREEITEM> children;

    ZeroMemory(&tvItem, sizeof(tvItem));
    tvItem.mask = TVIF_PARAM;

    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        children.push_back(item);
        TreeView_SetCheckState(GetHwnd(), item, checked);
        item = GetNextItem(item, TVGN_NEXT);
    }
    for (HTREEITEM child : children)
    {
        HTREEITEM subItem = GetChild(child);
        while (subItem)
        {
            tvItem.hItem = subItem;
            if (GetItem(tvItem))
            {
                IEditable *pedit = (IEditable *)tvItem.lParam;
                if(pedit!=NULL)
                    pedit->m_isVisible = checked;
            }

            TreeView_SetCheckState(GetHwnd(), subItem, checked);
            subItem = GetNextItem(subItem, TVGN_NEXT);
        }
    }
}

void LayerTreeView::DeleteAll()
{
    DeleteAllItems();
    hRootItem = AddItem(NULL, _T("Layers"), NULL, 0);
}

void LayerTreeView::ExpandAll()
{
    Expand(hRootItem, TVE_EXPAND);
}

void LayerTreeView::CollapsAll()
{
    Expand(hRootItem, TVE_COLLAPSE);
}

void LayerTreeView::SetActiveLayer(const std::string name)
{
    HTREEITEM item = GetChild(hRootItem);
    int count = 0;
    while (item)
    {
        std::string layerName = std::string(GetItemText(item).c_str());
        if (layerName == name)
        {
            hCurrentLayerItem = item;
            return;
        }
        item = GetNextItem(item, TVGN_NEXT);
    }

}

void LayerTreeView::OnAttach()
{
    DWORD style = GetStyle();
    style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_CHECKBOXES ;
    SetStyle(style);

    DeleteAllItems();
    hRootItem = AddItem(NULL, _T("Layers"), NULL, 0);
    Expand(hRootItem, TVE_EXPAND);
}

void LayerTreeView::PreCreate(CREATESTRUCT &cs)
{
    cs.style = TVS_NOTOOLTIPS | WS_CHILD | TVS_EDITLABELS;
    cs.lpszClass = WC_TREEVIEW;
}

LRESULT LayerTreeView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_MOUSEACTIVATE:
            SetFocus();
            break;
    }

    return WndProcDefault(msg, wparam, lparam);
}

LRESULT LayerTreeView::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
{
    LPNMHDR  lpnmh = (LPNMHDR)lparam;

    switch (lpnmh->code)
    {
        case TVN_SELCHANGED:    return OnTVNSelChanged((LPNMTREEVIEW)lparam);
        case NM_CLICK:          return OnNMClick(lpnmh);
        case NM_DBLCLK:         return OnNMDBClick(lpnmh);
        case TVN_ENDLABELEDIT:
        {
            LPNMTVDISPINFO pinfo = (LPNMTVDISPINFO)lparam;
            
            if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
            {
                return FALSE;
            }
            CCO(PinTable) *const pt = g_pvp->GetActiveTable();
            if (pt == nullptr)
                return FALSE;

            const string oldName = string(GetItemText(pinfo->item.hItem).c_str());
            const string newName = string(pinfo->item.pszText);

            for (size_t t = 0; t < pt->m_vedit.size(); t++)
            {
                ISelect *psel = pt->m_vedit[t]->GetISelect();
                if (psel->m_layerName == oldName)
                    psel->m_layerName = newName;
            }
            return TRUE;
        }
    }

    return 0;
}

LRESULT LayerTreeView::OnNMClick(LPNMHDR lpnmh)
{
    DWORD dwpos = GetMessagePos();
    TVHITTESTINFO ht = {0};
    ht.pt.x = GET_X_LPARAM(dwpos);
    ht.pt.y = GET_Y_LPARAM(dwpos);
    ::MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);
    HitTest(ht);

    if (ht.flags & TVHT_ONITEMSTATEICON)
    {
        CCO(PinTable) *const pt = g_pvp->GetActiveTable();

        if (ht.hItem == hRootItem)
            SetAllItemStates(IsItemChecked(hRootItem));
        else
        {
            TVITEM tvItem;
            ZeroMemory(&tvItem, sizeof(tvItem));
            tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
            tvItem.hItem = ht.hItem;
            if (GetItem(tvItem))
            {
                if (tvItem.cChildren == 1) // layer checkbox was clicked
                {
                    const bool checked = IsItemChecked(tvItem.hItem);
                    HTREEITEM subItem = GetChild(tvItem.hItem);
                    while (subItem)
                    {
                        tvItem.hItem = subItem;
                        if (GetItem(tvItem))
                        {
                            IEditable *pedit = (IEditable *)tvItem.lParam;
                            if (pedit != NULL)
                                pedit->m_isVisible = checked;
                        }

                        TreeView_SetCheckState(GetHwnd(), subItem, checked);
                        subItem = GetNextItem(subItem, TVGN_NEXT);
                    }
                }
                else // element checkbox was clicked
                {
                    IEditable *pedit = (IEditable *)tvItem.lParam;
                    if (pedit != NULL)
                        pedit->m_isVisible = IsItemChecked(tvItem.hItem);
                }
            }
        }
        if (pt != nullptr)
            pt->SetDirtyDraw();
    }
    return 0;
}

LRESULT LayerTreeView::OnNMDBClick(LPNMHDR lpnmh)
{
    DWORD dwpos = GetMessagePos();
    TVHITTESTINFO ht = { 0 };
    ht.pt.x = GET_X_LPARAM(dwpos);
    ht.pt.y = GET_Y_LPARAM(dwpos);
    ::MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);
    HitTest(ht);

    CCO(PinTable)* const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return TRUE;

    pt->ClearMultiSel();

    TVITEM tvItem;
    ZeroMemory(&tvItem, sizeof(tvItem));
    tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
    tvItem.hItem = ht.hItem;
    if (GetItem(tvItem))
    {
        if (tvItem.cChildren == 1) // layer checkbox was clicked
        {
            const bool checked = IsItemChecked(tvItem.hItem);
            HTREEITEM subItem = GetChild(tvItem.hItem);
            while (subItem)
            {
                tvItem.hItem = subItem;
                if (GetItem(tvItem))
                {
                    IEditable* pedit = (IEditable*)tvItem.lParam;
                    if (pedit != NULL)
                        pt->AddMultiSel(pedit->GetISelect(), true, true, false);
                }

                subItem = GetNextItem(subItem, TVGN_NEXT);
            }
        }
        else // element checkbox was clicked
        {
            IEditable* pedit = (IEditable*)tvItem.lParam;
            if (pedit != NULL)
                pt->AddMultiSel(pedit->GetISelect(), false, true, false);
        }
    }
    pt->SetDirtyDraw();
    return 0;
}

LRESULT LayerTreeView::OnTVNSelChanged(LPNMTREEVIEW pNMTV)
{
    TVITEM tvItem;
    ZeroMemory(&tvItem, sizeof(tvItem));
    tvItem.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM;
    tvItem.hItem = pNMTV->itemNew.hItem;
    if (GetItem(tvItem))
    {
        if(tvItem.hItem!=hRootItem)
        {
            if (tvItem.cChildren == 1)
                hCurrentLayerItem = tvItem.hItem;
            else
                hCurrentElementItem = tvItem.hItem;
        }
    }
    return 0;
}
