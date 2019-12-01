#ifndef H_TABLE_LIGHTS_PROPERTY
#define H_TABLE_LIGHTS_PROPERTY

class TableLightsProperty: public BasePropertyDialog
{
public:
    TableLightsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TableLightsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CColorDialog m_colorDialog;
    ColorButton m_colorButton1;
    ColorButton m_colorButton2;
    CEdit       m_lightEmissionScaleEdit;
    CEdit       m_lightHeightEdit;
    CEdit       m_lightRangeEdit;
    CComboBox   m_envEmissionImageCombo;
    CEdit       m_envEmissionScaleEdit;
    CEdit       m_ambientOcclusionScaleEdit;
    CEdit       m_bloomStrengthEdit;
    CEdit       m_screenSpaceReflEdit;
};

#endif
