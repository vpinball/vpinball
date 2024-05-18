#include "stdafx.h"
#include "LayersListDialog.h"
#include <WindowsX.h>

LayersListDialog::LayersListDialog()
   : CDialog(IDD_LAYERS)
   , m_collapsed(true)
    , m_isCaseSensitive(false)
{
   m_accel = LoadAccelerators(g_pvp->theInstance, MAKEINTRESOURCE(IDR_VPSIMPELACCEL));
}

LayersListDialog::~LayersListDialog() { }

LRESULT LayersListDialog::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
   // Set window focus. The docker will now report this as active.
   if (!IsChild(::GetFocus()))
      SetFocus();

   return FinalWindowProc(msg, wparam, lparam);
}

bool LayersListDialog::AddLayer(const string& name, IEditable* piedit)
{
   bool success = false;

    if (!m_layerTreeView.ContainsLayer(name))
      success = m_layerTreeView.AddLayer(name);
   else
      m_layerTreeView.SetActiveLayer(name);
   if (piedit != nullptr)
      success = m_layerTreeView.AddElement(piedit->GetName(), piedit);

   if (success)
      m_layerTreeView.InvalidateRect();
   return success;
}

void LayersListDialog::DeleteLayer()
{
   if (m_layerTreeView.GetLayerCount() == 1)
   {
      ShowError("Can't delete all layers!");
      return;
   }

   if (m_activeTable == nullptr)
      return;

   const HTREEITEM layerToDelete = m_layerTreeView.GetCurrentLayerItem();
   const vector<HTREEITEM> allSubItems = m_layerTreeView.GetSubItems(layerToDelete);

   if (!allSubItems.empty())
   {
      const int ans = MessageBox("Are you sure you want to delete the complete layer?", "Confirm delete", MB_YESNO | MB_DEFBUTTON2);
      if (ans != IDYES)
         return;
   }

   HTREEITEM hFillLayer = m_layerTreeView.GetChild(m_layerTreeView.GetRootItem());
   if (hFillLayer == m_layerTreeView.GetCurrentLayerItem())
   {
      hFillLayer = m_layerTreeView.GetNextItem(hFillLayer, TVGN_NEXT);
   }
   const string fillLayerName{m_layerTreeView.GetItemText(hFillLayer)};
   m_layerTreeView.SetActiveLayer(fillLayerName);
   for (const HTREEITEM item : allSubItems)
   {
      TVITEM tvItem = {};
      tvItem.mask = TVIF_PARAM | TVIF_HANDLE;
      tvItem.hItem = item;
      if (m_layerTreeView.GetItem(tvItem))
      {
         IEditable* const pedit = (IEditable*)tvItem.lParam;
         if (pedit)
         {
            ISelect* const psel = pedit->GetISelect();
            if (psel != nullptr)
               psel->m_layerName = fillLayerName;
            m_layerTreeView.AddElement(pedit->GetName(), pedit);
         }
      }
   }
   for (const HTREEITEM item : allSubItems)
      m_layerTreeView.DeleteItem(item);
   m_layerTreeView.DeleteItem(layerToDelete);
}

void LayersListDialog::ClearList() { m_layerTreeView.DeleteAll(); }

