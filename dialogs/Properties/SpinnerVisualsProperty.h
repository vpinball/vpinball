#ifndef H_SPINNER_VISUALS_PROPERTY
#define H_SPINNER_VISUALS_PROPERTY

class SpinnerVisualsProperty: public BasePropertyDialog
{
public:
    SpinnerVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~SpinnerVisualsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    CEdit   m_posXEdit;
    CEdit   m_posYEdit;
    CEdit   m_lengthEdit;
    CEdit   m_heightEdit;
    CEdit   m_rotationEdit;
    CEdit   m_angleMaxEdit;
    CEdit   m_angleMinEdit;
    CEdit   m_elasticityEdit;
    CComboBox m_imageCombo;
    CComboBox m_materialCombo;
    CComboBox m_surfaceCombo;
    HWND      m_hShowBracketCheck;

};

#endif
