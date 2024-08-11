// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/BackglassVisualsProperty.h"
#include <WindowsX.h>

BackglassVisualsProperty::BackglassVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPBACKGLASS_VISUALS, pvsel)
{
    m_3dStereoOffsetEdit.SetDialog(this);
    m_3dStereoSeparationEdit.SetDialog(this);
    m_3dSteroZPDEdit.SetDialog(this);
    m_dtImageCombo.SetDialog(this);
    m_fsImageCombo.SetDialog(this);
    m_fssImageCombo.SetDialog(this);
    m_colorGradingCombo.SetDialog(this);
}

void BackglassVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;

    if(dispid == IDC_BG_NIGHT_DAY || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hApplyNightDayCheck, table->m_ImageBackdropNightDay);
    if (dispid == DISPID_Image2 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_dtImageCombo, table->m_BG_image[0]);
    if (dispid == DISPID_Image6 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_fsImageCombo, table->m_BG_image[1]);
    if (dispid == DISPID_Image8 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_fssImageCombo, table->m_BG_image[2]);
    if (dispid == DISPID_Image5 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_colorGradingCombo, table->m_imageColorGrade);
    if (dispid == IDC_ENABLE_DECAL_CHECK || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hEnableDecal, table->m_renderDecals);
    if (dispid == IDC_ENABLE_EMREEL_CHECK || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hEnableEMReelCheck, table->m_renderEMReels);
    if (dispid == IDC_GLOBAL_3DSTEREO || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hOverwriteGlobalStereoSettingsCheck, table->m_overwriteGlobalStereo3D);
    if (dispid == IDC_3D_STEREO_OFFSET_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_3dStereoOffsetEdit, table->Get3DOffset());
    if (dispid == IDC_3D_STEREO_SEPARATION_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_3dStereoSeparationEdit, table->GetMaxSeparation());
    if (dispid == IDC_3D_STEREO_ZPD_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_3dSteroZPDEdit, table->GetZPD());
    if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
        m_colorButton1.SetColor(table->m_colorbackdrop);
}

void BackglassVisualsProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;

    switch (dispid)
    {
        case IDC_BG_NIGHT_DAY:
            CHECK_UPDATE_ITEM(table->m_ImageBackdropNightDay, PropertyDialog::GetCheckboxState(m_hApplyNightDayCheck), table);
            break;
        case DISPID_Image2:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_BG_image[0], m_dtImageCombo, table);
            break;
        case DISPID_Image6:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_BG_image[1], m_fsImageCombo, table);
            break;
        case DISPID_Image8:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_BG_image[2], m_fssImageCombo, table);
            break;
        case DISPID_Image5:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_imageColorGrade, m_colorGradingCombo, table);
            break;
        case IDC_ENABLE_EMREEL_CHECK:
            CHECK_UPDATE_ITEM(table->m_renderEMReels, PropertyDialog::GetCheckboxState(m_hEnableEMReelCheck), table);
            break;
        case IDC_ENABLE_DECAL_CHECK:
            CHECK_UPDATE_ITEM(table->m_renderDecals, PropertyDialog::GetCheckboxState(m_hEnableDecal), table);
            break;
        case IDC_GLOBAL_3DSTEREO:
            CHECK_UPDATE_ITEM(table->m_overwriteGlobalStereo3D, PropertyDialog::GetCheckboxState(m_hOverwriteGlobalStereoSettingsCheck), table);
            break;
        case IDC_3D_STEREO_OFFSET_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->Set3DOffset, table->Get3DOffset, PropertyDialog::GetFloatTextbox, m_3dStereoOffsetEdit, table);
            break;
        case IDC_3D_STEREO_SEPARATION_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->SetMaxSeparation, table->GetMaxSeparation, PropertyDialog::GetFloatTextbox, m_3dStereoSeparationEdit, table);
            break;
        case IDC_3D_STEREO_ZPD_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->SetZPD, table->GetZPD, PropertyDialog::GetFloatTextbox, m_3dSteroZPDEdit, table);
            break;
        case IDC_COLOR_BUTTON1:
        {
            CHOOSECOLOR cc = m_colorDialog.GetParameters();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            m_colorDialog.SetParameters(cc);
            m_colorDialog.SetColor(table->m_colorbackdrop);
            m_colorDialog.SetCustomColors(table->m_rgcolorcustom);
            if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
            {
                table->m_colorbackdrop = m_colorDialog.GetColor();
                m_colorButton1.SetColor(table->m_colorbackdrop);
                memcpy(table->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(table->m_rgcolorcustom));
            }
            break;
        }
        default:
            break;
    }
    UpdateVisuals(dispid);
}

BOOL BackglassVisualsProperty::OnInitDialog()
{
    m_hApplyNightDayCheck = ::GetDlgItem(GetHwnd(), IDC_BG_NIGHT_DAY);
    m_dtImageCombo.AttachItem(DISPID_Image2);
    m_fsImageCombo.AttachItem(DISPID_Image6);
    m_fssImageCombo.AttachItem(DISPID_Image8);
    m_colorGradingCombo.AttachItem(DISPID_Image5);
    m_hEnableEMReelCheck = ::GetDlgItem(GetHwnd(), IDC_ENABLE_EMREEL_CHECK);
    m_hEnableDecal = ::GetDlgItem(GetHwnd(), IDC_ENABLE_DECAL_CHECK);
    m_hOverwriteGlobalStereoSettingsCheck = ::GetDlgItem(GetHwnd(), IDC_GLOBAL_3DSTEREO);
    m_3dStereoOffsetEdit.AttachItem(IDC_3D_STEREO_OFFSET_EDIT);
    m_3dStereoSeparationEdit.AttachItem(IDC_3D_STEREO_SEPARATION_EDIT);
    m_3dSteroZPDEdit.AttachItem(IDC_3D_STEREO_ZPD_EDIT);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
    UpdateVisuals();

    m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(m_hApplyNightDayCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_dtImageCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_fsImageCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_fssImageCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_colorGradingCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hEnableEMReelCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hEnableDecal, CResizer::topleft, 0);
    m_resizer.AddChild(m_hOverwriteGlobalStereoSettingsCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_3dStereoOffsetEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_3dStereoSeparationEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_3dSteroZPDEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_colorButton1.GetHwnd(), CResizer::topleft, 0);

    return TRUE;
}

INT_PTR BackglassVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                m_colorButton1.DrawItem(lpDrawItemStruct);
            }
            return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}
