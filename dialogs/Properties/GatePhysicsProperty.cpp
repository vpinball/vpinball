#include "stdafx.h"
#include "Properties/GatePhysicsProperty.h"
#include <WindowsX.h>

GatePhysicsProperty::GatePhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPGATE_PHYSICS, pvsel)
{
}

void GatePhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemGate))
            continue;
        Gate *gate = (Gate *)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_elasticityEdit, gate->m_d.m_elasticity);
        PropertyDialog::SetFloatTextbox(m_frictionEdit, gate->m_d.m_friction);
        PropertyDialog::SetFloatTextbox(m_dampingEdit, gate->m_d.m_damping);
        PropertyDialog::SetFloatTextbox(m_gravityFactorEdit, gate->m_d.m_gravityfactor);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK), gate->m_d.m_collidable);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_TWO_WAY_CHECK), gate->m_d.m_twoWay);
    }
}

void GatePhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemGate))
            continue;
        Gate *gate = (Gate *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_ELASTICITY_EDIT:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_elasticity = PropertyDialog::GetFloatTextbox(m_elasticityEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case IDC_FRICTION_EDIT:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_friction = PropertyDialog::GetFloatTextbox(m_frictionEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case 13:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_damping = PropertyDialog::GetFloatTextbox(m_dampingEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case 17:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_gravityfactor = PropertyDialog::GetFloatTextbox(m_gravityFactorEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case IDC_COLLIDABLE_CHECK:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_collidable = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(gate);
                break;
            case IDC_TWO_WAY_CHECK:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_twoWay = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_TWO_WAY_CHECK));
                PropertyDialog::EndUndo(gate);
                break;

            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL GatePhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_ELASTICITY_EDIT, m_elasticityEdit);
    AttachItem(IDC_FRICTION_EDIT, m_frictionEdit);
    AttachItem(13, m_dampingEdit);
    AttachItem(17, m_gravityFactorEdit);
    UpdateVisuals();
    return TRUE;
}

BOOL GatePhysicsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
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

