#include "stdafx.h"
#include "Properties/DecalVisualsProperty.h"
#include <WindowsX.h>

DecalVisualsProperty::DecalVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPDECAL_VISUALS, pvsel), m_font(NULL)
{
    m_typeList.push_back("DecalText");
    m_typeList.push_back("DecalImage");

    m_sizingList.push_back("AutoSize");
    m_sizingList.push_back("AutoWidth");
    m_sizingList.push_back("ManualSize");
}

DecalVisualsProperty::~DecalVisualsProperty()
{
    if (m_font)
        delete m_font;
}

void DecalVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDecal))
            continue;
        Decal * const decal = (Decal *)m_pvsel->ElementAt(i);

        PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[decal->m_d.m_decaltype].c_str());
        PropertyDialog::UpdateComboBox(m_sizingList, m_sizingCombo, m_sizingList[decal->m_d.m_sizingtype].c_str());
        m_textEdit.SetWindowText(decal->m_d.m_sztext);
        PropertyDialog::SetCheckboxState(m_hVerticalTextCheck, decal->m_d.m_verticalText);
        m_fontColorButton.SetColor(decal->m_d.m_color);

        PropertyDialog::SetFloatTextbox(m_posXEdit, decal->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, decal->m_d.m_vCenter.y);
        PropertyDialog::SetFloatTextbox(m_widthEdit, decal->m_d.m_width);
        PropertyDialog::SetFloatTextbox(m_heigthEdit, decal->m_d.m_height);
        PropertyDialog::SetFloatTextbox(m_rotationEdit, decal->m_d.m_rotation);
        PropertyDialog::UpdateSurfaceComboBox(decal->GetPTable(), m_surfaceCombo, decal->m_d.m_szSurface);

        if (decal->m_pIFont)
        {
            m_fontDialogButton.SetWindowText(decal->GetFontName());
            m_font = new CFont(decal->GetFont());
        }

        UpdateBaseVisuals(decal, &decal->m_d);
        //only show the first element on multi-select
        break;
    }
}

void DecalVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDecal))
            continue;
        Decal * const decal = (Decal *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_FONT_TYPE_COMBO:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_decaltype = (DecalType)PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList);
                PropertyDialog::EndUndo(decal);
                break;
            case IDC_DECAL_TEXT_EDIT:
                PropertyDialog::StartUndo(decal);
                strncpy_s(decal->m_d.m_sztext, MAXSTRING - 1, m_textEdit.GetWindowText().c_str(), m_textEdit.GetWindowText().GetLength());
                PropertyDialog::EndUndo(decal);
                break;
            case IDC_DECAL_VERTICAL_TEXT_CHECK:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_verticalText = PropertyDialog::GetCheckboxState(m_hVerticalTextCheck);
                PropertyDialog::EndUndo(decal);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(decal->m_d.m_color);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    decal->m_d.m_color = m_colorDialog.GetColor();
                    m_fontColorButton.SetColor(decal->m_d.m_color);
                }
                break;
            }
            case IDC_FONT_DIALOG_BUTTON:
            {
                CHOOSEFONT  cf = m_fontDialog.GetParameters();
                cf.lpLogFont = &m_font->GetLogFont();
                cf.Flags |= CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
                cf.rgbColors = decal->m_d.m_color;
                m_fontDialog.SetParameters(cf);
                m_fontDialog.SetColor(decal->m_d.m_color);
                if (m_fontDialog.DoModal(GetHwnd()) == IDOK)
                {
                    FONTDESC fd;
                    m_font->CreateFontIndirect(m_fontDialog.GetLogFont());

                    fd.cbSizeofstruct = sizeof(FONTDESC);

                    int len = lstrlen(m_font->GetLogFont().lfFaceName) + 1;
                    fd.lpstrName = (LPOLESTR)malloc(len * sizeof(WCHAR));
                    memset(fd.lpstrName, 0, len * sizeof(WCHAR));
                    UNICODE_FROM_ANSI(fd.lpstrName, m_font->GetLogFont().lfFaceName, len);
                    fd.sWeight = (SHORT)m_font->GetLogFont().lfWidth;
                    fd.sCharset = m_font->GetLogFont().lfCharSet;
                    fd.fItalic = m_font->GetLogFont().lfItalic;
                    fd.fUnderline = m_font->GetLogFont().lfUnderline;
                    fd.fStrikethrough = m_font->GetLogFont().lfStrikeOut;

                    // free old font first
                    decal->m_pIFont->Release();
                    // create the new one
                    OleCreateFontIndirect(&fd, IID_IFont, (void **)&decal->m_pIFont);
                    free(fd.lpstrName);
                    decal->m_d.m_color = m_fontDialog.GetColor();
                }
                break;
            }
            case DISPID_Decal_SizingType:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_sizingtype = (SizingType)PropertyDialog::GetComboBoxIndex(m_sizingCombo, m_sizingList);
                PropertyDialog::EndUndo(decal);
                break;
            case 5:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_vCenter.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(decal);
                break;
            case 6:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_vCenter.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(decal);
                break;
            case 3:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_width = PropertyDialog::GetFloatTextbox(m_widthEdit);
                PropertyDialog::EndUndo(decal);
                break;
            case 4:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_height = PropertyDialog::GetFloatTextbox(m_heigthEdit);
                PropertyDialog::EndUndo(decal);
                break;
            case 1:
                PropertyDialog::StartUndo(decal);
                decal->m_d.m_rotation= PropertyDialog::GetFloatTextbox(m_rotationEdit);
                PropertyDialog::EndUndo(decal);
                break;
            case IDC_SURFACE_COMBO:
                PropertyDialog::StartUndo(decal);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, decal->m_d.m_szSurface);
                PropertyDialog::EndUndo(decal);
                break;

            default:
                UpdateBaseProperties(decal, &decal->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL DecalVisualsProperty::OnInitDialog()
{
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    AttachItem(IDC_FONT_TYPE_COMBO, m_typeCombo);
    AttachItem(IDC_DECAL_TEXT_EDIT, m_textEdit);
    m_hVerticalTextCheck = ::GetDlgItem(GetHwnd(), IDC_DECAL_VERTICAL_TEXT_CHECK);
    AttachItem(IDC_COLOR_BUTTON1, m_fontColorButton);
    AttachItem(IDC_FONT_DIALOG_BUTTON, m_fontDialogButton);
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;

    AttachItem(DISPID_Decal_SizingType, m_sizingCombo);
    AttachItem(5, m_posXEdit);
    AttachItem(6, m_posYEdit);
    AttachItem(3, m_widthEdit);
    AttachItem(4, m_heigthEdit);
    AttachItem(1, m_rotationEdit);
    AttachItem(IDC_SURFACE_COMBO, m_surfaceCombo);
    UpdateVisuals();
    return TRUE;
}

INT_PTR DecalVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            UINT nID = static_cast<UINT>(wParam);
            if (nID == IDC_COLOR_BUTTON1)
            {
                m_fontColorButton.DrawItem(lpDrawItemStruct);
            }
            return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}

