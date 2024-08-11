// license:GPLv3+

#pragma once

class TriggerPhysicsProperty: public BasePropertyDialog
{
public:
    TriggerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TriggerPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND    m_hEnableCheck;
    EditBox m_hitHeightEdit;
};
