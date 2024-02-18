#include "core/stdafx.h"
#include "ui/properties/DecalVisualsProperty.h"
#include <WindowsX.h>

DecalVisualsProperty::DecalVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPDECAL_VISUALS, pvsel), m_font(nullptr)
{
    m_typeList.push_back("Text"s);
    m_typeList.push_back("Image"s);

    m_sizingList.push_back("Auto Size"s);
    m_sizingList.push_back("Auto Width"s);
    m_sizingList.push_back("Manual Size"s);
    m_textEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_widthEdit.SetDialog(this);
    m_heightEdit.SetDialog(this);
    m_rotationEdit.SetDialog(this);
    m_materialCombo.SetDialog(this);
    m_typeCombo.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_sizingCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

DecalVisualsProperty::~DecalVisualsProperty()
{
    delete m_font;
}

void DecalVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDecal))
            continue;
        Decal * const decal = (Decal *)m_pvsel->ElementAt(i);

        if (dispid == IDC_FONT_TYPE_COMBO || dispid == -1)
        {
            PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[decal->m_d.m_decaltype]);
            decal->EnsureSize();
        }
        if (dispid == DISPID_Decal_SizingType || dispid == -1)
        {
            PropertyDialog::UpdateComboBox(m_sizingList, m_sizingCombo, m_sizingList[decal->m_d.m_sizingtype]);
            decal->EnsureSize();
        }
        if (dispid == IDC_DECAL_TEXT_EDIT || dispid == -1)
        {
            m_textEdit.SetWindowText(decal->m_d.m_sztext.c_str());
            decal->EnsureSize();
        }
        if (dispid == IDC_DECAL_VERTICAL_TEXT_CHECK || dispid == -1)
        {
            PropertyDialog::SetCheckboxState(m_hVerticalTextCheck, decal->m_d.m_verticalText);
            decal->EnsureSize();
        }
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_fontColorButton.SetColor(decal->m_d.m_color);

        if (dispid == 5 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, decal->m_d.m_vCenter.x);
        if (dispid == 6 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, decal->m_d.m_vCenter.y);
        if (dispid == 3 || dispid == -1)
        {
            PropertyDialog::SetFloatTextbox(m_widthEdit, decal->m_d.m_width);
            decal->EnsureSize();
        }
        if (dispid == 4 || dispid == -1)
        {
            PropertyDialog::SetFloatTextbox(m_heightEdit, decal->m_d.m_height);
            decal->EnsureSize();
        }
        if (dispid == 1 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotationEdit, decal->m_d.m_rotation);
        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(decal->GetPTable(), m_surfaceCombo, decal->m_d.m_szSurface);

        if (decal->m_pIFont)
        {
            m_fontDialogButton.SetWindowText(decal->GetFontName());
            m_font = new CFont(decal->GetFont());
        }

        UpdateBaseVisuals(decal, &decal->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void DecalVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDecal))
            continue;
        Decal * const decal = (Decal *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_FONT_TYPE_COMBO:
                CHECK_UPDATE_ITEM(decal->m_d.m_decaltype, (DecalType)PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList), decal);
                break;
            case IDC_DECAL_TEXT_EDIT:
            {
                const string szName(m_textEdit.GetWindowText());
                if (decal->m_d.m_sztext != szName)
                {
                    PropertyDialog::StartUndo(decal);
                    decal->m_d.m_sztext = szName;
                    PropertyDialog::EndUndo(decal);
                }
                break;
            }
            case IDC_DECAL_VERTICAL_TEXT_CHECK:
                CHECK_UPDATE_ITEM(decal->m_d.m_verticalText, PropertyDialog::GetCheckboxState(m_hVerticalTextCheck), decal);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(decal->m_d.m_color);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    decal->m_d.m_color = m_colorDialog.GetColor();
                    m_fontColorButton.SetColor(decal->m_d.m_color);
                    memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
                }
                break;
            }
            case IDC_FONT_DIALOG_BUTTON:
            {
                CHOOSEFONT cf = m_fontDialog.GetParameters();
                LOGFONT lf = m_font->GetLogFont();
                cf.lpLogFont = &lf;
                cf.Flags |= CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
                cf.rgbColors = decal->m_d.m_color;
                m_fontDialog.SetParameters(cf);
                m_fontDialog.SetColor(decal->m_d.m_color);
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
                    decal->m_pIFont->Release();
                    // create the new one
                    OleCreateFontIndirect(&fd, IID_IFont, (void **)&decal->m_pIFont);
                    free(fd.lpstrName);
                    decal->m_d.m_color = m_fontDialog.GetColor();
                }
                break;
            }
            case DISPID_Decal_SizingType:
                CHECK_UPDATE_ITEM(decal->m_d.m_sizingtype, (SizingType)PropertyDialog::GetComboBoxIndex(m_sizingCombo, m_sizingList), decal);
                break;
            case 5:
                CHECK_UPDATE_ITEM(decal->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), decal);
                break;
            case 6:
                CHECK_UPDATE_ITEM(decal->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), decal);
                break;
            case 3:
                CHECK_UPDATE_ITEM(decal->m_d.m_width, PropertyDialog::GetFloatTextbox(m_widthEdit), decal);
                break;
            case 4:
                CHECK_UPDATE_ITEM(decal->m_d.m_height, PropertyDialog::GetFloatTextbox(m_heightEdit), decal);
                break;
            case 1:
                CHECK_UPDATE_ITEM(decal->m_d.m_rotation, PropertyDialog::GetFloatTextbox(m_rotationEdit), decal);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT_STRING(decal->m_d.m_szSurface, m_surfaceCombo, decal);
                break;
            default:
                UpdateBaseProperties(decal, &decal->m_d, dispid);
                break;
        }
        decal->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL DecalVisualsProperty::OnInitDialog()
{
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_baseMaterialCombo = &m_materialCombo;
    m_typeCombo.AttachItem(IDC_FONT_TYPE_COMBO);
    m_hVerticalTextCheck = ::GetDlgItem(GetHwnd(), IDC_DECAL_VERTICAL_TEXT_CHECK);
    AttachItem(IDC_COLOR_BUTTON1, m_fontColorButton);
    AttachItem(IDC_FONT_DIALOG_BUTTON, m_fontDialogButton);
    m_imageCombo.AttachItem(DISPID_Image);
    m_baseImageCombo = &m_imageCombo;

    m_sizingCombo.AttachItem(DISPID_Decal_SizingType);
    m_textEdit.AttachItem(IDC_DECAL_TEXT_EDIT);
    m_posXEdit.AttachItem(5);
    m_posYEdit.AttachItem(6);
    m_widthEdit.AttachItem(3);
    m_heightEdit.AttachItem(4);
    m_rotationEdit.AttachItem(1);
    m_surfaceCombo.AttachItem(IDC_SURFACE_COMBO);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_typeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hVerticalTextCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_fontColorButton, CResizer::topleft, 0);
    m_resizer.AddChild(m_fontDialogButton, CResizer::topleft, 0);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_sizingCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_textEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_widthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_heightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotationEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_surfaceCombo, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR DecalVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                m_fontColorButton.DrawItem(lpDrawItemStruct);
            }
            return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}
