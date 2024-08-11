// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/PlungerPhysicsProperty.h"
#include <WindowsX.h>

PlungerPhysicsProperty::PlungerPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPLUNGER_PHYSICS, pvsel)
{
    m_pullSpeedEdit.SetDialog(this);
    m_releaseSpeedEdit.SetDialog(this);
    m_strokeLengthEdit.SetDialog(this);
    m_scatterVelocityEdit.SetDialog(this);
    m_mechStrengthEdit.SetDialog(this);
    m_momentumXferEdit.SetDialog(this);
    m_parkPositionEdit.SetDialog(this);
}

void PlungerPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);
        if (dispid == IDC_PULL_SPEED_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_pullSpeedEdit, plunger->m_d.m_speedPull);
        if (dispid == IDC_RELEASE_SPEED_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_releaseSpeedEdit, plunger->m_d.m_speedFire);
        if (dispid == IDC_STROKE_LENGTH_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_strokeLengthEdit, plunger->m_d.m_stroke);
        if (dispid == IDC_SCATTER_VELOCITY_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_scatterVelocityEdit, plunger->m_d.m_scatterVelocity);
        if (dispid == IDC_AUTO_PLUNGER_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hAutoPlungerCheck, plunger->m_d.m_autoPlunger);
        if (dispid == IDC_ENABLE_MECH_PLUNGER_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hEnableMechPlungerCheck, plunger->m_d.m_mechPlunger);
        if (dispid == IDC_MECH_STRENGTH_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_mechStrengthEdit, plunger->m_d.m_mechStrength);
        if (dispid == IDC_MOMENTUM_XFER_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_momentumXferEdit, plunger->m_d.m_momentumXfer);
        if (dispid == IDC_PARK_POSITION_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_parkPositionEdit, plunger->m_d.m_parkPosition);

        UpdateBaseVisuals(plunger, &plunger->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void PlungerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
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
    UpdateVisuals(dispid);
}

BOOL PlungerPhysicsProperty::OnInitDialog()
{
    m_pullSpeedEdit.AttachItem(IDC_PULL_SPEED_EDIT);
    m_releaseSpeedEdit.AttachItem(IDC_RELEASE_SPEED_EDIT);
    m_strokeLengthEdit.AttachItem(IDC_STROKE_LENGTH_EDIT);
    m_scatterVelocityEdit.AttachItem(IDC_SCATTER_VELOCITY_EDIT);
    m_hAutoPlungerCheck = ::GetDlgItem(GetHwnd(), IDC_AUTO_PLUNGER_CHECK);
    m_hEnableMechPlungerCheck = ::GetDlgItem(GetHwnd(), IDC_ENABLE_MECH_PLUNGER_CHECK);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_mechStrengthEdit.AttachItem(IDC_MECH_STRENGTH_EDIT);
    m_momentumXferEdit.AttachItem(IDC_MOMENTUM_XFER_EDIT);
    m_parkPositionEdit.AttachItem(IDC_PARK_POSITION_EDIT);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(m_pullSpeedEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_releaseSpeedEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_strokeLengthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_scatterVelocityEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hAutoPlungerCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hEnableMechPlungerCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_mechStrengthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_momentumXferEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_parkPositionEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR PlungerPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