void LayersListDialog::UpdateLayerList(const string& name)
{
   if (m_activeTable == nullptr)
      return;

   vector<HTREEITEM> visItemList;
   HTREEITEM item = m_layerTreeView.GetNextVisible(m_layerTreeView.GetRootItem());
   while (item)
   {
      TVITEM itemInfo;
      itemInfo.hItem = item;
      itemInfo.mask = TVIF_STATE;
      if (m_layerTreeView.GetItem(itemInfo))
      {
         if ((itemInfo.state & TVIS_EXPANDED) == TVIS_EXPANDED)
            visItemList.push_back(item);
      }
      item = m_layerTreeView.GetNextVisible(item);
   }

   ClearList();
   const bool checkName = name.empty() ? false : true;
   string sName{name};
   if (checkName) //transform the name to lower
      std::transform(sName.begin(), sName.end(), sName.begin(), tolower);

   for (size_t t = 0; t < m_activeTable->m_vedit.size(); t++)
   {
      ISelect* const psel = m_activeTable->m_vedit[t]->GetISelect();
      if (psel != nullptr)
      {
         if (!checkName)
            AddLayer(psel->m_layerName, m_activeTable->m_vedit[t]);
         else if (!GetCaseSensitiveFilter())
         {
            //filter obj name and filter to lower
            string objName{m_activeTable->m_vedit[t]->GetName()};
            std::transform(objName.begin(), objName.end(), objName.begin(), tolower);
            if (string(objName).find(sName) != string::npos)
               AddLayer(psel->m_layerName, m_activeTable->m_vedit[t]);
         }
         else
         {
            //filter std
            if (string(m_activeTable->m_vedit[t]->GetName()).find(name) != string::npos)
               AddLayer(psel->m_layerName, m_activeTable->m_vedit[t]);
         }
      }
   }
   if (!name.empty())
      ExpandAll();
   else
   {
      ExpandLayers();
      for (const auto& l_item : visItemList)
      {
         m_layerTreeView.Expand(l_item, TVE_EXPAND);
      }
   }
}

void LayersListDialog::UpdateElement(IEditable* const pedit)
{
   if (pedit == nullptr)
      return;

   const HTREEITEM item = m_layerTreeView.GetItemByElement(pedit);
   if (item == nullptr)
      return;

   m_layerTreeView.SetItemText(item, pedit->GetName());
}

void LayersListDialog::DeleteElement(IEditable* const pedit)
{
   if (pedit == nullptr)
      return;

   const HTREEITEM parent = m_layerTreeView.GetLayerByElement(pedit);
   const HTREEITEM item = m_layerTreeView.GetItemByElement(pedit);
   if (item == nullptr || parent == nullptr)
      return;
   m_layerTreeView.DeleteItem(item);
   if (m_layerTreeView.GetSubItemsCount(parent) == 0)
      m_layerTreeView.DeleteItem(parent);
   if (m_layerTreeView.GetLayerCount() == 1)
      m_layerTreeView.SetActiveLayer(m_layerTreeView.GetLayerName(m_layerTreeView.GetFirstLayer()));
}

string LayersListDialog::GetCurrentSelectedLayerName() const { return m_layerTreeView.GetCurrentLayerName(); }

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
   const HWND toolTipHwnd = CreateWindowEx(
      0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), nullptr, g_pvp->theInstance, nullptr);
   m_layerFilterEditBox.SetDialog(this);
   m_isCaseSensitive = BST_UNCHECKED;

   AttachItem(IDC_LAYER_TREEVIEW, m_layerTreeView);
   AttachItem(IDC_ADD_LAYER_BUTTON, m_addLayerButton);
   AttachItem(IDC_DELETE_LAYER_BUTTON, m_deleteLayerButton);
   AttachItem(IDC_ASSIGN_BUTTON, m_assignButton);
   AttachItem(IDC_LAYER_FILTER_EDIT, m_layerFilterEditBox);
   AttachItem(IDC_LAYER_FILTER_CASE_BUTTON, m_layerFilterCaseButton);
   AttachItem(IDC_EXPAND_COLLAPSE_BUTTON, m_expandCollapseButton);

   constexpr int iconSize = 16;
   HANDLE hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ASSIGN), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_assignButton.SetIcon((HICON)hIcon);
   hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_addLayerButton.SetIcon((HICON)hIcon);
   hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_REMOVE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_deleteLayerButton.SetIcon((HICON)hIcon);
   hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_EXPANDCOLLAPSE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_expandCollapseButton.SetIcon((HICON)hIcon);

   AddToolTip("Assign selected elements to selected layer", GetHwnd(), toolTipHwnd, m_assignButton.GetHwnd());
   AddToolTip("Collapse all", GetHwnd(), toolTipHwnd, m_expandCollapseButton.GetHwnd());
   AddToolTip("Add a new layer", GetHwnd(), toolTipHwnd, m_addLayerButton.GetHwnd());
   AddToolTip("Delete selected layer", GetHwnd(), toolTipHwnd, m_deleteLayerButton.GetHwnd());
   AddToolTip("Filter tree. Only elements that match the filter string will be shown!", GetHwnd(), toolTipHwnd, m_layerFilterEditBox.GetHwnd());
   AddToolTip("Enable case sensitive filtering", GetHwnd(), toolTipHwnd, m_layerFilterCaseButton.GetHwnd());

   m_resizer.Initialize(*this, CRect(0, 0, 200, 200));
   m_resizer.AddChild(m_layerTreeView, CResizer::topleft, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_addLayerButton, CResizer::topright, 0);
   m_resizer.AddChild(m_deleteLayerButton, CResizer::topright, 0);
   m_resizer.AddChild(m_assignButton, CResizer::topleft, 0);
   m_resizer.AddChild(m_layerFilterCaseButton, CResizer::topright, 0);
   m_resizer.AddChild(m_expandCollapseButton, CResizer::topleft, 0);
   m_resizer.AddChild(m_layerFilterEditBox, CResizer::topright, RD_STRETCH_WIDTH);
   m_resizer.RecalcLayout();

   return TRUE;
}

