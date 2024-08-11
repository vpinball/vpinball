// license:GPLv3+

#pragma once

class HitTargetPhysicsProperty: public BasePropertyDialog
{
public:
    HitTargetPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~HitTargetPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_physicsMaterialCombo;
    EditBox     m_hitThresholdEdit;
    EditBox     m_elasticityEdit;
    EditBox     m_elasticityFalloffEdit;
    EditBox     m_frictionEdit;
    EditBox     m_scatterAngleEdit;
    HWND        m_hLegacyModeCheck;
    HWND        m_hIsDroppedCheck;
};
