// license:GPLv3+

#include "core/stdafx.h"
#include "LayersListDialog.h"

#define WM_TREE_SEL_CHANGED (WM_USER + 1)

LayersListDialog::LayersListDialog()
   : CDialog(IDD_LAYERS)
   , m_accel(LoadAccelerators(g_pvp->theInstance, MAKEINTRESOURCE(IDR_VPSIMPELACCEL)))
{
}

LayersListDialog::~LayersListDialog() { }

PartGroup* LayersListDialog::GetSelectedPartGroup() const
{
   IEditable* selection = m_layerTreeView.GetSelection();
   if (selection == nullptr)
      return nullptr;
   if (selection->GetItemType() == eItemPartGroup)
      return static_cast<PartGroup*>(selection);
   return selection->GetPartGroup();
}

LRESULT LayersListDialog::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
   // Set window focus. The docker will now report this as active.
   if (!IsChild(::GetFocus()))
      SetFocus();

   return FinalWindowProc(msg, wparam, lparam);
}

void LayersListDialog::AddToolTip(const char* const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { 0 };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = (char*)text;
   ::SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

BOOL LayersListDialog::OnInitDialog()
{
   const HWND toolTipHwnd = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), nullptr, g_pvp->theInstance, nullptr);

   AttachItem(IDC_ADD_LAYER_BUTTON, m_addLayerButton);
   AttachItem(IDC_DELETE_LAYER_BUTTON, m_deleteLayerButton);
   AttachItem(IDC_ASSIGN_BUTTON, m_assignButton);
   AttachItem(IDC_EXPAND_COLLAPSE_BUTTON, m_expandCollapseButton);
   AttachItem(IDC_SELECT, m_selectButton);
   AttachItem(IDC_SYNC, m_syncButton);
   AttachItem(IDC_LAYER_FILTER_EDIT, m_layerFilterEditBox);
   AttachItem(IDC_LAYER_FILTER_CASE_BUTTON, m_layerFilterCaseButton);
   AttachItem(IDC_LAYER_TREEVIEW, m_layerTreeView);

   constexpr int iconSize = 16;
   HANDLE hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ASSIGN), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_assignButton.SetIcon((HICON)hIcon);
   hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_addLayerButton.SetIcon((HICON)hIcon);
   hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_REMOVE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_deleteLayerButton.SetIcon((HICON)hIcon);
   hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_EXPANDCOLLAPSE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_expandCollapseButton.SetIcon((HICON)hIcon);
   hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_SELECT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
   m_selectButton.SetIcon((HICON)hIcon);

   AddToolTip("Assign selected elements to selected layer", GetHwnd(), toolTipHwnd, m_assignButton.GetHwnd());
   AddToolTip("Collapse all", GetHwnd(), toolTipHwnd, m_expandCollapseButton.GetHwnd());
   AddToolTip("Add a new layer", GetHwnd(), toolTipHwnd, m_addLayerButton.GetHwnd());
   AddToolTip("Delete selected layer", GetHwnd(), toolTipHwnd, m_deleteLayerButton.GetHwnd());
   AddToolTip("Filter tree. Only elements that match the filter string will be shown!", GetHwnd(), toolTipHwnd, m_layerFilterEditBox.GetHwnd());
   AddToolTip("Enable case sensitive filtering", GetHwnd(), toolTipHwnd, m_layerFilterCaseButton.GetHwnd());
   AddToolTip("Sync tree with selection", GetHwnd(), toolTipHwnd, m_syncButton.GetHwnd());
   AddToolTip("Edit group properties", GetHwnd(), toolTipHwnd, m_selectButton.GetHwnd());

   m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 200, 200));
   m_resizer.AddChild(m_assignButton.GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_expandCollapseButton.GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_selectButton.GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_addLayerButton.GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_deleteLayerButton.GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_syncButton.GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_layerFilterEditBox.GetHwnd(), CResizer::topright, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_layerFilterCaseButton.GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(m_layerTreeView.GetHwnd(), CResizer::topleft, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
   m_resizer.RecalcLayout();

   UpdateCommands();

   return TRUE;
}

INT_PTR LayersListDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
   m_resizer.HandleMessage(msg, wparam, lparam);

   switch (msg)
   {
   case WM_TREE_SEL_CHANGED: UpdateCommands(); return TRUE;
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
      Update();
      return TRUE;

   case IDC_SELECT:
      if (m_activeTable && GetSelectedPartGroup())
      {
         m_activeTable->ClearMultiSel();
         m_activeTable->AddMultiSel(GetSelectedPartGroup(), true, false, false);
         m_activeTable->RefreshProperties();
         m_activeTable->SetDirtyDraw();
      }
      return TRUE;

   case IDC_ADD_LAYER_BUTTON:
      if (m_activeTable)
      {
         IEditable* const partGroup = EditableRegistry::CreateAndInit(eItemPartGroup, m_activeTable, 0, 0);
         m_activeTable->m_vedit.push_back(partGroup);
         partGroup->SetPartGroup(GetSelectedPartGroup());
         m_activeTable->BeginUndo();
         m_activeTable->m_undo.MarkForCreate(partGroup);
         m_activeTable->EndUndo();
         Update();
      }
      return TRUE;

   case IDC_DELETE_LAYER_BUTTON:
      if (m_activeTable && GetSelectedPartGroup())
      {
         PartGroup* oldParent = GetSelectedPartGroup();
         PartGroup* newParent = oldParent->GetPartGroup();
         if (newParent == nullptr)
         {
            auto existing = std::ranges::find_if(m_activeTable->m_vedit, [oldParent](const IEditable* e)
               { return (e->GetItemType() == eItemPartGroup) && (oldParent != e); });
            if (existing == m_activeTable->m_vedit.end())
               return FALSE;
            newParent = static_cast<PartGroup*>(*existing);
         }
         m_activeTable->BeginUndo();
         std::ranges::for_each(m_activeTable->m_vedit,
            [oldParent, newParent](IEditable* e)
            {
               if (e->GetPartGroup() == oldParent)
                  e->SetPartGroup(newParent);
            });
         oldParent->Delete();
         m_activeTable->EndUndo();
         Update();
      }
      return TRUE;

   case IDC_ASSIGN_BUTTON:
      AssignToSelectedGroup();
      return TRUE;

   case IDC_EXPAND_COLLAPSE_BUTTON:
      ResetView();
      return TRUE;

   case IDC_LAYER_FILTER_EDIT:
   case IDC_LAYER_FILTER_CASE_BUTTON:
      m_layerTreeView.SetFilter(m_layerFilterEditBox.GetWindowText().c_str(), m_layerFilterCaseButton.GetCheck() == BST_UNCHECKED);
      return TRUE;

   default: break;
   }
   return FALSE;
}

