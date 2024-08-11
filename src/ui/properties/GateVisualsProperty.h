// license:GPLv3+

#pragma once

class GateVisualsProperty: public BasePropertyDialog
{
public:
    GateVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~GateVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    virtual BOOL OnInitDialog();

private:
    ComboBox  m_typeCombo;
    ComboBox  m_materialCombo;
    ComboBox  m_surfaceCombo;
    EditBox   m_xposEdit;
    EditBox   m_yposEdit;
    EditBox   m_lengthEdit;
    EditBox   m_heightEdit;
    EditBox   m_rotationEdit;
    EditBox   m_openAngleEdit;
    EditBox   m_closeAngleEdit;
    vector<string> m_typeList;
};
