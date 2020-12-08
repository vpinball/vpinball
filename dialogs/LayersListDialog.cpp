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

    if (success)
        m_layerTreeView.InvalidateRect();
    return success;
}

void LayersListDialog::DeleteLayer()
{
    if (m_layerTreeView.GetLayerCount()==1)
    {
        ShowError("Can't delete all layers!");
        return;
    }

    if (m_activeTable == nullptr)
        return;

    const int ans = MessageBox("Are you sure you want to delete the complete layer?", "Confirm delete", MB_YESNO | MB_DEFBUTTON2);
    if (ans != IDYES)
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
               ISelect* const psel = pedit->GetISelect();
               if(psel!=nullptr)
                  psel->m_layerName = fillLayerName;
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
    if (m_activeTable == nullptr)
        return;

    ClearList();
    const bool checkName = name.empty() ? false : true;
    for (size_t t = 0; t < m_activeTable->m_vedit.size(); t++)
    {
        ISelect *const psel = m_activeTable->m_vedit[t]->GetISelect();
        if(psel!=nullptr)
        {
           if (!checkName)
              AddLayer(psel->m_layerName, m_activeTable->m_vedit[t]);
           else if (std::string(m_activeTable->m_vedit[t]->GetName()).find(name) != std::string::npos)
              AddLayer(psel->m_layerName, m_activeTable->m_vedit[t]);
        }
    }
    if (!name.empty())
        ExpandAll();
    else
        ExpandLayers();
}

void LayersListDialog::UpdateElement(IEditable * const pedit)
{
    if (pedit == nullptr)
        return;

    HTREEITEM item = m_layerTreeView.GetItemByElement(pedit);
    if (item == nullptr)
        return;

    m_layerTreeView.SetItemText(item, pedit->GetName());
}

void LayersListDialog::DeleteElement(IEditable * const pedit)
{
    if (pedit == nullptr)
        return;

    HTREEITEM parent = m_layerTreeView.GetLayerByElement(pedit);
    HTREEITEM item = m_layerTreeView.GetItemByElement(pedit);
    if (item == nullptr || parent == nullptr)
        return;
    m_layerTreeView.DeleteItem(item);
    if (m_layerTreeView.GetSubItemsCount(parent) == 0)
        m_layerTreeView.DeleteItem(parent);
    if (m_layerTreeView.GetLayerCount() == 1)
        m_layerTreeView.SetActiveLayer(m_layerTreeView.GetLayerName(m_layerTreeView.GetFirstLayer()));
}

string LayersListDialog::GetCurrentSelectedLayerName() const
{
    return m_layerTreeView.GetCurrentLayerName();
}

void LayersListDialog::AddToolTip(const char* const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = parentHwnd;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)controlHwnd;
    toolInfo.lpszText = (char*)text;
    SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

