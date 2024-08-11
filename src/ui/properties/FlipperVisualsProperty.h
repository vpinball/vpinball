// license:GPLv3+

#pragma once

class FlipperVisualsProperty: public BasePropertyDialog
{
public:
    FlipperVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~FlipperVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
