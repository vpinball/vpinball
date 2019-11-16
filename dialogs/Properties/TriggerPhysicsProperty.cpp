#include "stdafx.h"
#include "Properties/TriggerPhysicsProperty.h"
#include <WindowsX.h>

TriggerPhysicsProperty::TriggerPhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTRIGGER_STATE, pvsel)
{
}

void TriggerPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger *trigger = (Trigger *)m_pvsel->ElementAt(i);
        PropertyDialog::SetFloatTextbox(m_hitHeightEdit, trigger->m_d.m_hit_height);
        PropertyDialog::SetCheckboxState(m_hEnableCheck, trigger->m_d.m_enabled);
        //only show the first element on multi-select
        break;
    }
}

void TriggerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger * const trigger = (Trigger *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 904:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_enabled= PropertyDialog::GetCheckboxState(m_hEnableCheck);
                PropertyDialog::EndUndo(trigger);
                break;
           case IDC_HIT_HEIGHT_EDIT:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_hit_height = PropertyDialog::GetFloatTextbox(m_hitHeightEdit);
                PropertyDialog::EndUndo(trigger);
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL TriggerPhysicsProperty::OnInitDialog()
{
    m_hEnableCheck = ::GetDlgItem(GetHwnd(), 904);
    AttachItem(IDC_HIT_HEIGHT_EDIT, m_hitHeightEdit);
    UpdateVisuals();
    return TRUE;
}
