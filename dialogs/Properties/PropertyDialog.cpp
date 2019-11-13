#include "stdafx.h"
#include "Properties/PropertyDialog.h"
#include "Properties/WallVisualsProperty.h"
#include "Properties/WallPhysicsProperty.h"
#include "Properties/GateVisualsProperty.h"
#include "Properties/GatePhysicsProperty.h"
#include "Properties/RampVisualsProperty.h"
#include "Properties/RampPhysicsProperty.h"
#include "Properties/FlipperVisualsProperty.h"
#include "Properties/FlipperPhysicsProperty.h"
#include "Properties/DragpointVisualsProperty.h"
#include "Properties/PlungerVisualsProperty.h"
#include "Properties/PlungerPhysicsProperty.h"
#include "Properties/BumperVisualsProperty.h"
#include "Properties/BumperPhysicsProperty.h"
#include "Properties/SpinnerVisualsProperty.h"
#include "Properties/SpinnerPhysicsProperty.h"
#include "Properties/TriggerVisualsProperty.h"
#include "Properties/TriggerPhysicsProperty.h"
#include "Properties/LightVisualsProperty.h"
#include "Properties/LightStatesProperty.h"
#include "Properties/KickerVisualsProperty.h"
#include "Properties/KickerPhysicsProperty.h"
#include "Properties/HitTargetVisualsProperty.h"
#include "Properties/HitTargetPhysicsProperty.h"
#include "Properties/DecalVisualsProperty.h"
#include "Properties/TextboxVisualsProperty.h"
#include "Properties/DispreelVisualsProperty.h"
#include "Properties/DispreelStateProperty.h"
#include "Properties/LightseqStatesProperty.h"
#include "Properties/PrimitiveVisualsProperty.h"

#include <WindowsX.h>


PropertyDialog::PropertyDialog() : CDialog(IDD_PROPERTY_DIALOG), m_curTabIndex(0)
{
    memset(m_tabs, 0, sizeof(m_tabs));
}

void PropertyDialog::UpdateTextureComboBox(const vector<Texture *>& contentList, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
    }
    combo.SetCurSel(combo.FindStringExact(1,selectName));
}

void PropertyDialog::UpdateMaterialComboBox(const vector<Material *>& contentList, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateSurfaceComboBox(const PinTable * const ptable, CComboBox &combo, const char *selectName)
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

void PropertyDialog::UpdateSoundComboBox(const PinTable *const ptable, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i=0;i<ptable->m_vsound.size();i++)
    {
        combo.AddString(ptable->m_vsound[i]->m_szName);
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateCollectionComboBox(const PinTable *const ptable, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < ptable->m_vcollection.Size(); i++)
    {
        char szT[MAX_PATH] = {0};

        WideCharToMultiByte(CP_ACP, 0, ptable->m_vcollection[i].m_wzName, -1, szT, MAX_PATH, NULL, NULL);

        combo.AddString(szT);
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateComboBox(const vector<string>& contentList, CComboBox &combo, const char *selectName)
{
    combo.ResetContent();
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i].c_str());
    }
    combo.SetCurSel(combo.FindStringExact(0, selectName));
}

