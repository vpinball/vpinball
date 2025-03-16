// license:GPLv3+

#pragma once

class LayerTreeView final : public CTreeView
{
public:
   LayerTreeView();
   ~LayerTreeView() override = default;

   void SetActiveTable(PinTable* ptable);
   void SetFilter(const string& filter, bool isCaseSensitive);
   void Update();

   void ResetView();
   void Select(IEditable* editable);
   IEditable* GetSelection() const;

   bool PreTranslateMessage(MSG* msg);

protected:
   void OnAttach() override;
   void PreCreate(CREATESTRUCT& cs) override;
   LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
   LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam) override;

   LRESULT OnNMClick(LPNMHDR lpnmh);
   LRESULT OnNMDBClick(LPNMHDR lpnmh);

private:
   HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR text, IEditable* const pedit, int image);

   HTREEITEM   m_hRootItem = nullptr;
   CImageList  m_normalImages;
   CImageList  m_stateImages;
   bool        m_dragging = false;
   PinTable*   m_activeTable = nullptr;
   HACCEL      m_accel;
   vector<HTREEITEM> m_DragItems;

   string m_filter;
   bool m_isCaseSensitiveFilter = false;
   
   struct TreeEntry
   {
      string path;
      IEditable* editable;
      HTREEITEM item;
      bool pendingExpand;
      bool pendingDelete;
   };
   vector<TreeEntry> m_content;
};

class LayersListDialog final : public CDialog
{
public:
   LayersListDialog();
   ~LayersListDialog() override;
   
   void SetActiveTable(PinTable* ptable);
   void Update();
   void AssignToSelectedGroup();
   PartGroup* GetSelectedPartGroup() const;
   void ResetView() { m_layerTreeView.ResetView(); }

   LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
   bool PreTranslateMessage(MSG* msg);
   
protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

private:
   void AddToolTip(const char* const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);

   CResizer      m_resizer;
   LayerTreeView m_layerTreeView;
   CButton       m_assignButton;
   CButton       m_addLayerButton;
   CButton       m_deleteLayerButton;
   CButton       m_expandCollapseButton;
   CEdit         m_layerFilterEditBox;
   CButton       m_layerFilterCaseButton;
   PinTable*     m_activeTable = nullptr;
   HACCEL        m_accel;
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