BOOL LayersListDialog::OnInitDialog()
{
    const HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), NULL, g_pvp->theInstance, NULL);
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

    AddToolTip("Assign selected elements to selected layer", GetHwnd(), toolTipHwnd, m_assignButton.GetHwnd());
    AddToolTip("Collapse all", GetHwnd(), toolTipHwnd, m_expandCollapseButton.GetHwnd());
    AddToolTip("Add a new layer", GetHwnd(), toolTipHwnd, m_addLayerButton.GetHwnd());
    AddToolTip("Delete selected layer", GetHwnd(), toolTipHwnd, m_deleteLayerButton.GetHwnd());
    AddToolTip("Filter tree. Only elements that match the filter string will be shown!", GetHwnd(), toolTipHwnd, m_layerFilterEditBox.GetHwnd());

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
    if (m_activeTable == nullptr)
        return FALSE;

    UNREFERENCED_PARAMETER(lParam);
    const int id = LOWORD(wParam);
    switch (id)
    {
        case IDC_ADD_LAYER_BUTTON:
        {
            if (!AddLayer(string("New Layer 0"), nullptr))
            {
                int i = 0;
                while (!AddLayer(string("New Layer ") + std::to_string(i), nullptr))
                {
                    if (i > 10000) // something went wrong, so exit here and warn
                    {
                        ShowError("Cannot add to Layer");
                        return TRUE;
                    }
                    i++;
                }
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
            CollapseLayers();
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
    if (layerName.empty())
    {
        ShowError("Please select a layer!");
        return;
    }

    if (m_activeTable == nullptr || m_activeTable->MultiSelIsEmpty())
        return;

    for (int t=0;t<m_activeTable->m_vmultisel.size();t++)
    {
        ISelect* const psel = m_activeTable->m_vmultisel.ElementAt(t);
        IEditable* const pedit = psel->GetIEditable();
        psel->m_layerName = layerName;
        HTREEITEM oldItem = m_layerTreeView.GetItemByElement(pedit);
        m_layerTreeView.AddElement(pedit->GetName(), pedit);
        m_layerTreeView.DeleteItem(oldItem);
    }
}

bool LayersListDialog::PreTranslateMessage(MSG* msg)
{
   if (!IsWindow())
      return false;

   // only pre-translate mouse and keyboard input events
   if (((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST)))
   {
      const int keyPressed = LOWORD(msg->wParam);
      // only pass F1-F12 to the main VPinball class to open subdialogs from everywhere
      if (((keyPressed >= VK_F1 && keyPressed <= VK_F12) || (keyPressed == VK_ESCAPE)) && TranslateAccelerator(g_pvp->GetHwnd(), g_haccel, msg)) //!! VK_ESCAPE is a workaround, otherwise there is a hickup when changing a layername and pressing this
         return true;
   }

   if (m_layerTreeView.PreTranslateMessage(msg))
      return true;

   return !!IsDialogMessage(*msg);
}

CContainLayers::CContainLayers()
{
    SetView(m_layersDialog);
    SetTabText(_T("Layers"));
    SetTabIcon(IDI_LAYERS);
    SetDockCaption(_T("Layers"));
}

CDockLayers::CDockLayers()
{
    SetView(m_layersContainer);
    SetBarWidth(4);
}

void CDockLayers::OnClose()
{
    // nothing to do only to prevent closing the window
}

HTREEITEM LayerTreeView::AddItem(HTREEITEM hParent, LPCTSTR text, IEditable * const pedit, int image)
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
    return item;
}

bool LayerTreeView::AddLayer(const string& name)
{
    hCurrentLayerItem = AddItem(hRootItem, name.c_str(), NULL, 1);
    return hCurrentLayerItem != NULL;
}

bool LayerTreeView::AddElement(const string& name, IEditable * const pedit)
{
    return AddElementToLayer(hCurrentLayerItem, name, pedit);
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

HTREEITEM LayerTreeView::GetLayerByElement(const IEditable* const pedit)
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
                    return child;
            }
            subItem = GetNextItem(subItem, TVGN_NEXT);
        }
    }
    return NULL;
}

HTREEITEM LayerTreeView::GetLayerByItem(HTREEITEM hChildItem)
{
    std::vector<HTREEITEM> children;
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        children.push_back(item);
        if (hChildItem == item)
            return item;
        item = GetNextItem(item, TVGN_NEXT);
    }
    for (HTREEITEM child : children)
    {
        HTREEITEM subItem = GetChild(child);
        while (subItem)
        {
            if (hChildItem == subItem)
                return child;
            subItem = GetNextItem(subItem, TVGN_NEXT);
        }
    }

    return nullptr;
}

HTREEITEM LayerTreeView::GetItemByElement(const IEditable* const pedit)
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
    count = (int)children.size();
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

int LayerTreeView::GetSubItemsCount(HTREEITEM hParent) const
{
    int count = 0;
    HTREEITEM item = GetChild(hParent);
    while (item)
    {
        count++;
        item = GetNextItem(item, TVGN_NEXT);
    }
    return count;
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
                IEditable * const pedit = (IEditable *)tvItem.lParam;
                if (pedit != NULL)
                {
                   ISelect* const psel = pedit->GetISelect();
                   if(psel!=nullptr)
                      psel->m_isVisible = checked;
                }
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
    TreeView_SetCheckState(GetHwnd(), hRootItem, 1);
}

void LayerTreeView::ExpandAll()
{
    Expand(hRootItem, TVE_EXPAND);
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        Expand(item, TVE_EXPAND);
        item = GetNextItem(item, TVGN_NEXT);
    }
}

void LayerTreeView::CollapsAll()
{
    Expand(hRootItem, TVE_COLLAPSE);
    HTREEITEM item = GetChild(hRootItem);
    while (item)
    {
        Expand(item, TVE_COLLAPSE);
        item = GetNextItem(item, TVGN_NEXT);
    }
}

void LayerTreeView::ExpandLayers()
{
    Expand(hRootItem, TVE_EXPAND);
}

