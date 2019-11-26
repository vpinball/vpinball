#ifndef H_BACKGLASS_VISUALS_PROPERTY
#define H_BACKGLASS_VISUALS_PROPERTY

class BackglassVisualsProperty : public BasePropertyDialog
{
public:
    BackglassVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~BackglassVisualsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND            m_hApplyNightDayCheck;
    HWND            m_hEnableEMReelCheck;
    HWND            m_hEnableDecal;
    HWND            m_hOverwriteGlobalStereoSettingsCheck;
    CEdit           m_3dStereoOffsetEdit;
    CEdit           m_3dStereoSeparationEdit;
    CEdit           m_3dSteroZPDEdit;
    CComboBox       m_dtImageCombo;
    CComboBox       m_fsImageCombo;
    CComboBox       m_fssImageCombo;
    CComboBox       m_colorGradingCombo;

    CColorDialog m_colorDialog;
    ColorButton m_colorButton1;

};

#endif // !H_BACKGLASS_VISUALS_PROPERTY

