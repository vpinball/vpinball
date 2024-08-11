// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/LightStatesProperty.h"
#include <WindowsX.h>

LightStatesProperty::LightStatesProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPLIGHT_STATE, pvsel)
{
    m_stateList.push_back("Off"s);
    m_stateList.push_back("On"s);
    m_stateList.push_back("Blinking"s);

    m_blinkPatternEdit.SetDialog(this);
    m_blinkIntervalEdit.SetDialog(this);
    m_stateCombo.SetDialog(this);
}

void LightStatesProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);
        if (dispid == DISPID_Light_State || dispid == -1)
        {
            float state;
            light->get_State(&state);
            PropertyDialog::UpdateComboBox(m_stateList, m_stateCombo, m_stateList[(int)state]);
        }
        if (dispid == IDC_BLINK_PATTERN_EDIT || dispid == -1)
            m_blinkPatternEdit.SetWindowText(light->m_d.m_rgblinkpattern.c_str());
        if (dispid == DISPID_Light_BlinkInterval || dispid == -1)
            PropertyDialog::SetIntTextbox(m_blinkIntervalEdit, light->m_d.m_blinkinterval);
        //only show the first element on multi-select
        break;
    }
}

void LightStatesProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_Light_State:
            {
               float state;
               light->get_State(&state);
               const float value = (float)(PropertyDialog::GetComboBoxIndex(m_stateCombo, m_stateList));
               if (state != value)
               {
                  PropertyDialog::StartUndo(light);
                  light->put_State(value);
                  PropertyDialog::EndUndo(light);
               }
               break;
            }
            case IDC_BLINK_PATTERN_EDIT:
            {
                const string pattern(m_blinkPatternEdit.GetWindowText().c_str());
                if (pattern != light->m_d.m_rgblinkpattern)
                {
                    PropertyDialog::StartUndo(light);
                    light->m_d.m_rgblinkpattern = pattern;
                    PropertyDialog::EndUndo(light);
                }
                break;
            }
            case DISPID_Light_BlinkInterval:
                CHECK_UPDATE_ITEM(light->m_d.m_blinkinterval, PropertyDialog::GetIntTextbox(m_blinkIntervalEdit), light);
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
    m_stateCombo.SetDroppedWidth(20);

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(m_stateCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_blinkPatternEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_blinkIntervalEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR LightStatesProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
