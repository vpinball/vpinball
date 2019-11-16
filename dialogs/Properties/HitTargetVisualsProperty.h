#ifndef H_HIT_TARGET_VISUALS_PROPERTY
#define H_HIT_TARGET_VISUALS_PROPERTY

class HitTargetVisualsProperty: public BasePropertyDialog
{
public:
    HitTargetVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~HitTargetVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    vector<string> m_typeList;
    CComboBox   m_imageCombo;
    CComboBox   m_materialCombo;
    CComboBox   m_typeCombo;
    CEdit       m_dropSpeedEdit;
    CEdit       m_raiseDelayEdit;
    CEdit       m_depthBiasEdit;
    CEdit       m_disableLightingEdit;
    CEdit       m_disableLightBelowEdit;
    CEdit       m_posXEdit;
    CEdit       m_posYEdit;
    CEdit       m_posZEdit;
    CEdit       m_scaleXEdit;
    CEdit       m_scaleYEdit;
    CEdit       m_scaleZEdit;
    CEdit       m_orientationEdit;
};

#endif
