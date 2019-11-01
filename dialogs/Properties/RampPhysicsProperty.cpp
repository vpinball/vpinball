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
        Ramp *ramp = (Ramp *)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_hitThresholdEdit, ramp->m_d.m_threshold);
        PropertyDialog::SetFloatTextbox(m_leftWallEdit, ramp->m_d.m_leftwallheight);
        PropertyDialog::SetFloatTextbox(m_rightWallEdit, ramp->m_d.m_rightwallheight);
        PropertyDialog::SetFloatTextbox(m_elasticityEdit, ramp->m_d.m_elasticity);
        PropertyDialog::SetFloatTextbox(m_frictionEdit, ramp->m_d.m_friction);
        PropertyDialog::SetFloatTextbox(m_scatterAngleEdit, ramp->m_d.m_scatter);
        PropertyDialog::UpdateMaterialComboBox(ramp->GetPTable()->GetMaterialList(), m_physicsMaterialCombo, ramp->m_d.m_szPhysicsMaterial);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS), ramp->m_d.m_overwritePhysics);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK), ramp->m_d.m_hitEvent);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK), ramp->m_d.m_collidable);

        if (!ramp->m_d.m_collidable)
        {
            ::EnableWindow(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS), FALSE);
            ::EnableWindow(::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK), FALSE);
            m_hitThresholdEdit.EnableWindow(FALSE);
            m_physicsMaterialCombo.EnableWindow(FALSE);
            m_elasticityEdit.EnableWindow(FALSE);
            m_frictionEdit.EnableWindow(FALSE);
            m_scatterAngleEdit.EnableWindow(FALSE);
            m_leftWallEdit.EnableWindow(FALSE);
            m_rightWallEdit.EnableWindow(FALSE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS), TRUE);
            ::EnableWindow(::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK), TRUE);
            m_leftWallEdit.EnableWindow(TRUE);
            m_rightWallEdit.EnableWindow(TRUE);
            if (ramp->m_d.m_hitEvent)
                m_hitThresholdEdit.EnableWindow(TRUE);
            else
                m_hitThresholdEdit.EnableWindow(FALSE);

            if (!ramp->m_d.m_overwritePhysics)
            {
                m_physicsMaterialCombo.EnableWindow(TRUE);
                m_elasticityEdit.EnableWindow(FALSE);
                m_frictionEdit.EnableWindow(FALSE);
                m_scatterAngleEdit.EnableWindow(FALSE);
            }
            else
            {
                m_physicsMaterialCombo.EnableWindow(FALSE);
                m_elasticityEdit.EnableWindow(TRUE);
                m_frictionEdit.EnableWindow(TRUE);
                m_scatterAngleEdit.EnableWindow(TRUE);
            }
        }
    }
}

void RampPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp *ramp = (Ramp *)m_pvsel->ElementAt(i);
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
            case IDC_HIT_THRESHOLD_EDIT:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_threshold = PropertyDialog::GetFloatTextbox(m_hitThresholdEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_HAS_HITEVENT_CHECK:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_hitEvent = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(ramp);
            case IDC_ELASTICITY_EDIT:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_elasticity = PropertyDialog::GetFloatTextbox(m_elasticityEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_COLLIDABLE_CHECK:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_collidable = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_FRICTION_EDIT:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_friction = PropertyDialog::GetFloatTextbox(m_frictionEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_SCATTER_ANGLE_EDIT:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_scatter = PropertyDialog::GetFloatTextbox(m_scatterAngleEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_MATERIAL_COMBO4:
                PropertyDialog::StartUndo(ramp);
                PropertyDialog::GetComboBoxText(m_physicsMaterialCombo, ramp->m_d.m_szPhysicsMaterial);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_OVERWRITE_MATERIAL_SETTINGS:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_overwritePhysics = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS));
                PropertyDialog::EndUndo(ramp);
                break;
            default:
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
    UpdateVisuals();
    return TRUE;
}

BOOL RampPhysicsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
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


