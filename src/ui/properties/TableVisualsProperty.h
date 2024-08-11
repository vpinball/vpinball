// license:GPLv3+

#pragma once

class TableVisualsProperty: public BasePropertyDialog
{
public:
    TableVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TableVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
