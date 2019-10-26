#include "stdafx.h"
#include "Properties/WallVisualsProperty.h"
#include <WindowsX.h>

WallVisualsProperty::WallVisualsProperty(Surface *wall) : CDialog(IDD_PROPWALL_VISUALS), m_wall(wall)
{

}

void WallVisualsProperty::UpdateVisuals()
{
    PropertyDialog::UpdateTextureComboBox(m_wall->GetPTable()->GetImageList(), m_topImageCombo, m_wall->m_d.m_szImage);
    PropertyDialog::UpdateTextureComboBox(m_wall->GetPTable()->GetImageList(), m_sideImageCombo, m_wall->m_d.m_szSideImage);
    PropertyDialog::UpdateMaterialComboBox(m_wall->GetPTable()->GetMaterialList(), m_topMaterialCombo, m_wall->m_d.m_szTopMaterial);
    PropertyDialog::UpdateMaterialComboBox(m_wall->GetPTable()->GetMaterialList(), m_sideMaterialCombo, m_wall->m_d.m_szSideMaterial);
    PropertyDialog::UpdateMaterialComboBox(m_wall->GetPTable()->GetMaterialList(), m_slingshotMaterialCombo, m_wall->m_d.m_szSlingShotMaterial);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 16), m_wall->m_d.m_topBottomVisible);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 13), m_wall->m_d.m_displayTexture);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 109), m_wall->m_d.m_sideVisible);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 112), m_wall->m_d.m_slingshotAnimation);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 113), m_wall->m_d.m_flipbook);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_WALL_REFLECT_ENABLED_CHECK), m_wall->m_d.m_reflectionEnabled);
    PropertyDialog::SetFloatTextbox(m_disableLightingEdit, m_wall->m_d.m_disableLightingTop);
    PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, m_wall->m_d.m_disableLightingBelow);
}

void WallVisualsProperty::UpdateProperties(const int dispid)
{
    switch (dispid)
    {
        case DISPID_Image:
            PropertyDialog::GetComboBoxText(m_topImageCombo, m_wall->m_d.m_szImage);
            break;
        case DISPID_Image2:
            PropertyDialog::GetComboBoxText(m_sideImageCombo, m_wall->m_d.m_szSideImage);
            break;
        case IDC_MATERIAL_COMBO:
            PropertyDialog::GetComboBoxText(m_topMaterialCombo, m_wall->m_d.m_szTopMaterial);
            break;
        case IDC_MATERIAL_COMBO2:
            PropertyDialog::GetComboBoxText(m_sideMaterialCombo, m_wall->m_d.m_szSideMaterial);
            break;
        case IDC_MATERIAL_COMBO3:
            PropertyDialog::GetComboBoxText(m_slingshotMaterialCombo, m_wall->m_d.m_szSlingShotMaterial);
            break;
        case IDC_BLEND_DISABLE_LIGHTING:
            PropertyDialog::SetFloatTextbox(m_disableLightingEdit, m_wall->m_d.m_disableLightingTop);
            break;
        case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
            PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, m_wall->m_d.m_disableLightingBelow);
            break;
        case 16:
            m_wall->m_d.m_topBottomVisible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 16));
            break;
        case 13:
            m_wall->m_d.m_displayTexture = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 13));
            break;
        case 109:
            m_wall->m_d.m_sideVisible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 109));
            break;
        case 112:
            m_wall->m_d.m_slingshotAnimation = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 112));
            break;
        case 113:
            m_wall->m_d.m_flipbook = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 113));
            break;
        case IDC_WALL_REFLECT_ENABLED_CHECK:
            m_wall->m_d.m_reflectionEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_WALL_REFLECT_ENABLED_CHECK));
            break;
        default:
            break;
    }
}

BOOL WallVisualsProperty::OnInitDialog()
{
    AttachItem(DISPID_Image, m_topImageCombo);
    AttachItem(DISPID_Image2, m_sideImageCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_topMaterialCombo);
    AttachItem(IDC_MATERIAL_COMBO2, m_sideMaterialCombo);
    AttachItem(IDC_MATERIAL_COMBO3, m_slingshotMaterialCombo);
    AttachItem(IDC_BLEND_DISABLE_LIGHTING, m_disableLightingEdit);
    AttachItem(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW, m_disableLightFromBelowEdit);
    UpdateVisuals();
    return TRUE;
}

BOOL WallVisualsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        {
            int dispID = LOWORD(wParam);

            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}

