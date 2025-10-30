// license:GPLv3+

#pragma once

class BackglassVisualsProperty final : public BasePropertyDialog
{
public:
    BackglassVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~BackglassVisualsProperty() override { }

    void UpdateVisuals(const int dispid = -1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    HWND            m_hApplyNightDayCheck;
    HWND            m_hEnableEMReelCheck;
    HWND            m_hEnableDecal;
    ComboBox        m_dtImageCombo;
    ComboBox        m_fsImageCombo;
    ComboBox        m_fssImageCombo;
    ComboBox        m_colorGradingCombo;

    CColorDialog m_colorDialog;
    ColorButton m_colorButton1;
};
