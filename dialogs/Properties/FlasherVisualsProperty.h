#ifndef H_FLASHER_VISUALS_PROPERTY
#define H_FLASHER_VISUALS_PROPERTY

class FlasherVisualsProperty: public BasePropertyDialog
{
public:
    FlasherVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~FlasherVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CComboBox   m_imageACombo;
    CComboBox   m_imageBCombo;
    HWND        m_hAdditiveBlendCheck;
    HWND        m_hUseDMDCheck;
    HWND        m_hDisplayInEditorCheck;
    EditBox     m_opacyitAmountEdit;
    EditBox     m_filterAmountEdit;
    EditBox     m_intensityEdit;
    EditBox     m_fadeSpeedUpEdit;
    EditBox     m_fadeSpeedDownEdit;
    EditBox     m_heightEdit;
    EditBox     m_rotXEdit;
    EditBox     m_rotYEdit;
    EditBox     m_rotZEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    EditBox     m_depthBiasEdit;
    EditBox     m_modulateEdit;
    CComboBox   m_modeCombo;
    CComboBox   m_filterCombo;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton;
    vector<string> m_filterList;
    vector<string> m_imageAlignList;
};

#endif
