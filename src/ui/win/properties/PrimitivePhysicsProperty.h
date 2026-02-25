// license:GPLv3+

#pragma once

class PrimitivePhysicsProperty final : public BasePropertyDialog
{
public:
    PrimitivePhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~PrimitivePhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    HWND      m_hToyCheck;
    EditBox   m_reducePolyEdit;
    EditBox   m_elasticityFalloffEdit;
    EditBox   m_hitThresholdEdit;
    EditBox   m_elasticityEdit;
    EditBox   m_frictionEdit;
    EditBox   m_scatterAngleEdit;
    ComboBox  m_physicsMaterialCombo;
};