void LayerTreeView::CollapseLayer()
{
    CollapsAll();
    ExpandLayers();
}

void LayerTreeView::SetActiveLayer(const string& name)
{
    HTREEITEM item = GetChild(hRootItem);
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

bool LayerTreeView::PreTranslateMessage(MSG* msg)
{
   if (!IsWindow())
      return false;

   if (msg->hwnd != GetHwnd())
      return false;

   // only pre-translate mouse and keyboard input events
   if (((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST))
      && TranslateAccelerator(GetHwnd(), g_haccel, msg))
      return true;

   return !!IsDialogMessage(*msg);
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

#define MAKEPOINTS(l)       (*((POINTS FAR *)&(l)))

LRESULT LayerTreeView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_MOUSEACTIVATE:
            SetFocus();
            break;
        case WM_MOUSEMOVE:
        {
            if(m_dragging)
            {
                const POINTS Pos = MAKEPOINTS(lparam);
                ImageList_DragMove(Pos.x - 32, Pos.y - 25); // where to draw the drag from
                ImageList_DragShowNolock(FALSE);
                TVHITTESTINFO tvht;
                tvht.pt.x = Pos.x - 20; // the highlight items should be as the same points as the drag
                tvht.pt.y = Pos.y - 20; //
                HTREEITEM hitTarget = HitTest(tvht);
                if (hitTarget) // if there is a hit
                    SelectDropTarget(hitTarget);

                ImageList_DragShowNolock(TRUE);
            }
            break;
        }
        case WM_LBUTTONUP:
        {
            if (m_dragging)
            {
                ImageList_DragLeave(GetHwnd());
                ImageList_EndDrag();
                HTREEITEM hSelectedDrop;
                hSelectedDrop = GetDropHiLightItem();
                SelectItem(hSelectedDrop);
                SelectDropTarget(NULL);

                for(auto dragItem : m_DragItems)
                {
                    TVITEM tvItem;
                    ZeroMemory(&tvItem, sizeof(tvItem));
                    tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
                    tvItem.hItem = dragItem->m_hDragItem;
                    if (GetItem(tvItem))
                    {
                        IEditable* const pedit = (IEditable*)tvItem.lParam;
                        if (pedit != nullptr)
                        {
                            ISelect* const psel = pedit->GetISelect();
                            HTREEITEM hLayerItem = GetLayerByItem(hSelectedDrop);
                            if(psel!=nullptr)
                               psel->m_layerName = GetLayerName(hLayerItem);
                            HTREEITEM oldItem = GetItemByElement(pedit);
                            DeleteItem(oldItem);
                            AddElementToLayer(hLayerItem, pedit->GetName(), pedit);
                            std::vector<HTREEITEM> subItem = GetSubItems(dragItem->m_hDragLayer);
                            if (subItem.size() == 0)
                            {
                               if (dragItem->m_hDragLayer == hCurrentLayerItem)
                                  hCurrentLayerItem = hLayerItem;

                               DeleteItem(dragItem->m_hDragLayer);
                           }
                        }
                    }
                }
                m_DragItems.clear();
                ReleaseCapture();
                ShowCursor(TRUE);
                m_dragging = false;
            }
            break;
        }
    }

    return WndProcDefault(msg, wparam, lparam);
}

