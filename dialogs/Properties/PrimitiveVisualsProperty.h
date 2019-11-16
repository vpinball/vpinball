#ifndef H_PRIMITIVE_VISUALS_PROPERTY
#define H_PRIMITIVE_VISUALS_PROPERTY

class PrimitiveVisualsProperty: public BasePropertyDialog
{
public:
    PrimitiveVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~PrimitiveVisualsProperty()
    {
    }

    void UpdateVisuals();
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
    CEdit       m_depthBiasEdit;
    HWND        m_hStaticRenderingCheck;
    HWND        m_hRenderBackfacingCheck;
    HWND        m_hDrawTexturesInsideCheck;
    CEdit       m_disableLightingEdit;
    CEdit       m_disableLightFromBelowEdit;
    CEdit       m_legacySidesEdit;
    CEdit       m_editorEdit;
};

#endif // !H_PRIMITIVE_VISUALS_PROPERTY
