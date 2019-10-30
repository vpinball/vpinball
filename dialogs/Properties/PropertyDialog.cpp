#include "stdafx.h"
#include "Properties/PropertyDialog.h"
#include "Properties/WallVisualsProperty.h"
#include "Properties/WallPhysicsProperty.h"
#include "Properties/GateVisualsProperty.h"
#include "Properties/GatePhysicsProperty.h"
#include "Properties/RampVisualsProperty.h"
#include "Properties/RampPhysicsProperty.h"
#include <WindowsX.h>


PropertyDialog::PropertyDialog() : CDialog(IDD_PROPERTY_DIALOG)
{
    memset(m_tabs, 0, sizeof(m_tabs));
}

void PropertyDialog::UpdateTextureComboBox(vector<Texture *> contentList, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
    }
    combo.SetCurSel(combo.FindStringExact(1,selectName));
}

void PropertyDialog::UpdateMaterialComboBox(vector<Material *> contentList, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateSurfaceComboBox(PinTable *ptable, CComboBox &combo, const char *selectName)
{
    vector<string> contentList;

    for (size_t i = 0; i < ptable->m_vedit.size(); i++)
    {
        if (ptable->m_vedit[i]->GetItemType() == eItemSurface || (ptable->m_vedit[i]->GetItemType() == eItemRamp) ||
            //!! **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (ptable->m_vedit[i]->GetItemType() == eItemFlasher))
        {
            contentList.push_back(ptable->GetElementName(ptable->m_vedit[i]));
        }
    }
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i].c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateComboBox(vector<string> contentList, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i].c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateTabs(VectorProtected<ISelect> *pvsel)
{
    ISelect *psel = pvsel->ElementAt(0);

    if (psel == NULL)
        return;

    for (int i = 0; i < 5; i++)
        if (m_tabs[i] != NULL)
        {
            m_tab.RemoveTabPage(m_tab.GetTabIndex(m_tabs[i]));
            m_tabs[i] = NULL;
        }

    m_nameEdit.SetWindowText(psel->GetPTable()->GetElementName(psel->GetIEditable()));

    switch (psel->GetItemType())
    {
        case eItemSurface:
        {
            m_tabs[0] = static_cast<BaseProperty *>(m_tab.AddTabPage(new WallVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BaseProperty *>(m_tab.AddTabPage(new WallPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BaseProperty *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemGate:
        {
            m_tabs[0] = static_cast<BaseProperty *>(m_tab.AddTabPage(new GateVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BaseProperty *>(m_tab.AddTabPage(new GatePhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BaseProperty *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemRamp:
        {
            m_tabs[0] = static_cast<BaseProperty *>(m_tab.AddTabPage(new RampVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BaseProperty *>(m_tab.AddTabPage(new RampPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BaseProperty *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        default:
            break;
    }
}

BOOL PropertyDialog::OnInitDialog()
{
    AttachItem(IDC_PROP_TAB, m_tab);
    AttachItem(IDC_NAME_EDIT, m_nameEdit);
    m_resizer.Initialize(*this, CRect(0, 0, 243, 308));
    m_resizer.AddChild(m_nameEdit, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tab, topcenter, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
    return TRUE;
}

INT_PTR PropertyDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Pass resizing messages on to the resizer
    m_resizer.HandleMessage(msg, wparam, lparam);

//  switch (msg)
//  {
        //Additional messages to be handled go here
//  }
    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);
}


BOOL PropertyDialog::IsSubDialogMessage(MSG &msg) const
{
    for (int i = 0; i < 5; i++)
    {
        if (m_tabs[i]!=NULL)
        {
            BOOL ret = m_tabs[i]->IsDialogMessage(msg);
            if (ret)
                return TRUE;
        }
    }
    return IsDialogMessage(msg);
}

BOOL PropertyDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

TimerProperty::TimerProperty(VectorProtected<ISelect> *pvsel) : BaseProperty(IDD_PROPTIMER, pvsel)
{
}

void TimerProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if (m_pvsel->ElementAt(i) == NULL)
            continue;
        switch (m_pvsel->ElementAt(i)->GetItemType())
        {
            case eItemSurface:
            {
                Surface *wall = (Surface *)m_pvsel->ElementAt(i);
                wall->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                wall->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemLight:
            {
                Light *light = (Light *)m_pvsel->ElementAt(i);
                light->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                light->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemFlasher:
            {
                Flasher *flash = (Flasher *)m_pvsel->ElementAt(i);
                flash->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                flash->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemRubber:
            {
                Rubber *rubber = (Rubber *)m_pvsel->ElementAt(i);
                rubber->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                rubber->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemBumper:
            {
                Bumper *bumper = (Bumper *)m_pvsel->ElementAt(i);
                bumper->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                bumper->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemPlunger:
            {
                Plunger *plunger = (Plunger *)m_pvsel->ElementAt(i);
                plunger->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                plunger->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemSpinner:
            {
                Spinner *spinner = (Spinner *)m_pvsel->ElementAt(i);
                spinner->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                spinner->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemTimer:
            {
                Timer *timer = (Timer *)m_pvsel->ElementAt(i);
                timer->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                timer->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemHitTarget:
            {
                HitTarget *target= (HitTarget *)m_pvsel->ElementAt(i);
                target->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                target->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemTrigger:
            {
                Trigger *trigger = (Trigger *)m_pvsel->ElementAt(i);
                trigger->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                trigger->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemKicker:
            {
                Kicker *kicker = (Kicker *)m_pvsel->ElementAt(i);
                kicker->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                kicker->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemRamp:
            {
                Ramp *ramp = (Ramp *)m_pvsel->ElementAt(i);
                ramp->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                ramp->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemFlipper:
            {
                Flipper *flipper = (Flipper *)m_pvsel->ElementAt(i);
                flipper->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                flipper->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemGate:
            {
                Gate *gate = (Gate *)m_pvsel->ElementAt(i);
                gate->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                gate->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemTextbox:
            {
                Textbox *text = (Textbox *)m_pvsel->ElementAt(i);
                text->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                text->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemDispReel:
            {
                DispReel *reel = (DispReel *)m_pvsel->ElementAt(i);
                reel->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                reel->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemLightSeq:
            {
                LightSeq *lightseq = (LightSeq *)m_pvsel->ElementAt(i);
                lightseq->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                lightseq->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            default:
                break;
        }
    }
}

void TimerProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if(m_pvsel->ElementAt(i)==NULL)
            continue;
        switch (m_pvsel->ElementAt(i)->GetItemType())
        {
            case eItemSurface:
            {
                Surface *wall = (Surface *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, wall->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), wall->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemLight:
            {
                Light *light = (Light *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, light->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), light->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemFlasher:
            {
                Flasher *flash = (Flasher *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, flash->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), flash->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemRubber:
            {
                Rubber *rubber = (Rubber *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, rubber->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), rubber->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemBumper:
            {
                Bumper *bumper = (Bumper *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, bumper->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), bumper->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemPlunger:
            {
                Plunger *plunger = (Plunger *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, plunger->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), plunger->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemSpinner:
            {
                Spinner *spinner = (Spinner *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, spinner->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), spinner->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTimer:
            {
                Timer *timer = (Timer *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, timer->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), timer->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemHitTarget:
            {
                HitTarget *target = (HitTarget *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, target->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), target->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTrigger:
            {
                Trigger *trigger = (Trigger *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, trigger->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), trigger->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemKicker:
            {
                Kicker *kicker = (Kicker *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, kicker->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), kicker->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemRamp:
            {
                Ramp *ramp = (Ramp *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, ramp->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), ramp->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemFlipper:
            {
                Flipper *flipper = (Flipper *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, flipper->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), flipper->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemGate:
            {
                Gate *gate = (Gate *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, gate->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), gate->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTextbox:
            {
                Textbox *text = (Textbox *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, text->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), text->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemDispReel:
            {
                DispReel *reel = (DispReel *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, reel->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), reel->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemLightSeq:
            {
                LightSeq *lightseq = (LightSeq *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, lightseq->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), lightseq->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            default:
                break;
        }
    }
}

BOOL TimerProperty::OnInitDialog()
{
    AttachItem(901, m_timerIntervalEdit);
    AttachItem(1504, m_userValueEdit);
    UpdateVisuals();
    return TRUE;
}

BOOL TimerProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int dispID = LOWORD(wParam);

    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
        case BN_CLICKED:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}
