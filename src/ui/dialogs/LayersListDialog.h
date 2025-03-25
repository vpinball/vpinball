// license:GPLv3+

#pragma once

#include <memory>

class LayerTreeView final : public CTreeView
{
public:
   LayerTreeView();
   ~LayerTreeView() override { }

   HTREEITEM               AddItem(HTREEITEM hParent, LPCTSTR text, IEditable* const pedit, int image);
   bool                    AddLayer(const string& name);
   bool                    AddElement(const string& name, IEditable* const pedit);
   bool                    AddElementToLayer(const HTREEITEM hLayerItem, const string& name, IEditable* const pedit);
   bool                    ContainsLayer(const string& name) const;
   string                  GetCurrentLayerName() const;
   HTREEITEM               GetLayerByElement(const IEditable* const pedit);
   HTREEITEM               GetLayerByItem(HTREEITEM hChildItem);
   HTREEITEM               GetItemByElement(const IEditable* const pedit);
   HTREEITEM               GetCurrentElement() const { return hCurrentElementItem; }
   int                     GetItemCount() const;
   int                     GetLayerCount() const;
   vector<HTREEITEM>       GetAllLayerItems() const;
   vector<HTREEITEM>       GetSubItems(HTREEITEM hParent);
   int                     GetSubItemsCount(HTREEITEM hParent) const;
   bool                    IsItemChecked(HTREEITEM hItem) const;
   void                    SetItemCheck(HTREEITEM item, bool checked);
   void                    SetAllItemStates(const bool checked);
   void                    DeleteAll();
   void                    ExpandAll();
   void                    CollapseAll();
   void                    ExpandLayers();
   void                    CollapseLayer();
   void                    SetActiveLayer(const string& name);
   HTREEITEM               GetRootItem() const { return hRootItem; }
   HTREEITEM               GetCurrentLayerItem() const { return hCurrentLayerItem; }
   HTREEITEM               GetFirstLayer() const { return GetChild(hRootItem); }
   string                  GetLayerName(HTREEITEM item) const { return string{GetItemText(item)}; }
   bool                    PreTranslateMessage(MSG* msg);
   void                    SetActiveTable(PinTable* ptable) { m_activeTable = ptable; }
   vector<string>          GetAllLayerNames() const;


protected:
   void OnAttach() override;
   void PreCreate(CREATESTRUCT& cs) override;
   LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
   LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam) override;

   LRESULT OnNMClick(LPNMHDR lpnmh);
   LRESULT OnNMDBClick(LPNMHDR lpnmh);
   LRESULT OnTVNSelChanged(LPNMTREEVIEW pNMTV);

private:
   HTREEITEM   hRootItem;
   HTREEITEM   hCurrentLayerItem;
   HTREEITEM   hCurrentElementItem;
   CImageList  m_normalImages;
   bool        m_dragging;
   PinTable*   m_activeTable = nullptr;
   HACCEL      m_accel;
   struct  DragItem
   {
      HTREEITEM   m_hDragItem;
      HTREEITEM   m_hDragLayer;
   };
   vector<std::shared_ptr<DragItem>> m_DragItems;
};

class LayersListDialog;

class FilterEditBox final : public CEdit
{
public:
   FilterEditBox() : m_layerDialog(nullptr) {}
   ~FilterEditBox() override { }
   void SetDialog(LayersListDialog* dialog) { m_layerDialog = dialog; }

protected:
   LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
   BOOL    OnCommand(WPARAM wParam, LPARAM lParam) override;

private:
   LayersListDialog* m_layerDialog;   
};

class LayersListDialog final : public CDialog
{
public:
   LayersListDialog();
   ~LayersListDialog() override;

   LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
   bool AddLayer(const string& name, IEditable* piedit);
   void DeleteLayer();
   void ClearList();
   void UpdateLayerList(const string& name = string());
   void UpdateElement(IEditable* const pedit);
   void DeleteElement(IEditable* const pedit);
   string GetCurrentSelectedLayerName() const;
   void AddToolTip(const char* const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
   void OnAssignButton();
   void AssignToLayerByIndex(size_t index);
   bool PreTranslateMessage(MSG* msg);
   void SetActiveTable(PinTable* ptable) { m_activeTable = ptable; m_layerTreeView.SetActiveTable(ptable); }
   vector<string> GetAllLayerNames() const;

   void UpdateLayerInfo();

   void ExpandAll()
   {
      m_layerTreeView.ExpandAll();
      m_collapsed = false;
   }

   void CollapseAll()
   {
      m_layerTreeView.CollapseAll();
      m_collapsed = true;
   }

   void ExpandLayers()
   {
      m_layerTreeView.ExpandLayers();
      m_collapsed = false;
   }
   void CollapseLayers()
   {
      m_layerTreeView.CollapseLayer();
      m_collapsed = true;
   }
   bool GetCaseSensitiveFilter() const
   {
       return m_isCaseSensitive;
   }
   void SetCaseSensitiveFilter(const bool enable)
   {
       m_isCaseSensitive = enable;
   }
   
   HWND GetLayerTreeHwnd() const { return m_layerTreeView.GetHwnd(); }

protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

private:
   CResizer        m_resizer;
   LayerTreeView   m_layerTreeView;
   CButton         m_assignButton;
   CButton         m_addLayerButton;
   CButton         m_deleteLayerButton;
   CButton         m_expandCollapseButton;
   FilterEditBox   m_layerFilterEditBox;
   CButton         m_layerFilterCaseButton;
   bool            m_collapsed;
   bool            m_isCaseSensitive;
   PinTable* m_activeTable = nullptr;
   HACCEL          m_accel;
};

class CContainLayers final : public CDockContainer
{
public:
   CContainLayers();
   ~CContainLayers() override {}

   LayersListDialog* GetLayersDialog()
   {
      return &m_layersDialog;
   }

private:
   LayersListDialog m_layersDialog;
};

class CDockLayers final : public CDocker
{
public:
   CDockLayers();
   ~CDockLayers() override {}
   void OnClose() override;

   CContainLayers* GetContainLayers()
   {
      return &m_layersContainer;
   }

private:
   CContainLayers m_layersContainer;
};
