// license:GPLv3+

#pragma once

class LightStatesProperty: public BasePropertyDialog
{
public:
    LightStatesProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~LightStatesProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox     m_blinkPatternEdit;
    EditBox     m_blinkIntervalEdit;
    ComboBox    m_stateCombo;
    vector<string> m_stateList;
};
