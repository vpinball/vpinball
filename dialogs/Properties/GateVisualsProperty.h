#ifndef H_GATE_VISUALS_PROPERTY
#define H_GATE_VISUALS_PROPERTY

class GateVisualsProperty: public BasePropertyDialog
{
public:
    GateVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~GateVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_typeCombo;
    CComboBox m_materialCombo;
    CComboBox m_surfaceCombo;
    EditBox   m_xposEdit;
    EditBox   m_yposEdit;
    EditBox   m_lengthEdit;
    EditBox   m_heightEdit;
    EditBox   m_rotationEdit;
    EditBox   m_openAngleEdit;
    EditBox   m_closeAngleEdit;
    vector<string> m_typeList;
};

#endif
