#include "stdafx.h"
#include "LayersListDialog.h"
#include <WindowsX.h>

LayersListDialog::LayersListDialog() : CDialog(IDD_LAYERS), m_collapsed(true)
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
    if (m_layerTreeView.GetLayerCount()==1)
    {
        ShowError("Can't delete all layers!");
        return;
    }

    CCO(PinTable)* const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return;

    HTREEITEM layerToDelete = m_layerTreeView.GetCurrentLayerItem();
    std::vector<HTREEITEM> allSubItems = m_layerTreeView.GetSubItems(layerToDelete);
    HTREEITEM hFillLayer = m_layerTreeView.GetChild(m_layerTreeView.GetRootItem());
    if (hFillLayer == m_layerTreeView.GetCurrentLayerItem())
    {
        hFillLayer = m_layerTreeView.GetNextItem(hFillLayer, TVGN_NEXT);
    }
    const std::string fillLayerName(m_layerTreeView.GetItemText(hFillLayer));
    m_layerTreeView.SetActiveLayer(fillLayerName);
    for (HTREEITEM item : allSubItems)
    {
        TVITEM tvItem;
        ZeroMemory(&tvItem, sizeof(tvItem));
        tvItem.mask = TVIF_PARAM | TVIF_HANDLE;
        tvItem.hItem = item;
        if (m_layerTreeView.GetItem(tvItem))
        {
            IEditable* const pedit = (IEditable*)tvItem.lParam;
            if (pedit)
            {
                pedit->GetISelect()->m_layerName = fillLayerName;
                m_layerTreeView.AddElement(pedit->GetName(), pedit);
            }
        }
    }
    for (HTREEITEM item : allSubItems)
        m_layerTreeView.DeleteItem(item);
    m_layerTreeView.DeleteItem(layerToDelete);
}

void LayersListDialog::ClearList()
{
    m_layerTreeView.DeleteAll();
}

void LayersListDialog::UpdateLayerList(const std::string& name)
{
    CCO(PinTable) *const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return;

    ClearList();
    const bool checkName = (name == "") ? false : true;
    for (size_t t = 0; t < pt->m_vedit.size(); t++)
    {
        ISelect *const psel = pt->m_vedit[t]->GetISelect();
        if(!checkName)
            AddLayer(psel->m_layerName, pt->m_vedit[t]);
        else if(std::string(pt->m_vedit[t]->GetName()).find(name)!=std::string::npos)
            AddLayer(psel->m_layerName, pt->m_vedit[t]);
        
    }
    Expand();
}

void LayersListDialog::UpdateElement(IEditable *pedit)
{
    HTREEITEM item = m_layerTreeView.GetItemByElement(pedit);
    if (item == NULL)
        return;

    m_layerTreeView.SetItemText(item, pedit->GetName());
}

string LayersListDialog::GetCurrentSelectedLayerName() const
{
    return m_layerTreeView.GetCurrentLayerName();
}

BOOL LayersListDialog::OnInitDialog()
{
    m_layerFilterEditBox.SetDialog(this);
    AttachItem(IDC_LAYER_TREEVIEW, m_layerTreeView);
    AttachItem(IDC_ADD_LAYER_BUTTON, m_addLayerButton);
    AttachItem(IDC_DELETE_LAYER_BUTTON, m_deleteLayerButton);
    AttachItem(IDC_ASSIGN_BUTTON, m_assignButton);
    AttachItem(IDC_LAYER_FILTER_EDIT, m_layerFilterEditBox);
    AttachItem(IDC_EXPAND_COLLAPSE_BUTTON, m_expandCollapseButton);

    const int iconSize = 16;
    HANDLE hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ASSIGN), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_assignButton.SetIcon((HICON)hIcon);
    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_addLayerButton.SetIcon((HICON)hIcon);
    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_REMOVE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_deleteLayerButton.SetIcon((HICON)hIcon);
    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_EXPANDCOLLAPSE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_expandCollapseButton.SetIcon((HICON)hIcon);

    m_resizer.Initialize(*this, CRect(0, 0, 61, 200));
    m_resizer.AddChild(m_layerTreeView, leftcenter, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_addLayerButton, topright, 0);
    m_resizer.AddChild(m_deleteLayerButton, topright, 0);
    m_resizer.AddChild(m_assignButton, topleft, 0);
    m_resizer.AddChild(m_expandCollapseButton, topleft, 0);
    m_resizer.AddChild(m_layerFilterEditBox, topright, RD_STRETCH_WIDTH);
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
        case IDC_EXPAND_COLLAPSE_BUTTON:
        {
            if (m_collapsed)
                Expand();
            else
                Collaps();
            return TRUE;
        }
        default:
            break;
    }
    return FALSE;
}

