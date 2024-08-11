// license:GPLv3+

#pragma once

class TableLightsProperty: public BasePropertyDialog
{
public:
    TableLightsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TableLightsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CColorDialog m_colorDialog;
    ColorButton m_colorButton1;
    ColorButton m_colorButton2;
    ComboBox    m_envEmissionImageCombo;
    EditBox     m_lightEmissionScaleEdit;
    EditBox     m_lightHeightEdit;
    EditBox     m_lightRangeEdit;
    EditBox     m_envEmissionScaleEdit;
    EditBox     m_sceneLightScaleEdit;
};
