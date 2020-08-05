#include "stdafx.h"
#include "Properties/PrimitivePhysicsProperty.h"
#include <WindowsX.h>

PrimitivePhysicsProperty::PrimitivePhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPRIMITIVE_PHYSICS, pvsel)
{
    m_reducePolyEdit.SetDialog(this);
    m_elasticityFalloffEdit.SetDialog(this);
    m_hitThresholdEdit.SetDialog(this);
    m_elasticityEdit.SetDialog(this);
    m_frictionEdit.SetDialog(this);
    m_scatterAngleEdit.SetDialog(this);
    m_physicsMaterialCombo.SetDialog(this);
}

void PrimitivePhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive *)m_pvsel->ElementAt(i);

        if (dispid == IDC_PRIMITIVE_IS_TOY || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hToyCheck, prim->m_d.m_toy);
        if (dispid == IDC_ELASTICITY_FALLOFF_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_elasticityFalloffEdit, prim->m_d.m_elasticityFalloff);
        if (dispid == IDC_COLLISION_REDUCTION_FACTOR || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_reducePolyEdit, prim->m_d.m_collision_reductionFactor);

        UpdateBaseVisuals(prim, &prim->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void PrimitivePhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_ELASTICITY_FALLOFF_EDIT:
                CHECK_UPDATE_ITEM(prim->m_d.m_elasticityFalloff, PropertyDialog::GetFloatTextbox(m_elasticityFalloffEdit), prim);
                break;
            case IDC_COLLISION_REDUCTION_FACTOR:
                CHECK_UPDATE_ITEM(prim->m_d.m_collision_reductionFactor, PropertyDialog::GetFloatTextbox(m_reducePolyEdit), prim);
                break;
            case IDC_PRIMITIVE_IS_TOY:
                CHECK_UPDATE_ITEM(prim->m_d.m_toy, PropertyDialog::GetCheckboxState(m_hToyCheck), prim);
                break;
            default:
                UpdateBaseProperties(prim, &prim->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL PrimitivePhysicsProperty::OnInitDialog()
{
    m_hitThresholdEdit.AttachItem(IDC_HIT_THRESHOLD_EDIT);
    m_baseHitThresholdEdit = &m_hitThresholdEdit;
    m_elasticityEdit.AttachItem(IDC_ELASTICITY_EDIT);
    m_baseElasticityEdit = &m_elasticityEdit;
    m_frictionEdit.AttachItem(IDC_FRICTION_EDIT);
    m_baseFrictionEdit = &m_frictionEdit;

    m_elasticityFalloffEdit.AttachItem(IDC_ELASTICITY_FALLOFF_EDIT);
    m_physicsMaterialCombo.AttachItem(IDC_MATERIAL_COMBO4);
    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    m_scatterAngleEdit.AttachItem(IDC_SCATTER_ANGLE_EDIT);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;

    m_hToyCheck= ::GetDlgItem(GetHwnd(), IDC_PRIMITIVE_IS_TOY);
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    m_reducePolyEdit.AttachItem(IDC_COLLISION_REDUCTION_FACTOR);
    UpdateVisuals();
    return TRUE;
}
