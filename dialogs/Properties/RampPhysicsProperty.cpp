#include "stdafx.h"
#include "Properties/RampPhysicsProperty.h"
#include <WindowsX.h>

RampPhysicsProperty::RampPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRAMP_PHYSICS, pvsel)
{
    m_hitThresholdEdit.SetDialog(this);
    m_leftWallEdit.SetDialog(this);
    m_rightWallEdit.SetDialog(this);
    m_elasticityEdit.SetDialog(this);
    m_frictionEdit.SetDialog(this);
    m_scatterAngleEdit.SetDialog(this);
    m_physicsMaterialCombo.SetDialog(this);
}

void RampPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);

        if (dispid == 10 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_leftWallEdit, ramp->m_d.m_leftwallheight);
        if (dispid == 11 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rightWallEdit, ramp->m_d.m_rightwallheight);

        if (dispid == IDC_COLLIDABLE_CHECK || dispid == -1)
        {
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
        }
        UpdateBaseVisuals(ramp, &ramp->m_d, dispid);
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
                CHECK_UPDATE_ITEM(ramp->m_d.m_leftwallheight, PropertyDialog::GetFloatTextbox(m_leftWallEdit), ramp);
                break;
            case 11:
                CHECK_UPDATE_ITEM(ramp->m_d.m_rightwallheight, PropertyDialog::GetFloatTextbox(m_rightWallEdit), ramp);
                break;
            default:
                UpdateBaseProperties(ramp, &ramp->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL RampPhysicsProperty::OnInitDialog()
{
    m_hitThresholdEdit.AttachItem(IDC_HIT_THRESHOLD_EDIT);
    m_leftWallEdit.AttachItem(10);
    m_rightWallEdit.AttachItem(11);
    AttachItem(IDC_MATERIAL_COMBO4, m_physicsMaterialCombo);
    m_elasticityEdit.AttachItem(IDC_ELASTICITY_EDIT);
    m_frictionEdit.AttachItem(IDC_FRICTION_EDIT);
    m_scatterAngleEdit.AttachItem(IDC_SCATTER_ANGLE_EDIT);

    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    m_baseHitThresholdEdit = &m_hitThresholdEdit;
    m_baseFrictionEdit = &m_frictionEdit;
    m_baseElasticityEdit = &m_elasticityEdit;
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    UpdateVisuals();
    return TRUE;
}

