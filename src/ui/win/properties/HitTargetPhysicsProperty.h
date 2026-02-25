// license:GPLv3+

#pragma once

class HitTargetPhysicsProperty final : public BasePropertyDialog
{
public:
    HitTargetPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~HitTargetPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
