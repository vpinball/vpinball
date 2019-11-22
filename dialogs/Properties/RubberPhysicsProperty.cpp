#include "stdafx.h"
#include "Properties/RubberPhysicsProperty.h"
#include <WindowsX.h>

RubberPhysicsProperty::RubberPhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRUBBER_PHYSICS, pvsel)
{
}

void RubberPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRubber))
            continue;
        Rubber *const rubber = (Rubber *)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_elasticityFallOffEdit, rubber->m_d.m_elasticityFalloff);
        PropertyDialog::SetFloatTextbox(m_hitHeightEdit, rubber->m_d.m_hitHeight);
        UpdateBaseVisuals(rubber, &rubber->m_d);
        m_elasticityFallOffEdit.EnableWindow(rubber->m_d.m_collidable);
        if(rubber->m_d.m_collidable)
            m_elasticityFallOffEdit.EnableWindow(rubber->m_d.m_overwritePhysics);
        //only show the first element on multi-select
        break;
    }
}

void RubberPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRubber))
            continue;
        Rubber *const rubber = (Rubber *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 120:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_elasticityFalloff = PropertyDialog::GetFloatTextbox(m_elasticityFallOffEdit);
                PropertyDialog::EndUndo(rubber);
                break;
            case IDC_HIT_HEIGHT_EDIT:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_hitHeight = PropertyDialog::GetFloatTextbox(m_hitHeightEdit);
                PropertyDialog::EndUndo(rubber);
                break;
            default:
                UpdateBaseProperties(rubber, &rubber->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL RubberPhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_MATERIAL_COMBO4, m_physicsMaterialCombo);
    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    AttachItem(IDC_ELASTICITY_EDIT, m_elasticityEdit);
    m_baseElasticityEdit = &m_elasticityEdit;
    AttachItem(120, m_elasticityFallOffEdit);
    AttachItem(IDC_FRICTION_EDIT, m_frictionEdit);
    m_baseFrictionEdit = &m_frictionEdit;
    AttachItem(IDC_SCATTER_ANGLE_EDIT, m_scatterAngleEdit);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    AttachItem(IDC_HIT_HEIGHT_EDIT, m_hitHeightEdit);
    UpdateVisuals();
    return TRUE;
}
