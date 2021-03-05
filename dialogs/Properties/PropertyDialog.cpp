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
#include "Properties/PrimitivePositionProperty.h"
#include "Properties/PrimitivePhysicsProperty.h"
#include "Properties/FlasherVisualsProperty.h"
#include "Properties/RubberVisualsProperty.h"
#include "Properties/RubberPhysicsProperty.h"
#include "Properties/BackglassVisualsProperty.h"
#include "Properties/BackglassCameraProperty.h"
#include "Properties/TableCustomProperty.h"
#include "Properties/TableVisualsProperty.h"
#include "Properties/TablePhysicsProperty.h"
#include "Properties/TableLightsProperty.h"
#include <WindowsX.h>

#pragma region PropertyDialog

LRESULT EditBox::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_KEYUP:
        {
            if (wparam == VK_RETURN)
            {
                if (m_basePropertyDialog)
                    m_basePropertyDialog->UpdateProperties(m_id);
                return 0;
            }
        }
    }
    return WndProcDefault(msg, wparam, lparam);
}

LRESULT ComboBox::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYUP:
    {
        if (wparam == VK_RETURN)
        {
            if (m_basePropertyDialog)
                m_basePropertyDialog->UpdateProperties(m_id);
            ShowDropDown(FALSE);
            return 0;
        }
    }
    }
    return WndProcDefault(msg, wparam, lparam);
}

PropertyDialog::PropertyDialog() : CDialog(IDD_PROPERTY_DIALOG), m_previousType((ItemTypeEnum)0), m_backglassView(false), m_curTabIndex(0)
{
    memset(m_tabs, 0, sizeof(m_tabs));
}

