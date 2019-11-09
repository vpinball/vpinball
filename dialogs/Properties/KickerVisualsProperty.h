#ifndef H_KICKER_VISUALS_PROPERTY
#define H_KICKER_VISUALS_PROPERTY

class KickerVisualsProperty: public BasePropertyDialog
{
public:
    KickerVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~KickerVisualsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_materialCombo;
    CComboBox m_displayCombo;
    CEdit     m_radiusEdit;
    CEdit     m_orientationEdit;
    CEdit     m_posXEdit;
    CEdit     m_posYEdit;
    CComboBox m_surfaceCombo;
    vector<string> m_typeList;
};

#endif // !H_KICKER_VISUALS_PROPERTY

