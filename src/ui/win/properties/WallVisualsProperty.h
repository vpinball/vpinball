// license:GPLv3+

#pragma once

class WallVisualsProperty final : public BasePropertyDialog
{
public:
    WallVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~WallVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox  m_topImageCombo;
    ComboBox  m_sideImageCombo;
    ComboBox  m_topMaterialCombo;
    ComboBox  m_sideMaterialCombo;
    ComboBox  m_slingshotMaterialCombo;
    EditBox   m_disableLightingEdit;
    EditBox   m_disableLightFromBelowEdit;
    EditBox   m_topHeightEdit;
    EditBox   m_bottomHeightEdit;
    HWND      m_hDisplayInEditor;
    HWND      m_hTopImageVisible;
    HWND      m_hSideImageVisible;
    HWND      m_hAnimateSlingshot;
    HWND      m_hFlipbook;
};
