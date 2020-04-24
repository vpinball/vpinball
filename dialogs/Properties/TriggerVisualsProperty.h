#ifndef H_TRIGGER_VISUALS_PROPERTY
#define H_TRIGGER_VISUALS_PROPERTY

class TriggerVisualsProperty: public BasePropertyDialog
{
public:
    TriggerVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TriggerVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_shapeCombo;
    CComboBox   m_materialCombo;
    CComboBox   m_surfaceCombo;
    EditBox     m_wireThicknessEdit;
    EditBox     m_starRadiusEdit;
    EditBox     m_rotationEdit;
    EditBox     m_animationSpeedEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    vector<string> m_shapeList;
};

#endif