void LayersListDialog::OnAssignButton()
{
    const std::string layerName = m_layerTreeView.GetCurrentLayerName();
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
        ISelect* const psel = pt->m_vmultisel.ElementAt(t);
        IEditable* const pedit = psel->GetIEditable();
        psel->m_layerName = layerName;
        HTREEITEM oldItem = m_layerTreeView.GetItemByElement(pedit);
        m_layerTreeView.AddElement(pedit->GetName(), pedit);
        m_layerTreeView.DeleteItem(oldItem);
    }
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
    const CRect rect = GetWindowRect();
    int   dockParent = 0;
    SaveValueInt("Editor", "LayersPosX", rect.left);
    SaveValueInt("Editor", "LayersPosY", rect.top);
    SaveValueBool("Editor", "LayersDocked", !!IsDocked());
    SaveValueInt("Editor", "LayersDockStyle", GetDockStyle());
    if (IsDocked())
    {
        if (GetDockParent() == g_pvp->GetPropertiesDocker())
        {
            dockParent = 1;
        }
        else if (GetDockParent() == g_pvp->GetToolbarDocker())
        {
            dockParent = 2;
        }
        else if (GetDockParent() == g_pvp)
        {
            dockParent = 3;
        }
    }
    SaveValueInt("Editor", "LayersDockParent", dockParent);
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

bool LayerTreeView::AddLayer(const string& name)
{
    hCurrentLayerItem = AddItem(hRootItem, name.c_str(), NULL, 1);
    return hCurrentLayerItem != NULL;
}

bool LayerTreeView::AddElement(const string& name, IEditable *pedit)
{
    hCurrentElementItem = AddItem(hCurrentLayerItem, name.c_str(), pedit, 2);
    return hCurrentElementItem != NULL;
}

bool LayerTreeView::ContainsLayer(const string& name) const
{
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        const string itemName(GetItemText(item));
        if (itemName == name)
            return true;
        item = GetNextItem(item, TVGN_NEXT);
    }
    return false;
}

string LayerTreeView::GetCurrentLayerName() const
{
    return string(GetItemText(hCurrentLayerItem));
}

HTREEITEM LayerTreeView::GetItemByElement(const IEditable* pedit)
{
    std::vector<HTREEITEM> children;
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        children.push_back(item);
        item = GetNextItem(item, TVGN_NEXT);
    }
    for (HTREEITEM child : children)
    {
        HTREEITEM subItem = GetChild(child);
        while (subItem)
        {
            char text[MAX_PATH];
            TVITEM tvItem;
            ZeroMemory(&tvItem, sizeof(tvItem));
            tvItem.mask = TVIF_PARAM | TVIF_TEXT;
            tvItem.cchTextMax = MAX_PATH;
            tvItem.pszText = text;
            tvItem.hItem = subItem;
            if (GetItem(tvItem))
            {
                if (pedit == (IEditable*)tvItem.lParam)
                    return subItem;
            }
            subItem = GetNextItem(subItem, TVGN_NEXT);
        }
    }
    return NULL;
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

int LayerTreeView::GetLayerCount() const
{
    HTREEITEM item = GetChild(hRootItem);
    int count = 0;
    while (item)
    {
        count++;
        item = GetNextItem(item, TVGN_NEXT);
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
    std::vector<HTREEITEM> children;
    
    Expand(hRootItem, TVE_EXPAND);
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        children.push_back(item);
        item = GetNextItem(item, TVGN_NEXT);
    }
    for (HTREEITEM child : children)
    {
        Expand(child, TVE_EXPAND);
    }
}

void LayerTreeView::CollapsAll()
{
    std::vector<HTREEITEM> children;

    Expand(hRootItem, TVE_COLLAPSE);
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        children.push_back(item);
        item = GetNextItem(item, TVGN_NEXT);
    }
    for (HTREEITEM child : children)
    {
        Expand(child, TVE_COLLAPSE);
    }
}

void LayerTreeView::SetActiveLayer(const string& name)
{
    HTREEITEM item = GetChild(hRootItem);
    int count = 0;
    while (item)
    {
        const std::string layerName(GetItemText(item));
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
    m_normalImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 0);
    CBitmap bm(IDB_LAYER_ICONS16);
    m_normalImages.Add(bm, RGB(255, 255, 255));
    SetImageList(m_normalImages, LVSIL_NORMAL);

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

            TVITEM tvItem;
            tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
            tvItem.hItem = pinfo->item.hItem;
            if (!GetItem(tvItem))
                return FALSE;

            if(tvItem.cChildren==1)
            {
                const string oldName = string(GetItemText(pinfo->item.hItem));
                const string newName = string(pinfo->item.pszText);

                for (size_t t = 0; t < pt->m_vedit.size(); t++)
                {
                    ISelect* const psel = pt->m_vedit[t]->GetISelect();
                    if (psel->m_layerName == oldName)
                        psel->m_layerName = newName;
                }
            }
            else
            {
                IEditable* pedit = (IEditable*)tvItem.lParam;
                if (pedit)
                    pedit->SetName(pinfo->item.pszText);
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
                        pt->AddMultiSel(pedit->GetISelect(), true, false, false);
                }

                subItem = GetNextItem(subItem, TVGN_NEXT);
            }
        }
        else // element checkbox was clicked
        {
            IEditable* pedit = (IEditable*)tvItem.lParam;
            if (pedit != NULL)
            {
                pt->AddMultiSel(pedit->GetISelect(), false, false, false);
                pt->RefreshProperties();
            }
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

LRESULT FilterEditBox::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
        switch (msg)
        {
            case WM_KEYUP:
                if ((wparam == VK_RETURN) || (wparam == VK_TAB))
                {
                    if (m_layerDialog)
                        m_layerDialog->UpdateLayerList(std::string(GetWindowText().c_str()));
                    return FALSE;
                }
        }
        return WndProcDefault(msg, wparam, lparam);
}
