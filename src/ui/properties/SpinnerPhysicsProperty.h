// license:GPLv3+

#pragma once

class SpinnerPhysicsProperty: public BasePropertyDialog
{
public:
    SpinnerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~SpinnerPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox m_dampingEdit;
    EditBox m_elasticityEdit;
};
