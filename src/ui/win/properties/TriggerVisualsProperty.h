// license:GPLv3+

#pragma once

class TriggerVisualsProperty final : public BasePropertyDialog
{
public:
    TriggerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~TriggerVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox    m_shapeCombo;
    ComboBox    m_materialCombo;
    ComboBox    m_surfaceCombo;
    EditBox     m_wireThicknessEdit;
    EditBox     m_starRadiusEdit;
    EditBox     m_rotationEdit;
    EditBox     m_animationSpeedEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    vector<string> m_shapeList;
};
