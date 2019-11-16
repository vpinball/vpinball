#ifndef H_FLASHER_VISUALS_PROPERTY
#define H_FLASHER_VISUALS_PROPERTY

class FlasherVisualsProperty: public BasePropertyDialog
{
public:
    FlasherVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~FlasherVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CEdit       m_opacyitAmountEdit;
    CEdit       m_filterAmountEdit;
    CEdit       m_intensityEdit;
    CEdit       m_fadeSpeedUpEdit;
    CEdit       m_fadeSpeedDownEdit;
    CComboBox   m_imageACombo;
    CComboBox   m_imageBCombo;
    HWND        m_hAdditiveBlendCheck;
    HWND        m_hUseDMDCheck;
    HWND        m_hDisplayInEditorCheck;
    CEdit       m_heightEdit;
    CEdit       m_rotXEdit;
    CEdit       m_rotYEdit;
    CEdit       m_rotZEdit;
    CEdit       m_posXEdit;
    CEdit       m_posYEdit;
    CEdit       m_depthBiasEdit;
    CEdit       m_modulateEdit;
    CComboBox   m_modeCombo;
    CComboBox   m_filterCombo;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton;
    vector<string> m_filterList;
    vector<string> m_imageAlignList;
};

#endif
