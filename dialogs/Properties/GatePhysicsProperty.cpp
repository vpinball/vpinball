#include "stdafx.h"
#include "Properties/GatePhysicsProperty.h"
#include <WindowsX.h>

GatePhysicsProperty::GatePhysicsProperty(VectorProtected<ISelect> *pvsel) : BaseProperty(IDD_PROPGATE_PHYSICS, pvsel)
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
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 2146), gate->m_d.m_collidable);
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
            case 11:
                gate->m_d.m_elasticity = PropertyDialog::GetFloatTextbox(m_elasticityEdit);
                break;
            case 12:
                gate->m_d.m_friction = PropertyDialog::GetFloatTextbox(m_frictionEdit);
                break;
            case 13:
                gate->m_d.m_damping = PropertyDialog::GetFloatTextbox(m_dampingEdit);
                break;
            case 17:
                gate->m_d.m_gravityfactor = PropertyDialog::GetFloatTextbox(m_gravityFactorEdit);
                break;
            case 2146:
                gate->m_d.m_collidable = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 2146));
                break;
            case IDC_TWO_WAY_CHECK:
                gate->m_d.m_twoWay = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_TWO_WAY_CHECK));
                break;

            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL GatePhysicsProperty::OnInitDialog()
{
    AttachItem(11, m_elasticityEdit);
    AttachItem(12, m_frictionEdit);
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

