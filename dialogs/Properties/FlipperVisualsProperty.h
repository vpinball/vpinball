#ifndef H_FLIPPER_VISUALS_PROPERTY
#define H_FLIPPER_VISUALS_PROPERTY

class FlipperVisualsProperty: public BasePropertyDialog
{
public:
    FlipperVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~FlipperVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_imageCombo;
    CComboBox m_materialCombo;
    CComboBox m_rubberMaterialCombo;
    CComboBox m_surfaceCombo;
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

#endif // !H_FLIPPER_VISUALS_PROPERTY
