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
        Plunger *plunger = (Plunger *)m_pvsel->ElementAt(i);

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
        Plunger *plunger = (Plunger *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_PULL_SPEED_EDIT:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_speedPull = PropertyDialog::GetFloatTextbox(m_pullSpeedEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_RELEASE_SPEED_EDIT:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_speedFire = PropertyDialog::GetFloatTextbox(m_releaseSpeedEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_STROKE_LENGTH_EDIT:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_stroke = PropertyDialog::GetFloatTextbox(m_strokeLengthEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_SCATTER_VELOCITY_EDIT:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_scatterVelocity = PropertyDialog::GetFloatTextbox(m_scatterVelocityEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_MECH_STRENGTH_EDIT:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_mechStrength = PropertyDialog::GetFloatTextbox(m_mechStrengthEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_MOMENTUM_XFER_EDIT:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_momentumXfer = PropertyDialog::GetFloatTextbox(m_momentumXferEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_PARK_POSITION_EDIT:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_parkPosition = PropertyDialog::GetFloatTextbox(m_parkPositionEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_AUTO_PLUNGER_CHECK:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_autoPlunger = PropertyDialog::GetCheckboxState(m_hAutoPlungerCheck);
                PropertyDialog::EndUndo(plunger);
                break;
            case IDC_ENABLE_MECH_PLUNGER_CHECK:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_mechPlunger = PropertyDialog::GetCheckboxState(m_hEnableMechPlungerCheck);
                PropertyDialog::EndUndo(plunger);
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

BOOL PlungerPhysicsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
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

