#ifndef H_RAMP_VISUALS_PROPERTY
#define H_RAMP_VISUALS_PROPERTY

class RampVisualsProperty: public BasePropertyDialog
{

public:
    RampVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~RampVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    ComboBox  m_typeCombo;
    ComboBox  m_materialCombo;
    ComboBox  m_imageCombo;
    ComboBox  m_modeCombo;
    EditBox   m_depthBiasEdit;
    EditBox   m_topHeightEdit;
    EditBox   m_bottomHeightEdit;
    EditBox   m_topWidthEdit;
    EditBox   m_bottomWidthEdit;
    EditBox   m_leftWallEdit;
    EditBox   m_rightWallEdit;
    EditBox   m_diameterEdit;
    EditBox   m_distanceXEdit;
    EditBox   m_distanceYEdit;
    vector<string> m_typeList;
    vector<string> m_imageModeList;
};

#endif
