#include "stdafx.h"
#include "Properties/LightStatesProperty.h"
#include <WindowsX.h>

LightStatesProperty::LightStatesProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPLIGHT_STATE, pvsel)
{
    m_stateList.push_back("LightStateOff");
    m_stateList.push_back("LightStateOn");
    m_stateList.push_back("LightStateBlinking");
}

void LightStatesProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light *light = (Light *)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateComboBox(m_stateList, m_stateCombo, m_stateList[(int)light->m_d.m_state].c_str());
        m_blinkPatternEdit.SetWindowText(light->m_rgblinkpattern);
        PropertyDialog::SetIntTextbox(m_blinkIntervalEdit, light->m_blinkinterval);
        //only show the first element on multi-select
        break;
    }
}

void LightStatesProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light *light = (Light *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_Light_State:
                PropertyDialog::StartUndo(light);
                light->m_d.m_state = (LightState)(PropertyDialog::GetComboBoxIndex(m_stateCombo, m_stateList));
                PropertyDialog::EndUndo(light);
                break;
            case IDC_BLINK_PATTERN_EDIT:
            {
                PropertyDialog::StartUndo(light);
                CString pattern = m_blinkPatternEdit.GetWindowText();
                strncpy_s(light->m_rgblinkpattern, 32, pattern, pattern.GetLength());
                PropertyDialog::EndUndo(light);
                break;
            }
            case DISPID_Light_BlinkInterval:
                PropertyDialog::StartUndo(light);
                light->m_blinkinterval = PropertyDialog::GetIntTextbox(m_blinkIntervalEdit);
                PropertyDialog::EndUndo(light);
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL LightStatesProperty::OnInitDialog()
{
    AttachItem(DISPID_Light_State, m_stateCombo);
    AttachItem(IDC_BLINK_PATTERN_EDIT, m_blinkPatternEdit);
    AttachItem(DISPID_Light_BlinkInterval, m_blinkIntervalEdit);
    UpdateVisuals();
    return TRUE;
}


