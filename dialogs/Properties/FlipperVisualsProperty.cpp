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
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_BaseRadius = PropertyDialog::GetFloatTextbox(m_baseRadiusEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 2:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_EndRadius = PropertyDialog::GetFloatTextbox(m_endRadiusEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 3:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_FlipperRadiusMax = PropertyDialog::GetFloatTextbox(m_lengthEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 4:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_StartAngle = PropertyDialog::GetFloatTextbox(m_startAngleEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 7:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_EndAngle = PropertyDialog::GetFloatTextbox(m_endAngleEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 13:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_Center.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 14:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_Center.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 18:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_rubberthickness = PropertyDialog::GetFloatTextbox(m_rubberThicknessEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 20:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_visible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 20));
                PropertyDialog::EndUndo(flipper);
                break;
            case 24:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_rubberheight = PropertyDialog::GetFloatTextbox(m_rubberOffsetHeightEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 25:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_rubberwidth = PropertyDialog::GetFloatTextbox(m_rubberWidthEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 107:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_height = PropertyDialog::GetFloatTextbox(m_heightEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 111:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_FlipperRadiusMin = PropertyDialog::GetFloatTextbox(m_maxDifficultLengthEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 1502:
                PropertyDialog::StartUndo(flipper);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, flipper->m_d.m_szSurface);
                PropertyDialog::EndUndo(flipper);
                break;
            case IDC_FLIPPER_ENABLED:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_enabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_FLIPPER_ENABLED));
                PropertyDialog::EndUndo(flipper);
                break;
            case IDC_FLIPPER_REFLECT_ENABLED_CHECK:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_reflectionEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_FLIPPER_REFLECT_ENABLED_CHECK));
                PropertyDialog::EndUndo(flipper);
                break;
            case DISPID_Image:
                PropertyDialog::StartUndo(flipper);
                PropertyDialog::GetComboBoxText(m_imageCombo, flipper->m_d.m_szImage);
                PropertyDialog::EndUndo(flipper);
                break;
            case IDC_MATERIAL_COMBO:
                PropertyDialog::StartUndo(flipper);
                PropertyDialog::GetComboBoxText(m_materialCombo, flipper->m_d.m_szMaterial);
                PropertyDialog::EndUndo(flipper);
                break;
            case IDC_MATERIAL_COMBO2:
                PropertyDialog::StartUndo(flipper);
                PropertyDialog::GetComboBoxText(m_rubberMaterialCombo, flipper->m_d.m_szRubberMaterial);
                PropertyDialog::EndUndo(flipper);
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

