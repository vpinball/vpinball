// license:GPLv3+

#pragma once

class TableLightsProperty final : public BasePropertyDialog
{
public:
    TableLightsProperty(const VectorProtected<ISelect> *pvsel);
    ~TableLightsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