void PropertyDialog::CreateTabs(VectorProtected<ISelect> &pvsel)
{
    ISelect* const psel = pvsel.ElementAt(0);
    if (psel == NULL)
        return;

    int activePage = m_tab.m_activePage;
    m_backglassView = g_pvp->m_backglassView;
    switch (psel->GetItemType())
    {
    case eItemTable:
    {
        if (g_pvp->m_backglassView)
        {
            m_elementTypeName.SetWindowText("Backglass");
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new BackglassVisualsProperty(&pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new BackglassCameraProperty(&pvsel), _T("Camera")));
            if (m_tab.m_activeTabText == CString("Visuals"))
                activePage = 0;
        }
        else
        {
            m_elementTypeName.SetWindowText("Table");
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TableCustomProperty(&pvsel), _T("User")));
            m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TableVisualsProperty(&pvsel), _T("Visuals")));
            m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TablePhysicsProperty(&pvsel), _T("Physics")));
            m_tabs[3] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TableLightsProperty(&pvsel), _T("Lights")));
            if (m_tab.m_activeTabText == CString("User"))
                activePage = 0;
            else if (m_tab.m_activeTabText == CString("Visuals"))
                activePage = 1;
            else if (m_tab.m_activeTabText == CString("Physics"))
                activePage = 2;
            else if (m_tab.m_activeTabText == CString("Lights"))
                activePage = 3;

        }
        break;
    }
    case eItemSurface:
    {
        m_elementTypeName.SetWindowText("Wall");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new WallVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new WallPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemGate:
    {
        m_elementTypeName.SetWindowText("Gate");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new GateVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new GatePhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemRamp:
    {
        m_elementTypeName.SetWindowText("Ramp");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new RampVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new RampPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemFlipper:
    {
        m_elementTypeName.SetWindowText("Flipper");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new FlipperVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new FlipperPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemPlunger:
    {
        m_elementTypeName.SetWindowText("Plunger");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new PlungerVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new PlungerPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemBumper:
    {
        m_elementTypeName.SetWindowText("Bumper");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new BumperVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new BumperPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemSpinner:
    {
        m_elementTypeName.SetWindowText("Spinner");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new SpinnerVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new SpinnerPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemTimer:
    {
        m_elementTypeName.SetWindowText("Timer");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        activePage = 0;
        break;
    }
    case eItemTrigger:
    {
        m_elementTypeName.SetWindowText("Trigger");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TriggerVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TriggerPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemLight:
    {
        m_elementTypeName.SetWindowText("Light");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new LightVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new LightStatesProperty(&pvsel), _T("States")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("States"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemKicker:
    {
        m_elementTypeName.SetWindowText("Kicker");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new KickerVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new KickerPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemHitTarget:
    {
        m_elementTypeName.SetWindowText("Target");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new HitTargetVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new HitTargetPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemDecal:
    {
        m_elementTypeName.SetWindowText("Decal");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DecalVisualsProperty(&pvsel), _T("Visuals")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemTextbox:
    {
        m_elementTypeName.SetWindowText("Textbox");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TextboxVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 1;
        break;
    }
    case eItemDispReel:
    {
        m_elementTypeName.SetWindowText("Reel");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DispreelVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DispreelStateProperty(&pvsel), _T("States")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("States"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemLightSeq:
    {
        m_elementTypeName.SetWindowText("Light Sequence");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new LightseqStatesProperty(&pvsel), _T("States")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("States"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 1;
        break;
    }
    case eItemPrimitive:
    {
        m_elementTypeName.SetWindowText("Primitive");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new PrimitiveVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new PrimitivePositionProperty(&pvsel), _T("Position")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new PrimitivePhysicsProperty(&pvsel), _T("Physics")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Position"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 2;
        break;
    }
    case eItemFlasher:
    {
        m_elementTypeName.SetWindowText("Flasher");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new FlasherVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 1;
        break;
    }
    case eItemRubber:
    {
        m_elementTypeName.SetWindowText("Rubber");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new RubberVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new RubberPhysicsProperty(&pvsel), _T("Physics")));
        m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TimerProperty(&pvsel), _T("Timer")));
        if (m_tab.m_activeTabText == CString("Visuals"))
            activePage = 0;
        else if (m_tab.m_activeTabText == CString("Physics"))
            activePage = 1;
        else if (m_tab.m_activeTabText == CString("Timer"))
            activePage = 2;
        break;
    }
    case eItemDragPoint:
    {
        m_elementTypeName.SetWindowText("Control Point");
        const DragPoint* const dpoint = (DragPoint*)psel;
        const ItemTypeEnum itemType = dpoint->m_pihdp->GetIEditable()->GetItemType();
        if (itemType == eItemRamp)
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALSWHEIGHT, &pvsel), _T("Visuals")));
        else if (itemType == eItemLight || itemType == eItemTrigger)
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALS, &pvsel), _T("Visuals")));
        else
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALSWTEX, &pvsel), _T("Visuals")));

        break;
    }
    default:
        break;
    }
    m_tab.SetCurFocus(activePage);
    m_previousType = psel->GetItemType();
}

void PropertyDialog::DeleteAllTabs()
{
    BasePropertyDialog::m_disableEvents = true;
    for (int i = 0; i < PROPERTY_TABS; i++)
        if (m_tabs[i] != NULL)
        {
            m_tab.RemoveTabPage(m_tab.GetTabIndex(m_tabs[i]));
            m_tabs[i] = NULL;
        }
    m_previousType = (ItemTypeEnum)0;
    m_backglassView = false;
}

void PropertyDialog::UpdateTextureComboBox(const vector<Texture *>& contentList, CComboBox &combo, const std::string &selectName)
{
    bool texelFound = false;
    for (auto texel : contentList)
    {
        if (strncmp(texel->m_szName.c_str(), selectName.c_str(), MAXTOKEN) == 0) //!! _stricmp?
            texelFound = true;
    }
    if (combo.FindStringExact(1, selectName.c_str()) == CB_ERR || !texelFound)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < contentList.size(); i++)
            combo.AddString(contentList[i]->m_szName.c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void PropertyDialog::UpdateMaterialComboBox(const vector<Material *>& contentList, CComboBox &combo, const std::string &selectName)
{
    bool matFound = false;
    for (auto mat : contentList)
    {
        if (mat->m_szName==selectName)
            matFound = true;
    }
    if(combo.FindStringExact(1, selectName.c_str()) == CB_ERR || !matFound)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < contentList.size(); i++)
            combo.AddString(contentList[i]->m_szName.c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void PropertyDialog::UpdateSurfaceComboBox(const PinTable * const ptable, CComboBox &combo, const char *selectName)
{
    vector<string> contentList;

    if(combo.FindStringExact(1, selectName) == CB_ERR)
    {
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
            combo.AddString(contentList[i].c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateSoundComboBox(const PinTable *const ptable, CComboBox &combo, const string& selectName)
{
    if(combo.FindStringExact(1, selectName.c_str())==CB_ERR)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < ptable->m_vsound.size(); i++)
            combo.AddString(ptable->m_vsound[i]->m_szName.c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void PropertyDialog::UpdateCollectionComboBox(const PinTable *const ptable, CComboBox &combo, const char *selectName)
{
    if(combo.FindStringExact(1, selectName)==CB_ERR)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (int i = 0; i < ptable->m_vcollection.Size(); i++)
        {
            char szT[sizeof(ptable->m_vcollection[i].m_wzName)/sizeof(ptable->m_vcollection[i].m_wzName[0])];
            WideCharToMultiByteNull(CP_ACP, 0, ptable->m_vcollection[i].m_wzName, -1, szT, sizeof(szT), NULL, NULL);
            combo.AddString(szT);
        }
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateComboBox(const vector<string>& contentList, CComboBox &combo, const string& selectName)
{
    bool strFound = false;
    for (auto str : contentList)
    {
        if (str == selectName)
            strFound = true;
    }
    if(combo.FindStringExact(1, selectName.c_str())==CB_ERR || !strFound)
    {
        combo.ResetContent();
        for (size_t i = 0; i < contentList.size(); i++)
            combo.AddString(contentList[i].c_str());
    }
    combo.SetCurSel(combo.FindStringExact(0, selectName.c_str()));
}

void PropertyDialog::UpdateTabs(VectorProtected<ISelect> &pvsel)
{
    ISelect * const psel = pvsel.ElementAt(0);

    if (psel == nullptr)
        return;

    ShowWindow(SW_HIDE);
    if (m_previousType != psel->GetItemType() || m_backglassView!=g_pvp->m_backglassView)
    {
        BasePropertyDialog::m_disableEvents = true;
        m_curTabIndex = m_tab.GetCurSel();
        for (int i = 0; i < PROPERTY_TABS; i++)
            if (m_tabs[i] != NULL)
            {
                m_tab.RemoveTabPage(m_tab.GetTabIndex(m_tabs[i]));
                m_tabs[i] = NULL;
            }

        for (int i = 0; i < pvsel.Size(); i++)
        {
            // check for multiple selection
            if (psel->GetItemType() != pvsel.ElementAt(i)->GetItemType())
            {
                m_multipleElementsStatic.ShowWindow(SW_SHOW);
                m_nameEdit.ShowWindow(SW_HIDE);
                m_elementTypeName.ShowWindow(SW_HIDE);
                m_tab.ShowWindow(SW_HIDE);
                ShowWindow(SW_SHOW);
                BasePropertyDialog::m_disableEvents = false;
                return;
            }
        }
        CreateTabs(pvsel);
    }


    if (m_multipleElementsStatic.IsWindowVisible())
    {
        m_multipleElementsStatic.ShowWindow(SW_HIDE);
        m_nameEdit.ShowWindow(SW_SHOW);
        m_elementTypeName.ShowWindow(SW_SHOW);
        m_tab.ShowWindow(SW_SHOW);
    }

    if (pvsel.Size() > 1)
    {
        char header[64];
        char collection[64] = {0};
        char name[64];
        const WCHAR * const wzName = psel->GetPTable()->GetCollectionNameByElement(psel);
        if (wzName != NULL)
        {
            WideCharToMultiByteNull(CP_ACP, 0, wzName, -1, collection, 64, NULL, NULL);
        }
        
        CComBSTR bstr;
        psel->GetTypeName(&bstr);
        WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, name, 64, NULL, NULL);
        sprintf_s(header, "%s(%d)", name, pvsel.Size());

        if (collection[0] != 0)
            sprintf_s(header, "%s [%s](%d)", collection, name, pvsel.Size());
        else
            sprintf_s(header, "%s(%d)", name, pvsel.Size());

        m_nameEdit.SetWindowText(header);
        m_nameEdit.SetReadOnly();
    }
    else
    {
        m_nameEdit.SetWindowText(psel->GetPTable()->GetElementName(psel->GetIEditable()));
        m_nameEdit.SetReadOnly(0);
    }

    for (int i = 0; i < PROPERTY_TABS; i++)
    {
        if (m_tabs[i])
            m_tabs[i]->UpdateVisuals(-1);
    }
    BasePropertyDialog::m_disableEvents = false;
    ShowWindow();
}

bool PropertyDialog::PreTranslateMessage(MSG* msg)
{
   if (!IsWindow())
      return false;

   // only pre-translate mouse and keyboard input events
   if (((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST)))
   {
      const int keyPressed = LOWORD(msg->wParam);
      // only pass F1-F12 to the main VPinball class to open subdialogs from everywhere
      if(keyPressed>=VK_F1 && keyPressed<=VK_F12 && TranslateAccelerator(g_pvp->GetHwnd(), g_haccel, msg))
         return true;
   }

   return !!IsSubDialogMessage(*msg);
}

BOOL PropertyDialog::OnInitDialog()
{
    AttachItem(IDC_MULTIPLE_ELEMENTS_SELECTED_STATIC, m_multipleElementsStatic);
    AttachItem(IDC_PROP_TAB, m_tab);
    AttachItem(IDC_NAME_EDIT, m_nameEdit);
    AttachItem(IDC_STATIC_ELEMENT_TYPE, m_elementTypeName);
    m_multipleElementsStatic.ShowWindow(SW_HIDE);

    m_resizer.Initialize(*this, CRect(0, 0, 243, 308));
    m_resizer.AddChild(m_elementTypeName, topcenter, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_nameEdit, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_multipleElementsStatic, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tab, topcenter, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
    return TRUE;
}

INT_PTR PropertyDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Pass resizing messages on to the resizer
    m_resizer.HandleMessage(msg, wparam, lparam);
    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);
}


void PropertyDialog::OnClose()
{
    CDialog::OnCancel();
}



BOOL PropertyDialog::IsSubDialogMessage(MSG &msg) const
{
    for (int i = 0; i < PROPERTY_TABS; i++)
    {
        if (m_tabs[i]!=nullptr)
        {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)
                return TRUE;                    //disable enter key for any input otherwise the app would crash!?
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_DELETE)
            {
                const CString className = GetFocus().GetClassName();
                if (className != "Edit")
                {
                    g_pvp->ParseCommand(ID_DELETE, false);
                    return TRUE;
                }
            }
            else
            {
                const BOOL ret = m_tabs[i]->IsDialogMessage(msg);

                if (ret==TRUE)
                    return TRUE;
            }
        }
    }
    return IsDialogMessage(msg);
}

LRESULT PropertyDialog::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
// Respond to a mouse click on the window
{
    // Set window focus. The docker will now report this as active.
    if (!IsChild(::GetFocus()))
        SetFocus();

    return FinalWindowProc(msg, wparam, lparam);
}

BOOL PropertyDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //const int dispID = LOWORD(wParam);

    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        case CBN_SELCHANGE:
        case BN_CLICKED:
        {
            if (m_tabs[0] && m_tabs[0]->m_pvsel->ElementAt(0) != NULL)
                m_tabs[0]->m_pvsel->ElementAt(0)->GetIEditable()->SetName(string(m_nameEdit.GetWindowText()));
            return TRUE;
        }
    }
    return FALSE;
}

#pragma endregion

#pragma region TimeProperty

TimerProperty::TimerProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTIMER, pvsel)
{
    m_timerIntervalEdit.SetDialog(this);
    m_userValueEdit.SetDialog(this);
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
    UpdateVisuals(dispid);
}

void TimerProperty::UpdateVisuals(const int dispid/*=-1*/)
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
    m_timerIntervalEdit.AttachItem(901);
    m_userValueEdit.AttachItem(1504);
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
        case CBN_SELCHANGE:
        case BN_CLICKED:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}

