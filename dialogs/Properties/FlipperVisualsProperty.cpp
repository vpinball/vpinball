#include "stdafx.h"
#include "Properties/FlipperVisualsProperty.h"
#include <WindowsX.h>

FlipperVisualsProperty::FlipperVisualsProperty(VectorProtected<ISelect> *pvsel) : BaseProperty(IDD_PROPFLIPPER_VISUALS, pvsel)
{
}

void FlipperVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlipper))
            continue;
        Flipper *flipper = (Flipper *)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateTextureComboBox(flipper->GetPTable()->GetImageList(), m_imageCombo, flipper->m_d.m_szImage);
        PropertyDialog::UpdateMaterialComboBox(flipper->GetPTable()->GetMaterialList(), m_materialCombo, flipper->m_d.m_szMaterial);
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
        PropertyDialog::SetFloatTextbox(m_maxDifficultLengthEdit, flipper->m_d.m_FlipperRadiusMin);
        PropertyDialog::UpdateSurfaceComboBox(flipper->GetPTable(), m_surfaceCombo, flipper->m_d.m_szSurface);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 20), flipper->m_d.m_visible);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_FLIPPER_ENABLED), flipper->m_d.m_enabled);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_FLIPPER_REFLECT_ENABLED_CHECK), flipper->m_d.m_reflectionEnabled);
    }
}

void FlipperVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlipper))
            continue;
        Flipper *flipper = (Flipper *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 1:
                flipper->m_d.m_BaseRadius = PropertyDialog::GetFloatTextbox(m_baseRadiusEdit);
                break;
            case 2:
                flipper->m_d.m_EndRadius = PropertyDialog::GetFloatTextbox(m_endRadiusEdit);
                break;
            case 3:
                flipper->m_d.m_FlipperRadiusMax = PropertyDialog::GetFloatTextbox(m_lengthEdit);
                break;
            case 4:
                flipper->m_d.m_StartAngle = PropertyDialog::GetFloatTextbox(m_startAngleEdit);
                break;
            case 7:
                flipper->m_d.m_EndAngle = PropertyDialog::GetFloatTextbox(m_endAngleEdit);
                break;
            case 13:
                flipper->m_d.m_Center.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                break;
            case 14:
                flipper->m_d.m_Center.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                break;
            case 18:
                flipper->m_d.m_rubberthickness = PropertyDialog::GetFloatTextbox(m_rubberThicknessEdit);
                break;
            case 20:
                flipper->m_d.m_visible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 20));
                break;
            case 24:
                flipper->m_d.m_rubberheight = PropertyDialog::GetFloatTextbox(m_rubberOffsetHeightEdit);
                break;
            case 25:
                flipper->m_d.m_rubberwidth = PropertyDialog::GetFloatTextbox(m_rubberWidthEdit);
                break;
            case 107:
                flipper->m_d.m_height = PropertyDialog::GetFloatTextbox(m_heightEdit);
                break;
            case 111:
                flipper->m_d.m_FlipperRadiusMin = PropertyDialog::GetFloatTextbox(m_maxDifficultLengthEdit);
                break;
            case 1502:
                PropertyDialog::GetComboBoxText(m_surfaceCombo, flipper->m_d.m_szSurface);
                break;
            case IDC_FLIPPER_ENABLED:
                flipper->m_d.m_enabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_FLIPPER_ENABLED));
                break;
            case IDC_FLIPPER_REFLECT_ENABLED_CHECK:
                flipper->m_d.m_reflectionEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_FLIPPER_REFLECT_ENABLED_CHECK));
                break;
            case DISPID_Image:
                PropertyDialog::GetComboBoxText(m_imageCombo, flipper->m_d.m_szImage);
                break;
            case IDC_MATERIAL_COMBO:
                PropertyDialog::GetComboBoxText(m_materialCombo, flipper->m_d.m_szMaterial);
                break;
            case IDC_MATERIAL_COMBO2:
                PropertyDialog::GetComboBoxText(m_rubberMaterialCombo, flipper->m_d.m_szRubberMaterial);
                break;

            default:
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
    UpdateVisuals();
    return TRUE;
}

BOOL FlipperVisualsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int dispID = LOWORD(wParam);

    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        case BN_CLICKED:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}

