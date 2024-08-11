// license:GPLv3+

#pragma once

class DispreelStateProperty: public BasePropertyDialog
{
public:
    DispreelStateProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~DispreelStateProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_soundCombo;
    EditBox     m_motorStepsEdit;
    EditBox     m_updateIntervalEdit;
};
