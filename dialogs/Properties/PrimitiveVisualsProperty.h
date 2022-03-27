#ifndef H_PRIMITIVE_VISUALS_PROPERTY
#define H_PRIMITIVE_VISUALS_PROPERTY

class PrimitiveVisualsProperty: public BasePropertyDialog
{
public:
    PrimitiveVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~PrimitiveVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CButton     m_importMeshButton;
    CButton     m_exportMeshButton;
    ComboBox    m_imageCombo;
    ComboBox    m_normalMapCombo;
    ComboBox    m_materialCombo;
    HWND        m_hDisplayImageCheck;
    HWND        m_hObjectSpaceCheck;
    EditBox     m_depthBiasEdit;
    HWND        m_hStaticRenderingCheck;
    HWND        m_hRenderBackfacingCheck;
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

#endif // !H_PRIMITIVE_VISUALS_PROPERTY
