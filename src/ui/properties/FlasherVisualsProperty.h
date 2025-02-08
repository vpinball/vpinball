// license:GPLv3+

#pragma once

class FlasherVisualsProperty: public BasePropertyDialog
{
public:
    FlasherVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~FlasherVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void UpdateLightmapComboBox(const PinTable *table, const CComboBox &combo, const string &selectName);

private:
    ComboBox    m_modeCombo;
    EditBox     m_depthBiasEdit;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton;
    ComboBox    m_texModeCombo;
    vector<string> m_imageAlignList;

    ComboBox    m_styleCombo;
    EditBox     m_linkEdit;

    ComboBox    m_glassImageCombo;
    EditBox     m_glassRoughnessEdit;
    ColorButton m_glassAmbientButton;
    EditBox     m_glassPadTopEdit;
    EditBox     m_glassPadBottomEdit;
    EditBox     m_glassPadLeftEdit;
    EditBox     m_glassPadRightEdit;

    ComboBox    m_imageACombo;
    ComboBox    m_imageBCombo;
    ComboBox    m_filterCombo;
    vector<string> m_filterList;
    EditBox     m_filterAmountEdit;
    HWND        m_hDisplayInEditorCheck = NULL;

    EditBox     m_opacityAmountEdit;
    ComboBox    m_lightmapCombo;
    HWND        m_hAdditiveBlendCheck = NULL;
    EditBox     m_modulateEdit;

    EditBox     m_heightEdit;
    EditBox     m_rotXEdit;
    EditBox     m_rotYEdit;
    EditBox     m_rotZEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
};
