#include "stdafx.h"
#include "Properties/GatePhysicsProperty.h"
#include <WindowsX.h>

GatePhysicsProperty::GatePhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPGATE_PHYSICS, pvsel)
{
    m_elasticityEdit.SetDialog(this);
    m_frictionEdit.SetDialog(this);
    m_dampingEdit.SetDialog(this);
    m_gravityFactorEdit.SetDialog(this);
}

void GatePhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemGate))
            continue;
        Gate * const gate = (Gate *)m_pvsel->ElementAt(i);

        if (dispid == 13 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_dampingEdit, gate->m_d.m_damping);
        if (dispid == 17 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_gravityFactorEdit, gate->m_d.m_gravityfactor);
        if (dispid == IDC_TWO_WAY_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_TWO_WAY_CHECK), gate->m_d.m_twoWay);
        UpdateBaseVisuals(gate, &gate->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void GatePhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemGate))
            continue;
        Gate * const gate = (Gate *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 13:
                CHECK_UPDATE_ITEM(gate->m_d.m_damping, PropertyDialog::GetFloatTextbox(m_dampingEdit), gate);
                break;
            case 17:
                CHECK_UPDATE_ITEM(gate->m_d.m_gravityfactor, PropertyDialog::GetFloatTextbox(m_gravityFactorEdit), gate);
                break;
            case IDC_TWO_WAY_CHECK:
                CHECK_UPDATE_ITEM(gate->m_d.m_twoWay, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_TWO_WAY_CHECK)), gate);
                break;

            default:
                UpdateBaseProperties(gate, &gate->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL GatePhysicsProperty::OnInitDialog()
{
    m_dampingEdit.AttachItem(13);
    m_gravityFactorEdit.AttachItem(17);
    m_elasticityEdit.AttachItem(IDC_ELASTICITY_EDIT);
    m_frictionEdit.AttachItem(IDC_FRICTION_EDIT);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_baseElasticityEdit = &m_elasticityEdit;
    m_baseFrictionEdit = &m_frictionEdit;
    UpdateVisuals();
    return TRUE;
}

