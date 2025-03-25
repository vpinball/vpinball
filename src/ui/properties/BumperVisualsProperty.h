// license:GPLv3+

#pragma once

class BumperVisualsProperty final : public BasePropertyDialog
{
public:
    BumperVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~BumperVisualsProperty() override{ }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox  m_capMaterialCombo;
    ComboBox  m_bumpBaseMaterialCombo;
    ComboBox  m_skirtMaterialCombo;
    ComboBox  m_ringMaterialCombo;
    ComboBox  m_surfaceCombo;
    EditBox   m_radiusEdit;
    EditBox   m_heightScaleEdit;
    EditBox   m_orientationEdit;
    EditBox   m_ringSpeedEdit;
    EditBox   m_ringDropOffsetEdit;
    HWND      m_hCapVisibleCheck;
    HWND      m_hBaseVisibleCheck;
    HWND      m_hRingVisibleCheck;
    HWND      m_hSkirtVisibleCheck;
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
};
