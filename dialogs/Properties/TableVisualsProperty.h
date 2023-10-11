#ifndef H_TABLE_VISUALS_PROPERTY
#define H_TABLE_VISUALS_PROPERTY

class TableVisualsProperty: public BasePropertyDialog
{
public:
    TableVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TableVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_imageCombo;
    ComboBox    m_materialCombo;
    ComboBox    m_ballImageCombo;
    ComboBox    m_ballDecalCombo;
    EditBox     m_reflectionStrengthEdit;
    EditBox     m_ballReflectPlayfieldEdit;
    EditBox     m_ballDefaultBulbIntensScaleEdit;
    HWND        m_hSphericalMapCheck;
    HWND        m_hLogoModeCheck;
    HWND        m_hReflectElementsCheck;
    HWND        m_hEnableAOCheck;
};

#endif
