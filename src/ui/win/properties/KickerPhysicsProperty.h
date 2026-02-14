// license:GPLv3+

#pragma once

class KickerPhysicsProperty final : public BasePropertyDialog
{
public:
    KickerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~KickerPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    HWND    m_hEnableCheck;
    HWND    m_hFallThroughCheck;
    HWND    m_hLegacyCheck;
    EditBox m_scatterAngleEdit;
    EditBox m_hitAccuracyEdit;
    EditBox m_hitHeightEdit;
};
