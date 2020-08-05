#include "stdafx.h"
#include "Properties/WallVisualsProperty.h"
#include <WindowsX.h>

WallVisualsProperty::WallVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPWALL_VISUALS, pvsel)
{
    m_disableLightingEdit.SetDialog(this);
    m_disableLightFromBelowEdit.SetDialog(this);
    m_topHeightEdit.SetDialog(this);
    m_bottomHeightEdit.SetDialog(this);
    m_topImageCombo.SetDialog(this);
    m_sideImageCombo.SetDialog(this);
    m_topMaterialCombo.SetDialog(this);
    m_sideMaterialCombo.SetDialog(this);
    m_slingshotMaterialCombo.SetDialog(this);
}

void WallVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    //only show the first element on multi-select
    Surface* const wall = (Surface*)m_pvsel->ElementAt(0);

    if (dispid == DISPID_Image2 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(wall->GetPTable()->GetImageList(), m_sideImageCombo, wall->m_d.m_szSideImage);
    if (dispid == IDC_MATERIAL_COMBO || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_topMaterialCombo, wall->m_d.m_szTopMaterial);
    if (dispid == IDC_MATERIAL_COMBO2 || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_sideMaterialCombo, wall->m_d.m_szSideMaterial);
    if (dispid == IDC_MATERIAL_COMBO3 || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_slingshotMaterialCombo, wall->m_d.m_szSlingShotMaterial);
    if (dispid == 16 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 16), wall->m_d.m_topBottomVisible);
    if (dispid == 13 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 13), wall->m_d.m_displayTexture);
    if (dispid == 109 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 109), wall->m_d.m_sideVisible);
    if (dispid == 112 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 112), wall->m_d.m_slingshotAnimation);
    if (dispid == 113 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 113), wall->m_d.m_flipbook);
    if (dispid == IDC_BLEND_DISABLE_LIGHTING || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_disableLightingEdit, wall->m_d.m_disableLightingTop);
    if (dispid == IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, wall->m_d.m_disableLightingBelow);
    if (dispid == 9 || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_topHeightEdit, wall->m_d.m_heighttop);
    if (dispid == 8 || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_bottomHeightEdit, wall->m_d.m_heightbottom);

    UpdateBaseVisuals(wall, &wall->m_d, dispid);
}

void WallVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSurface))
            continue;
        Surface * const wall = (Surface*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 9:
                CHECK_UPDATE_ITEM(wall->m_d.m_heighttop, PropertyDialog::GetFloatTextbox(m_topHeightEdit), wall);
                break;
            case 8:
                CHECK_UPDATE_ITEM(wall->m_d.m_heightbottom, PropertyDialog::GetFloatTextbox(m_bottomHeightEdit), wall);
                break;
            case DISPID_Image2:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szSideImage, m_sideImageCombo, wall);
                break;
            case IDC_MATERIAL_COMBO:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szTopMaterial, m_topMaterialCombo, wall);
                break;
            case IDC_MATERIAL_COMBO2:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szSideMaterial, m_sideMaterialCombo, wall);
                break;
            case IDC_MATERIAL_COMBO3:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szSlingShotMaterial, m_slingshotMaterialCombo, wall);
                break;
            case IDC_BLEND_DISABLE_LIGHTING:
                CHECK_UPDATE_ITEM(wall->m_d.m_disableLightingTop, PropertyDialog::GetFloatTextbox(m_disableLightingEdit), wall);
                break;
            case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
                CHECK_UPDATE_ITEM(wall->m_d.m_disableLightingBelow, PropertyDialog::GetFloatTextbox(m_disableLightFromBelowEdit), wall);
                break;
            case 16:
                CHECK_UPDATE_ITEM(wall->m_d.m_topBottomVisible, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 13:
                CHECK_UPDATE_ITEM(wall->m_d.m_displayTexture, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 109:
                CHECK_UPDATE_ITEM(wall->m_d.m_sideVisible, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 112:
                CHECK_UPDATE_ITEM(wall->m_d.m_slingshotAnimation, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 113:
                CHECK_UPDATE_ITEM(wall->m_d.m_flipbook, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            default:
                UpdateBaseProperties(wall, &wall->m_d, dispid);
                break;
        }
        wall->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL WallVisualsProperty::OnInitDialog()
{
    m_topImageCombo.AttachItem(DISPID_Image);
    m_sideImageCombo.AttachItem(DISPID_Image2);
    m_topMaterialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_sideMaterialCombo.AttachItem(IDC_MATERIAL_COMBO2);
    m_slingshotMaterialCombo.AttachItem(IDC_MATERIAL_COMBO3);
    m_disableLightingEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING);
    m_disableLightFromBelowEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW);
    m_topHeightEdit.AttachItem(9);
    m_bottomHeightEdit.AttachItem(8);

    m_baseImageCombo = &m_topImageCombo;
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);

    UpdateVisuals();
    return TRUE;
}
