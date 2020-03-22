#include "stdafx.h"
#include "Properties/DispreelStateProperty.h"
#include <WindowsX.h>

DispreelStateProperty::DispreelStateProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPDISPREEL_STATE, pvsel)
{
}

void DispreelStateProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
            continue;
        DispReel * const reel = (DispReel *)m_pvsel->ElementAt(i);

        PropertyDialog::SetIntTextbox(m_motorStepsEdit, reel->GetMotorSteps());
        PropertyDialog::SetIntTextbox(m_updateIntervalEdit, reel->GetUpdateInterval());
        PropertyDialog::UpdateSoundComboBox(reel->GetPTable(), m_soundCombo, reel->m_d.m_szSound);
        //only show the first element on multi-select
        break;
    }
}

void DispreelStateProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
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
                CHECK_UPDATE_COMBO_TEXT(reel->m_d.m_szSound, m_soundCombo, reel);
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL DispreelStateProperty::OnInitDialog()
{
    AttachItem(IDC_MOTOR_STEPS_EDIT, m_motorStepsEdit);
    AttachItem(IDC_UPDATE_INTERVAL_EDIT, m_updateIntervalEdit);
    AttachItem(DISPID_Sound, m_soundCombo);
    UpdateVisuals();
    return TRUE;
}
