// license:GPLv3+

#pragma once

class KickerVisualsProperty: public BasePropertyDialog
{
public:
    KickerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~KickerVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox  m_materialCombo;
    ComboBox  m_displayCombo;
    ComboBox  m_surfaceCombo;
    EditBox   m_radiusEdit;
    EditBox   m_orientationEdit;
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    vector<string> m_typeList;
};
