// license:GPLv3+

#pragma once

class BackglassCameraProperty final : public BasePropertyDialog
{
public:
    BackglassCameraProperty(const VectorProtected<ISelect>* pvsel);
    ~BackglassCameraProperty() override{ }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
