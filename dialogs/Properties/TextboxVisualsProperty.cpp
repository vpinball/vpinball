#include "stdafx.h"
#include "Properties/TextboxVisualsProperty.h"
#include <WindowsX.h>

TextboxVisualsProperty::TextboxVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTEXTBOX_VISUALS, pvsel)
{
    m_alignList.push_back("Align Left");
    m_alignList.push_back("Align Center");
    m_alignList.push_back("Align Right");

    m_textIntensityEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_widthEdit.SetDialog(this);
    m_heightEdit.SetDialog(this);
    m_textEdit.SetDialog(this);
    m_alignmentCombo.SetDialog(this);
}

TextboxVisualsProperty::~TextboxVisualsProperty()
{
    if (m_font)
        delete m_font;
}

void TextboxVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTextbox))
            continue;
        Textbox* const text = (Textbox *)m_pvsel->ElementAt(i);

        if (dispid == IDC_TEXTBOX_TRANSP_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hTransparentCheck, text->m_d.m_transparent);
        if (dispid == IDC_USE_SCRIPT_DMD_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hUseScriptDMDCheck, text->m_d.m_isDMD);
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_backgroundColorButton.SetColor(text->m_d.m_backcolor);
        if (dispid == IDC_COLOR_BUTTON2 || dispid == -1)
            m_textColorButton.SetColor(text->m_d.m_fontcolor);
        if (dispid == 60000 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, text->m_d.m_v1.x);
        if (dispid == 60001 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, text->m_d.m_v1.y);
        if (dispid == 60002 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_widthEdit, text->m_d.m_v2.x - text->m_d.m_v1.x);
        if (dispid == 60003 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_heightEdit, text->m_d.m_v2.y - text->m_d.m_v1.y);
        if (dispid == IDC_TEXTBOX_ALIGN_COMBO || dispid == -1)
            PropertyDialog::UpdateComboBox(m_alignList, m_alignmentCombo, m_alignList[text->m_d.m_talign]);
        if (dispid == IDC_TEXT_INTENSITY || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_textIntensityEdit, text->m_d.m_intensity_scale);
        if (dispid == IDC_TEXTBOX_TEXT_EDIT || dispid == -1)
            m_textEdit.SetWindowText(text->m_d.m_sztext.c_str());

        if (dispid == IDC_FONT_DIALOG_BUTTON || dispid == -1)
        {
            if (text->m_pIFont)
            {
                m_fontDialogButton.SetWindowText(text->GetFontName());
                m_font = new CFont(text->GetFont());
            }
        }
        //only show the first element on multi-select
        break;
    }
}

void TextboxVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTextbox))
            continue;
        Textbox* const text = (Textbox *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_TEXTBOX_TRANSP_CHECK:
                CHECK_UPDATE_ITEM(text->m_d.m_transparent, PropertyDialog::GetCheckboxState(m_hTransparentCheck), text);
                break;
            case IDC_TEXTBOX_ALIGN_COMBO:
                CHECK_UPDATE_ITEM(text->m_d.m_talign, (TextAlignment)PropertyDialog::GetComboBoxIndex(m_alignmentCombo, m_alignList), text);
                break;
            case IDC_USE_SCRIPT_DMD_CHECK:
                CHECK_UPDATE_ITEM(text->m_d.m_isDMD, PropertyDialog::GetCheckboxState(m_hUseScriptDMDCheck), text);
                break;
            case IDC_TEXT_INTENSITY:
                CHECK_UPDATE_ITEM(text->m_d.m_intensity_scale, PropertyDialog::GetFloatTextbox(m_textIntensityEdit), text);
                break;
            case IDC_TEXTBOX_TEXT_EDIT:
            {
                PropertyDialog::StartUndo(text);
                const CString pattern = m_textEdit.GetWindowText();
                text->m_d.m_sztext = pattern.c_str();
                PropertyDialog::EndUndo(text);
                break;
            }
            case 60000:
            {
                PropertyDialog::StartUndo(text);
                const float newValue = PropertyDialog::GetFloatTextbox(m_posXEdit);
                const float delta = newValue - text->m_d.m_v1.x;
                text->m_d.m_v1.x += delta;
                text->m_d.m_v2.x += delta;
                PropertyDialog::EndUndo(text);
                break;
            }
            case 60001:
            {
                PropertyDialog::StartUndo(text);
                const float newValue = PropertyDialog::GetFloatTextbox(m_posYEdit);
                const float delta = newValue - text->m_d.m_v1.y;
                text->m_d.m_v1.y += delta;
                text->m_d.m_v2.y += delta;
                PropertyDialog::EndUndo(text);
                break;
            }
            case 60002:
            {
                PropertyDialog::StartUndo(text);
                const float newValue = PropertyDialog::GetFloatTextbox(m_widthEdit);
                text->m_d.m_v2.x = text->m_d.m_v1.x + newValue;
                PropertyDialog::EndUndo(text);
                break;
            }
            case 60003:
            {
                PropertyDialog::StartUndo(text);
                const float newValue = PropertyDialog::GetFloatTextbox(m_heightEdit);
                text->m_d.m_v2.y = text->m_d.m_v1.y + newValue;
                PropertyDialog::EndUndo(text);
                break;
            }
            case IDC_COLOR_BUTTON1:
            {
                CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(text->m_d.m_backcolor);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    text->m_d.m_backcolor = m_colorDialog.GetColor();
                    m_backgroundColorButton.SetColor(text->m_d.m_backcolor);
                }
                break;
            }
            case IDC_COLOR_BUTTON2:
            {
                CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(text->m_d.m_fontcolor);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    text->m_d.m_fontcolor = m_colorDialog.GetColor();
                    m_textColorButton.SetColor(text->m_d.m_fontcolor);
                }
                break;
            }
            case IDC_FONT_DIALOG_BUTTON:
            {
                CHOOSEFONT cf = m_fontDialog.GetParameters();
                LOGFONT lf = m_font->GetLogFont();
                cf.lpLogFont = &lf;
                cf.Flags |= CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
                cf.rgbColors = text->m_d.m_fontcolor;
                m_fontDialog.SetParameters(cf);
                m_fontDialog.SetColor(text->m_d.m_fontcolor);
                if (m_fontDialog.DoModal(GetHwnd()) == IDOK)
                {
                    FONTDESC fd;
                    m_font->CreateFontIndirect(m_fontDialog.GetLogFont());

                    fd.cbSizeofstruct = sizeof(FONTDESC);

                    const int len = lstrlen(m_font->GetLogFont().lfFaceName) + 1;
                    fd.lpstrName = (LPOLESTR)malloc(len * sizeof(WCHAR));
                    memset(fd.lpstrName, 0, len * sizeof(WCHAR));
                    MultiByteToWideCharNull(CP_ACP, 0, m_font->GetLogFont().lfFaceName, -1, fd.lpstrName, len);

                    fd.sWeight = (SHORT)m_font->GetLogFont().lfWidth;
                    fd.sCharset = m_font->GetLogFont().lfCharSet;
                    fd.fItalic = m_font->GetLogFont().lfItalic;
                    fd.fUnderline = m_font->GetLogFont().lfUnderline;
                    fd.fStrikethrough = m_font->GetLogFont().lfStrikeOut;

                    // free old font first
                    text->m_pIFont->Release();
                    // create the new one
                    OleCreateFontIndirect(&fd, IID_IFont, (void **)&text->m_pIFont);
                    free(fd.lpstrName);
                    text->m_d.m_fontcolor = m_fontDialog.GetColor();
                }
                break;
            }
            default:
                break;
        }
        text->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL TextboxVisualsProperty::OnInitDialog()
{
    m_hTransparentCheck = ::GetDlgItem(GetHwnd(), IDC_TEXTBOX_TRANSP_CHECK);
    AttachItem(IDC_COLOR_BUTTON1, m_backgroundColorButton);
    AttachItem(IDC_COLOR_BUTTON2, m_textColorButton);
    AttachItem(IDC_FONT_DIALOG_BUTTON, m_fontDialogButton);
    m_alignmentCombo.AttachItem(IDC_TEXTBOX_ALIGN_COMBO);
    m_posXEdit.AttachItem(60000);
    m_posYEdit.AttachItem(60001);
    m_widthEdit.AttachItem(60002);
    m_heightEdit.AttachItem(60003);
    m_textIntensityEdit.AttachItem(IDC_TEXT_INTENSITY);
    m_hUseScriptDMDCheck = ::GetDlgItem(GetHwnd(), IDC_USE_SCRIPT_DMD_CHECK);
    m_textEdit.AttachItem(IDC_TEXTBOX_TEXT_EDIT);
    UpdateVisuals();
    return TRUE;
}

INT_PTR TextboxVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            UINT nID = static_cast<UINT>(wParam);
            if (nID == IDC_COLOR_BUTTON1)
            {
                m_backgroundColorButton.DrawItem(lpDrawItemStruct);
            }
            else if (nID == IDC_COLOR_BUTTON2)
            {
                m_textColorButton.DrawItem(lpDrawItemStruct);
            }

            return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}