LRESULT LayerTreeView::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
{
    if (m_activeTable == nullptr)
        return FALSE;

    const LPNMHDR lpnmh = (LPNMHDR)lparam;

    switch (lpnmh->code)
    {
        case TVN_BEGINDRAG:
        {
            HIMAGELIST hImg;
            LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lparam;
            hImg = TreeView_CreateDragImage(GetHwnd(), lpnmtv->itemNew.hItem);
            ImageList_BeginDrag(hImg, 0, 0, 0);
            ImageList_DragEnter(GetHwnd(), lpnmtv->ptDrag.x, lpnmtv->ptDrag.y);
            
            std::shared_ptr<DragItem> dragItem = std::make_shared<DragItem>();
            dragItem->m_hDragItem = lpnmtv->itemNew.hItem;
            dragItem->m_hDragLayer = GetLayerByItem(dragItem->m_hDragItem);
            m_DragItems.push_back(dragItem);

            ShowCursor(FALSE);
            SetCapture();
            m_dragging = true;
            return TRUE;
        }
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

            TVITEM tvItem;
            tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
            tvItem.hItem = pinfo->item.hItem;
            if (!GetItem(tvItem))
                return FALSE;

            if(tvItem.cChildren==1)
            {
                const string oldName(GetItemText(pinfo->item.hItem));
                const string newName(pinfo->item.pszText);

                for (size_t t = 0; t < m_activeTable->m_vedit.size(); t++)
                {
                    ISelect* const psel = m_activeTable->m_vedit[t]->GetISelect();
                    if (psel!=nullptr && psel->m_layerName == oldName)
                        psel->m_layerName = newName;
                }
            }
            else
            {
                IEditable* const pedit = (IEditable*)tvItem.lParam;
                if (pedit)
                    pedit->SetName(pinfo->item.pszText);
            }
            return TRUE;
        }
    }

    return FALSE;
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
                            IEditable * const pedit = (IEditable *)tvItem.lParam;
                            if (pedit != NULL)
                            {
                               ISelect* const psel = pedit->GetISelect();
                               if(psel!=nullptr)
                                  psel->m_isVisible = checked;
                            }
                        }

                        TreeView_SetCheckState(GetHwnd(), subItem, checked);
                        subItem = GetNextItem(subItem, TVGN_NEXT);
                    }
                }
                else // element checkbox was clicked
                {
                    IEditable * const pedit = (IEditable *)tvItem.lParam;
                    if (pedit != NULL)
                    {
                       ISelect* const psel = pedit->GetISelect();
                       if (psel != nullptr)
                          psel->m_isVisible = IsItemChecked(tvItem.hItem);
                    }
                }
            }
        }

        if (m_activeTable != nullptr)
        {
            m_activeTable->SetDirty(eSaveDirty);
            m_activeTable->SetDirtyDraw();
        }
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

    if (m_activeTable == nullptr)
        return TRUE;

    m_activeTable->ClearMultiSel();

    TVITEM tvItem;
    ZeroMemory(&tvItem, sizeof(tvItem));
    tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
    tvItem.hItem = ht.hItem;
    if (GetItem(tvItem))
    {
        if (tvItem.cChildren == 1) // layer checkbox was clicked
        {
            HTREEITEM subItem = GetChild(tvItem.hItem);
            while (subItem)
            {
                tvItem.hItem = subItem;
                if (GetItem(tvItem))
                {
                    IEditable* const pedit = (IEditable*)tvItem.lParam;
                    if (pedit != NULL)
                    {
                       ISelect* const psel = pedit->GetISelect();
                       if(psel!=nullptr)
                          m_activeTable->AddMultiSel(psel, true, false, false);
                    }
                }

                subItem = GetNextItem(subItem, TVGN_NEXT);
            }
        }
        else // element checkbox was clicked
        {
            IEditable* const pedit = (IEditable*)tvItem.lParam;
            if (pedit != NULL)
            {
               ISelect* const psel = pedit->GetISelect();
               if(psel!=nullptr)
                  m_activeTable->AddMultiSel(psel, false, false, false);
               m_activeTable->RefreshProperties();
            }
        }
    }
    m_activeTable->SetDirty(eSaveDirty);
    m_activeTable->SetDirtyDraw();
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

bool LayerTreeView::AddElementToLayer(const HTREEITEM hLayerItem, const string& name, IEditable* const pedit)
{
    hCurrentElementItem = AddItem(hLayerItem, name.c_str(), pedit, 2);
    ISelect* const psel = pedit->GetISelect();
    if(psel!=nullptr)
    {
       if (!psel->m_isVisible)
       {
          TreeView_SetCheckState(GetHwnd(), hCurrentElementItem, 0);
       }
       else
       {
          TreeView_SetCheckState(GetHwnd(), hCurrentElementItem, 1);
          TreeView_SetCheckState(GetHwnd(), hLayerItem, 1);
       }
    }
    return hCurrentElementItem != NULL;
}

LRESULT FilterEditBox::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
        switch (msg)
        {
            case WM_KEYUP:
                if ((wparam == VK_RETURN) || (wparam == VK_TAB))
                {
                    if (m_layerDialog)
                        m_layerDialog->UpdateLayerList(string(GetWindowText()));
                    return FALSE;
                }
        }
        return WndProcDefault(msg, wparam, lparam);
}

BOOL FilterEditBox::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int dispID = LOWORD(wParam);

    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        {
            if (m_layerDialog)
                m_layerDialog->UpdateLayerList(string(GetWindowText()));
            return TRUE;
        }
    }
    return FALSE;
}
