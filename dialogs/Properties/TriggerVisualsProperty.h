#ifndef H_TRIGGER_VISUALS_PROPERTY
#define H_TRIGGER_VISUALS_PROPERTY

class TriggerVisualsProperty: public BasePropertyDialog
{
public:
    TriggerVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TriggerVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_shapeCombo;
    CComboBox   m_materialCombo;
    CComboBox   m_surfaceCombo;
    CEdit       m_wireThicknessEdit;
    CEdit       m_starRadiusEdit;
    CEdit       m_rotationEdit;
    CEdit       m_animationSpeedEdit;
    CEdit       m_posXEdit;
    CEdit       m_posYEdit;
    vector<string> m_shapeList;
};

#endif