void LayersListDialog::AssignToSelectedGroup()
{
   if (m_activeTable == nullptr || m_activeTable->MultiSelIsEmpty())
      return;

   PartGroup* group = GetSelectedPartGroup();
   if (group == nullptr)
   {
      ShowError("Please select a group!");
      return;
   }

   m_activeTable->AssignSelectionToPartGroup(group);
}

BOOL LayersListDialog::PreTranslateMessage(MSG& msg)
{
   if (!IsWindow())
      return FALSE;

   // only pre-translate mouse and keyboard input events
   if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) || (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
   {
      const int keyPressed = LOWORD(msg.wParam);
      // only pass F1-F12 to the main VPinball class to open subdialogs from everywhere
      if (((keyPressed >= VK_F3 && keyPressed <= VK_F12) || (keyPressed == VK_ESCAPE))
         && TranslateAccelerator(g_pvp->GetHwnd(), m_accel, &msg)) //!! VK_ESCAPE is a workaround, otherwise there is a hickup when changing a layername and pressing this
         return TRUE;
   }

   if (m_layerTreeView.PreTranslateMessage(msg))
      return TRUE;

   return IsDialogMessage(msg);
}

void LayersListDialog::SetActiveTable(PinTable* ptable)
{
   m_activeTable = ptable;
   m_layerTreeView.SetActiveTable(ptable);
   if (m_syncButton.GetCheck() == BST_CHECKED)
   {
      ISelect* const sel = m_activeTable->m_vmultisel.ElementAt(0);
      m_layerTreeView.Select(sel ? sel->GetIEditable() : nullptr);
   }
   UpdateCommands();
}

