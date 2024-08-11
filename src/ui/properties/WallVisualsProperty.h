// license:GPLv3+

#pragma once

class WallVisualsProperty: public BasePropertyDialog
{
public:
    WallVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~WallVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
