#ifndef H_LAYERS_LIST_DIALOG
#define  H_LAYERS_LIST_DIALOG

class LayerTreeView: public CTreeView
{
public:
    LayerTreeView(){ }
    ~LayerTreeView(){ }
    virtual HTREEITEM       AddItem(HTREEITEM hParent, LPCTSTR text, IEditable *pedit, int image);
    bool                    AddLayer(const std::string name);
    bool                    AddElement(const std::string name, IEditable *pedit);
    bool                    ContainsLayer(const std::string name) const;
    std::string             GetCurrentLayerName() const;
    int                     GetItemCount() const;
    std::vector<HTREEITEM>  GetSubItems(HTREEITEM hParent);
    bool                    IsItemChecked(HTREEITEM hItem) const;
    void                    SetAllItemStates(const bool checked);
    void                    DeleteAll();
    void                    ExpandAll();
    void                    CollapsAll();
    void                    SetActiveLayer(const std::string name);

protected:
    virtual void OnAttach();
    virtual void PreCreate(CREATESTRUCT &cs);
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

    virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);
    virtual LRESULT OnNMClick(LPNMHDR lpnmh);
    virtual LRESULT OnNMDBClick(LPNMHDR lpnmh);
    virtual LRESULT OnTVNSelChanged(LPNMTREEVIEW pNMTV);

private:
    HTREEITEM   hRootItem;
    HTREEITEM   hCurrentLayerItem;
    HTREEITEM   hCurrentElementItem;
};

class LayersListDialog: public CDialog
{
public:
    LayersListDialog();
    virtual ~LayersListDialog();
    virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
    bool AddLayer(const string &name, IEditable *piedit);
    void DeleteLayer();
    void ClearList();
    void UpdateLayerList();
    string GetCurrentSelectedLayerName() const;
    void Expand()
    {
        m_layerTreeView.ExpandAll();
    }
    void Collaps()
    {
        m_layerTreeView.CollapsAll();
    }

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);


private:
    BOOL OnEditListItemLabel(LPARAM lparam);
    BOOL OnListItemChanged(LPARAM lparam);
    void OnAssignButton();

    CResizer        m_resizer;
    LayerTreeView   m_layerTreeView;
    CButton         m_assignButton;
    CButton         m_addLayerButton;      
    CButton         m_deleteLayerButton;
    CEdit           m_layerNameEditBox;
    int             m_layerCount;
};

class CContainLayers: public CDockContainer
{
public:
    CContainLayers();
    ~CContainLayers()
    {
    }

    LayersListDialog *GetLayersDialog()
    {
        return &m_layersDialog;
    }

private:
    LayersListDialog m_layersDialog;
};

class CDockLayers: public CDocker
{
public:
    CDockLayers();
    virtual ~CDockLayers()
    {
    }
    virtual void OnDestroy();

    CContainLayers *GetContainLayers()
    {
        return &m_layersContainer;
    }

private:
    CContainLayers m_layersContainer;
};

#endif
