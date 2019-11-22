#ifndef H_RUBBER_VISUALS_PROPERTY
#define H_RUBBER_VISUALS_PROPERTY

class RubberVisualsProperty: public BasePropertyDialog
{
public:
    RubberVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~RubberVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_imageCombo;
    CComboBox   m_materialCombo;
    HWND        m_hStaticRenderingCheck;
    CEdit       m_heightEdit;
    CEdit       m_thicknessEdit;
    CEdit       m_rotXEdit;
    CEdit       m_rotYEdit;
    CEdit       m_rotZEdit;
    HWND        m_hShowInEditorCheck;
};

#endif
