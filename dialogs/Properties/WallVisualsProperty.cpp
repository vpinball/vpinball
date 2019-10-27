#include "stdafx.h"
#include "Properties/WallVisualsProperty.h"
#include <WindowsX.h>

WallVisualsProperty::WallVisualsProperty(VectorProtected<ISelect> *pvsel) : BaseProperty(IDD_PROPWALL_VISUALS), m_pvsel(pvsel)
{

}

void WallVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        Surface *wall = (Surface*)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateTextureComboBox(wall->GetPTable()->GetImageList(), m_topImageCombo, wall->m_d.m_szImage);
        PropertyDialog::UpdateTextureComboBox(wall->GetPTable()->GetImageList(), m_sideImageCombo, wall->m_d.m_szSideImage);
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_topMaterialCombo, wall->m_d.m_szTopMaterial);
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_sideMaterialCombo, wall->m_d.m_szSideMaterial);
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_slingshotMaterialCombo, wall->m_d.m_szSlingShotMaterial);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 16), wall->m_d.m_topBottomVisible);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 13), wall->m_d.m_displayTexture);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 109), wall->m_d.m_sideVisible);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 112), wall->m_d.m_slingshotAnimation);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 113), wall->m_d.m_flipbook);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_WALL_REFLECT_ENABLED_CHECK), wall->m_d.m_reflectionEnabled);
        PropertyDialog::SetFloatTextbox(m_disableLightingEdit, wall->m_d.m_disableLightingTop);
        PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, wall->m_d.m_disableLightingBelow);
        PropertyDialog::SetFloatTextbox(m_topHeightEdit, wall->m_d.m_heighttop);
        PropertyDialog::SetFloatTextbox(m_bottomHeightEdit, wall->m_d.m_heightbottom);
    }
}

void WallVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        Surface *wall = (Surface*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 9:
                wall->m_d.m_heighttop = PropertyDialog::GetFloatTextbox(m_topHeightEdit);
                break;
            case 8:
                wall->m_d.m_heightbottom = PropertyDialog::GetFloatTextbox(m_bottomHeightEdit);
                break;
            case DISPID_Image:
                PropertyDialog::GetComboBoxText(m_topImageCombo, wall->m_d.m_szImage);
                break;
            case DISPID_Image2:
                PropertyDialog::GetComboBoxText(m_sideImageCombo, wall->m_d.m_szSideImage);
                break;
            case IDC_MATERIAL_COMBO:
                PropertyDialog::GetComboBoxText(m_topMaterialCombo, wall->m_d.m_szTopMaterial);
                break;
            case IDC_MATERIAL_COMBO2:
                PropertyDialog::GetComboBoxText(m_sideMaterialCombo, wall->m_d.m_szSideMaterial);
                break;
            case IDC_MATERIAL_COMBO3:
                PropertyDialog::GetComboBoxText(m_slingshotMaterialCombo, wall->m_d.m_szSlingShotMaterial);
                break;
            case IDC_BLEND_DISABLE_LIGHTING:
                wall->m_d.m_disableLightingTop = PropertyDialog::GetFloatTextbox(m_disableLightingEdit);
                break;
            case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
                wall->m_d.m_disableLightingBelow = PropertyDialog::GetFloatTextbox(m_disableLightFromBelowEdit);
                break;
            case 16:
                wall->m_d.m_topBottomVisible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 16));
                break;
            case 13:
                wall->m_d.m_displayTexture = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 13));
                break;
            case 109:
                wall->m_d.m_sideVisible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 109));
                break;
            case 112:
                wall->m_d.m_slingshotAnimation = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 112));
                break;
            case 113:
                wall->m_d.m_flipbook = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 113));
                break;
            case IDC_WALL_REFLECT_ENABLED_CHECK:
                wall->m_d.m_reflectionEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_WALL_REFLECT_ENABLED_CHECK));
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
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
    AttachItem(9, m_topHeightEdit);
    AttachItem(8, m_bottomHeightEdit);
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

