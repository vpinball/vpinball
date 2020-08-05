#include "stdafx.h"
#include "Properties/HitTargetPhysicsProperty.h"
#include <WindowsX.h>

HitTargetPhysicsProperty::HitTargetPhysicsProperty(const VectorProtected<ISelect> *pvsel): BasePropertyDialog(IDD_PROPHITTARGET_PHYSICS, pvsel)
{
    m_hitThresholdEdit.SetDialog(this);
    m_elasticityEdit.SetDialog(this);
    m_elasticityFalloffEdit.SetDialog(this);
    m_frictionEdit.SetDialog(this);
    m_scatterAngleEdit.SetDialog(this);
    m_physicsMaterialCombo.SetDialog(this);
}

void HitTargetPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
            continue;
        HitTarget * const target = (HitTarget*)m_pvsel->ElementAt(i);

        if (dispid == IDC_TARGET_ISDROPPED_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hIsDroppedCheck, target->m_d.m_isDropped);
        if (dispid == IDC_TARGET_LEGACY_MODE_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hLegacyModeCheck, target->m_d.m_legacy);
        if (dispid == 112 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_elasticityFalloffEdit, target->m_d.m_elasticityFalloff);
        UpdateBaseVisuals(target, &target->m_d, dispid);
        if (dispid == IDC_COLLIDABLE_CHECK || dispid == -1)
            m_elasticityFalloffEdit.EnableWindow(target->m_d.m_collidable);
        if (target->m_d.m_collidable)
            m_elasticityFalloffEdit.EnableWindow(target->m_d.m_overwritePhysics);

        //only show the first element on multi-select
        break;
    }
}

void HitTargetPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
            continue;
        HitTarget * const target = (HitTarget*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_TARGET_ISDROPPED_CHECK:
                CHECK_UPDATE_ITEM(target->m_d.m_isDropped, PropertyDialog::GetCheckboxState(m_hIsDroppedCheck), target);
                break;
            case IDC_TARGET_LEGACY_MODE_CHECK:
                CHECK_UPDATE_ITEM(target->m_d.m_legacy, PropertyDialog::GetCheckboxState(m_hLegacyModeCheck), target);
                break;
            case 112:
                CHECK_UPDATE_ITEM(target->m_d.m_elasticityFalloff, PropertyDialog::GetFloatTextbox(m_elasticityFalloffEdit), target);
                break;
            default:
                UpdateBaseProperties(target, &target->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL HitTargetPhysicsProperty::OnInitDialog()
{
    m_hitThresholdEdit.AttachItem(IDC_HIT_THRESHOLD_EDIT);
    m_baseHitThresholdEdit = &m_hitThresholdEdit;

    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    m_hLegacyModeCheck = ::GetDlgItem(GetHwnd(), IDC_TARGET_LEGACY_MODE_CHECK);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hIsDroppedCheck = ::GetDlgItem(GetHwnd(), IDC_TARGET_ISDROPPED_CHECK);
    m_physicsMaterialCombo.AttachItem(IDC_MATERIAL_COMBO4);
    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    m_elasticityEdit.AttachItem(IDC_ELASTICITY_EDIT);
    m_baseElasticityEdit = &m_elasticityEdit;

    m_frictionEdit.AttachItem(IDC_FRICTION_EDIT);
    m_baseFrictionEdit = &m_frictionEdit;

    m_scatterAngleEdit.AttachItem(IDC_SCATTER_ANGLE_EDIT);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    m_elasticityFalloffEdit.AttachItem(112);
    UpdateVisuals();
    return TRUE;
}
