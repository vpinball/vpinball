// license:GPLv3+

#pragma once

class TextboxVisualsProperty : public BasePropertyDialog
{
public:
    TextboxVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TextboxVisualsProperty();

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND         m_hTransparentCheck;
    HWND         m_hUseScriptDMDCheck;
    ComboBox     m_alignmentCombo;
    CButton      m_fontDialogButton;
    CFontDialog  m_fontDialog;
    ColorButton  m_backgroundColorButton;
    ColorButton  m_textColorButton;
    CColorDialog m_colorDialog;
    EditBox      m_textIntensityEdit;
    EditBox      m_posXEdit;
    EditBox      m_posYEdit;
    EditBox      m_widthEdit;
    EditBox      m_heightEdit;
    EditBox      m_textEdit;
    CFont       *m_font;
    vector<string> m_alignList;
};
