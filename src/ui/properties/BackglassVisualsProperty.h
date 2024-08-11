// license:GPLv3+

#pragma once

class BackglassVisualsProperty : public BasePropertyDialog
{
public:
    BackglassVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~BackglassVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND            m_hApplyNightDayCheck;
    HWND            m_hEnableEMReelCheck;
    HWND            m_hEnableDecal;
    HWND            m_hOverwriteGlobalStereoSettingsCheck;
    EditBox         m_3dStereoOffsetEdit;
    EditBox         m_3dStereoSeparationEdit;
    EditBox         m_3dSteroZPDEdit;
    ComboBox        m_dtImageCombo;
    ComboBox        m_fsImageCombo;
    ComboBox        m_fssImageCombo;
    ComboBox        m_colorGradingCombo;

    CColorDialog m_colorDialog;
    ColorButton m_colorButton1;
};
