// license:GPLv3+

#pragma once

class GatePhysicsProperty final : public BasePropertyDialog
{
public:
    GatePhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~GatePhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox    m_elasticityEdit;
    EditBox    m_frictionEdit;
    EditBox    m_dampingEdit;
    EditBox    m_gravityFactorEdit;
};
