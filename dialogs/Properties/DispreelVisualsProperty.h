#ifndef H_DISPREEL_VISUALS_PROPERTY
#define H_DISPREEL_VISUALS_PROPERTY

class DispreelVisualsProperty: public BasePropertyDialog
{
public:
    DispreelVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~DispreelVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND         m_hBackgroundTransparentCheck;
    HWND         m_hUseImageGridCheck;
    CColorDialog m_colorDialog;
    ColorButton  m_colorButton;
    CEdit        m_singleDigitRangeEdit;
    CComboBox    m_imageCombo;
    CEdit        m_imagePerRowEdit;
    CEdit        m_posXEdit;
    CEdit        m_posYEdit;
    CEdit        m_reelsEdit;
    CEdit        m_reelWidthEdit;
    CEdit        m_reelHeightEdit;
    CEdit        m_reelSpacingEdit;
};

#endif // !H_DISPREEL_VISUALS_PROPERTY
