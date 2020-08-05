#include "stdafx.h"
#include "Properties/TriggerPhysicsProperty.h"
#include <WindowsX.h>

TriggerPhysicsProperty::TriggerPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTRIGGER_STATE, pvsel)
{
    m_hitHeightEdit.SetDialog(this);
}

void TriggerPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger *trigger = (Trigger *)m_pvsel->ElementAt(i);

        if (dispid == IDC_HIT_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_hitHeightEdit, trigger->m_d.m_hit_height);
        if (dispid == 904 || dispid == -1)
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
                CHECK_UPDATE_ITEM(trigger->m_d.m_enabled, PropertyDialog::GetCheckboxState(m_hEnableCheck), trigger);
                break;
           case IDC_HIT_HEIGHT_EDIT:
                CHECK_UPDATE_ITEM(trigger->m_d.m_hit_height, PropertyDialog::GetFloatTextbox(m_hitHeightEdit), trigger);
                break;
            default:
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL TriggerPhysicsProperty::OnInitDialog()
{
    m_hEnableCheck = ::GetDlgItem(GetHwnd(), 904);
    m_hitHeightEdit.AttachItem(IDC_HIT_HEIGHT_EDIT);
    UpdateVisuals();
    return TRUE;
}
