// license:GPLv3+

#pragma once

class PlungerPhysicsProperty final : public BasePropertyDialog
{
public:
    PlungerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~PlungerPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox m_pullSpeedEdit;
    EditBox m_releaseSpeedEdit;
    EditBox m_strokeLengthEdit;
    EditBox m_scatterVelocityEdit;
    EditBox m_mechStrengthEdit;
    EditBox m_momentumXferEdit;
    EditBox m_parkPositionEdit;
    HWND    m_hEnableMechPlungerCheck;
    HWND    m_hAutoPlungerCheck;
};
