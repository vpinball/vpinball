// license:GPLv3+

#pragma once

class RubberPhysicsProperty final : public BasePropertyDialog
{
public:
    RubberPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~RubberPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox    m_physicsMaterialCombo;
    EditBox     m_elasticityEdit;
    EditBox     m_elasticityFallOffEdit;
    EditBox     m_frictionEdit;
    EditBox     m_scatterAngleEdit;
    EditBox     m_hitHeightEdit;
};
