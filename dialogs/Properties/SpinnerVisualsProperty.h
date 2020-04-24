#ifndef H_SPINNER_VISUALS_PROPERTY
#define H_SPINNER_VISUALS_PROPERTY

class SpinnerVisualsProperty: public BasePropertyDialog
{
public:
    SpinnerVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~SpinnerVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    EditBox   m_lengthEdit;
    EditBox   m_heightEdit;
    EditBox   m_rotationEdit;
    EditBox   m_angleMaxEdit;
    EditBox   m_angleMinEdit;
    EditBox   m_elasticityEdit;
    CComboBox m_imageCombo;
    CComboBox m_materialCombo;
    CComboBox m_surfaceCombo;
    HWND      m_hShowBracketCheck;
};

#endif
