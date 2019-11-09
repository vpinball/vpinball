#ifndef H_WALL_VISUALS_PROPERTY
#define H_WALL_VISUALS_PROPERTY

class WallVisualsProperty: public BasePropertyDialog
{
public:
    WallVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~WallVisualsProperty() {}

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
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
