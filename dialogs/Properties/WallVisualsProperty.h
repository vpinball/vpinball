#ifndef H_WALL_VISUALS_PROPERTY
#define H_WALL_VISUALS_PROPERTY

class WallVisualsProperty: public BasePropertyDialog
{
public:
    WallVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~WallVisualsProperty() {}

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    ComboBox  m_topImageCombo;
    ComboBox  m_sideImageCombo;
    ComboBox  m_topMaterialCombo;
    ComboBox  m_sideMaterialCombo;
    ComboBox  m_slingshotMaterialCombo;
    EditBox   m_disableLightingEdit;
    EditBox   m_disableLightFromBelowEdit;
    EditBox   m_topHeightEdit;
    EditBox   m_bottomHeightEdit;
};

#endif 
