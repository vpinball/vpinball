// license:GPLv3+

#pragma once

class HitTargetVisualsProperty final : public BasePropertyDialog
{
public:
    HitTargetVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~HitTargetVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    vector<string> m_typeList;
    ComboBox    m_imageCombo;
    ComboBox    m_materialCombo;
    ComboBox    m_typeCombo;
    EditBox     m_dropSpeedEdit;
    EditBox     m_raiseDelayEdit;
    EditBox     m_depthBiasEdit;
    EditBox     m_disableLightingEdit;
    EditBox     m_disableLightBelowEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    EditBox     m_posZEdit;
    EditBox     m_scaleXEdit;
    EditBox     m_scaleYEdit;
    EditBox     m_scaleZEdit;
    EditBox     m_orientationEdit;
};
