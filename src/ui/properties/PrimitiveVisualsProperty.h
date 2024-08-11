// license:GPLv3+

#pragma once

class PrimitiveVisualsProperty: public BasePropertyDialog
{
public:
    PrimitiveVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~PrimitiveVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void AddToolTip(const char *const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
    void UpdateLightmapComboBox(const PinTable *table, const CComboBox &combo, const string &selectName);
    void UpdateRenderProbeComboBox(const vector<RenderProbe *> &contentList, const CComboBox &combo, const string &selectName);

private:
    CButton     m_importMeshButton;
    CButton     m_exportMeshButton;
    ComboBox    m_imageCombo;
    ComboBox    m_normalMapCombo;
    ComboBox    m_materialCombo;
    ComboBox    m_lightmapCombo;
    ComboBox    m_reflectionCombo;
    EditBox     m_reflectionAmountEdit;
    ComboBox    m_refractionCombo;
    EditBox     m_refractionThicknessEdit;
    HWND        m_hDisplayImageCheck;
    HWND        m_hObjectSpaceCheck;
    EditBox     m_depthBiasEdit;
    HWND        m_hStaticRenderingCheck;
    HWND        m_hRenderBackfacingCheck;
    HWND        m_hDepthMaskWriteCheck;
    HWND        m_hDrawTexturesInsideCheck;
    HWND        m_hAdditiveBlendCheck;
    EditBox     m_opacityAmountEdit;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton;
    EditBox     m_disableLightingEdit;
    EditBox     m_disableLightFromBelowEdit;
    EditBox     m_legacySidesEdit;
    EditBox     m_edgeFactorUIEdit;
};
