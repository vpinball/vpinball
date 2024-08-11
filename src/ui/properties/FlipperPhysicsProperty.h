// license:GPLv3+

#pragma once

class FlipperPhysicsProperty: public BasePropertyDialog
{
public:
    FlipperPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~FlipperPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox m_massEdit;
    EditBox m_strengthEdit;
    EditBox m_elasticityEdit;
    EditBox m_elasticityFalloffEdit;
    EditBox m_frictionEdit;
    EditBox m_returnStrengthEdit;
    EditBox m_coilUpRampEdit;
    EditBox m_scatterAngleEdit;
    EditBox m_eosTorqueEdit;
    EditBox m_eosTorqueAngleEdit;
    ComboBox m_overwriteSettingsCombo;
    vector<string> m_physicSetList;
};
