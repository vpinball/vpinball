#ifndef H_LIGHT_VISUALS_PROPERTY
#define H_LIGHT_VISUALS_PROPERTY

class LightVisualsProperty: public BasePropertyDialog
{
public:
    LightVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~LightVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CEdit       m_falloffEdit;
    CEdit       m_falloffPowerEdit;
    CEdit       m_intensityEdit;
    CEdit       m_fadeSpeedUpEdit;
    CEdit       m_fadeSpeedDownEdit;
    CEdit       m_depthBiasEdit;
    CComboBox   m_imageCombo;
    HWND        m_hPassThroughCheck;
    HWND        m_hEnableCheck;
    HWND        m_hShowMeshCheck;
    HWND        m_hStaticMeshCheck;
    CEdit       m_haloHeightEdit;
    CEdit       m_scaleMeshEdit;
    CEdit       m_modulateEdit;
    CEdit       m_transmitEdit;
    HWND        m_hRelectOnBalls;
    CEdit       m_posXEdit;
    CEdit       m_posYEdit;
    CComboBox   m_surfaceCombo;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton1;
    ColorButton m_colorButton2;
};

#endif // !H_LIGHT_VISUALS_PROPERTY
