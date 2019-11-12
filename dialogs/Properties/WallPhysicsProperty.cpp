#include "stdafx.h"
#include "Properties/WallPhysicsProperty.h"
#include <WindowsX.h>

WallPhysicsProperty::WallPhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPWALL_PHYSICS, pvsel)
{
}

void WallPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        Surface * const wall = (Surface*)m_pvsel->ElementAt(i);
        PropertyDialog::SetFloatTextbox(m_slingshotForceEdit, wall->GetSlingshotStrength());
        PropertyDialog::SetFloatTextbox(m_slingshotThresholdEdit, wall->m_d.m_slingshot_threshold);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 11), wall->m_d.m_droppable);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 116), wall->m_d.m_isBottomSolid);

        if (!wall->m_d.m_collidable)
        {
            ::EnableWindow(::GetDlgItem(GetHwnd(), 116), FALSE);
            m_slingshotForceEdit.EnableWindow(FALSE);
            m_slingshotThresholdEdit.EnableWindow(FALSE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(GetHwnd(), 116), TRUE);
        }
        UpdateBaseVisuals(wall, &wall->m_d);
        //only show the first element on multi-select
        break;
    }
}

void WallPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        Surface * const wall = (Surface*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 11:
                PropertyDialog::StartUndo(wall);
                wall->m_d.m_droppable = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(wall);
                break;
            case 14:
                PropertyDialog::StartUndo(wall);
                wall->SetSlingshotStrength(PropertyDialog::GetFloatTextbox(m_slingshotForceEdit));
                PropertyDialog::EndUndo(wall);
                break;
            case 427:
                PropertyDialog::StartUndo(wall);
                wall->m_d.m_slingshot_threshold = PropertyDialog::GetFloatTextbox(m_slingshotThresholdEdit);
                PropertyDialog::EndUndo(wall);
                break;
            case 116:
                PropertyDialog::StartUndo(wall);
                wall->m_d.m_isBottomSolid = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(wall);
                break;
            default:
                UpdateBaseProperties(wall, &wall->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL WallPhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_HIT_THRESHOLD_EDIT, m_hitThresholdEdit);
    AttachItem(14, m_slingshotForceEdit);
    AttachItem(427, m_slingshotThresholdEdit);
    AttachItem(IDC_MATERIAL_COMBO4, m_physicsMaterialCombo);
    AttachItem(IDC_ELASTICITY_EDIT, m_elasticityEdit);
    AttachItem(IDC_FRICTION_EDIT, m_frictionEdit);
    AttachItem(IDC_SCATTER_ANGLE_EDIT, m_scatterAngleEdit);

    m_baseHitThresholdEdit = &m_hitThresholdEdit;
    m_baseElasticityEdit = &m_elasticityEdit;
    m_baseFrictionEdit = &m_frictionEdit;
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    m_hCollidableCheck= ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    
    UpdateVisuals();
    return TRUE;
}

