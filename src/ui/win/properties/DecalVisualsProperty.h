// license:GPLv3+

#pragma once

class DecalVisualsProperty final : public BasePropertyDialog
{
public:
    DecalVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~DecalVisualsProperty() override;

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox    m_materialCombo;
    ComboBox    m_typeCombo;
    ComboBox    m_imageCombo;
    ComboBox    m_sizingCombo;
    ComboBox    m_surfaceCombo;
    HWND        m_hVerticalTextCheck;
    ColorButton m_fontColorButton;
    CColorDialog m_colorDialog;
    CButton     m_fontDialogButton;
    CFontDialog m_fontDialog;
    EditBox     m_textEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    EditBox     m_widthEdit;
    EditBox     m_heightEdit;
    EditBox     m_rotationEdit;
    CFont       *m_font;
    vector<string> m_typeList;
    vector<string> m_sizingList;
};
