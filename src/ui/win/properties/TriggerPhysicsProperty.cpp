// license:GPLv3+

#include "core/stdafx.h"
#include "TriggerPhysicsProperty.h"

#include "parts/Collection.h"
#include "parts/trigger.h"
#include "ui/win/dialogs/ResURIDialog.h"
#include "ui/win/resource.h"


TriggerPhysicsProperty::TriggerPhysicsProperty(const vector<IWinUIPart *> *pvsel) : BasePropertyDialog(IDD_PROPTRIGGER_STATE, pvsel)
{
    m_hitHeightEdit.SetDialog(this);
    m_stateSrcLinkEdit.SetDialog(this);
}

void TriggerPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < SelCount(); i++)
    {
        if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemTrigger))
            continue;
        const Trigger *const trigger = (Trigger *)SelAt(i)->GetEditable();

        if (dispid == IDC_HIT_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_hitHeightEdit, trigger->m_d.m_hit_height);
        if (dispid == DISPID_Enabled || dispid == -1)
        {
           PropertyDialog::SetCheckboxState(m_hEnableCheck, trigger->m_d.m_enabled);
           m_stateSrcLinkEdit.EnableWindow(trigger->m_d.m_enabled);
           GetDlgItem(IDC_STATE_SOURCE_BUTTON).EnableWindow(trigger->m_d.m_enabled);
        }
        if (dispid == IDC_STATE_SOURCE_EDIT || dispid == -1)
           m_stateSrcLinkEdit.SetWindowText(trigger->m_d.m_stateSrcLink.c_str());
        //only show the first element on multi-select
        break;
    }
}

void TriggerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < SelCount(); i++)
    {
        if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger * const trigger = (Trigger *)SelAt(i)->GetEditable();
        switch (dispid)
        {
           case DISPID_Enabled:
                CHECK_UPDATE_ITEM(trigger->m_d.m_enabled, PropertyDialog::GetCheckboxState(m_hEnableCheck), trigger);
                break;
           case IDC_STATE_SOURCE_EDIT:
           {
              const string link = m_stateSrcLinkEdit.GetWindowText().GetString();
              if (link != trigger->m_d.m_stateSrcLink)
              {
                 PropertyDialog::StartUndo(trigger);
                 trigger->m_d.m_stateSrcLink = link;
                 PropertyDialog::EndUndo(trigger);
              }
              break;
           }
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
    m_hEnableCheck = GetDlgItem(DISPID_Enabled);
    m_hitHeightEdit.AttachItem(IDC_HIT_HEIGHT_EDIT);
    m_stateSrcLinkEdit.AttachItem(IDC_STATE_SOURCE_EDIT);
    UpdateVisuals();

    m_resizer.Initialize(GetHwnd(), CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(m_hEnableCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hitHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_stateSrcLinkEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATE_SOURCE_BUTTON), CResizer::topright, 0);

    return TRUE;
}

BOOL TriggerPhysicsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
   if ((LOWORD(wParam) == IDC_STATE_SOURCE_BUTTON) && (HIWORD(wParam) == BN_CLICKED))
   {
      string current;
      for (int i = 0; i < SelCount(); i++)
      {
         if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemTrigger))
            continue;
         current = ((Trigger *)SelAt(i)->GetEditable())->m_d.m_stateSrcLink;
         break;
      }
      ResURIDialog dlg(
         PinballPlugin::ResURIKind::State, current,
         [this](const string &uri)
         {
            for (int i = 0; i < SelCount(); i++)
            {
               if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemTrigger))
                  continue;
               Trigger *const trigger = (Trigger *)SelAt(i)->GetEditable();
               if (uri != trigger->m_d.m_stateSrcLink)
               {
                  PropertyDialog::StartUndo(trigger);
                  trigger->m_d.m_stateSrcLink = uri;
                  PropertyDialog::EndUndo(trigger);
               }
            }
         },
         GetTable()->GetSettings(), PinballPlugin::DisplayFamily::Any, CTLPI_STATE_TYPE_SWITCH, true, -1);
      dlg.DoModal(GetHwnd());
      UpdateVisuals(IDC_STATE_SOURCE_EDIT);
      return TRUE;
   }
   return BasePropertyDialog::OnCommand(wParam, lParam);
}

INT_PTR TriggerPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
