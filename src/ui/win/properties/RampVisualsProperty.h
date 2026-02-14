// license:GPLv3+

#pragma once

class RampVisualsProperty final : public BasePropertyDialog
{
public:
    RampVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~RampVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
    HWND      m_hApplyImageToWallCheck;
};
