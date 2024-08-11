// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/PrimitivePhysicsProperty.h"
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
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive *)m_pvsel->ElementAt(i);

        if (dispid == IDC_PRIMITIVE_IS_TOY || dispid == -1)
        {
           PropertyDialog::SetCheckboxState(m_hToyCheck, prim->m_d.m_toy);
           if(prim->m_d.m_toy)
           {
              prim->m_d.m_collidable=false;
              PropertyDialog::SetCheckboxState(m_hCollidableCheck, false);
              m_elasticityFalloffEdit.EnableWindow(false);
           }
           ::EnableWindow(m_hCollidableCheck, !prim->m_d.m_toy);
        }
        if (dispid == IDC_COLLIDABLE_CHECK || dispid == -1)
        {
           m_elasticityFalloffEdit.EnableWindow(prim->m_d.m_collidable);
        }
        if (dispid == IDC_ELASTICITY_FALLOFF_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_elasticityFalloffEdit, prim->m_d.m_elasticityFalloff);
        if (dispid == IDC_COLLISION_REDUCTION_FACTOR || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_reducePolyEdit, prim->m_d.m_collision_reductionFactor);

        UpdateBaseVisuals(prim, &prim->m_d, dispid);

        // Disable playfield settings that are taken from table settings to avoid confusing the user
        if (m_hCollidableCheck)
        {
            if (m_hHitEventCheck)           ::EnableWindow(m_hHitEventCheck, prim->m_d.m_collidable && !prim->IsPlayfield());
            if (m_hOverwritePhysicsCheck)   ::EnableWindow(m_hOverwritePhysicsCheck, prim->m_d.m_collidable && !prim->IsPlayfield());
            if (m_baseHitThresholdEdit)     m_baseHitThresholdEdit->EnableWindow(prim->m_d.m_collidable && !prim->IsPlayfield());
            if (m_basePhysicsMaterialCombo) m_basePhysicsMaterialCombo->EnableWindow(prim->m_d.m_collidable && !prim->IsPlayfield());
            if (m_baseElasticityEdit)       m_baseElasticityEdit->EnableWindow(prim->m_d.m_collidable && !prim->IsPlayfield());
            if (m_baseFrictionEdit)         m_baseFrictionEdit->EnableWindow(prim->m_d.m_collidable && !prim->IsPlayfield());
            if (m_baseScatterAngleEdit)     m_baseScatterAngleEdit->EnableWindow(prim->m_d.m_collidable && !prim->IsPlayfield());
            if (m_elasticityFalloffEdit)    m_elasticityFalloffEdit.EnableWindow(prim->m_d.m_collidable && !prim->IsPlayfield());
        }
        if (m_hHitEventCheck && prim->m_d.m_collidable)
            if (m_baseHitThresholdEdit)     m_baseHitThresholdEdit->EnableWindow(prim->m_d.m_hitEvent&& !prim->IsPlayfield());
        if (m_hOverwritePhysicsCheck && prim->m_d.m_collidable)
        {
            if (m_basePhysicsMaterialCombo) m_basePhysicsMaterialCombo->EnableWindow(!prim->m_d.m_overwritePhysics&& !prim->IsPlayfield());
            if (m_baseElasticityEdit)       m_baseElasticityEdit->EnableWindow(prim->m_d.m_overwritePhysics&& !prim->IsPlayfield());
            if (m_baseFrictionEdit)         m_baseFrictionEdit->EnableWindow(prim->m_d.m_overwritePhysics&& !prim->IsPlayfield());
            if (m_baseScatterAngleEdit)     m_baseScatterAngleEdit->EnableWindow(prim->m_d.m_overwritePhysics&& !prim->IsPlayfield());
        }

        //only show the first element on multi-select
        break;
    }
}

void PrimitivePhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
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

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(m_hitThresholdEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_elasticityEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_frictionEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_elasticityFalloffEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_physicsMaterialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_scatterAngleEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hToyCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hHitEventCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hCollidableCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hOverwritePhysicsCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_reducePolyEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR PrimitivePhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
