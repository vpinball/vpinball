// license:GPLv3+

#pragma once

class KickerPhysicsProperty: public BasePropertyDialog
{
public:
    KickerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~KickerPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND    m_hEnableCheck;
    HWND    m_hFallThroughCheck;
    HWND    m_hLegacyCheck;
    EditBox m_scatterAngleEdit;
    EditBox m_hitAccuracyEdit;
    EditBox m_hitHeightEdit;
};
