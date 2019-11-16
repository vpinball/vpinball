#ifndef H_GATE_VISUALS_PROPERTY
#define H_GATE_VISUALS_PROPERTY

class GateVisualsProperty: public BasePropertyDialog
{
public:
    GateVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~GateVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_typeCombo;
    CComboBox m_materialCombo;
    CComboBox m_surfaceCombo;
    CEdit     m_xposEdit;
    CEdit     m_yposEdit;
    CEdit     m_lengthEdit;
    CEdit     m_heightEdit;
    CEdit     m_rotationEdit;
    CEdit     m_openAngleEdit;
    CEdit     m_closeAngleEdit;
    vector<string> m_typeList;
};

#endif
