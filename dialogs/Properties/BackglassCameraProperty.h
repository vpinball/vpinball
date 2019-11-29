#ifndef H_BACKGLASS_CAMERA_PROPERTY
#define H_BACKGLASS_CAMERA_PROPERTY

class BackglassCameraProperty : public BasePropertyDialog
{
public:
    BackglassCameraProperty(VectorProtected<ISelect> *pvsel);
    virtual ~BackglassCameraProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    HWND            m_hFssModeCheck;
    HWND            m_hTestDesktopCheck;
    CComboBox       m_modeCombo;
    CEdit           m_inclinationEdit;
    CEdit           m_fovEdit;
    CEdit           m_laybackEdit;
    CEdit           m_xyRotationEdit;
    CEdit           m_xScaleEdit;
    CEdit           m_yScaleEdit;
    CEdit           m_zScaleEdit;
    CEdit           m_xOffsetEdit;
    CEdit           m_yOffsetEdit;
    CEdit           m_zOffsetEdit;
    vector<string>  m_modeList;
};

#endif // !H_BACKGLASS_CAMERA_PROPERTY
