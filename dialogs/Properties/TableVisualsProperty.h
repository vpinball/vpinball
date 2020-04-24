#ifndef H_TABLE_VISUALS_PROPERTY
#define H_TABLE_VISUALS_PROPERTY

class TableVisualsProperty: public BasePropertyDialog
{
public:
    TableVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TableVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_imageCombo;
    CComboBox   m_materialCombo;
    CComboBox   m_ballImageCombo;
    CComboBox   m_ballDecalCombo;
    EditBox     m_reflectionStrengthEdit;
    EditBox     m_ballReflectPlayfieldEdit;
    EditBox     m_ballDefaultBulbIntensScaleEdit;
    HWND        m_hLogoModeCheck;
    HWND        m_hReflectElementsCheck;
};

#endif
