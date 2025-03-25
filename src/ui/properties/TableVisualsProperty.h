// license:GPLv3+

#pragma once

class TableVisualsProperty final : public BasePropertyDialog
{
public:
    TableVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~TableVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox    m_imageCombo;
    ComboBox    m_materialCombo;
    ComboBox    m_ballImageCombo;
    ComboBox    m_ballDecalCombo;
    ComboBox    m_toneMapperCombo;
    EditBox     m_exposureEdit;
    EditBox     m_reflectionStrengthEdit;
    EditBox     m_ballReflectPlayfieldEdit;
    EditBox     m_ballDefaultBulbIntensScaleEdit;
    HWND        m_hSphericalMapCheck = NULL;
    HWND        m_hLogoModeCheck = NULL;
    HWND        m_hEnableAOCheck = NULL;
    EditBox     m_ambientOcclusionScaleEdit;
    HWND        m_hEnableSSRCheck = NULL;
    EditBox     m_screenSpaceReflEdit;
    EditBox     m_bloomStrengthEdit;
};
