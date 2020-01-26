#include "stdafx.h"
#include "LayersListDialog.h"
#include <WindowsX.h>

LayersListDialog::LayersListDialog() : CDialog(IDD_LAYERS), m_layerCount(0), m_currentLayerName("")
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

bool LayersListDialog::AddLayer(const string &name) 
{
    LVITEM lvitem;
    int newIndex = -1;
    lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
    lvitem.iItem = 0;
    lvitem.iSubItem = 0;
    lvitem.lParam = NULL;
    lvitem.pszText = (LPSTR)name.c_str();

    if (m_layerListView.GetItemCount() == 0)
        newIndex = m_layerListView.InsertItem(lvitem);
    else
    {
        if(ListContains(name.c_str())==-1)
            newIndex = m_layerListView.InsertItem(lvitem);
    }
    if (newIndex != -1)
    {
        m_layerListView.SetCheckState(newIndex, 1);
        m_layerListView.SetItemState(newIndex, LVIS_SELECTED, LVIS_SELECTED);
    }
    return newIndex != -1;
}

void LayersListDialog::DeleteLayer()
{
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

}

void LayersListDialog::ClearList()
{
    m_layerListView.DeleteAllItems();
}

void LayersListDialog::UpdateLayerList()
{
    CCO(PinTable) *const pt = g_pvp->GetActiveTable();
    if (pt == nullptr)
        return;

    m_layerListView.DeleteAllItems();
    for (size_t t = 0; t < pt->m_vedit.size(); t++)
    {
        ISelect *psel = pt->m_vedit[t]->GetISelect();
        AddLayer(psel->m_layerName);
    }

}

BOOL LayersListDialog::OnInitDialog()
{
    AttachItem(IDC_LAYER_LISTCONTROL, m_layerListView);
    AttachItem(IDC_ADD_LAYER_BUTTON, m_addLayerButton);
    AttachItem(IDC_DELETE_LAYER_BUTTON, m_deleteLayerButton);
    AttachItem(IDC_ASSIGN_BUTTON, m_assignButton);
    AttachItem(IDC_LAYER_EDIT_BUTTON, m_layerEditButton);

    m_resizer.Initialize(*this, CRect(0, 0, 61, 200));
    m_resizer.AddChild(m_layerListView, leftcenter, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_addLayerButton, topcenter, 0);
    m_resizer.AddChild(m_deleteLayerButton, topcenter, 0);
    m_resizer.AddChild(m_assignButton, topleft, 0);
    m_resizer.AddChild(m_layerEditButton, topleft, 0);

    m_resizer.RecalcLayout();

    m_layerListView.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    LVCOLUMN lvc;
    ZeroMemory(&lvc, sizeof(LVCOLUMN));
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = _T("Name");
    lvc.cx = 230;

    m_layerListView.InsertColumn(0, lvc);
    m_layerListView.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

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
            if (!AddLayer(string("New Layer 0")))
            {
                int i = 0;
                while (!AddLayer(string("New Layer ") + std::to_string(i))) i++;
            }
            return TRUE;
        }
        case IDC_DELETE_LAYER_BUTTON:
        {
            DeleteLayer();
            return TRUE;
        }
        case IDC_LAYER_EDIT_BUTTON:
        {
            const int sel = ListView_GetNextItem(m_layerListView.GetHwnd(), -1, LVNI_SELECTED);
            if (sel != -1)
            {
                m_layerListView.SetFocus();
                m_layerListView.EditLabel(sel);
            }
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

int LayersListDialog::ListContains(const string &name)
{
    LVFINDINFO findInfo;
    findInfo.flags = LVFI_STRING;
    findInfo.psz = name.c_str();
    findInfo.lParam = NULL;
    
    return m_layerListView.FindItem(findInfo, -1);
}

BOOL LayersListDialog::OnEditListItemLabel(LPARAM lparam)
{
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
    return TRUE;
}

BOOL LayersListDialog::OnListItemChanged(LPARAM lparam)
{
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
    return TRUE;
}

void LayersListDialog::OnAssignButton()
{
    if (m_currentLayerName == "")
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
        psel->m_layerName = m_currentLayerName;
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
//     const CRect rect = GetWindowRect();
//     SaveValueInt("Editor", "ToolbarPosX", rect.left);
//     SaveValueInt("Editor", "ToolbarPosY", rect.top);
//     SaveValueBool("Editor", "ToolbarDocked", IsDocked());
}
