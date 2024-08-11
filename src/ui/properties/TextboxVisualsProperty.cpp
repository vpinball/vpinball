// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/TextboxVisualsProperty.h"
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
    delete m_font;
}

void TextboxVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTextbox))
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
                if (!m_font)
                  delete m_font;
                m_font = new CFont(text->GetFont());
            }
        }
        //only show the first element on multi-select
        break;
    }
}

void TextboxVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTextbox))
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

                    const LOGFONT font = m_font->GetLogFont();
                    const int len = lstrlen(font.lfFaceName) + 1;
                    fd.lpstrName = (LPOLESTR)malloc(len * sizeof(WCHAR));
                    memset(fd.lpstrName, 0, len * sizeof(WCHAR));
                    MultiByteToWideCharNull(CP_ACP, 0, font.lfFaceName, -1, fd.lpstrName, len);

                    fd.sWeight = (SHORT)font.lfWidth;
                    fd.sCharset = font.lfCharSet;
                    fd.fItalic = font.lfItalic;
                    fd.fUnderline = font.lfUnderline;
                    fd.fStrikethrough = font.lfStrikeOut;

                    // free old font first
                    text->m_pIFont->Release();
                    // create the new one
                    OleCreateFontIndirect(&fd, IID_IFont, (void **)&text->m_pIFont);
                    float fontsize = (float)((abs(font.lfHeight) * 72) / GetDeviceCaps(g_pvp->GetDC(), LOGPIXELSY));
                    CY size;
                    size.int64 = (LONGLONG)(fontsize * 10000.0f);
                    text->m_pIFont->put_Size(size);
                    free(fd.lpstrName);
                    text->m_d.m_fontcolor = m_fontDialog.GetColor();
                    delete m_font;
                    m_font = new CFont(text->GetFont());
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

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(m_hTransparentCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_backgroundColorButton, CResizer::topleft, 0);
    m_resizer.AddChild(m_textColorButton, CResizer::topleft, 0);
    m_resizer.AddChild(m_fontDialogButton, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_alignmentCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_widthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_heightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_textIntensityEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hUseScriptDMDCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_textEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR TextboxVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            const LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            const UINT nID = static_cast<UINT>(wParam);
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
