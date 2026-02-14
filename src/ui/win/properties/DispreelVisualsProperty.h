// license:GPLv3+

#pragma once

class DispreelVisualsProperty final : public BasePropertyDialog
{
public:
    DispreelVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~DispreelVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    HWND         m_hBackgroundTransparentCheck;
    HWND         m_hUseImageGridCheck;
    CColorDialog m_colorDialog;
    ColorButton  m_colorButton;
    ComboBox     m_imageCombo;
    EditBox      m_singleDigitRangeEdit;
    EditBox      m_imagePerRowEdit;
    EditBox      m_posXEdit;
    EditBox      m_posYEdit;
    EditBox      m_reelsEdit;
    EditBox      m_reelWidthEdit;
    EditBox      m_reelHeightEdit;
    EditBox      m_reelSpacingEdit;
};
