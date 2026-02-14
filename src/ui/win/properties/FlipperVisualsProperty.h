// license:GPLv3+

#pragma once

class FlipperVisualsProperty final : public BasePropertyDialog
{
public:
    FlipperVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~FlipperVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox  m_imageCombo;
    ComboBox  m_materialCombo;
    ComboBox  m_rubberMaterialCombo;
    ComboBox  m_surfaceCombo;
    EditBox   m_rubberThicknessEdit;
    EditBox   m_rubberOffsetHeightEdit;
    EditBox   m_rubberWidthEdit;
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    EditBox   m_baseRadiusEdit;
    EditBox   m_endRadiusEdit;
    EditBox   m_lengthEdit;
    EditBox   m_startAngleEdit;
    EditBox   m_endAngleEdit;
    EditBox   m_heightEdit;
    EditBox   m_maxDifficultLengthEdit;
};
