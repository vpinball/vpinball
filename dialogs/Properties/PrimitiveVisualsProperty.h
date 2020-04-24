#ifndef H_PRIMITIVE_VISUALS_PROPERTY
#define H_PRIMITIVE_VISUALS_PROPERTY

class PrimitiveVisualsProperty: public BasePropertyDialog
{
public:
    PrimitiveVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~PrimitiveVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CButton     m_importMeshButton;
    CButton     m_exportMeshButton;
    CComboBox   m_imageCombo;
    HWND        m_hDisplayImageCheck;
    HWND        m_hObjectSpaceCheck;
    CComboBox   m_normalMapCombo;
    CComboBox   m_materialCombo;
    EditBox     m_depthBiasEdit;
    HWND        m_hStaticRenderingCheck;
    HWND        m_hRenderBackfacingCheck;
    HWND        m_hDrawTexturesInsideCheck;
    EditBox     m_disableLightingEdit;
    EditBox     m_disableLightFromBelowEdit;
    EditBox     m_legacySidesEdit;
    EditBox     m_edgeFactorUIEdit;
};

#endif // !H_PRIMITIVE_VISUALS_PROPERTY
