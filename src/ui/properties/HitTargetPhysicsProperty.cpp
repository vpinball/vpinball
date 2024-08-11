// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/HitTargetPhysicsProperty.h"
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
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
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
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
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

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(m_hitThresholdEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hHitEventCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hOverwritePhysicsCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hLegacyModeCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hCollidableCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hIsDroppedCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_physicsMaterialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_elasticityEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_frictionEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_scatterAngleEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_elasticityFalloffEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR HitTargetPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
