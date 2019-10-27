#ifndef H_WALL_VISUALS_PROPERTY
#define H_WALL_VISUALS_PROPERTY

class WallVisualsProperty: public CDialog
{
public:
    WallVisualsProperty(Surface *wall);
    virtual ~WallVisualsProperty() {}

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    Surface  *m_wall;
    CComboBox m_topImageCombo;
    CComboBox m_sideImageCombo;
    CComboBox m_topMaterialCombo;
    CComboBox m_sideMaterialCombo;
    CComboBox m_slingshotMaterialCombo;
    CEdit     m_disableLightingEdit;
    CEdit     m_disableLightFromBelowEdit;
    CEdit     m_topHeightEdit;
    CEdit     m_bottomHeightEdit;

};

#endif 

