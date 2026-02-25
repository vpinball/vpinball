// license:GPLv3+

#pragma once

class RampPhysicsProperty final : public BasePropertyDialog
{
public:
    RampPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~RampPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox  m_physicsMaterialCombo;
    EditBox   m_hitThresholdEdit;
    EditBox   m_leftWallEdit;
    EditBox   m_rightWallEdit;
    EditBox   m_elasticityEdit;
    EditBox   m_frictionEdit;
    EditBox   m_scatterAngleEdit;
};
