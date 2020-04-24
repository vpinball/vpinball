#ifndef H_KICKER_VISUALS_PROPERTY
#define H_KICKER_VISUALS_PROPERTY

class KickerVisualsProperty: public BasePropertyDialog
{
public:
    KickerVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~KickerVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_materialCombo;
    CComboBox m_displayCombo;
    EditBox   m_radiusEdit;
    EditBox   m_orientationEdit;
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    CComboBox m_surfaceCombo;
    vector<string> m_typeList;
};

#endif // !H_KICKER_VISUALS_PROPERTY
