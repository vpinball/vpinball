#ifndef H_FLIPPER_PHYSICS_PROPERTY
#define H_FLIPPER_PHYSICS_PROPERTY

class FlipperPhysicsProperty: public BasePropertyDialog
{
public:
    FlipperPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~FlipperPhysicsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit m_massEdit;
    CEdit m_strengthEdit;
    CEdit m_elasticityEdit;
    CEdit m_elasticityFalloffEdit;
    CEdit m_frictionEdit;
    CEdit m_returnStrengthEdit;
    CEdit m_coilUpRampEdit;
    CEdit m_scatterAngleEdit;
    CEdit m_eosTorqueEdit;
    CEdit m_eosTorqueAngleEdit;
    CComboBox m_overwriteSettingsCombo;
    vector<string> m_physicSetList;
};

#endif

