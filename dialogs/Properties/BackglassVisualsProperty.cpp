#include "stdafx.h"
#include "Properties/BackglassVisualsProperty.h"
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
    if(dispid == IDC_BG_NIGHT_DAY || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hApplyNightDayCheck, table->m_ImageBackdropNightDay);
    if (dispid == DISPID_Image2 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_dtImageCombo, table->m_BG_szImage[0]);
    if (dispid == DISPID_Image6 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_fsImageCombo, table->m_BG_szImage[1]);
    if (dispid == DISPID_Image8 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_fssImageCombo, table->m_BG_szImage[2]);
    if (dispid == 1509 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_colorGradingCombo, table->m_szImageColorGrade);
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
    switch (dispid)
    {
        case IDC_BG_NIGHT_DAY:
            CHECK_UPDATE_ITEM(table->m_ImageBackdropNightDay, PropertyDialog::GetCheckboxState(m_hApplyNightDayCheck), table);
            break;
        case DISPID_Image2:
            CHECK_UPDATE_COMBO_TEXT(table->m_BG_szImage[0], m_dtImageCombo, table);
            break;
        case DISPID_Image6:
            CHECK_UPDATE_COMBO_TEXT(table->m_BG_szImage[1], m_fsImageCombo, table);
            break;
        case DISPID_Image8:
            CHECK_UPDATE_COMBO_TEXT(table->m_BG_szImage[2], m_fssImageCombo, table);
            break;
        case 1509:
            CHECK_UPDATE_COMBO_TEXT(table->m_szImageColorGrade, m_colorGradingCombo, table);
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
            CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
            if (ptable == nullptr)
                break;
            CHOOSECOLOR cc = m_colorDialog.GetParameters();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            m_colorDialog.SetParameters(cc);
            m_colorDialog.SetColor(table->m_colorbackdrop);
            m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
            if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
            {
                table->m_colorbackdrop = m_colorDialog.GetColor();
                m_colorButton1.SetColor(table->m_colorbackdrop);
                memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
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
    m_colorGradingCombo.AttachItem(1509);
    m_hEnableEMReelCheck = ::GetDlgItem(GetHwnd(), IDC_ENABLE_EMREEL_CHECK);
    m_hEnableDecal = ::GetDlgItem(GetHwnd(), IDC_ENABLE_DECAL_CHECK);
    m_hOverwriteGlobalStereoSettingsCheck = ::GetDlgItem(GetHwnd(), IDC_GLOBAL_3DSTEREO);
    m_3dStereoOffsetEdit.AttachItem(IDC_3D_STEREO_OFFSET_EDIT);
    m_3dStereoSeparationEdit.AttachItem(IDC_3D_STEREO_SEPARATION_EDIT);
    m_3dSteroZPDEdit.AttachItem(IDC_3D_STEREO_ZPD_EDIT);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
    UpdateVisuals();
    return TRUE;
}

INT_PTR BackglassVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
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
