#ifndef H_RAMP_VISUALS_PROPERTY
#define H_RAMP_VISUALS_PROPERTY

class RampVisualsProperty: public BasePropertyDialog
{

public:
    RampVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~RampVisualsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CComboBox m_typeCombo;
    CComboBox m_materialCombo;
    CComboBox m_imageCombo;
    CComboBox m_modeCombo;
    CEdit     m_depthBiasEdit;

    CEdit     m_topHeightEdit;
    CEdit     m_bottomHeightEdit;
    CEdit     m_topWidthEdit;
    CEdit     m_bottomWdthEdit;
    CEdit     m_leftWallEdit;
    CEdit     m_rightWallEdit;
    CEdit     m_diameterEdit;
    CEdit     m_distanceXEdit;
    CEdit     m_distanceYEdit;
    vector<string> m_typeList;
    vector<string> m_imageModeList;
};

#endif