#pragma endregion

#pragma region BasePropertyDialog

void BasePropertyDialog::UpdateBaseProperties(ISelect *psel, BaseProperty *property, const int dispid)
{
    if (!property || psel==NULL)
        return;

    switch (dispid)
    {
        case IDC_HIT_THRESHOLD_EDIT:
            CHECK_UPDATE_ITEM(property->m_threshold, PropertyDialog::GetFloatTextbox(*m_baseHitThresholdEdit), psel);
            break;
        case IDC_HAS_HITEVENT_CHECK:
            CHECK_UPDATE_ITEM(property->m_hitEvent, PropertyDialog::GetCheckboxState(m_hHitEventCheck), psel);
            break;
        case IDC_ELASTICITY_EDIT:
            CHECK_UPDATE_ITEM(property->m_elasticity, PropertyDialog::GetFloatTextbox(*m_baseElasticityEdit), psel);
            break;
        case IDC_COLLIDABLE_CHECK:
            CHECK_UPDATE_ITEM(property->m_collidable, PropertyDialog::GetCheckboxState(m_hCollidableCheck), psel);
            break;
        case IDC_VISIBLE_CHECK:
            CHECK_UPDATE_ITEM(property->m_visible, PropertyDialog::GetCheckboxState(m_hVisibleCheck), psel);
            break;
        case IDC_REFLECT_ENABLED_CHECK:
            CHECK_UPDATE_ITEM(property->m_reflectionEnabled, PropertyDialog::GetCheckboxState(m_hReflectionEnabledCheck), psel);
            break;
        case IDC_FRICTION_EDIT:
            CHECK_UPDATE_ITEM(property->m_friction, PropertyDialog::GetFloatTextbox(*m_baseFrictionEdit), psel);
            break;
        case IDC_SCATTER_ANGLE_EDIT:
            CHECK_UPDATE_ITEM(property->m_scatter, PropertyDialog::GetFloatTextbox(*m_baseScatterAngleEdit), psel);
            break;
        case DISPID_Image:
            CHECK_UPDATE_COMBO_TEXT_STRING(property->m_szImage, *m_baseImageCombo, psel);
            break;
        case IDC_MATERIAL_COMBO:
            CHECK_UPDATE_COMBO_TEXT_STRING(property->m_szMaterial, *m_baseMaterialCombo, psel);
            break;
        case IDC_MATERIAL_COMBO4:
            CHECK_UPDATE_COMBO_TEXT_STRING(property->m_szPhysicsMaterial, *m_basePhysicsMaterialCombo, psel);
            break;
        case IDC_OVERWRITE_MATERIAL_SETTINGS:
            CHECK_UPDATE_ITEM(property->m_overwritePhysics, PropertyDialog::GetCheckboxState(m_hOverwritePhysicsCheck), psel);
            break;
    }
}

