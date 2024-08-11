// license:GPLv3+

#pragma once

class BumperVisualsProperty: public BasePropertyDialog
{
public:
    BumperVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~BumperVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
