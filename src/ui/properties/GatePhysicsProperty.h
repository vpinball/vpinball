// license:GPLv3+

#pragma once

class GatePhysicsProperty: public BasePropertyDialog
{
public:
    GatePhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~GatePhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox    m_elasticityEdit;
    EditBox    m_frictionEdit;
    EditBox    m_dampingEdit;
    EditBox    m_gravityFactorEdit;
};