void BasePropertyDialog::UpdateBaseVisuals(ISelect *psel, BaseProperty *property, const int dispid)
{
    if (!property)
        return;

    if (m_baseHitThresholdEdit && (dispid == IDC_HIT_THRESHOLD_EDIT || dispid == -1))
        PropertyDialog::SetFloatTextbox(*m_baseHitThresholdEdit, property->m_threshold);
    if (m_baseElasticityEdit && (dispid == IDC_ELASTICITY_EDIT || dispid == -1))
        PropertyDialog::SetFloatTextbox(*m_baseElasticityEdit, property->m_elasticity);
    if (m_baseFrictionEdit && (dispid == IDC_FRICTION_EDIT || dispid == -1))
        PropertyDialog::SetFloatTextbox(*m_baseFrictionEdit, property->m_friction);
    if (m_baseScatterAngleEdit && (dispid == IDC_SCATTER_ANGLE_EDIT || dispid == -1))
        PropertyDialog::SetFloatTextbox(*m_baseScatterAngleEdit, property->m_scatter);
    if (m_hHitEventCheck && (dispid == IDC_HAS_HITEVENT_CHECK || dispid == -1))
        PropertyDialog::SetCheckboxState(m_hHitEventCheck, property->m_hitEvent);
    if (m_hCollidableCheck && (dispid == IDC_COLLIDABLE_CHECK || dispid == -1))
        PropertyDialog::SetCheckboxState(m_hCollidableCheck, property->m_collidable);
    if (m_hReflectionEnabledCheck && (dispid == IDC_REFLECT_ENABLED_CHECK || dispid == -1))
        PropertyDialog::SetCheckboxState(m_hReflectionEnabledCheck, property->m_reflectionEnabled);
    if (m_hVisibleCheck && (dispid == IDC_VISIBLE_CHECK || dispid == -1))
        PropertyDialog::SetCheckboxState(m_hVisibleCheck, property->m_visible);
    if(m_basePhysicsMaterialCombo && (dispid == IDC_MATERIAL_COMBO4 || dispid == -1))
        PropertyDialog::UpdateMaterialComboBox(psel->GetPTable()->GetMaterialList(), *m_basePhysicsMaterialCombo, property->m_szPhysicsMaterial);
    if(m_hOverwritePhysicsCheck && (dispid == IDC_OVERWRITE_MATERIAL_SETTINGS || dispid == -1))
        PropertyDialog::SetCheckboxState(m_hOverwritePhysicsCheck, property->m_overwritePhysics);
    if(m_baseMaterialCombo && (dispid == IDC_MATERIAL_COMBO || dispid == -1))
        PropertyDialog::UpdateMaterialComboBox(psel->GetPTable()->GetMaterialList(), *m_baseMaterialCombo, property->m_szMaterial);
    if (m_baseImageCombo && (dispid == DISPID_Image || dispid == -1))
        PropertyDialog::UpdateTextureComboBox(psel->GetPTable()->GetImageList(), *m_baseImageCombo, property->m_szImage);

    if (m_hCollidableCheck)
    {
        if(m_hHitEventCheck)            ::EnableWindow(m_hHitEventCheck, property->m_collidable);
        if(m_hOverwritePhysicsCheck)    ::EnableWindow(m_hOverwritePhysicsCheck, property->m_collidable);
        if(m_baseHitThresholdEdit)      m_baseHitThresholdEdit->EnableWindow(property->m_collidable);
        if(m_basePhysicsMaterialCombo)  m_basePhysicsMaterialCombo->EnableWindow(property->m_collidable);
        if(m_baseElasticityEdit)        m_baseElasticityEdit->EnableWindow(property->m_collidable);
        if(m_baseFrictionEdit)          m_baseFrictionEdit->EnableWindow(property->m_collidable);
        if(m_baseScatterAngleEdit)      m_baseScatterAngleEdit->EnableWindow(property->m_collidable);
    }
    if (m_hHitEventCheck && property->m_collidable)
        if (m_baseHitThresholdEdit)     m_baseHitThresholdEdit->EnableWindow(property->m_hitEvent);
    if (m_hOverwritePhysicsCheck && property->m_collidable)
    {
        if (m_basePhysicsMaterialCombo) m_basePhysicsMaterialCombo->EnableWindow(!property->m_overwritePhysics);
        if (m_baseElasticityEdit)       m_baseElasticityEdit->EnableWindow(property->m_overwritePhysics);
        if (m_baseFrictionEdit)         m_baseFrictionEdit->EnableWindow(property->m_overwritePhysics);
        if (m_baseScatterAngleEdit)     m_baseScatterAngleEdit->EnableWindow(property->m_overwritePhysics);
    }
}

bool BasePropertyDialog::m_disableEvents = false;
INT_PTR BasePropertyDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);

}
#pragma endregion

#pragma region Docking

CContainProperties::CContainProperties()
{
    SetTabText(_T("Properties"));
    SetTabIcon(IDI_OPTIONS);
    SetDockCaption(_T("Properties"));
    SetView(m_propertyDialog);
}

CDockProperty::CDockProperty()
{
    SetView(m_propContainer);
    SetBarWidth(4);
}

void CDockProperty::OnClose()
{
    // nothing to do only to prevent closing the window
}

#pragma endregion
