#include "stdafx.h"
#include "Properties/FlipperVisualsProperty.h"
#include <WindowsX.h>

FlipperVisualsProperty::FlipperVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPFLIPPER_VISUALS, pvsel)
{
}

void FlipperVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlipper))
            continue;
        Flipper * const flipper = (Flipper *)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateMaterialComboBox(flipper->GetPTable()->GetMaterialList(), m_rubberMaterialCombo, flipper->m_d.m_szRubberMaterial);
        PropertyDialog::SetFloatTextbox(m_rubberThicknessEdit, flipper->m_d.m_rubberthickness);
        PropertyDialog::SetFloatTextbox(m_rubberOffsetHeightEdit, flipper->m_d.m_rubberheight);
        PropertyDialog::SetFloatTextbox(m_rubberWidthEdit, flipper->m_d.m_rubberwidth);
        PropertyDialog::SetFloatTextbox(m_posXEdit, flipper->m_d.m_Center.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, flipper->m_d.m_Center.y);
        PropertyDialog::SetFloatTextbox(m_baseRadiusEdit, flipper->m_d.m_BaseRadius);
        PropertyDialog::SetFloatTextbox(m_endRadiusEdit, flipper->m_d.m_EndRadius);
        PropertyDialog::SetFloatTextbox(m_lengthEdit, flipper->m_d.m_FlipperRadiusMax);
        PropertyDialog::SetFloatTextbox(m_startAngleEdit, flipper->m_d.m_StartAngle);
        PropertyDialog::SetFloatTextbox(m_endAngleEdit, flipper->m_d.m_EndAngle);
        PropertyDialog::SetFloatTextbox(m_heightEdit, flipper->m_d.m_height);
        PropertyDialog::SetFloatTextbox(m_maxDifficultLengthEdit, flipper->GetFlipperRadiusMin());
        PropertyDialog::UpdateSurfaceComboBox(flipper->GetPTable(), m_surfaceCombo, flipper->m_d.m_szSurface);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_FLIPPER_ENABLED), flipper->m_d.m_enabled);
        UpdateBaseVisuals(flipper, &flipper->m_d);
        //only show the first element on multi-select
        break;
    }
}

void FlipperVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlipper))
            continue;
        Flipper * const flipper = (Flipper *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 1:
                CHECK_UPDATE_ITEM(flipper->m_d.m_BaseRadius, PropertyDialog::GetFloatTextbox(m_baseRadiusEdit), flipper);
                break;
            case 2:
                CHECK_UPDATE_ITEM(flipper->m_d.m_EndRadius, PropertyDialog::GetFloatTextbox(m_endRadiusEdit), flipper);
                break;
            case 3:
                CHECK_UPDATE_ITEM(flipper->m_d.m_FlipperRadiusMax, PropertyDialog::GetFloatTextbox(m_lengthEdit), flipper);
                break;
            case 4:
                CHECK_UPDATE_ITEM(flipper->m_d.m_StartAngle, PropertyDialog::GetFloatTextbox(m_startAngleEdit), flipper);
                break;
            case 7:
                CHECK_UPDATE_ITEM(flipper->m_d.m_EndAngle, PropertyDialog::GetFloatTextbox(m_endAngleEdit), flipper);
                break;
            case 13:
                CHECK_UPDATE_ITEM(flipper->m_d.m_Center.x, PropertyDialog::GetFloatTextbox(m_posXEdit), flipper);
                break;
            case 14:
                CHECK_UPDATE_ITEM(flipper->m_d.m_Center.y, PropertyDialog::GetFloatTextbox(m_posYEdit), flipper);
                break;
            case 18:
                CHECK_UPDATE_ITEM(flipper->m_d.m_rubberthickness, PropertyDialog::GetFloatTextbox(m_rubberThicknessEdit), flipper);
                break;
            case 24:
                CHECK_UPDATE_ITEM(flipper->m_d.m_rubberheight, PropertyDialog::GetFloatTextbox(m_rubberOffsetHeightEdit), flipper);
                break;
            case 25:
                CHECK_UPDATE_ITEM(flipper->m_d.m_rubberwidth, PropertyDialog::GetFloatTextbox(m_rubberWidthEdit), flipper);
                break;
            case 107:
                CHECK_UPDATE_ITEM(flipper->m_d.m_height, PropertyDialog::GetFloatTextbox(m_heightEdit), flipper);
                break;
            case 111:
                CHECK_UPDATE_VALUE_SETTER(flipper->SetFlipperRadiusMin, flipper->GetFlipperRadiusMin, PropertyDialog::GetFloatTextbox, m_maxDifficultLengthEdit, flipper);
                break;
            case 1502:
                CHECK_UPDATE_COMBO_TEXT(flipper->m_d.m_szSurface, m_surfaceCombo, flipper);
                break;
            case IDC_FLIPPER_ENABLED:
                CHECK_UPDATE_ITEM(flipper->m_d.m_enabled, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), flipper);
                break;
            case IDC_MATERIAL_COMBO2:
                CHECK_UPDATE_COMBO_TEXT(flipper->m_d.m_szRubberMaterial, m_rubberMaterialCombo, flipper);
                break;

            default:
                UpdateBaseProperties(flipper, &flipper->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL FlipperVisualsProperty::OnInitDialog()
{
    AttachItem(DISPID_Image, m_imageCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    AttachItem(IDC_MATERIAL_COMBO2, m_rubberMaterialCombo);
    AttachItem(18, m_rubberThicknessEdit);
    AttachItem(24, m_rubberOffsetHeightEdit);
    AttachItem(25, m_rubberWidthEdit);
    AttachItem(13, m_posXEdit);
    AttachItem(14, m_posYEdit);
    AttachItem(1, m_baseRadiusEdit);
    AttachItem(2, m_endRadiusEdit);
    AttachItem(3, m_lengthEdit);
    AttachItem(4, m_startAngleEdit);
    AttachItem(7, m_endAngleEdit);
    AttachItem(107, m_heightEdit);
    AttachItem(111, m_maxDifficultLengthEdit);
    AttachItem(1502, m_surfaceCombo);

    m_baseImageCombo = &m_imageCombo;
    m_baseMaterialCombo = &m_materialCombo;
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    UpdateVisuals();
    return TRUE;
}

