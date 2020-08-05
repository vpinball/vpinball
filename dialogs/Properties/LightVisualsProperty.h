#ifndef H_LIGHT_VISUALS_PROPERTY
#define H_LIGHT_VISUALS_PROPERTY

class LightVisualsProperty: public BasePropertyDialog
{
public:
    LightVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~LightVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_imageCombo;
    ComboBox    m_surfaceCombo;
    HWND        m_hPassThroughCheck;
    HWND        m_hEnableCheck;
    HWND        m_hShowMeshCheck;
    HWND        m_hStaticMeshCheck;
    HWND        m_hRelectOnBalls;
    EditBox     m_falloffEdit;
    EditBox     m_falloffPowerEdit;
    EditBox     m_intensityEdit;
    EditBox     m_fadeSpeedUpEdit;
    EditBox     m_fadeSpeedDownEdit;
    EditBox     m_depthBiasEdit;
    EditBox     m_haloHeightEdit;
    EditBox     m_scaleMeshEdit;
    EditBox     m_modulateEdit;
    EditBox     m_transmitEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton1;
    ColorButton m_colorButton2;
};

#endif // !H_LIGHT_VISUALS_PROPERTY
