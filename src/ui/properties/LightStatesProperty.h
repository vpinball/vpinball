// license:GPLv3+

#pragma once

class LightStatesProperty final : public BasePropertyDialog
{
public:
    LightStatesProperty(const VectorProtected<ISelect> *pvsel);
    ~LightStatesProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox     m_blinkPatternEdit;
    EditBox     m_blinkIntervalEdit;
    ComboBox    m_stateCombo;
    vector<string> m_stateList;
};
