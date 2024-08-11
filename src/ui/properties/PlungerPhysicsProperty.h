// license:GPLv3+

#pragma once

class PlungerPhysicsProperty: public BasePropertyDialog
{
public:
    PlungerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~PlungerPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