void LayersListDialog::UpdateCommands()
{
   // Enable/disable treeview and buttons depending on the active table
   m_layerTreeView.EnableWindow(m_activeTable ? TRUE : FALSE);
   m_addLayerButton.EnableWindow(m_activeTable ? TRUE : FALSE);
   m_expandCollapseButton.EnableWindow(m_activeTable ? TRUE : FALSE);
   m_syncButton.EnableWindow(m_activeTable ? TRUE : FALSE);
   m_layerFilterEditBox.EnableWindow(m_activeTable ? TRUE : FALSE);
   m_layerFilterCaseButton.EnableWindow(m_activeTable ? TRUE : FALSE);
   // Enable/disable buttons depending on the current selection
   const bool isPartGroupSelected = GetSelectedPartGroup() != nullptr;
   m_assignButton.EnableWindow(m_activeTable && isPartGroupSelected ? TRUE : FALSE);
   m_deleteLayerButton.EnableWindow(m_activeTable && isPartGroupSelected ? TRUE : FALSE);
   m_selectButton.EnableWindow(m_activeTable && isPartGroupSelected ? TRUE : FALSE);
}

void LayersListDialog::Update()
{
   m_layerTreeView.Update();
   if (m_syncButton.GetCheck() == BST_CHECKED)
   {
      ISelect* const sel = m_activeTable->m_vmultisel.ElementAt(0);
      m_layerTreeView.Select(sel ? sel->GetIEditable() : nullptr);
   }
   UpdateCommands();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////

LayerTreeView::LayerTreeView()
   : m_accel(LoadAccelerators(g_pvp->theInstance, MAKEINTRESOURCE(IDR_VPSIMPELACCEL)))
{
}

void LayerTreeView::SetActiveTable(PinTable* ptable)
{
   m_activeTable = ptable;
   Update();
}

void LayerTreeView::Select(IEditable* editable)
{
   auto existing = std::ranges::find_if(m_content, [editable](const TreeEntry& te) { return te.editable == editable; });
   if (existing != m_content.end())
      SelectItem(existing->item);
}

IEditable* LayerTreeView::GetSelection() const
{
   HTREEITEM item = CTreeView::GetSelection();
   auto existing = std::ranges::find_if(m_content, [item](const TreeEntry& te) { return te.item == item; });
   return existing == m_content.end() ? nullptr : existing->editable;
}

void LayerTreeView::SetFilter(const string& filter, bool isCaseSensitive)
{
   if ((m_filter != filter) || (m_isCaseSensitiveFilter != isCaseSensitive))
   {
      m_filter = filter;
      m_isCaseSensitiveFilter = isCaseSensitive;
      Update();
   }
}

void LayerTreeView::Update()
{
   if (m_activeTable == nullptr)
   {
      DeleteAllItems();
      m_hRootItem = AddItem(nullptr, _T("Layers"), nullptr, 0);
      TreeView_SetItemState(GetHwnd(), m_hRootItem, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
      return;
   }
   
   // Build new content list (in the end, a tree is a hierarchically displayed list)
   std::stringstream ss;
   vector<PartGroup*> itemPath;
   vector<TreeEntry> newContent;
   const string filter = m_isCaseSensitiveFilter ? lowerCase(m_filter) : m_filter;
   for (const auto& editable : m_activeTable->m_vedit)
   {
      string name = editable->GetName();
      if (!filter.empty() && editable->GetItemType() != eItemPartGroup)
      {
         if (!m_isCaseSensitiveFilter)
         {
            const string objName { lowerCase(name) };
            if (objName.find(filter) == string::npos)
               continue;
         }
         else
         {
            if (name.find(filter) == string::npos)
               continue;
         }
      }
      itemPath.clear();
      PartGroup* parent = editable->GetPartGroup();
      while (parent != nullptr)
      {
         itemPath.insert(itemPath.begin(), parent);
         parent = parent->GetPartGroup();
      }
      ss.clear();
      for (const auto& group : itemPath)
         ss << group->GetName() << '/';
      ss << name;
      newContent.emplace_back(ss.str(), editable, nullptr, false, false);
      StrToLower(newContent.back().path);
   }
   std::ranges::sort(newContent,
      [](const TreeEntry& a, const TreeEntry& b)
      {
         // Put PartGroup at beginning of content list, then sort alphabetically
         bool aPG = a.editable->GetItemType() == eItemPartGroup;
         bool bPG = b.editable->GetItemType() == eItemPartGroup;
         return (aPG && !bPG) ? true
              : (!aPG && bPG) ? false
              : a.path < b.path;
      });

   // Identify differences and update Win32 control
   SetRedraw(FALSE);
   auto oldContentIt = m_content.begin();
   IEditable* prevSelection = GetSelection();
   int globalVisibility = -1;
   for (auto& node : newContent)
   {
      while ((oldContentIt < m_content.end()) && (oldContentIt->pendingDelete))
         ++oldContentIt;
      if ((oldContentIt < m_content.end()) && (oldContentIt->path == node.path) && (oldContentIt->editable == node.editable))
      {
         // Reuse existing element
         node.item = oldContentIt->item;
         ++oldContentIt;
      }
      else
      {
         // Insert the new element at the right place
         if (node.editable->GetPartGroup() == nullptr)
         {
            node.item = AddItem(m_hRootItem, node.editable->GetName(), node.editable, 1);
         }
         else
         {
            const auto& parent = std::ranges::find_if(newContent, [node](const auto& node2) { return node.editable->GetPartGroup() == node2.editable; });
            node.item = AddItem(parent->item, node.editable->GetName(), node.editable, 2);
         }
         // If new elements was already part of the tree but at another place, persists its state
         auto existing = std::find_if(oldContentIt, m_content.end(), [node](const TreeEntry& te) { return te.editable == node.editable; });
         if (existing != m_content.end())
         {
            // Persist collapsed/expnded state
            TVITEM tvi = {};
            tvi.mask = TVIF_STATE;
            tvi.hItem = existing->item;
            tvi.stateMask = TVIS_EXPANDED;
            if ((node.editable->GetItemType() == eItemPartGroup) && GetItem(tvi) && ((tvi.state & TVIS_EXPANDED) != 0))
               node.pendingExpand = true;

            // Mark for deletion, with all its children
            existing->pendingDelete = true;
            if (existing->editable->GetItemType() == eItemPartGroup)
               std::ranges::for_each(m_content, [existing](TreeEntry& te) {
                     PartGroup* pg = te.editable->GetPartGroup();
                     while (pg != nullptr && pg != existing->editable)
                        pg = pg->GetPartGroup();
                     if (pg == existing->editable)
                        te.pendingDelete = true;
                  });
         }
      }

      // Update visibility checkboxes
      int state;
      if (node.editable->GetItemType() == eItemPartGroup)
      {
         bool show = false, hide = false;
         for (auto e : m_activeTable->m_vedit)
            if (e->GetPartGroup() == node.editable && e->GetISelect())
            {
               show |= e->GetISelect()->m_isVisible;
               hide |= !e->GetISelect()->m_isVisible;
            }
         state = (show && !hide) ? 1 : (!show && hide) ? 2 : 3;
      }
      else
      {
         const ISelect* const select = node.editable->GetISelect();
         state = (select && select->m_isVisible) ? 1 : (select && !select->m_isVisible) ? 2 : 3;
      }
      TreeView_SetItemState(GetHwnd(), node.item, INDEXTOSTATEIMAGEMASK(state), TVIS_STATEIMAGEMASK);
      globalVisibility = globalVisibility == 3 ? 3 : globalVisibility == -1 ? state : (globalVisibility != state) ? 3 : globalVisibility;
   }
   TreeView_SetItemState(GetHwnd(), m_hRootItem, INDEXTOSTATEIMAGEMASK(globalVisibility), TVIS_STATEIMAGEMASK);
   while (oldContentIt < m_content.end())
   {
      oldContentIt->pendingDelete = true;
      ++oldContentIt;
   }
   
   // Remove old items
   std::ranges::for_each(m_content, [&](TreeEntry& te)
      {
         if (te.pendingDelete)
            DeleteItem(te.item);
      });
   m_content = newContent;
   
   // Sort tree (somewhat overkill, but since we don't bother inserting items at the right place...)
   SortChildren(m_hRootItem, false);
   std::ranges::for_each(m_content, [&](TreeEntry& te)
      {
         if (te.pendingExpand)
            Expand(te.item, TVE_EXPAND);
         if (te.editable->GetItemType() == eItemPartGroup)
            SortChildren(te.item, false);
      });

   // Persist selection
   Select(prevSelection);

   SetRedraw(TRUE);
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

void LayerTreeView::ResetView()
{
   SetRedraw(FALSE);
   HTREEITEM item = GetChild(m_hRootItem);
   while (item)
   {
      Expand(item, TVE_COLLAPSE);
      item = GetNextItem(item, TVGN_NEXT);
   }
   Expand(m_hRootItem, TVE_EXPAND);
   SetRedraw(TRUE);
}

BOOL LayerTreeView::PreTranslateMessage(MSG& msg)
{
   if (!IsWindow())
      return FALSE;

   if (msg.hwnd != GetHwnd())
      return FALSE;

   const int keyPressed = LOWORD(msg.wParam);
   if (keyPressed != VK_F2)
   {
      // only pre-translate mouse and keyboard input events
      if (((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) || (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
         && TranslateAccelerator(GetHwnd(), m_accel, &msg))
         return TRUE;
   }

   return IsDialogMessage(msg);
}

void LayerTreeView::OnAttach()
{
   m_normalImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 0);
   m_normalImages.Add(CBitmap(IDB_LAYER_ICONS16), RGB(255, 255, 255));
   SetImageList(m_normalImages, TVSIL_NORMAL);

   DWORD style = GetStyle();
   style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_CHECKBOXES;
   SetStyle(style);

   m_stateImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 0);
   m_stateImages.Add(CBitmap(IDB_TRISTATE_ICONS16), RGB(255, 255, 255));
   SetImageList(m_stateImages, TVSIL_STATE);

   SetActiveTable(nullptr);
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
         EditLabel(CTreeView::GetSelection());
      break;
      
   case WM_MOUSEACTIVATE:
      SetFocus();
      break;
      
   case WM_MOUSEMOVE:
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

   case WM_LBUTTONUP:
      if (m_dragging)
      {
         ImageList_DragLeave(GetHwnd());
         ImageList_EndDrag();
         HTREEITEM hSelectedDrop = GetDropHiLightItem();
         SelectDropTarget(nullptr);
         auto dropTarget = std::ranges::find_if(m_content, [hSelectedDrop](const TreeEntry& te) { return te.item == hSelectedDrop; });
         if ((hSelectedDrop == m_hRootItem) || (dropTarget != m_content.end()))
         {
            SelectItem(hSelectedDrop);
            PartGroup* group = (hSelectedDrop == m_hRootItem) ? nullptr 
                             : (dropTarget->editable->GetItemType() == eItemPartGroup) ? static_cast<PartGroup*>(dropTarget->editable)
                             : dropTarget->editable->GetPartGroup();
            m_activeTable->BeginUndo();
            for (const auto& dragItem : m_DragItems)
            {
               auto existing = std::ranges::find_if(m_content, [dragItem](const TreeEntry& te) { return te.item == dragItem; });
               if (existing == m_content.end())
                  continue; // Item destroyed while dragging. Can this even happen ?
               if (group == nullptr && existing->editable->GetItemType() != eItemPartGroup)
                  continue;
               existing->editable->SetPartGroup(group);
               Select(existing->editable);
            }
            m_activeTable->EndUndo();
            Update();
         }
         m_DragItems.clear();
         ReleaseCapture();
         m_dragging = false;
      }
      break;
   }
   return WndProcDefault(msg, wparam, lparam);
}

LRESULT LayerTreeView::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
{
   if (m_activeTable == nullptr)
      return FALSE;

   const LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lparam);

   switch (lpnmh->code)
   {
   case TVN_BEGINDRAG:
   {
      const LPNMTREEVIEW lpnmtv = reinterpret_cast<LPNMTREEVIEW>(lpnmh);
      HIMAGELIST hImg = TreeView_CreateDragImage(GetHwnd(), lpnmtv->itemNew.hItem);
      ImageList_BeginDrag(hImg, 0, 0, 0);
      ImageList_DragEnter(GetHwnd(), lpnmtv->ptDrag.x, lpnmtv->ptDrag.y);
      m_DragItems.push_back(lpnmtv->itemNew.hItem);
      SetCapture();
      m_dragging = true;
      return TRUE;
   }
   case TVN_SELCHANGED:
   {
      GetParent().SendMessage(WM_TREE_SEL_CHANGED);
      return FALSE;
   }
   case TVN_ENDLABELEDIT:
   {
      const LPNMTVDISPINFO pinfo = reinterpret_cast<LPNMTVDISPINFO>(lpnmh);
      if (pinfo->item.pszText == nullptr || pinfo->item.pszText[0] == '\0')
         return FALSE;
      auto existing = std::ranges::find_if(m_content, [pinfo](const TreeEntry& te) { return te.item == pinfo->item.hItem; });
      if (existing != m_content.end())
      {
         existing->editable->SetName(pinfo->item.pszText);
         Update();
      }
      return TRUE;
   }
      
   case NM_CLICK:
      return OnNMClick(lpnmh);
      
   case NM_DBLCLK:
      return OnNMDBClick(lpnmh);
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
      if (ht.hItem == m_hRootItem)
      {
         bool visible = std::ranges::find_if(m_content, [](const TreeEntry& te) { return !te.editable->GetISelect()->m_isVisible; }) != m_content.end();
         std::ranges::for_each(m_content, [visible](const TreeEntry& te) { te.editable->GetISelect()->m_isVisible = visible; });
      }
      else
      {
         auto selectedItem = std::ranges::find_if(m_content, [ht](const TreeEntry& te) { return te.item == ht.hItem; });
         if (selectedItem != m_content.end())
         {
            auto selected = selectedItem->editable;
            if (selected->GetItemType() != eItemPartGroup)
               selected->GetISelect()->m_isVisible = !selected->GetISelect()->m_isVisible;
            else
            {
               bool visible = std::ranges::find_if(m_content, 
                  [&, selected](const TreeEntry& te)
                  {
                     PartGroup* pg = te.editable->GetPartGroup();
                     while (pg != nullptr && pg != selected)
                        pg = pg->GetPartGroup();
                     if (pg == selected)
                        return !te.editable->GetISelect()->m_isVisible; 
                     return false;
                  }) != m_content.end();
               std::ranges::for_each(m_content,
                  [&, selected, visible](const TreeEntry& te)
                  {
                     PartGroup* pg = te.editable->GetPartGroup();
                     while (pg != nullptr && pg != selected)
                        pg = pg->GetPartGroup();
                     if (pg == selected)
                        te.editable->GetISelect()->m_isVisible = visible;
                  });
            }
         }
      }
      if (m_activeTable != nullptr)
      {
         m_activeTable->SetDirty(eSaveDirty);
         m_activeTable->SetDirtyDraw();
      }
      Update();
      return TRUE;
   }
   return FALSE;
}

LRESULT LayerTreeView::OnNMDBClick(LPNMHDR lpnmh)
{
   if (m_activeTable == nullptr)
      return FALSE;

   const DWORD dwpos = GetMessagePos();
   TVHITTESTINFO ht = {};
   ht.pt.x = GET_X_LPARAM(dwpos);
   ht.pt.y = GET_Y_LPARAM(dwpos);
   ::MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);
   HitTest(ht);
   if (ht.flags & TVHT_ONITEMSTATEICON)
      return FALSE;

   auto selectedItem = std::ranges::find_if(m_content, [ht](const TreeEntry& te) { return te.item == ht.hItem; });
   if (selectedItem == m_content.end())
      return FALSE;
   auto selected = selectedItem->editable;

   m_activeTable->ClearMultiSel();
   if (selected->GetItemType() == eItemPartGroup)
      std::ranges::for_each(m_content,[&, selected](const TreeEntry& te)
         { 
            PartGroup* pg = te.editable->GetPartGroup();
            while (pg != nullptr && pg != selected)
               pg = pg->GetPartGroup();
            if (pg == selected)
               m_activeTable->AddMultiSel(te.editable->GetISelect(), true, false, false);
         });
   else
      m_activeTable->AddMultiSel(selected->GetISelect(), true, false, false);
   m_activeTable->RefreshProperties();
   m_activeTable->SetDirtyDraw();
   return TRUE;
}
