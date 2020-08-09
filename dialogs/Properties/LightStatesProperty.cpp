#include "stdafx.h"
#include "Properties/LightStatesProperty.h"
#include <WindowsX.h>

LightStatesProperty::LightStatesProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPLIGHT_STATE, pvsel)
{
    m_stateList.push_back("Off");
    m_stateList.push_back("On");
    m_stateList.push_back("Blinking");
    m_blinkPatternEdit.SetDialog(this);
    m_blinkIntervalEdit.SetDialog(this);
    m_stateCombo.SetDialog(this);
}

void LightStatesProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);
        if (dispid == DISPID_Light_State || dispid == -1)
            PropertyDialog::UpdateComboBox(m_stateList, m_stateCombo, m_stateList[(int)light->getLightState()]);
        if (dispid == IDC_BLINK_PATTERN_EDIT || dispid == -1)
            m_blinkPatternEdit.SetWindowText(light->m_rgblinkpattern);
        if (dispid == DISPID_Light_BlinkInterval || dispid == -1)
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
        Light * const light = (Light *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_Light_State:
                CHECK_UPDATE_COMBO_VALUE_SETTER(light->setLightState, light->getLightState, (LightState)(PropertyDialog::GetComboBoxIndex(m_stateCombo, m_stateList)), light);
                break;
            case IDC_BLINK_PATTERN_EDIT:
            {
                const CString pattern = m_blinkPatternEdit.GetWindowText();
                if (pattern != CString(light->m_rgblinkpattern))
                {
                    PropertyDialog::StartUndo(light);
                    strncpy_s(light->m_rgblinkpattern, pattern.c_str(), sizeof(light->m_rgblinkpattern)-1);
                    PropertyDialog::EndUndo(light);
                }
                break;
            }
            case DISPID_Light_BlinkInterval:
                CHECK_UPDATE_ITEM(light->m_blinkinterval, PropertyDialog::GetIntTextbox(m_blinkIntervalEdit), light);
                break;
            default:
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL LightStatesProperty::OnInitDialog()
{
    m_stateCombo.AttachItem(DISPID_Light_State);
    m_blinkPatternEdit.AttachItem(IDC_BLINK_PATTERN_EDIT);
    m_blinkIntervalEdit.AttachItem(DISPID_Light_BlinkInterval);
    UpdateVisuals();
    return TRUE;
}
