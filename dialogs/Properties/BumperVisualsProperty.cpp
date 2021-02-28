#include "stdafx.h"
#include "Properties/BumperVisualsProperty.h"
#include <WindowsX.h>

BumperVisualsProperty::BumperVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPBUMPER_VISUALS, pvsel)
{
    m_radiusEdit.SetDialog(this);
    m_heightScaleEdit.SetDialog(this);
    m_orientationEdit.SetDialog(this);
    m_ringSpeedEdit.SetDialog(this);
    m_ringDropOffsetEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_capMaterialCombo.SetDialog(this);
    m_bumpBaseMaterialCombo.SetDialog(this);
    m_skirtMaterialCombo.SetDialog(this);
    m_ringMaterialCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

void BumperVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBumper))
            continue;
        Bumper *const bumper = (Bumper*)m_pvsel->ElementAt(i);
        if (dispid == IDC_MATERIAL_COMBO || dispid == -1)
            PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_capMaterialCombo, bumper->m_d.m_szCapMaterial);
        if (dispid == IDC_MATERIAL_COMBO2 || dispid == -1)
            PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_bumpBaseMaterialCombo, bumper->m_d.m_szBaseMaterial);
        if (dispid == IDC_MATERIAL_COMBO3 || dispid == -1)
            PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_skirtMaterialCombo, bumper->m_d.m_szSkirtMaterial);
        if (dispid == IDC_MATERIAL_COMBO4 || dispid == -1)
            PropertyDialog::UpdateMaterialComboBox(bumper->GetPTable()->GetMaterialList(), m_ringMaterialCombo, bumper->m_d.m_szRingMaterial);
        if (dispid == IDC_BUMPER_RADIUS_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_radiusEdit, bumper->m_d.m_radius);
        if (dispid == IDC_BUMPER_HEIGHT_SCALE_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_heightScaleEdit, bumper->m_d.m_heightScale);
        if (dispid == IDC_ORIENTATION_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_orientationEdit, bumper->m_d.m_orientation);
        if (dispid == IDC_RINGSPEED_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_ringSpeedEdit, bumper->m_d.m_ringSpeed);
        if (dispid == IDC_RINGDROPOFFSET_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_ringDropOffsetEdit, bumper->m_d.m_ringDropOffset);
        if (dispid == IDC_CAP_VISIBLE_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hCapVisibleCheck, bumper->m_d.m_capVisible);
        if (dispid == IDC_BASE_VISIBLE_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hBaseVisibleCheck, bumper->m_d.m_baseVisible);
        if (dispid == IDC_RING_VISIBLE || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hRingVisibleCheck, bumper->m_d.m_ringVisible);
        if (dispid == IDC_SKIRT_VISIBLE || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hSkirtVisibleCheck, bumper->m_d.m_skirtVisible);
        if (dispid == 902 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, bumper->m_d.m_vCenter.x);
        if (dispid == 903 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, bumper->m_d.m_vCenter.y);
        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(bumper->GetPTable(), m_surfaceCombo, bumper->m_d.m_szSurface);
        UpdateBaseVisuals(bumper, &bumper->m_d, dispid);
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
                CHECK_UPDATE_COMBO_TEXT_STRING(bumper->m_d.m_szCapMaterial, m_capMaterialCombo, bumper);
                break;
            case IDC_MATERIAL_COMBO2:
                CHECK_UPDATE_COMBO_TEXT_STRING(bumper->m_d.m_szBaseMaterial, m_bumpBaseMaterialCombo, bumper);
                break;
            case IDC_MATERIAL_COMBO3:
                CHECK_UPDATE_COMBO_TEXT_STRING(bumper->m_d.m_szSkirtMaterial, m_skirtMaterialCombo, bumper);
                break;
            case IDC_MATERIAL_COMBO4:
                CHECK_UPDATE_COMBO_TEXT_STRING(bumper->m_d.m_szRingMaterial, m_ringMaterialCombo, bumper);
                break;
            case IDC_BUMPER_RADIUS_EDIT:
                CHECK_UPDATE_ITEM(bumper->m_d.m_radius, PropertyDialog::GetFloatTextbox(m_radiusEdit), bumper);
                break;
            case IDC_BUMPER_HEIGHT_SCALE_EDIT:
                CHECK_UPDATE_ITEM(bumper->m_d.m_heightScale, PropertyDialog::GetFloatTextbox(m_heightScaleEdit), bumper);
                break;
            case IDC_ORIENTATION_EDIT:
                CHECK_UPDATE_ITEM(bumper->m_d.m_orientation, PropertyDialog::GetFloatTextbox(m_orientationEdit), bumper);
                break;
            case IDC_RINGSPEED_EDIT:
                CHECK_UPDATE_ITEM(bumper->m_d.m_ringSpeed, PropertyDialog::GetFloatTextbox(m_ringSpeedEdit), bumper);
                break;
            case IDC_RINGDROPOFFSET_EDIT:
                CHECK_UPDATE_ITEM(bumper->m_d.m_ringDropOffset, PropertyDialog::GetFloatTextbox(m_ringDropOffsetEdit), bumper);
                break;
            case IDC_CAP_VISIBLE_CHECK:
                CHECK_UPDATE_ITEM(bumper->m_d.m_capVisible, PropertyDialog::GetCheckboxState(m_hCapVisibleCheck), bumper);
                break;
            case IDC_BASE_VISIBLE_CHECK:
                CHECK_UPDATE_ITEM(bumper->m_d.m_baseVisible, PropertyDialog::GetCheckboxState(m_hBaseVisibleCheck), bumper);
                break;
            case IDC_RING_VISIBLE:
                CHECK_UPDATE_ITEM(bumper->m_d.m_ringVisible, PropertyDialog::GetCheckboxState(m_hRingVisibleCheck), bumper);
                break;
            case IDC_SKIRT_VISIBLE:
                CHECK_UPDATE_ITEM(bumper->m_d.m_skirtVisible, PropertyDialog::GetCheckboxState(m_hSkirtVisibleCheck), bumper);
                break;
            case 902:
                CHECK_UPDATE_ITEM(bumper->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), bumper);
                break;
            case 903:
                CHECK_UPDATE_ITEM(bumper->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), bumper);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT(bumper->m_d.m_szSurface, m_surfaceCombo, bumper);
                break;

            default:
                UpdateBaseProperties(bumper, &bumper->m_d, dispid);
                break;
        }
        bumper->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL BumperVisualsProperty::OnInitDialog()
{
    m_capMaterialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_bumpBaseMaterialCombo.AttachItem(IDC_MATERIAL_COMBO2);
    m_skirtMaterialCombo.AttachItem(IDC_MATERIAL_COMBO3);
    m_ringMaterialCombo.AttachItem(IDC_MATERIAL_COMBO4);
    m_radiusEdit.AttachItem(IDC_BUMPER_RADIUS_EDIT);
    m_heightScaleEdit.AttachItem(IDC_BUMPER_HEIGHT_SCALE_EDIT);
    m_orientationEdit.AttachItem(IDC_ORIENTATION_EDIT);
    m_ringSpeedEdit.AttachItem(IDC_RINGSPEED_EDIT);
    m_ringDropOffsetEdit.AttachItem(IDC_RINGDROPOFFSET_EDIT);
    m_hCapVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_CAP_VISIBLE_CHECK);
    m_hBaseVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_BASE_VISIBLE_CHECK);
    m_hRingVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_RING_VISIBLE);
    m_hSkirtVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_SKIRT_VISIBLE);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_posXEdit.AttachItem(902);
    m_posYEdit.AttachItem(903);
    m_surfaceCombo.AttachItem(1502);
    UpdateVisuals();
    return TRUE;
}
