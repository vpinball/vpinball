#include "stdafx.h"
#include "Properties/RubberPhysicsProperty.h"
#include <WindowsX.h>

RubberPhysicsProperty::RubberPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRUBBER_PHYSICS, pvsel)
{
    m_elasticityEdit.SetDialog(this);
    m_elasticityFallOffEdit.SetDialog(this);
    m_frictionEdit.SetDialog(this);
    m_scatterAngleEdit.SetDialog(this);
    m_hitHeightEdit.SetDialog(this);
    m_physicsMaterialCombo.SetDialog(this);
}

void RubberPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRubber))
            continue;
        Rubber *const rubber = (Rubber *)m_pvsel->ElementAt(i);

        if (dispid == 120 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_elasticityFallOffEdit, rubber->m_d.m_elasticityFalloff);
        if (dispid == IDC_HIT_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_hitHeightEdit, rubber->m_d.m_hitHeight);

        UpdateBaseVisuals(rubber, &rubber->m_d, dispid);

        if (dispid == IDC_COLLIDABLE_CHECK || dispid == -1)
        {
            m_elasticityFallOffEdit.EnableWindow(rubber->m_d.m_collidable);
            if (rubber->m_d.m_collidable)
                m_elasticityFallOffEdit.EnableWindow(rubber->m_d.m_overwritePhysics);
        }
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
                CHECK_UPDATE_ITEM(rubber->m_d.m_elasticityFalloff, PropertyDialog::GetFloatTextbox(m_elasticityFallOffEdit), rubber);
                break;
            case IDC_HIT_HEIGHT_EDIT:
                CHECK_UPDATE_ITEM(rubber->m_d.m_hitHeight, PropertyDialog::GetFloatTextbox(m_hitHeightEdit), rubber);
                break;
            default:
                UpdateBaseProperties(rubber, &rubber->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL RubberPhysicsProperty::OnInitDialog()
{
    m_physicsMaterialCombo.AttachItem(IDC_MATERIAL_COMBO4);
    m_basePhysicsMaterialCombo = &m_physicsMaterialCombo;
    m_hOverwritePhysicsCheck = ::GetDlgItem(GetHwnd(), IDC_OVERWRITE_MATERIAL_SETTINGS);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    m_elasticityEdit.AttachItem(IDC_ELASTICITY_EDIT);
    m_baseElasticityEdit = &m_elasticityEdit;

    m_elasticityFallOffEdit.AttachItem(120);
    m_frictionEdit.AttachItem(IDC_FRICTION_EDIT);
    m_baseFrictionEdit = &m_frictionEdit;

    m_scatterAngleEdit.AttachItem(IDC_SCATTER_ANGLE_EDIT);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;

    m_hitHeightEdit.AttachItem(IDC_HIT_HEIGHT_EDIT);
    UpdateVisuals();
    return TRUE;
}
