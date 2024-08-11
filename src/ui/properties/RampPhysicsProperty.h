// license:GPLv3+

#pragma once

class RampPhysicsProperty: public BasePropertyDialog
{
public:
    RampPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~RampPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox  m_physicsMaterialCombo;
    EditBox   m_hitThresholdEdit;
    EditBox   m_leftWallEdit;
    EditBox   m_rightWallEdit;
    EditBox   m_elasticityEdit;
    EditBox   m_frictionEdit;
    EditBox   m_scatterAngleEdit;
};
