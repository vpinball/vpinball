#ifndef H_FLASHER_VISUALS_PROPERTY
#define H_FLASHER_VISUALS_PROPERTY

class FlasherVisualsProperty: public BasePropertyDialog
{
public:
    FlasherVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~FlasherVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void UpdateLightmapComboBox(const PinTable *table, const CComboBox &combo, const string &selectName);

private :
    ComboBox    m_imageACombo;
    ComboBox    m_imageBCombo;
    ComboBox    m_modeCombo;
    ComboBox    m_filterCombo;
    HWND        m_hAdditiveBlendCheck;
    HWND        m_hUseDMDCheck;
    HWND        m_hDisplayInEditorCheck;
    EditBox     m_opacityAmountEdit;
    EditBox     m_filterAmountEdit;
    EditBox     m_intensityEdit;
    EditBox     m_fadeSpeedUpEdit;
    EditBox     m_fadeSpeedDownEdit;
    ComboBox    m_lightmapCombo;
    EditBox     m_heightEdit;
    EditBox     m_rotXEdit;
    EditBox     m_rotYEdit;
    EditBox     m_rotZEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    EditBox     m_depthBiasEdit;
    EditBox     m_modulateEdit;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton;
    vector<string> m_filterList;
    vector<string> m_imageAlignList;
};

#endif
