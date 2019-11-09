#include "stdafx.h"
#include "Properties/RampPhysicsProperty.h"
#include <WindowsX.h>

RampPhysicsProperty::RampPhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRAMP_PHYSICS, pvsel)
{
}

void RampPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_leftWallEdit, ramp->m_d.m_leftwallheight);
        PropertyDialog::SetFloatTextbox(m_rightWallEdit, ramp->m_d.m_rightwallheight);

        if (!ramp->m_d.m_collidable)
        {
            m_leftWallEdit.EnableWindow(FALSE);
            m_rightWallEdit.EnableWindow(FALSE);
        }
        else
        {
            m_leftWallEdit.EnableWindow(TRUE);
            m_rightWallEdit.EnableWindow(TRUE);
        }
        UpdateBaseVisuals(ramp, &ramp->m_d);
        //only show the first element on multi-select
        break;
    }
}

void RampPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 10:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_leftwallheight = PropertyDialog::GetFloatTextbox(m_leftWallEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case 11:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_rightwallheight = PropertyDialog::GetFloatTextbox(m_rightWallEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            default:
                UpdateBaseProperties(ramp, &ramp->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL RampPhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_HIT_THRESHOLD_EDIT, m_hitThresholdEdit);
    AttachItem(10, m_leftWallEdit);
    AttachItem(11, m_rightWallEdit);
    AttachItem(IDC_MATERIAL_COMBO4, m_physicsMaterialCombo);
    AttachItem(IDC_ELASTICITY_EDIT, m_elasticityEdit);
    AttachItem(IDC_FRICTION_EDIT, m_frictionEdit);
    AttachItem(IDC_SCATTER_ANGLE_EDIT, m_scatterAngleEdit);

    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    m_baseHitThresholdEdit = &m_hitThresholdEdit;
    m_baseFrictionEdit = &m_frictionEdit;
    m_baseElasticityEdit = &m_elasticityEdit;
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    UpdateVisuals();
    return TRUE;
}

