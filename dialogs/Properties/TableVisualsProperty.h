#ifndef H_TABLE_VISUALS_PROPERTY
#define H_TABLE_VISUALS_PROPERTY

class TableVisualsProperty: public BasePropertyDialog
{
public:
    TableVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TableVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_imageCombo;
    CComboBox   m_materialCombo;
    CEdit       m_reflectionStrengthEdit;
    CComboBox   m_ballImageCombo;
    CComboBox   m_ballDecalCombo;
    CEdit       m_ballReflectPlayfieldEdit;
    CEdit       m_ballDefaultBulbIntensScaleEdit;
    HWND        m_hLogoModeCheck;
    HWND        m_hReflectElementsCheck;
};

#endif
