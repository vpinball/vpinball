#ifndef H_WALL_VISUALS_PROPERTY
#define H_WALL_VISUALS_PROPERTY

class WallVisualsProperty: public BasePropertyDialog
{
public:
    WallVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~WallVisualsProperty() {}

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_topImageCombo;
    CComboBox m_sideImageCombo;
    CComboBox m_topMaterialCombo;
    CComboBox m_sideMaterialCombo;
    CComboBox m_slingshotMaterialCombo;
    EditBox   m_disableLightingEdit;
    EditBox   m_disableLightFromBelowEdit;
    EditBox   m_topHeightEdit;
    EditBox   m_bottomHeightEdit;
};

#endif 