INT_PTR LayersListDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
   m_resizer.HandleMessage(msg, wparam, lparam);

   switch (msg)
   {
   case WM_MOUSEACTIVATE: return OnMouseActivate(msg, wparam, lparam);
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
       case IDC_SYNC:
       {
          UpdateLayerInfo();
		   return TRUE;
       }
   case IDC_ADD_LAYER_BUTTON:
   {
      if (!AddLayer("New Layer 0", nullptr))
      {
         int i = 0;
         while (!AddLayer("New Layer " + std::to_string(i), nullptr))
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
   case IDC_LAYER_FILTER_CASE_BUTTON:
      SetCaseSensitiveFilter(!GetCaseSensitiveFilter());
      Button_SetCheck(m_layerFilterCaseButton, GetCaseSensitiveFilter() ? BST_CHECKED : BST_UNCHECKED);
      UpdateLayerList(GetWindowText().c_str());
      return TRUE;
   default: break;
   }
   return FALSE;
}

void LayersListDialog::OnAssignButton()
{
   const string layerName{m_layerTreeView.GetCurrentLayerName()};
   if (layerName.empty())
   {
      ShowError("Please select a layer!");
      return;
   }

   if (m_activeTable == nullptr || m_activeTable->MultiSelIsEmpty())
      return;

   const bool layerIsVisible = !m_layerTreeView.IsItemChecked(m_layerTreeView.GetCurrentLayerItem());
   bool visibilityChanged = false;
   for (int t = 0; t < m_activeTable->m_vmultisel.size(); t++)
   {
      ISelect* const psel = m_activeTable->m_vmultisel.ElementAt(t);
      IEditable* const pedit = psel->GetIEditable();
      psel->m_layerName = layerName;
      const HTREEITEM oldItem = m_layerTreeView.GetItemByElement(pedit);
      m_layerTreeView.AddElement(pedit->GetName(), pedit);
      m_layerTreeView.DeleteItem(oldItem);
      if (psel->m_isVisible && !layerIsVisible)
      {
         psel->m_isVisible = false;
         m_layerTreeView.SetItemCheck(m_layerTreeView.GetCurrentElement(), false);
         visibilityChanged = true;
      }
      else if (!psel->m_isVisible && layerIsVisible)
      {
         psel->m_isVisible = true;
         m_layerTreeView.SetItemCheck(m_layerTreeView.GetCurrentElement(), true);
         visibilityChanged = true;
      }
   }
   if (visibilityChanged)
   {
      m_layerTreeView.SetItemCheck(m_layerTreeView.GetCurrentLayerItem(), layerIsVisible);
      m_activeTable->SetDirtyDraw();
   }
}

void LayersListDialog::AssignToLayerByIndex(size_t index)
{
   if (m_activeTable == nullptr || m_activeTable->MultiSelIsEmpty())
      return;
   const vector<string> layerList = GetAllLayerNames();

   if (index >= layerList.size())
      return;

   vector<HTREEITEM> children = m_layerTreeView.GetAllLayerItems();
   const bool layerIsVisible = !m_layerTreeView.IsItemChecked(children[index]);
   bool visibilityChanged = false;
   for (int t = 0; t < m_activeTable->m_vmultisel.size(); t++)
   {
      ISelect* const psel = m_activeTable->m_vmultisel.ElementAt(t);
      IEditable* const pedit = psel->GetIEditable();
      psel->m_layerName = layerList[index];
      
      const HTREEITEM oldItem = m_layerTreeView.GetItemByElement(pedit);
      m_layerTreeView.AddElementToLayer(children[index], pedit->GetName(), pedit);
      m_layerTreeView.DeleteItem(oldItem);

      if (psel->m_isVisible && !layerIsVisible)
      {
         psel->m_isVisible = false;
         m_layerTreeView.SetItemCheck(m_layerTreeView.GetCurrentElement(), false);
         visibilityChanged = true;
      }
      else if (!psel->m_isVisible && layerIsVisible)
      {
         psel->m_isVisible = true;
         m_layerTreeView.SetItemCheck(m_layerTreeView.GetCurrentElement(), true);
         visibilityChanged = true;
      }
   }
   if (visibilityChanged) 
   {
      m_layerTreeView.SetItemCheck(children[index], layerIsVisible);
      m_activeTable->SetDirtyDraw();
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
      if (((keyPressed >= VK_F3 && keyPressed <= VK_F12) || (keyPressed == VK_ESCAPE))
         && TranslateAccelerator(g_pvp->GetHwnd(), m_accel, msg)) //!! VK_ESCAPE is a workaround, otherwise there is a hickup when changing a layername and pressing this
         return true;
   }

   if (m_layerTreeView.PreTranslateMessage(msg))
      return true;

   return !!IsDialogMessage(*msg);
}

vector<string> LayersListDialog::GetAllLayerNames() 
{ 
    return m_layerTreeView.GetAllLayerNames();
}

// If SYNC checkbox is checked, the layer list will be updated
void LayersListDialog::UpdateLayerInfo()
{
    // If active table is not set, return
    if (m_activeTable == nullptr)
        return;

    // If Radio button IDC_SYNC is unchecked, return
    if (IsDlgButtonChecked(IDC_SYNC) == BST_UNCHECKED)
        return;

    // Get the selected element
    ISelect* const psel = m_activeTable->m_vmultisel.ElementAt(0);
	if (psel != nullptr)
	{
        // Get the layer name of the selected element
    	const string layerName{psel->m_layerName};
        // Set the active layer
	    m_layerTreeView.SetActiveLayer(layerName);
        // Find the selected element name and find it in the layer, selecting it
        const HTREEITEM item = m_layerTreeView.GetItemByElement(psel->GetIEditable());
        m_layerTreeView.SelectItem(item);
	}
    else
    {
	    m_layerTreeView.SetActiveLayer("");
    }
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

LayerTreeView::LayerTreeView()
{
   m_dragging = false;
   m_accel = LoadAccelerators(g_pvp->theInstance, MAKEINTRESOURCE(IDR_VPSIMPELACCEL));
}

HTREEITEM LayerTreeView::AddItem(HTREEITEM hParent, LPCTSTR text, IEditable* const pedit, int image)
{
   TVITEM tvi = {};
   tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
   tvi.iImage = image;
   tvi.iSelectedImage = image;
   tvi.pszText = const_cast<LPTSTR>(text);
   tvi.lParam = (LPARAM)pedit;

   TVINSERTSTRUCT tvis = {};
   tvis.hParent = hParent;
   tvis.item = tvi;

   const HTREEITEM item = InsertItem(tvis);
   return item;
}

bool LayerTreeView::AddLayer(const string& name)
{
   hCurrentLayerItem = AddItem(hRootItem, name.c_str(), nullptr, 1);
   return hCurrentLayerItem != nullptr;
}

bool LayerTreeView::AddElement(const string& name, IEditable* const pedit) { return AddElementToLayer(hCurrentLayerItem, name, pedit); }

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

string LayerTreeView::GetCurrentLayerName() const { return string(GetItemText(hCurrentLayerItem)); }

HTREEITEM LayerTreeView::GetLayerByElement(const IEditable* const pedit)
{
   for (const HTREEITEM child : GetAllLayerItems())
   {
      HTREEITEM subItem = GetChild(child);
      while (subItem)
      {
         char text[MAX_PATH];
         TVITEM tvItem = {};
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
   return nullptr;
}

HTREEITEM LayerTreeView::GetLayerByItem(HTREEITEM hChildItem)
{
   for (const HTREEITEM child : GetAllLayerItems())
   {
      if (hChildItem == child)
         return child;

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
   vector<HTREEITEM> children = GetAllLayerItems();

   for (const HTREEITEM child : children)
   {
      HTREEITEM subItem = GetChild(child);
      while (subItem)
      {
         char text[MAX_PATH];
         TVITEM tvItem = {};
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
   return nullptr;
}

int LayerTreeView::GetItemCount() const
{
   vector<HTREEITEM> children = GetAllLayerItems();

   int count = (int)children.size();
   for (const HTREEITEM child : children)
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

vector<HTREEITEM> LayerTreeView::GetAllLayerItems() const 
{
   vector<HTREEITEM> children;
   HTREEITEM item = GetChild(hRootItem);
   while (item)
   {
      children.push_back(item);
      item = GetNextItem(item, TVGN_NEXT);
   }
   return children;
}

vector<HTREEITEM> LayerTreeView::GetSubItems(HTREEITEM hParent)
{
   vector<HTREEITEM> allSubItems;
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

void LayerTreeView::SetItemCheck(HTREEITEM item, bool checked) 
{ 
    if (checked)
    {
        TreeView_SetCheckState(GetHwnd(), item, 1);
    }
    else
    {
        TreeView_SetCheckState(GetHwnd(), item, 0);
    }
}

void LayerTreeView::SetAllItemStates(const bool checked)
{
   TVITEM tvItem = {};
   tvItem.mask = TVIF_PARAM;

   vector<HTREEITEM> children = GetAllLayerItems();
   for (const HTREEITEM child : children)
   {
      TreeView_SetCheckState(GetHwnd(), child, checked);
      HTREEITEM subItem = GetChild(child);
      while (subItem)
      {
         tvItem.hItem = subItem;
         if (GetItem(tvItem))
         {
            IEditable* const pedit = (IEditable*)tvItem.lParam;
            if (pedit != nullptr)
            {
               ISelect* const psel = pedit->GetISelect();
               if (psel != nullptr)
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
   hRootItem = AddItem(nullptr, _T("Layers"), nullptr, 0);
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

void LayerTreeView::ExpandLayers() { Expand(hRootItem, TVE_EXPAND); }

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
      const string layerName{GetItemText(item)};
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

   const int keyPressed = LOWORD(msg->wParam);
   if (keyPressed != VK_F2)
   {
      // only pre-translate mouse and keyboard input events
      if (((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST)) && TranslateAccelerator(GetHwnd(), m_accel, msg))
         return true;
   }

   return !!IsDialogMessage(*msg);
}

vector<string> LayerTreeView::GetAllLayerNames()
{
   vector<HTREEITEM> children;
   HTREEITEM item = GetChild(hRootItem);
   while (item)
   {
      children.push_back(item);
      item = GetNextItem(item, TVGN_NEXT);
   }
   vector<string> layerList;
   layerList.reserve(children.size());
   for (auto& layer : children) 
   { 
      layerList.push_back(GetItemText(layer).c_str());
   }
   return layerList;
}

void LayerTreeView::OnAttach()
{
   m_normalImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 0);
   const CBitmap bm(IDB_LAYER_ICONS16);
   m_normalImages.Add(bm, RGB(255, 255, 255));
   SetImageList(m_normalImages, LVSIL_NORMAL);

   DWORD style = GetStyle();
   style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_CHECKBOXES;
   SetStyle(style);

   DeleteAllItems();
   hRootItem = AddItem(nullptr, _T("Layers"), nullptr, 0);
   Expand(hRootItem, TVE_EXPAND);
}

void LayerTreeView::PreCreate(CREATESTRUCT& cs)
{
   cs.style = TVS_NOTOOLTIPS | WS_CHILD | TVS_EDITLABELS;
   cs.lpszClass = WC_TREEVIEW;
}

#ifndef MAKEPOINTS
#define MAKEPOINTS(l) (*((POINTS FAR*)&(l)))
#endif

LRESULT LayerTreeView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_KEYUP:
      if (wparam == VK_F2)
      {
         const HTREEITEM item = GetSelection();
         EditLabel(item);
      }
      break;
   case WM_MOUSEACTIVATE: SetFocus(); break;
   case WM_MOUSEMOVE:
   {
      if (m_dragging)
      {
         const POINTS Pos = MAKEPOINTS(lparam);
         ImageList_DragMove(Pos.x - 32, Pos.y - 25); // where to draw the drag from
         ImageList_DragShowNolock(FALSE);
         TVHITTESTINFO tvht;
         tvht.pt.x = Pos.x - 20; // the highlight items should be as the same points as the drag
         tvht.pt.y = Pos.y - 20; //
         const HTREEITEM hitTarget = HitTest(tvht);
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
         HTREEITEM hSelectedDrop = GetDropHiLightItem();
         SelectItem(hSelectedDrop);
         SelectDropTarget(nullptr);

         vector<HTREEITEM> layers = GetAllLayerItems();

         for (auto dragItem : m_DragItems)
         {
            TVITEM tvItem = {};
            tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
            tvItem.hItem = dragItem->m_hDragItem;
            if (GetItem(tvItem))
            {
               IEditable* const pedit = (IEditable*)tvItem.lParam;
               if (pedit != nullptr)
               {
                  ISelect* const psel = pedit->GetISelect();
                  const HTREEITEM hLayerItem = GetLayerByItem(hSelectedDrop);
                  if (psel != nullptr)
                     psel->m_layerName = GetLayerName(hLayerItem);
                  const HTREEITEM oldItem = GetItemByElement(pedit);
                  DeleteItem(oldItem);
                  AddElementToLayer(hLayerItem, pedit->GetName(), pedit);
                  vector<HTREEITEM> subItem = GetSubItems(dragItem->m_hDragLayer);
                  if (subItem.empty())
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
         while(ShowCursor(FALSE)>=0) ;
         while(ShowCursor(TRUE)<0) ;
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
      const LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lparam;
      HIMAGELIST hImg = TreeView_CreateDragImage(GetHwnd(), lpnmtv->itemNew.hItem);
      ImageList_BeginDrag(hImg, 0, 0, 0);
      ImageList_DragEnter(GetHwnd(), lpnmtv->ptDrag.x, lpnmtv->ptDrag.y);

      const std::shared_ptr<DragItem> dragItem = std::make_shared<DragItem>();
      dragItem->m_hDragItem = lpnmtv->itemNew.hItem;
      dragItem->m_hDragLayer = GetLayerByItem(dragItem->m_hDragItem);
      m_DragItems.push_back(dragItem);

      while(ShowCursor(TRUE)<0) ;
      while(ShowCursor(FALSE)>=0) ;
      SetCapture();
      m_dragging = true;
      return TRUE;
   }
   case TVN_SELCHANGED: return OnTVNSelChanged((LPNMTREEVIEW)lparam);
   case NM_CLICK: return OnNMClick(lpnmh);
   case NM_DBLCLK: return OnNMDBClick(lpnmh);
   case TVN_ENDLABELEDIT:
   {
      const LPNMTVDISPINFO pinfo = (LPNMTVDISPINFO)lparam;

      if (pinfo->item.pszText == nullptr || pinfo->item.pszText[0] == '\0')
      {
         return FALSE;
      }

      TVITEM tvItem;
      tvItem.mask = TVIF_CHILDREN | TVIF_PARAM;
      tvItem.hItem = pinfo->item.hItem;
      if (!GetItem(tvItem))
         return FALSE;

      if (tvItem.cChildren == 1) //!! <= ?
      {
         const string oldName(GetItemText(pinfo->item.hItem));
         const string newName(pinfo->item.pszText);

         for (size_t t = 0; t < m_activeTable->m_vedit.size(); t++)
         {
            ISelect* const psel = m_activeTable->m_vedit[t]->GetISelect();
            if (psel != nullptr && psel->m_layerName == oldName)
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
   const DWORD dwpos = GetMessagePos();
   TVHITTESTINFO ht = {};
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
         TVITEM tvItem = {};
         tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
         tvItem.hItem = ht.hItem;
         if (GetItem(tvItem))
         {
            if (tvItem.cChildren == 1) //!! <= ? // layer checkbox was clicked
            {
               const bool checked = IsItemChecked(tvItem.hItem);
               HTREEITEM subItem = GetChild(tvItem.hItem);
               while (subItem)
               {
                  tvItem.hItem = subItem;
                  if (GetItem(tvItem))
                  {
                     IEditable* const pedit = (IEditable*)tvItem.lParam;
                     if (pedit != nullptr)
                     {
                        ISelect* const psel = pedit->GetISelect();
                        if (psel != nullptr)
                           psel->m_isVisible = checked;
                     }
                  }

                  TreeView_SetCheckState(GetHwnd(), subItem, checked);
                  subItem = GetNextItem(subItem, TVGN_NEXT);
               }
            }
            else // element checkbox was clicked
            {
               IEditable* const pedit = (IEditable*)tvItem.lParam;
               if (pedit != nullptr)
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
   const DWORD dwpos = GetMessagePos();
   TVHITTESTINFO ht = {};
   ht.pt.x = GET_X_LPARAM(dwpos);
   ht.pt.y = GET_Y_LPARAM(dwpos);
   ::MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);
   HitTest(ht);

   if (m_activeTable == nullptr)
      return TRUE;

   m_activeTable->ClearMultiSel();

   TVITEM tvItem = {};
   tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
   tvItem.hItem = ht.hItem;
   if (GetItem(tvItem))
   {
      if (tvItem.cChildren == 1) //!! <= ? // layer checkbox was clicked
      {
         HTREEITEM subItem = GetChild(tvItem.hItem);
         while (subItem)
         {
            tvItem.hItem = subItem;
            if (GetItem(tvItem))
            {
               IEditable* const pedit = (IEditable*)tvItem.lParam;
               if (pedit != nullptr)
               {
                  ISelect* const psel = pedit->GetISelect();
                  if (psel != nullptr)
                     m_activeTable->AddMultiSel(psel, true, false, false);
               }
            }

            subItem = GetNextItem(subItem, TVGN_NEXT);
         }
      }
      else // element checkbox was clicked
      {
         IEditable* const pedit = (IEditable*)tvItem.lParam;
         if (pedit != nullptr)
         {
            ISelect* const psel = pedit->GetISelect();
            if (psel != nullptr)
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
   TVITEM tvItem = {};
   tvItem.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM;
   tvItem.hItem = pNMTV->itemNew.hItem;
   if (GetItem(tvItem))
   {
      if (tvItem.hItem != hRootItem)
      {
         if (GetSubItemsCount(tvItem.hItem)>0)
            hCurrentLayerItem = tvItem.hItem;
         else
         {
            hCurrentElementItem = tvItem.hItem;
            hCurrentLayerItem = GetLayerByItem(tvItem.hItem);
         }
      }
   }
   return 0;
}

bool LayerTreeView::AddElementToLayer(const HTREEITEM hLayerItem, const string& name, IEditable* const pedit)
{
   hCurrentElementItem = AddItem(hLayerItem, name.c_str(), pedit, 2);
   ISelect* const psel = pedit->GetISelect();
   if (psel != nullptr)
   {
      SetItemCheck(hCurrentElementItem, psel->m_isVisible);
      if (psel->m_isVisible)
         SetItemCheck(hLayerItem, true);
   }
   return hCurrentElementItem != nullptr;
}

LRESULT FilterEditBox::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_KEYUP:
      if ((wparam == VK_RETURN) || (wparam == VK_TAB))
      {
         if (m_layerDialog)
            m_layerDialog->UpdateLayerList(GetWindowText().c_str());
         return FALSE;
      }
   }
   return WndProcDefault(msg, wparam, lparam);
}

BOOL FilterEditBox::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   //const int dispID = LOWORD(wParam);

   switch (HIWORD(wParam))
   {
   case EN_KILLFOCUS:
   case CBN_KILLFOCUS:
   {
      if (m_layerDialog)
         m_layerDialog->UpdateLayerList(GetWindowText().c_str());
      return TRUE;
   }
   }
   return FALSE;
}

