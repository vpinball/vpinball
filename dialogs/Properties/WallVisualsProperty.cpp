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
    return FALSE;
}

