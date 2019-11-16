#include "stdafx.h"
#include "Properties/PrimitivePhysicsProperty.h"
#include <WindowsX.h>

PrimitivePhysicsProperty::PrimitivePhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPRIMITIVE_PHYSICS, pvsel)
{
}

void PrimitivePhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive *)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(m_hToyCheck, prim->m_d.m_toy);
        PropertyDialog::SetFloatTextbox(m_elasticityFalloffEdit, prim->m_d.m_elasticityFalloff);
        PropertyDialog::SetFloatTextbox(m_recudePolyEdit, prim->m_d.m_collision_reductionFactor);
        UpdateBaseVisuals(prim, &prim->m_d);
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
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_elasticityFalloff= PropertyDialog::GetFloatTextbox(m_elasticityFalloffEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_COLLISION_REDUCTION_FACTOR:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_collision_reductionFactor = PropertyDialog::GetFloatTextbox(m_recudePolyEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_PRIMITIVE_IS_TOY:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_toy = PropertyDialog::GetCheckboxState(m_hToyCheck);
                PropertyDialog::EndUndo(prim);
                break;
            default:
                UpdateBaseProperties(prim, &prim->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL PrimitivePhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_HIT_THRESHOLD_EDIT, m_hitThresholdEdit);
    m_baseHitThresholdEdit = &m_hitThresholdEdit;
    AttachItem(IDC_ELASTICITY_EDIT, m_elasticityEdit);
    m_baseElasticityEdit = &m_elasticityEdit;
    AttachItem(IDC_FRICTION_EDIT, m_frictionEdit);
    m_baseFrictionEdit = &m_frictionEdit;
    AttachItem(IDC_ELASTICITY_FALLOFF_EDIT, m_elasticityFalloffEdit);
    AttachItem(IDC_MATERIAL_COMBO4, m_physicsMatrtialCombo);
    m_basePhysicsMaterialCombo = &m_physicsMatrtialCombo;
    AttachItem(IDC_SCATTER_ANGLE_EDIT, m_scatterAngleEdit);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    m_hToyCheck= ::GetDlgItem(GetHwnd(), IDC_PRIMITIVE_IS_TOY);
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    AttachItem(IDC_COLLISION_REDUCTION_FACTOR, m_recudePolyEdit);
    UpdateVisuals();
    return TRUE;
}
