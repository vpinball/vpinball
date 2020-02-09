#ifndef H_LAYERS_LIST_DIALOG
#define  H_LAYERS_LIST_DIALOG

class LayerTreeView: public CTreeView
{
public:
    LayerTreeView(){ }
    ~LayerTreeView(){ }
    virtual HTREEITEM       AddItem(HTREEITEM hParent, LPCTSTR text, IEditable *pedit, int image);
    bool                    AddLayer(const string& name);
    bool                    AddElement(const string& name, IEditable *pedit);
    bool                    ContainsLayer(const string& name) const;
    std::string             GetCurrentLayerName() const;
    HTREEITEM               GetItemByElement(const IEditable* pedit);
    int                     GetItemCount() const;
    int                     GetLayerCount() const;
    std::vector<HTREEITEM>  GetSubItems(HTREEITEM hParent);
    bool                    IsItemChecked(HTREEITEM hItem) const;
    void                    SetAllItemStates(const bool checked);
    void                    DeleteAll();
    void                    ExpandAll();
    void                    CollapsAll();
    void                    SetActiveLayer(const string& name);
    HTREEITEM               GetRootItem() { return hRootItem; }
    HTREEITEM               GetCurrentLayerItem() { return hCurrentLayerItem; }

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
    CImageList  m_normalImages;
};

class LayersListDialog;
class FilterEditBox : public CEdit
{
public:
    FilterEditBox() : m_layerDialog(nullptr) {}
    virtual ~FilterEditBox() {}
    void SetDialog(LayersListDialog* dialog) { m_layerDialog = dialog; }
protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);
private:
    LayersListDialog *m_layerDialog;
};

class LayersListDialog: public CDialog
{
public:
    LayersListDialog();
    virtual ~LayersListDialog();
    virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
    bool AddLayer(const string& name, IEditable *piedit);
    void DeleteLayer();
    void ClearList();
    void UpdateLayerList(const std::string& name="");
    void UpdateElement(IEditable* pedit);
    string GetCurrentSelectedLayerName() const;
    void Expand()
    {
        m_layerTreeView.ExpandAll();
        m_collapsed = false;
    }
    void Collaps()
    {
        m_layerTreeView.CollapsAll();
        m_collapsed = true;
    }

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    void OnAssignButton();

    CResizer        m_resizer;
    LayerTreeView   m_layerTreeView;
    CButton         m_assignButton;
    CButton         m_addLayerButton;      
    CButton         m_deleteLayerButton;
    CButton         m_expandCollapseButton;
    FilterEditBox   m_layerFilterEditBox;
    bool            m_collapsed;
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
