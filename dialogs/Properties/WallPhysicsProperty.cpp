#include "stdafx.h"
#include "Properties/WallPhysicsProperty.h"
#include <WindowsX.h>

WallPhysicsProperty::WallPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPWALL_PHYSICS, pvsel)
{
    m_hitThresholdEdit.SetDialog(this);
    m_slingshotForceEdit.SetDialog(this);
    m_slingshotThresholdEdit.SetDialog(this);
    m_elasticityEdit.SetDialog(this);
    m_elasticityFallOffEdit.SetDialog(this);
    m_frictionEdit.SetDialog(this);
    m_scatterAngleEdit.SetDialog(this);
    m_physicsMaterialCombo.SetDialog(this);
}

void WallPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    //only show the first element on multi-select
    Surface * const wall = (Surface*)m_pvsel->ElementAt(0);
    if (dispid == 14 || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_slingshotForceEdit, wall->GetSlingshotStrength());
    if (dispid == 427 || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_slingshotThresholdEdit, wall->m_d.m_slingshot_threshold);
    if (dispid == 11 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 11), wall->m_d.m_droppable);
    if (dispid == 116 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 116), wall->m_d.m_isBottomSolid);
    if (dispid == 120 || dispid == -1)
       PropertyDialog::SetFloatTextbox(m_elasticityFallOffEdit, wall->m_d.m_elasticityFalloff);

    if (dispid == IDC_COLLIDABLE_CHECK || dispid == -1)
    {
        if (!wall->m_d.m_collidable)
        {
            ::EnableWindow(::GetDlgItem(GetHwnd(), 116), FALSE);
            m_slingshotForceEdit.EnableWindow(FALSE);
            m_slingshotThresholdEdit.EnableWindow(FALSE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(GetHwnd(), 116), TRUE);
            m_slingshotForceEdit.EnableWindow(TRUE);
            m_slingshotThresholdEdit.EnableWindow(TRUE);
        }
    }
    UpdateBaseVisuals(wall, &wall->m_d, dispid);
}

void WallPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSurface))
            continue;
        Surface * const wall = (Surface*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 11:
                CHECK_UPDATE_ITEM(wall->m_d.m_droppable, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 14:
                CHECK_UPDATE_VALUE_SETTER(wall->SetSlingshotStrength, wall->GetSlingshotStrength, PropertyDialog::GetFloatTextbox, m_slingshotForceEdit, wall);
                break;
            case 427:
                CHECK_UPDATE_ITEM(wall->m_d.m_slingshot_threshold, PropertyDialog::GetFloatTextbox(m_slingshotThresholdEdit), wall);
                break;
            case 116:
                PropertyDialog::StartUndo(wall);
                wall->m_d.m_isBottomSolid = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(wall);
                CHECK_UPDATE_ITEM(wall->m_d.m_isBottomSolid, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 120:
               CHECK_UPDATE_ITEM(wall->m_d.m_elasticityFalloff, PropertyDialog::GetFloatTextbox(m_elasticityFallOffEdit), wall);
               break;
            default:
                UpdateBaseProperties(wall, &wall->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL WallPhysicsProperty::OnInitDialog()
{
    m_hitThresholdEdit.AttachItem(IDC_HIT_THRESHOLD_EDIT);
    m_slingshotForceEdit.AttachItem(14);
    m_slingshotThresholdEdit.AttachItem(427);
    m_physicsMaterialCombo.AttachItem(IDC_MATERIAL_COMBO4);

    m_elasticityEdit.AttachItem(IDC_ELASTICITY_EDIT);
    m_frictionEdit.AttachItem(IDC_FRICTION_EDIT);
    m_scatterAngleEdit.AttachItem(IDC_SCATTER_ANGLE_EDIT);
    m_elasticityFallOffEdit.AttachItem(120);

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

