#include "stdafx.h"
#include "Properties/PlungerPhysicsProperty.h"
#include <WindowsX.h>

PlungerPhysicsProperty::PlungerPhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPLUNGER_PHYSICS, pvsel)
{
}

void PlungerPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);
        PropertyDialog::SetFloatTextbox(m_pullSpeedEdit, plunger->m_d.m_speedPull);
        PropertyDialog::SetFloatTextbox(m_releaseSpeedEdit, plunger->m_d.m_speedFire);
        PropertyDialog::SetFloatTextbox(m_strokeLengthEdit, plunger->m_d.m_stroke);
        PropertyDialog::SetFloatTextbox(m_scatterVelocityEdit, plunger->m_d.m_scatterVelocity);
        PropertyDialog::SetCheckboxState(m_hAutoPlungerCheck, plunger->m_d.m_autoPlunger);
        PropertyDialog::SetCheckboxState(m_hEnableMechPlungerCheck, plunger->m_d.m_mechPlunger);
        PropertyDialog::SetFloatTextbox(m_mechStrengthEdit, plunger->m_d.m_mechStrength);
        PropertyDialog::SetFloatTextbox(m_momentumXferEdit, plunger->m_d.m_momentumXfer);
        PropertyDialog::SetFloatTextbox(m_parkPositionEdit, plunger->m_d.m_parkPosition);
        UpdateBaseVisuals(plunger, &plunger->m_d);
        //only show the first element on multi-select
        break;
    }
}

void PlungerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_PULL_SPEED_EDIT:
                CHECK_UPDATE_ITEM(plunger->m_d.m_speedPull, PropertyDialog::GetFloatTextbox(m_pullSpeedEdit), plunger);
                break;
            case IDC_RELEASE_SPEED_EDIT:
                CHECK_UPDATE_ITEM(plunger->m_d.m_speedFire, PropertyDialog::GetFloatTextbox(m_releaseSpeedEdit), plunger);
                break;
            case IDC_STROKE_LENGTH_EDIT:
                CHECK_UPDATE_ITEM(plunger->m_d.m_stroke, PropertyDialog::GetFloatTextbox(m_strokeLengthEdit), plunger);
                break;
            case IDC_SCATTER_VELOCITY_EDIT:
                CHECK_UPDATE_ITEM(plunger->m_d.m_scatterVelocity, PropertyDialog::GetFloatTextbox(m_scatterVelocityEdit), plunger);
                break;
            case IDC_MECH_STRENGTH_EDIT:
                CHECK_UPDATE_ITEM(plunger->m_d.m_mechStrength, PropertyDialog::GetFloatTextbox(m_mechStrengthEdit), plunger);
                break;
            case IDC_MOMENTUM_XFER_EDIT:
                CHECK_UPDATE_ITEM(plunger->m_d.m_momentumXfer, PropertyDialog::GetFloatTextbox(m_momentumXferEdit), plunger);
                break;
            case IDC_PARK_POSITION_EDIT:
                CHECK_UPDATE_ITEM(plunger->m_d.m_parkPosition, PropertyDialog::GetFloatTextbox(m_parkPositionEdit), plunger);
                break;
            case IDC_AUTO_PLUNGER_CHECK:
                CHECK_UPDATE_ITEM(plunger->m_d.m_autoPlunger, PropertyDialog::GetCheckboxState(m_hAutoPlungerCheck), plunger);
                break;
            case IDC_ENABLE_MECH_PLUNGER_CHECK:
                CHECK_UPDATE_ITEM(plunger->m_d.m_mechPlunger, PropertyDialog::GetCheckboxState(m_hEnableMechPlungerCheck), plunger);
                break;
            default:
                UpdateBaseProperties(plunger, &plunger->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL PlungerPhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_PULL_SPEED_EDIT, m_pullSpeedEdit);
    AttachItem(IDC_RELEASE_SPEED_EDIT, m_releaseSpeedEdit);
    AttachItem(IDC_STROKE_LENGTH_EDIT, m_strokeLengthEdit);
    AttachItem(IDC_SCATTER_VELOCITY_EDIT, m_scatterVelocityEdit);
    m_hAutoPlungerCheck = ::GetDlgItem(GetHwnd(), IDC_AUTO_PLUNGER_CHECK);
    m_hEnableMechPlungerCheck = ::GetDlgItem(GetHwnd(), IDC_ENABLE_MECH_PLUNGER_CHECK);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    AttachItem(IDC_MECH_STRENGTH_EDIT, m_mechStrengthEdit);
    AttachItem(IDC_MOMENTUM_XFER_EDIT, m_momentumXferEdit);
    AttachItem(IDC_PARK_POSITION_EDIT, m_parkPositionEdit);
    UpdateVisuals();
    return TRUE;
}
