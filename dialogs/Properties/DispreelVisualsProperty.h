#ifndef H_DISPREEL_VISUALS_PROPERTY
#define H_DISPREEL_VISUALS_PROPERTY

class DispreelVisualsProperty: public BasePropertyDialog
{
public:
    DispreelVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~DispreelVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND         m_hBackgroundTransparentCheck;
    HWND         m_hUseImageGridCheck;
    CColorDialog m_colorDialog;
    ColorButton  m_colorButton;
    CComboBox    m_imageCombo;
    EditBox      m_singleDigitRangeEdit;
    EditBox      m_imagePerRowEdit;
    EditBox      m_posXEdit;
    EditBox      m_posYEdit;
    EditBox      m_reelsEdit;
    EditBox      m_reelWidthEdit;
    EditBox      m_reelHeightEdit;
    EditBox      m_reelSpacingEdit;
};

#endif // !H_DISPREEL_VISUALS_PROPERTY
