// license:GPLv3+

#pragma once

class BackglassCameraProperty : public BasePropertyDialog
{
public:
    BackglassCameraProperty(const VectorProtected<ISelect>* pvsel);
    virtual ~BackglassCameraProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND            m_hFssModeCheck;
    ComboBox        m_testOverrideCombo;
    ComboBox        m_viewCombo;
    ComboBox        m_modeCombo;
    EditBox         m_inclinationEdit;
    EditBox         m_fovEdit;
    EditBox         m_laybackEdit;
    EditBox         m_xyRotationEdit;
    EditBox         m_xScaleEdit;
    EditBox         m_yScaleEdit;
    EditBox         m_zScaleEdit;
    EditBox         m_xViewOffsetEdit;
    EditBox         m_yViewOffsetEdit;
    EditBox         m_xOffsetEdit;
    EditBox         m_yOffsetEdit;
    EditBox         m_zOffsetEdit;
    vector<string>  m_viewList;
    vector<string>  m_modeList;
    vector<string>  m_overrideList;
};
