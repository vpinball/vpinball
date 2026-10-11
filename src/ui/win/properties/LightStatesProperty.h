// license:GPLv3+

#pragma once

#include "PropertyDialog.h"

class LightStatesProperty final : public BasePropertyDialog
{
public:
    LightStatesProperty(const vector<IWinUIPart *> *pvsel);
    ~LightStatesProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox     m_blinkPatternEdit;
    EditBox     m_blinkIntervalEdit;
    EditBox m_stateSrcLinkEdit;
    ComboBox    m_stateCombo;
    vector<string> m_stateList;
};
