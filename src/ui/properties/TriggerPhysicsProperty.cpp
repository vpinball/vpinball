// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/TriggerPhysicsProperty.h"
#include <WindowsX.h>

TriggerPhysicsProperty::TriggerPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTRIGGER_STATE, pvsel)
{
    m_hitHeightEdit.SetDialog(this);
}

void TriggerPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        const Trigger *const trigger = (Trigger *)m_pvsel->ElementAt(i);

        if (dispid == IDC_HIT_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_hitHeightEdit, trigger->m_d.m_hit_height);
        if (dispid == DISPID_Enabled || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hEnableCheck, trigger->m_d.m_enabled);
        //only show the first element on multi-select
        break;
    }
}

void TriggerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger * const trigger = (Trigger *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
           case DISPID_Enabled:
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
    m_hEnableCheck = ::GetDlgItem(GetHwnd(), DISPID_Enabled);
    m_hitHeightEdit.AttachItem(IDC_HIT_HEIGHT_EDIT);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(m_hEnableCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hitHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR TriggerPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