void PropertyDialog::UpdateTabs(VectorProtected<ISelect> *pvsel)
{
    ISelect *psel = pvsel->ElementAt(0);

    if (psel == NULL)
        return;

    m_curTabIndex = m_tab.GetCurSel();
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
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new WallVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new WallPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemGate:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new GateVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new GatePhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemRamp:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new RampVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new RampPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemFlipper:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new FlipperVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new FlipperPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemPlunger:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new PlungerVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new PlungerPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemBumper:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new BumperVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new BumperPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemSpinner:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new SpinnerVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new SpinnerPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemTimer:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemTrigger:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TriggerVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TriggerPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemLight:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new LightVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new LightStatesProperty(pvsel), _T("States")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemKicker:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new KickerVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new KickerPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemHitTarget:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new HitTargetVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new HitTargetPhysicsProperty(pvsel), _T("Physics")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemDecal:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new DecalVisualsProperty(pvsel), _T("Visuals")));
            break;
        }
        case eItemTextbox:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TextboxVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemDispReel:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new DispreelVisualsProperty(pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new DispreelStateProperty(pvsel), _T("States")));
            m_tabs[2] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemLightSeq:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new LightseqStatesProperty(pvsel), _T("States")));
            m_tabs[1] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new TimerProperty(pvsel), _T("Timer")));
            break;
        }
        case eItemPrimitive:
        {
            m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new PrimitiveVisualsProperty(pvsel), _T("Visuals")));
            break;
        }
        case eItemDragPoint:
        {
            const DragPoint * const dpoint = (DragPoint *)psel;
            const int itemType = dpoint->m_pihdp->GetIEditable()->GetItemType();
            if(itemType==eItemRamp)
                m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALSWHEIGHT, pvsel), _T("Visuals")));
            else if(itemType == eItemLight || itemType==eItemTrigger)
                m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALS, pvsel), _T("Visuals")));
            else
                m_tabs[0] = static_cast<BasePropertyDialog *>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALSWTEX, pvsel), _T("Visuals")));

            break;
        }
        default:
            break;
    }
    m_tab.SetCurFocus(0);
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
//     switch (msg)
//     {
//     }
    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);
}


BOOL PropertyDialog::IsSubDialogMessage(MSG &msg) const
{
    for (int i = 0; i < 5; i++)
    {
        if (m_tabs[i]!=NULL)
        {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)
                return TRUE;                    //disable enter key for any input otherwise the app would crash!?
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_DELETE)
            {
                CString className = GetFocus().GetClassName();
                if (className!="Edit")
                {
                    g_pvp->ParseCommand(ID_DELETE, GetHwnd(), FALSE);
                    return TRUE;
                }
            }
            else
            {
                const BOOL ret = m_tabs[i]->IsDialogMessage(msg);
                if (ret)
                    return TRUE;
            }
        }
    }
    return IsDialogMessage(msg);
}

BOOL PropertyDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

