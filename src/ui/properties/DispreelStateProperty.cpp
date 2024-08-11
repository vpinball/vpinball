// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/DispreelStateProperty.h"
#include <WindowsX.h>

DispreelStateProperty::DispreelStateProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPDISPREEL_STATE, pvsel)
{
    m_motorStepsEdit.SetDialog(this);
    m_updateIntervalEdit.SetDialog(this);
    m_soundCombo.SetDialog(this);
}

void DispreelStateProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
            continue;
        DispReel * const reel = (DispReel *)m_pvsel->ElementAt(i);
        if (dispid == IDC_MOTOR_STEPS_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_motorStepsEdit, reel->GetMotorSteps());
        if (dispid == IDC_UPDATE_INTERVAL_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_updateIntervalEdit, reel->GetUpdateInterval());
        if (dispid == DISPID_Sound || dispid == -1)
            PropertyDialog::UpdateSoundComboBox(reel->GetPTable(), m_soundCombo, reel->m_d.m_szSound);
        //only show the first element on multi-select
        break;
    }
}

void DispreelStateProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
            continue;
        DispReel * const reel = (DispReel *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_MOTOR_STEPS_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetMotorSteps, reel->GetMotorSteps, PropertyDialog::GetIntTextbox, m_motorStepsEdit, reel);
                break;
            case IDC_UPDATE_INTERVAL_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetUpdateInterval, reel->GetUpdateInterval, PropertyDialog::GetIntTextbox, m_updateIntervalEdit, reel);
                break;
            case DISPID_Sound:
                CHECK_UPDATE_COMBO_TEXT_STRING(reel->m_d.m_szSound, m_soundCombo, reel);
                break;
            default:
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL DispreelStateProperty::OnInitDialog()
{
    m_motorStepsEdit.AttachItem(IDC_MOTOR_STEPS_EDIT);
    m_updateIntervalEdit.AttachItem(IDC_UPDATE_INTERVAL_EDIT);
    m_soundCombo.AttachItem(DISPID_Sound);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(m_motorStepsEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_updateIntervalEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_soundCombo, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR DispreelStateProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
