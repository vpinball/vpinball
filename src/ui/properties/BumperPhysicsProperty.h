// license:GPLv3+

#pragma once

class BumperPhysicsProperty: public BasePropertyDialog
{
public:
    BumperPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~BumperPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox   m_forceEdit;
    EditBox   m_hitThresholdEdit;
    EditBox   m_scatterAngleEdit;
};
