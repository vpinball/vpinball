#ifndef H_LAYERS_LIST_DIALOG
#define  H_LAYERS_LIST_DIALOG

class LayersListDialog: public CDialog
{
public:
    LayersListDialog();
    virtual ~LayersListDialog();
    virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
    bool AddLayer(const string &name);
    void DeleteLayer();
    void ClearList();
    void UpdateLayerList();

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);


private:
    BOOL OnEditListItemLabel(LPARAM lparam);
    BOOL OnListItemChanged(LPARAM lparam);
    void OnAssignButton();

    int ListContains(const string &name);

    CResizer        m_resizer;
    CListView       m_layerListView;
    CButton         m_assignButton;
    CButton         m_addLayerButton;      
    CButton         m_deleteLayerButton;
    CButton         m_layerEditButton;
    CEdit           m_layerNameEditBox;
    int             m_layerCount;
    string          m_currentLayerName;
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
