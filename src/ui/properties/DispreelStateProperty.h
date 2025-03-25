// license:GPLv3+

#pragma once

class DispreelStateProperty final : public BasePropertyDialog
{
public:
    DispreelStateProperty(const VectorProtected<ISelect> *pvsel);
    ~DispreelStateProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox    m_soundCombo;
    EditBox     m_motorStepsEdit;
    EditBox     m_updateIntervalEdit;
};