TimerProperty::TimerProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTIMER, pvsel)
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
                Surface * const wall = (Surface *)m_pvsel->ElementAt(i);
                wall->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                wall->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemLight:
            {
                Light * const light = (Light *)m_pvsel->ElementAt(i);
                light->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                light->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemFlasher:
            {
                Flasher * const flash = (Flasher *)m_pvsel->ElementAt(i);
                flash->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                flash->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemRubber:
            {
                Rubber * const rubber = (Rubber *)m_pvsel->ElementAt(i);
                rubber->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                rubber->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemBumper:
            {
                Bumper * const bumper = (Bumper *)m_pvsel->ElementAt(i);
                bumper->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                bumper->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemPlunger:
            {
                Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);
                plunger->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                plunger->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemSpinner:
            {
                Spinner * const spinner = (Spinner *)m_pvsel->ElementAt(i);
                spinner->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                spinner->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemTimer:
            {
                Timer * const timer = (Timer *)m_pvsel->ElementAt(i);
                timer->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                timer->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemHitTarget:
            {
                HitTarget * const target= (HitTarget *)m_pvsel->ElementAt(i);
                target->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                target->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemTrigger:
            {
                Trigger * const trigger = (Trigger *)m_pvsel->ElementAt(i);
                trigger->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                trigger->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemKicker:
            {
                Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);
                kicker->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                kicker->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemRamp:
            {
                Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);
                ramp->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                ramp->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemFlipper:
            {
                Flipper * const flipper = (Flipper *)m_pvsel->ElementAt(i);
                flipper->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                flipper->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemGate:
            {
                Gate * const gate = (Gate *)m_pvsel->ElementAt(i);
                gate->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                gate->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemTextbox:
            {
                Textbox * const text = (Textbox *)m_pvsel->ElementAt(i);
                text->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                text->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemDispReel:
            {
                DispReel * const reel = (DispReel *)m_pvsel->ElementAt(i);
                reel->m_d.m_tdr.m_TimerInterval = PropertyDialog::GetIntTextbox(m_timerIntervalEdit);

                /*TODO: uservalue is missing due to VARIANT handling*/

                reel->m_d.m_tdr.m_TimerEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 900));
                break;
            }
            case eItemLightSeq:
            {
                LightSeq * const lightseq = (LightSeq *)m_pvsel->ElementAt(i);
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
                const Surface * const wall = (Surface *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, wall->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), wall->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemLight:
            {
                const Light * const light = (Light *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, light->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), light->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemFlasher:
            {
                const Flasher * const flash = (Flasher *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, flash->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), flash->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemRubber:
            {
                const Rubber * const rubber = (Rubber *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, rubber->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), rubber->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemBumper:
            {
                const Bumper * const bumper = (Bumper *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, bumper->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), bumper->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemPlunger:
            {
                const Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, plunger->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), plunger->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemSpinner:
            {
                const Spinner * const spinner = (Spinner *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, spinner->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), spinner->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTimer:
            {
                const Timer * const timer = (Timer *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, timer->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), timer->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemHitTarget:
            {
                const HitTarget * const target = (HitTarget *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, target->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), target->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTrigger:
            {
                const Trigger * const trigger = (Trigger *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, trigger->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), trigger->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemKicker:
            {
                const Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, kicker->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), kicker->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemRamp:
            {
                const Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, ramp->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), ramp->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemFlipper:
            {
                const Flipper * const flipper = (Flipper *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, flipper->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), flipper->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemGate:
            {
                const Gate * const gate = (Gate *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, gate->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), gate->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTextbox:
            {
                const Textbox * const text = (Textbox *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, text->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), text->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemDispReel:
            {
                const DispReel * const reel = (DispReel *)m_pvsel->ElementAt(i);
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, reel->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 900), reel->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemLightSeq:
            {
                const LightSeq * const lightseq = (LightSeq *)m_pvsel->ElementAt(i);
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
        case BN_CLICKED:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}

void BasePropertyDialog::UpdateBaseProperties(ISelect *psel, BaseProperty *property, const int dispid)
{
    if (!property || psel==NULL)
        return;

    switch (dispid)
    {
        case IDC_HIT_THRESHOLD_EDIT:
            PropertyDialog::StartUndo(psel);
            property->m_threshold = PropertyDialog::GetFloatTextbox(*m_baseHitThresholdEdit);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_HAS_HITEVENT_CHECK:
            PropertyDialog::StartUndo(psel);
            property->m_hitEvent = PropertyDialog::GetCheckboxState(m_hHitEventCheck);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_ELASTICITY_EDIT:
            PropertyDialog::StartUndo(psel);
            property->m_elasticity = PropertyDialog::GetFloatTextbox(*m_baseElasticityEdit);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_COLLIDABLE_CHECK:
            PropertyDialog::StartUndo(psel);
            property->m_collidable = PropertyDialog::GetCheckboxState(m_hCollidableCheck);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_VISIBLE_CHECK:
            PropertyDialog::StartUndo(psel);
            property->m_visible = PropertyDialog::GetCheckboxState(m_hVisibleCheck);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_REFLECT_ENABLED_CHECK:
            PropertyDialog::StartUndo(psel);
            property->m_reflectionEnabled = PropertyDialog::GetCheckboxState(m_hReflectionEnabledCheck);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_FRICTION_EDIT:
            PropertyDialog::StartUndo(psel);
            property->m_friction = PropertyDialog::GetFloatTextbox(*m_baseFrictionEdit);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_SCATTER_ANGLE_EDIT:
            PropertyDialog::StartUndo(psel);
            property->m_scatter = PropertyDialog::GetFloatTextbox(*m_baseScatterAngleEdit);
            PropertyDialog::EndUndo(psel);
            break;
        case DISPID_Image:
            PropertyDialog::StartUndo(psel);
            PropertyDialog::GetComboBoxText(*m_baseImageCombo, property->m_szImage);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_MATERIAL_COMBO:
            PropertyDialog::StartUndo(psel);
            PropertyDialog::GetComboBoxText(*m_baseMaterialCombo, property->m_szMaterial);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_MATERIAL_COMBO4:
            PropertyDialog::StartUndo(psel);
            PropertyDialog::GetComboBoxText(*m_basePhysicsMaterialCombo, property->m_szPhysicsMaterial);
            PropertyDialog::EndUndo(psel);
            break;
        case IDC_OVERWRITE_MATERIAL_SETTINGS:
            PropertyDialog::StartUndo(psel);
            property->m_overwritePhysics = PropertyDialog::GetCheckboxState(m_hOverwritePhysicsCheck);
            PropertyDialog::EndUndo(psel);
            break;
    }
}

void BasePropertyDialog::UpdateBaseVisuals(ISelect *psel, BaseProperty *property)
{
    if (!property)
        return;

    if (m_baseHitThresholdEdit)
        PropertyDialog::SetFloatTextbox(*m_baseHitThresholdEdit, property->m_threshold);
    if (m_baseElasticityEdit)
        PropertyDialog::SetFloatTextbox(*m_baseElasticityEdit, property->m_elasticity);
    if (m_baseFrictionEdit)
        PropertyDialog::SetFloatTextbox(*m_baseFrictionEdit, property->m_friction);
    if (m_baseScatterAngleEdit)
        PropertyDialog::SetFloatTextbox(*m_baseScatterAngleEdit, property->m_scatter);
    if (m_hHitEventCheck)
        PropertyDialog::SetCheckboxState(m_hHitEventCheck, property->m_hitEvent);
    if (m_hCollidableCheck)
        PropertyDialog::SetCheckboxState(m_hCollidableCheck, property->m_collidable);
    if (m_hReflectionEnabledCheck)
        PropertyDialog::SetCheckboxState(m_hReflectionEnabledCheck, property->m_reflectionEnabled);
    if (m_hVisibleCheck)
        PropertyDialog::SetCheckboxState(m_hVisibleCheck, property->m_visible);
    if(m_basePhysicsMaterialCombo)
        PropertyDialog::UpdateMaterialComboBox(psel->GetPTable()->GetMaterialList(), *m_basePhysicsMaterialCombo, property->m_szPhysicsMaterial);
    if(m_hOverwritePhysicsCheck)
        PropertyDialog::SetCheckboxState(m_hOverwritePhysicsCheck, property->m_overwritePhysics);
    if(m_baseMaterialCombo)
        PropertyDialog::UpdateMaterialComboBox(psel->GetPTable()->GetMaterialList(), *m_baseMaterialCombo, property->m_szMaterial);
    if (m_baseImageCombo)
        PropertyDialog::UpdateTextureComboBox(psel->GetPTable()->GetImageList(), *m_baseImageCombo, property->m_szImage);

    if (m_hCollidableCheck)
    {
        if(m_hHitEventCheck)            ::EnableWindow(m_hHitEventCheck, property->m_collidable);
        if(m_hOverwritePhysicsCheck)         ::EnableWindow(m_hOverwritePhysicsCheck, property->m_collidable);
        if(m_baseHitThresholdEdit)      m_baseHitThresholdEdit->EnableWindow(property->m_collidable);
        if(m_basePhysicsMaterialCombo)  m_basePhysicsMaterialCombo->EnableWindow(property->m_collidable);
        if(m_baseElasticityEdit)        m_baseElasticityEdit->EnableWindow(property->m_collidable);
        if(m_baseFrictionEdit)          m_baseFrictionEdit->EnableWindow(property->m_collidable);
        if(m_baseScatterAngleEdit)      m_baseScatterAngleEdit->EnableWindow(property->m_collidable);
    }
    if (m_hHitEventCheck && property->m_collidable)
        if (m_baseHitThresholdEdit)      m_baseHitThresholdEdit->EnableWindow(property->m_hitEvent);
    if (m_hOverwritePhysicsCheck && property->m_collidable)
        if (m_basePhysicsMaterialCombo)  m_basePhysicsMaterialCombo->EnableWindow(!property->m_overwritePhysics);
}
