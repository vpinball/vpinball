#ifndef H_PLUNGER_VISUALS_PROPERTY
#define H_PLUNGER_VISUALS_PROPERTY

class PlungerVisualsProperty: public BasePropertyDialog
{
public:
    PlungerVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~PlungerVisualsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_typeCombo;
    CComboBox m_imageCombo;
    CComboBox m_materialCombo;
    CEdit     m_flatFramesEdit;
    CEdit     m_widthEdit;
    CEdit     m_zAdjustmentEdit;
    CEdit     m_rodDiameterEdit;
    CEdit     m_tipShapeEdit;
    CEdit     m_ringGapEdit;
    CEdit     m_ringDiamEdit;
    CEdit     m_ringWidthEdit;
    CEdit     m_springDiamEdit;
    CEdit     m_springGaugeEdit;
    CEdit     m_springLoopsEdit;
    CEdit     m_endLoopsEdit;
    CEdit     m_posXEdit;
    CEdit     m_posYEdit;
    CComboBox m_surfaceCombo;
    vector<string> m_typeList;
};

#endif // !H_PLUNGER_VISUALS_PROPERTY

