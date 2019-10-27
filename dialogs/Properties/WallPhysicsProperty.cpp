#include "stdafx.h"
#include "Properties/WallPhysicsProperty.h"
#include <WindowsX.h>

WallPhysicsProperty::WallPhysicsProperty(Surface *wall) : CDialog(IDD_PROPWALL_PHYSICS), m_wall(wall)
{
}

void WallPhysicsProperty::UpdateVisuals()
{
    PropertyDialog::UpdateMaterialComboBox(m_wall->GetPTable()->GetMaterialList(), m_physicsMaterialCombo, m_wall->m_d.m_szPhysicsMaterial);

    PropertyDialog::SetFloatTextbox(m_hitThresholdEdit, m_wall->m_d.m_threshold);
    PropertyDialog::SetFloatTextbox(m_slingshotForceEdit, m_wall->m_d.m_slingshotforce);
    PropertyDialog::SetFloatTextbox(m_slingshotThresholdEdit, m_wall->m_d.m_slingshot_threshold);
    PropertyDialog::SetFloatTextbox(m_elasticityEdit, m_wall->m_d.m_elasticity);
    PropertyDialog::SetFloatTextbox(m_frictionEdit, m_wall->m_d.m_friction);
    PropertyDialog::SetFloatTextbox(m_scatterAngleEdit, m_wall->m_d.m_scatter);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 3), m_wall->m_d.m_hitEvent);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS), m_wall->m_d.m_overwritePhysics);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 11), m_wall->m_d.m_droppable);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 110), m_wall->m_d.m_collidable);
    PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 116), m_wall->m_d.m_isBottomSolid);

    if (!m_wall->m_d.m_collidable)
    {
        ::EnableWindow(::GetDlgItem(GetHwnd(), 116), FALSE);
        ::EnableWindow(::GetDlgItem(GetHwnd(), 3), FALSE);
        ::EnableWindow(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS), FALSE);
        m_hitThresholdEdit.EnableWindow(FALSE);
        m_physicsMaterialCombo.EnableWindow(FALSE);
        m_elasticityEdit.EnableWindow(FALSE);
        m_frictionEdit.EnableWindow(FALSE);
        m_scatterAngleEdit.EnableWindow(FALSE);
        m_slingshotForceEdit.EnableWindow(FALSE);
        m_slingshotThresholdEdit.EnableWindow(FALSE);
    }
    else
    {
        ::EnableWindow(::GetDlgItem(GetHwnd(), 116), TRUE);
        ::EnableWindow(::GetDlgItem(GetHwnd(), 3), TRUE);
        ::EnableWindow(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS), TRUE);
        m_slingshotForceEdit.EnableWindow(TRUE);
        m_slingshotThresholdEdit.EnableWindow(TRUE);
        if (m_wall->m_d.m_hitEvent)
            m_hitThresholdEdit.EnableWindow(TRUE);
        else
            m_hitThresholdEdit.EnableWindow(FALSE);

        if (!m_wall->m_d.m_overwritePhysics)
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

void WallPhysicsProperty::UpdateProperties(const int dispid)
{
    switch (dispid)
    {
        case 4:
            m_wall->m_d.m_threshold = PropertyDialog::GetFloatTextbox(m_hitThresholdEdit);
            break;
        case 3:
            m_wall->m_d.m_hitEvent = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 3));
            break;
        case 11:
            m_wall->m_d.m_droppable = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 11));
            break;
        case 14:
            m_wall->m_d.m_slingshotforce = PropertyDialog::GetFloatTextbox(m_slingshotForceEdit);
            break;
        case 15:
            m_wall->m_d.m_elasticity = PropertyDialog::GetFloatTextbox(m_elasticityEdit);
            break;
        case 110:
            m_wall->m_d.m_collidable = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 110));
            break;
        case 111:
            m_wall->m_d.m_slingshot_threshold = PropertyDialog::GetFloatTextbox(m_slingshotThresholdEdit);
            break;
        case 114:
            m_wall->m_d.m_friction = PropertyDialog::GetFloatTextbox(m_frictionEdit);
            break;
        case 115:
            m_wall->m_d.m_scatter = PropertyDialog::GetFloatTextbox(m_scatterAngleEdit);
            break;
        case 116:
            m_wall->m_d.m_isBottomSolid = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 116));
            break;
        case IDC_OVERWRITE_MATERIAL_SETTINGS:
            m_wall->m_d.m_overwritePhysics = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS));
            break;
        default:
            break;
    }
    UpdateVisuals();
}

BOOL WallPhysicsProperty::OnInitDialog()
{
    AttachItem(4, m_hitThresholdEdit);
    AttachItem(14, m_slingshotForceEdit);
    AttachItem(111, m_slingshotThresholdEdit);
    AttachItem(IDC_MATERIAL_COMBO4, m_physicsMaterialCombo);
    AttachItem(15, m_elasticityEdit);
    AttachItem(114, m_frictionEdit);
    AttachItem(115, m_scatterAngleEdit);
    UpdateVisuals();
    return TRUE;
}

BOOL WallPhysicsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int dispID = LOWORD(wParam);

    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
        case BN_CLICKED:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}

