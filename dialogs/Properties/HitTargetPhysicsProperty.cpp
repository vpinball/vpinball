#include "stdafx.h"
#include "Properties/HitTargetPhysicsProperty.h"
#include <WindowsX.h>

HitTargetPhysicsProperty::HitTargetPhysicsProperty(VectorProtected<ISelect> *pvsel): BasePropertyDialog(IDD_PROPHITTARGET_PHYSICS, pvsel)
{
}

void HitTargetPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
            continue;
        HitTarget * const target = (HitTarget*)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(m_hIsDroppedCheck, target->m_d.m_isDropped);
        PropertyDialog::SetCheckboxState(m_hLegacyModeCheck, target->m_d.m_legacy);
        PropertyDialog::SetFloatTextbox(m_elasticityFalloffEdit, target->m_d.m_elasticityFalloff);
        UpdateBaseVisuals(target, &target->m_d);
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
                PropertyDialog::StartUndo(target);
                target->m_d.m_isDropped = PropertyDialog::GetCheckboxState(m_hIsDroppedCheck);
                PropertyDialog::EndUndo(target);
                break;
            case IDC_TARGET_LEGACY_MODE_CHECK:
                PropertyDialog::StartUndo(target);
                target->m_d.m_legacy = PropertyDialog::GetCheckboxState(m_hLegacyModeCheck);
                PropertyDialog::EndUndo(target);
                break;
            case 112:
                PropertyDialog::StartUndo(target);
                target->m_d.m_elasticityFalloff = PropertyDialog::GetFloatTextbox(m_elasticityFalloffEdit);
                PropertyDialog::EndUndo(target);
                break;
            default:
                UpdateBaseProperties(target, &target->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL HitTargetPhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_HIT_THRESHOLD_EDIT, m_hitThresholdEdit);
    m_baseHitThresholdEdit = &m_hitThresholdEdit;
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    m_hLegacyModeCheck = ::GetDlgItem(GetHwnd(), IDC_TARGET_LEGACY_MODE_CHECK);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hIsDroppedCheck = ::GetDlgItem(GetHwnd(), IDC_TARGET_ISDROPPED_CHECK);
    AttachItem(IDC_MATERIAL_COMBO4, m_physicsMaterialCombo);
    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    AttachItem(IDC_ELASTICITY_EDIT, m_elasticityEdit);
    m_baseElasticityEdit = &m_elasticityEdit;
    AttachItem(IDC_FRICTION_EDIT, m_frictionEdit);
    m_baseFrictionEdit = &m_frictionEdit;
    AttachItem(IDC_SCATTER_ANGLE_EDIT, m_scatterAngleEdit);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    AttachItem(112, m_elasticityFalloffEdit);
    UpdateVisuals();
    return TRUE;
}
