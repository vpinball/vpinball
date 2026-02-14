// license:GPLv3+

#pragma once

class TriggerPhysicsProperty final : public BasePropertyDialog
{
public:
    TriggerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~TriggerPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    HWND    m_hEnableCheck;
    EditBox m_hitHeightEdit;
};
