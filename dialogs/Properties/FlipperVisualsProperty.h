#ifndef H_FLIPPER_VISUALS_PROPERTY
#define H_FLIPPER_VISUALS_PROPERTY

class FlipperVisualsProperty: public BaseProperty
{
public:
    FlipperVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~FlipperVisualsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CComboBox m_imageCombo;
    CComboBox m_materialCombo;
    CComboBox m_rubberMaterialCombo;
    CComboBox m_surfaceCombo;
    CEdit     m_rubberThicknessEdit;
    CEdit     m_rubberOffsetHeightEdit;
    CEdit     m_rubberWidthEdit;
    CEdit     m_posXEdit;
    CEdit     m_posYEdit;
    CEdit     m_baseRadiusEdit;
    CEdit     m_endRadiusEdit;
    CEdit     m_lengthEdit;
    CEdit     m_startAngleEdit;
    CEdit     m_endAngleEdit;
    CEdit     m_heightEdit;
    CEdit     m_maxDifficultLengthEdit;

};

#endif // !H_FLIPPER_VISUALS_PROPERTY

