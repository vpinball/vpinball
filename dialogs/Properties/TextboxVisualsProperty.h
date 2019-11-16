#ifndef H_TEXTBOX_VISUALS_PROPERTY
#define H_TEXTBOX_VISUALS_PROPERTY

class TextboxVisualsProperty : public BasePropertyDialog
{
public:
    TextboxVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TextboxVisualsProperty();

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND        m_hTransparentCheck;
    HWND        m_hUseScriptDMDCheck;
    CComboBox   m_alignmentCombo;
    CButton     m_fontDialogButton;
    CFontDialog m_fontDialog;
    ColorButton  m_backgroundColorButton;
    ColorButton  m_textColorButton;
    CColorDialog m_colorDialog;
    CEdit        m_textIntensityEdit;
    CEdit        m_posXEdit;
    CEdit        m_posYEdit;
    CEdit        m_widthEdit;
    CEdit        m_heightEdit;
    CEdit        m_textEdit;
    CFont       *m_font;
    vector<string> m_alignList;
};

#endif
