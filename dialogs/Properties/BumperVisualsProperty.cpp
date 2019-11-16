#include "stdafx.h"
#include "Properties/BumperVisualsProperty.h"
#include <WindowsX.h>

BumperVisualsProperty::BumperVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPBUMPER_VISUALS, pvsel)
{
}

void BumperVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBumper))
            continue;
        Bumper *const bumper = (Bumper*)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_capMaterialCombo, bumper->m_d.m_szCapMaterial);
        PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_bumpBaseMaterialCombo, bumper->m_d.m_szBaseMaterial);
        PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_skirtMaterialCombo, bumper->m_d.m_szSkirtMaterial);
        PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_ringMaterialCombo, bumper->m_d.m_szRingMaterial);
        PropertyDialog::SetFloatTextbox(m_radiusEdit, bumper->m_d.m_radius);
        PropertyDialog::SetFloatTextbox(m_heightScaleEdit, bumper->m_d.m_heightScale);
        PropertyDialog::SetFloatTextbox(m_orientationEdit, bumper->m_d.m_orientation);
        PropertyDialog::SetFloatTextbox(m_ringSpeedEdit, bumper->m_d.m_ringSpeed);
        PropertyDialog::SetFloatTextbox(m_ringSpeedOffsetEdit, bumper->m_d.m_ringDropOffset);
        PropertyDialog::SetCheckboxState(m_hCapVisibleCheck, bumper->m_d.m_capVisible);
        PropertyDialog::SetCheckboxState(m_hBaseVisibleCheck, bumper->m_d.m_baseVisible);
        PropertyDialog::SetCheckboxState(m_hRingVisibleCheck, bumper->m_d.m_ringVisible);
        PropertyDialog::SetCheckboxState(m_hSkirtVisibleCheck, bumper->m_d.m_skirtVisible);
        PropertyDialog::SetFloatTextbox(m_posXEdit, bumper->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, bumper->m_d.m_vCenter.y);
        PropertyDialog::UpdateSurfaceComboBox(bumper->GetPTable(), m_surfaceCombo, bumper->m_d.m_szSurface);
        UpdateBaseVisuals(bumper, &bumper->m_d);
        //only show the first element on multi-select
        break;
    }
}

void BumperVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBumper))
            continue;
        Bumper *const bumper = (Bumper *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_MATERIAL_COMBO:
                PropertyDialog::StartUndo(bumper);
                PropertyDialog::GetComboBoxText(m_capMaterialCombo, bumper->m_d.m_szCapMaterial);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_MATERIAL_COMBO2:
                PropertyDialog::StartUndo(bumper);
                PropertyDialog::GetComboBoxText(m_bumpBaseMaterialCombo, bumper->m_d.m_szBaseMaterial);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_MATERIAL_COMBO3:
                PropertyDialog::StartUndo(bumper);
                PropertyDialog::GetComboBoxText(m_skirtMaterialCombo, bumper->m_d.m_szSkirtMaterial);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_MATERIAL_COMBO4:
                PropertyDialog::StartUndo(bumper);
                PropertyDialog::GetComboBoxText(m_ringMaterialCombo, bumper->m_d.m_szRingMaterial);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_BUMPER_RADIUS_EDIT:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_radius = PropertyDialog::GetFloatTextbox(m_radiusEdit);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_BUMPER_HEIGHT_SCALE_EDIT:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_heightScale = PropertyDialog::GetFloatTextbox(m_heightScaleEdit);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_ORIENTATION_EDIT:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_orientation = PropertyDialog::GetFloatTextbox(m_orientationEdit);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_RINGSPEED_EDIT:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_ringSpeed = PropertyDialog::GetFloatTextbox(m_ringSpeedEdit);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_RINGDROPOFFSET_EDIT:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_ringDropOffset = PropertyDialog::GetFloatTextbox(m_ringSpeedOffsetEdit);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_CAP_VISIBLE_CHECK:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_capVisible = PropertyDialog::GetCheckboxState(m_hCapVisibleCheck);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_BASE_VISIBLE_CHECK:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_baseVisible = PropertyDialog::GetCheckboxState(m_hBaseVisibleCheck);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_RING_VISIBLE:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_ringVisible = PropertyDialog::GetCheckboxState(m_hRingVisibleCheck);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_SKIRT_VISIBLE:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_skirtVisible = PropertyDialog::GetCheckboxState(m_hSkirtVisibleCheck);
                PropertyDialog::EndUndo(bumper);
                break;
            case 902:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_vCenter.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(bumper);
                break;
            case 903:
                PropertyDialog::StartUndo(bumper);
                bumper->m_d.m_vCenter.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(bumper);
                break;
            case IDC_SURFACE_COMBO:
                PropertyDialog::StartUndo(bumper);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, bumper->m_d.m_szSurface);
                PropertyDialog::EndUndo(bumper);
                break;

            default:
                UpdateBaseProperties(bumper, &bumper->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL BumperVisualsProperty::OnInitDialog()
{
    AttachItem(IDC_MATERIAL_COMBO, m_capMaterialCombo);
    AttachItem(IDC_MATERIAL_COMBO2, m_bumpBaseMaterialCombo);
    AttachItem(IDC_MATERIAL_COMBO3, m_skirtMaterialCombo);
    AttachItem(IDC_MATERIAL_COMBO4, m_ringMaterialCombo);
    AttachItem(IDC_BUMPER_RADIUS_EDIT, m_radiusEdit);
    AttachItem(IDC_BUMPER_HEIGHT_SCALE_EDIT, m_heightScaleEdit);
    AttachItem(IDC_ORIENTATION_EDIT, m_orientationEdit);
    AttachItem(IDC_RINGSPEED_EDIT, m_ringSpeedEdit);
    AttachItem(IDC_RINGDROPOFFSET_EDIT, m_ringSpeedOffsetEdit);
    m_hCapVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_CAP_VISIBLE_CHECK);
    m_hBaseVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_BASE_VISIBLE_CHECK);
    m_hRingVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_RING_VISIBLE);
    m_hSkirtVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_SKIRT_VISIBLE);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    AttachItem(902, m_posXEdit);
    AttachItem(903, m_posYEdit);
    AttachItem(1502, m_surfaceCombo);
    UpdateVisuals();
    return TRUE;
}
