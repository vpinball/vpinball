// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/PropertyDialog.h"
#include "ui/properties/WallVisualsProperty.h"
#include "ui/properties/WallPhysicsProperty.h"
#include "ui/properties/GateVisualsProperty.h"
#include "ui/properties/GatePhysicsProperty.h"
#include "ui/properties/RampVisualsProperty.h"
#include "ui/properties/RampPhysicsProperty.h"
#include "ui/properties/FlipperVisualsProperty.h"
#include "ui/properties/FlipperPhysicsProperty.h"
#include "ui/properties/DragpointVisualsProperty.h"
#include "ui/properties/PlungerVisualsProperty.h"
#include "ui/properties/PlungerPhysicsProperty.h"
#include "ui/properties/BallVisualsProperty.h"
#include "ui/properties/BallPhysicsProperty.h"
#include "ui/properties/BumperVisualsProperty.h"
#include "ui/properties/BumperPhysicsProperty.h"
#include "ui/properties/SpinnerVisualsProperty.h"
#include "ui/properties/SpinnerPhysicsProperty.h"
#include "ui/properties/TriggerVisualsProperty.h"
#include "ui/properties/TriggerPhysicsProperty.h"
#include "ui/properties/LightVisualsProperty.h"
#include "ui/properties/LightStatesProperty.h"
#include "ui/properties/KickerVisualsProperty.h"
#include "ui/properties/KickerPhysicsProperty.h"
#include "ui/properties/HitTargetVisualsProperty.h"
#include "ui/properties/HitTargetPhysicsProperty.h"
#include "ui/properties/DecalVisualsProperty.h"
#include "ui/properties/TextboxVisualsProperty.h"
#include "ui/properties/DispreelVisualsProperty.h"
#include "ui/properties/DispreelStateProperty.h"
#include "ui/properties/LightseqStatesProperty.h"
#include "ui/properties/PrimitiveVisualsProperty.h"
#include "ui/properties/PrimitivePositionProperty.h"
#include "ui/properties/PrimitivePhysicsProperty.h"
#include "ui/properties/FlasherVisualsProperty.h"
#include "ui/properties/RubberVisualsProperty.h"
#include "ui/properties/RubberPhysicsProperty.h"
#include "ui/properties/BackglassVisualsProperty.h"
#include "ui/properties/BackglassCameraProperty.h"
#include "ui/properties/TableAudioProperty.h"
#include "ui/properties/TableVisualsProperty.h"
#include "ui/properties/TablePhysicsProperty.h"
#include "ui/properties/TableLightsProperty.h"
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
            if (wparam == VK_ESCAPE)
               return 1;
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
    m_accel = LoadAccelerators(g_pvp->theInstance, MAKEINTRESOURCE(IDR_VPSIMPELACCEL));
}

void PropertyDialog::CreateTabs(VectorProtected<ISelect> &pvsel)
{
    ISelect* const psel = pvsel.ElementAt(0);
    if (psel == nullptr)
        return;

    int activePage = m_tab.m_activePage;
    m_backglassView = g_pvp->m_backglassView;
    m_isPlayfieldMesh = false;

    while (m_tab.GetItemCount() > 0)
       m_tab.RemoveTabPage(0);
    memset(m_tabs, 0, sizeof(m_tabs));

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
            else if (m_tab.m_activeTabText == CString("Camera"))
                activePage = 1;
        }
        else
        {
            m_elementTypeName.SetWindowText("Table");
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TableVisualsProperty(&pvsel), _T("Visuals")));
            m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TableLightsProperty(&pvsel), _T("Lights")));
            m_tabs[2] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TablePhysicsProperty(&pvsel), _T("Physics")));
            m_tabs[3] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new TableAudioProperty(&pvsel), _T("Sound")));
            if (m_tab.m_activeTabText == CString("Visuals"))
                activePage = 0;
            else if (m_tab.m_activeTabText == CString("Lights"))
                activePage = 1;
            else if (m_tab.m_activeTabText == CString("Physics"))
                activePage = 2;
            else if (m_tab.m_activeTabText == CString("Sound"))
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
    case eItemBall:
    {
        m_elementTypeName.SetWindowText("Ball");
        m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new BallVisualsProperty(&pvsel), _T("Visuals")));
        m_tabs[1] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new BallPhysicsProperty(&pvsel), _T("Physics")));
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
        activePage = 0;
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
        m_isPlayfieldMesh = pvsel.size() == 1 && ((Primitive *)psel)->IsPlayfield();
        m_elementTypeName.SetWindowText(m_isPlayfieldMesh ? "Playfield Primitive" : "Primitive");
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
        const ItemTypeEnum itemType = dpoint->GetIEditable()->GetItemType();
        if (itemType == eItemRamp)
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALSWHEIGHT, &pvsel), _T("Visuals")));
        else if (itemType == eItemLight || itemType == eItemTrigger)
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALS, &pvsel), _T("Visuals")));
        else
            m_tabs[0] = static_cast<BasePropertyDialog*>(m_tab.AddTabPage(new DragpointVisualsProperty(IDD_PROPPOINT_VISUALSWTEX, &pvsel), _T("Visuals")));
        activePage = 0;
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
        if (m_tabs[i] != nullptr)
        {
            m_tab.RemoveTabPage(m_tab.GetTabIndex(m_tabs[i]));
            m_tabs[i] = nullptr;
        }
    m_previousType = (ItemTypeEnum)0;
    m_backglassView = false;
}

void PropertyDialog::UpdateTextureComboBox(const vector<Texture *>& contentList, const CComboBox &combo, const string &selectName)
{
    bool need_reset = combo.GetCount() != (int)contentList.size() + 1; // Not the same number of items
    need_reset |= combo.FindStringExact(1, selectName.c_str()) == CB_ERR; // Selection is not part of combo
    if (!need_reset)
    {
        bool texelFound = false;
        for (const auto texel : contentList)
        {
            if (strncmp(texel->m_szName.c_str(), selectName.c_str(), MAXTOKEN) == 0) //!! lstrcmpi?
                texelFound = true;
            need_reset |= combo.FindStringExact(1, texel->m_szName.c_str()) == CB_ERR; // Combo does not contain an image from the image list
        }
        need_reset |= !texelFound; // Selection is not part of image list
    }
    if (need_reset)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < contentList.size(); i++)
            combo.AddString(contentList[i]->m_szName.c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void PropertyDialog::UpdateMaterialComboBox(const vector<Material *>& contentList, const CComboBox &combo, const string &selectName)
{
    bool need_reset = combo.GetCount() != (int)contentList.size() + 1; // Not the same number of items
    need_reset |= combo.FindStringExact(1, selectName.c_str()) == CB_ERR; // Selection is not part of combo
    if (!need_reset)
    {
       bool matFound = false;
       for (const auto mat : contentList)
       {
           if (mat->m_szName==selectName)
           {
               matFound = true;
               break;
           }
       }
       need_reset |= !matFound; // Selection is not part of the list
    }
    if (need_reset)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < contentList.size(); i++)
            combo.AddString(contentList[i]->m_szName.c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void PropertyDialog::UpdateSurfaceComboBox(const PinTable * const ptable, const CComboBox &combo, const string& selectName)
{
    if(combo.FindStringExact(1, selectName.c_str()) == CB_ERR)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < ptable->m_vedit.size(); i++)
        {
            if (ptable->m_vedit[i]->GetItemType() == eItemSurface || (ptable->m_vedit[i]->GetItemType() == eItemRamp) ||
                //!! **************** warning **********************
                // added to render to surface of DMD style lights and emreels
                // but no checks are being performed at moment:
                (ptable->m_vedit[i]->GetItemType() == eItemFlasher))
            {
                combo.AddString(ptable->GetElementName(ptable->m_vedit[i]));
            }
        }
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void PropertyDialog::UpdateSoundComboBox(const PinTable *const ptable, const CComboBox &combo, const string& selectName)
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

void PropertyDialog::UpdateCollectionComboBox(const PinTable *const ptable, const CComboBox &combo, const char *selectName)
{
    if(combo.FindStringExact(1, selectName)==CB_ERR)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (int i = 0; i < ptable->m_vcollection.size(); i++)
        {
            char szT[sizeof(ptable->m_vcollection[i].m_wzName)/sizeof(ptable->m_vcollection[i].m_wzName[0])];
            WideCharToMultiByteNull(CP_ACP, 0, ptable->m_vcollection[i].m_wzName, -1, szT, sizeof(szT), nullptr, nullptr);
            combo.AddString(szT);
        }
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateComboBox(const vector<string>& contentList, const CComboBox &combo, const string& selectName)
{
    bool strFound = false;
    for (const auto& str : contentList)
    {
        if (str == selectName)
        {
            strFound = true;
            break;
        }
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
   // Table is locked: just disable property pane
   if (g_pvp->m_ptableActive && g_pvp->m_ptableActive->IsLocked())
   {
      m_multipleElementsStatic.ShowWindow(SW_HIDE);
      m_nameEdit.ShowWindow(SW_HIDE);
      m_elementTypeName.ShowWindow(SW_HIDE);
      m_tab.ShowWindow(SW_HIDE);
      while (m_tab.GetItemCount() > 0)
         m_tab.RemoveTabPage(0);
      memset(m_tabs, 0, sizeof(m_tabs));
      m_previousType = eItemTypeCount;
      return;
   }

   // Invalid selection: discard update
   ISelect *const psel = pvsel.ElementAt(0);
   if (psel == nullptr)
      return;

   ShowWindow(SW_HIDE);
   m_multipleElementsStatic.ShowWindow();
   m_nameEdit.ShowWindow();
   m_elementTypeName.ShowWindow();
   m_tab.ShowWindow();

   const bool is_playfield_mesh = psel->GetItemType() == eItemPrimitive && ((Primitive *)psel)->IsPlayfield();
   if (m_previousType != psel->GetItemType() || m_isPlayfieldMesh != is_playfield_mesh || m_backglassView != g_pvp->m_backglassView || m_multipleElementsStatic.IsWindowVisible())
   {
      BasePropertyDialog::m_disableEvents = true;
      m_curTabIndex = m_tab.GetCurSel();
      while (m_tab.GetItemCount() > 0)
         m_tab.RemoveTabPage(0);
      memset(m_tabs, 0, sizeof(m_tabs));

        for (int i = 0; i < pvsel.size(); i++)
        {
            // check for multiple selection
            if (psel->GetItemType() != pvsel.ElementAt(i)->GetItemType())
            {
                m_multipleElementsStatic.ShowWindow(SW_SHOW);
                m_nameEdit.ShowWindow(SW_HIDE);
                m_elementTypeName.ShowWindow(SW_HIDE);
                m_tab.ShowWindow(SW_HIDE);
                ShowWindow();
                BasePropertyDialog::m_disableEvents = false;
                return;
            }
        }

        if (m_multipleElementsStatic.IsWindowVisible())
        {
           m_multipleElementsStatic.ShowWindow(SW_HIDE);
           m_nameEdit.ShowWindow(SW_SHOW);
           m_elementTypeName.ShowWindow(SW_SHOW);
           m_tab.ShowWindow(SW_SHOW);
        }

        CreateTabs(pvsel);
    }


    if (pvsel.size() > 1)
    {
        char header[64];
        char collection[64] = {0};
        char name[64];
        const WCHAR * const wzName = psel->GetPTable()->GetCollectionNameByElement(psel);
        if (wzName != nullptr)
        {
            WideCharToMultiByteNull(CP_ACP, 0, wzName, -1, collection, 64, nullptr, nullptr);
        }

        CComBSTR bstr;
        psel->GetTypeName(&bstr);
        WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, name, 64, nullptr, nullptr);

        if (collection[0] != '\0')
            sprintf_s(header, sizeof(header), "%s [%s](%d)", collection, name, pvsel.size());
        else
            sprintf_s(header, sizeof(header), "%s(%d)", name, pvsel.size());

        m_nameEdit.SetWindowText(header);
        m_nameEdit.SetReadOnly();
    }
    else
    {
        m_nameEdit.SetWindowText(psel->GetPTable()->GetElementName(psel->GetIEditable()));
        m_nameEdit.SetReadOnly(0);
    }

    m_nameEdit.EnableWindow(psel->GetItemType() != eItemLightCenter); // Cannot rename light center

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
      //!! also grab VK_ESCAPE here to avoid weird results when pressing ESC in textboxes (property gets stuck then)
      if((keyPressed>=VK_F1 && keyPressed<=VK_F12) && TranslateAccelerator(g_pvp->GetHwnd(), m_accel, msg))
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

    //set minimize size of the resizer at which scrollbars are shown when going under the given size
    m_resizer.Initialize(*this, CRect(0, 0, 200, 800)); 
    m_resizer.AddChild(m_elementTypeName, CResizer::topcenter, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_nameEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_multipleElementsStatic, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tab, CResizer::topcenter, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);

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
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
            {
               const CString className = GetFocus().GetClassName();
               // filter ESC-key otherwise VPX will enter an endless event loop!?
               if (className == "Edit" || className == "msctls_trackbar32" || className=="Button")
                  return TRUE;
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
            if (m_tabs[0] && m_tabs[0]->m_pvsel->ElementAt(0) != nullptr)
            {
                m_tabs[0]->m_pvsel->ElementAt(0)->GetIEditable()->SetName(m_nameEdit.GetWindowText().c_str());
                m_nameEdit.SetWindowText(m_tabs[0]->m_pvsel->ElementAt(0)->GetIEditable()->GetName()); // set it again in case it was truncated
            }
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
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        ISelect* const el = m_pvsel->ElementAt(i);
        if (el == nullptr)
            continue;

        switch (el->GetItemType())
        {
            case eItemSurface:
            {
                Surface * const wall = (Surface *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(wall->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), wall);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(wall->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), wall);
                break;
            }
            case eItemLight:
            {
                Light * const light = (Light *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(light->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), light);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(light->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), light);
                break;
            }
            case eItemFlasher:
            {
                Flasher * const flash = (Flasher *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(flash->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), flash);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(flash->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), flash);
                break;
            }
            case eItemRubber:
            {
                Rubber * const rubber = (Rubber *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(rubber->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), rubber);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(rubber->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), rubber);
                break;
            }
            case eItemBumper:
            {
                Bumper * const bumper = (Bumper *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(bumper->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), bumper);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(bumper->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), bumper);
                break;
            }
            case eItemPlunger:
            {
                Plunger * const plunger = (Plunger *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(plunger->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), plunger);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(plunger->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), plunger);
                break;
            }
            case eItemSpinner:
            {
                Spinner * const spinner = (Spinner *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(spinner->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), spinner);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(spinner->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), spinner);
                break;
            }
            case eItemTimer:
            {
                Timer * const timer = (Timer *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(timer->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), timer);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(timer->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), timer);
                break;
            }
            case eItemHitTarget:
            {
                HitTarget * const target= (HitTarget *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(target->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), target);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(target->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), target);
                break;
            }
            case eItemTrigger:
            {
                Trigger * const trigger = (Trigger *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(trigger->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), trigger);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(trigger->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), trigger);
                break;
            }
            case eItemKicker:
            {
                Kicker * const kicker = (Kicker *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(kicker->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), kicker);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(kicker->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), kicker);
                break;
            }
            case eItemRamp:
            {
                Ramp * const ramp = (Ramp *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(ramp->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), ramp);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(ramp->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), ramp);
                break;
            }
            case eItemFlipper:
            {
                Flipper * const flipper = (Flipper *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(flipper->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), flipper);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(flipper->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), flipper);
                break;
            }
            case eItemGate:
            {
                Gate * const gate = (Gate *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(gate->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), gate);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(gate->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), gate);
                break;
            }
            case eItemTextbox:
            {
                Textbox * const text = (Textbox *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(text->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), text);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(text->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), text);
                break;
            }
            case eItemDispReel:
            {
                DispReel * const reel = (DispReel *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(reel->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), reel);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(reel->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), reel);
                break;
            }
            case eItemLightSeq:
            {
                LightSeq * const lightseq = (LightSeq *)el;
                if (dispid == DISPID_Timer_Interval)
                   CHECK_UPDATE_ITEM(lightseq->m_d.m_tdr.m_TimerInterval, PropertyDialog::GetIntTextbox(m_timerIntervalEdit), lightseq);

                /*TODO: uservalue is missing due to VARIANT handling*/

                if (dispid == DISPID_Timer_Enabled)
                   CHECK_UPDATE_ITEM(lightseq->m_d.m_tdr.m_TimerEnabled, PropertyDialog::GetCheckboxState(GetDlgItem(DISPID_Timer_Enabled).GetHwnd()), lightseq);
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
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        ISelect* const el = m_pvsel->ElementAt(i);
        if (el == nullptr)
            continue;

        switch (el->GetItemType())
        {
            case eItemSurface:
            {
                const Surface * const wall = (Surface *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, wall->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), wall->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemLight:
            {
                const Light * const light = (Light *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, light->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), light->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemFlasher:
            {
                const Flasher * const flash = (Flasher *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, flash->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), flash->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemRubber:
            {
                const Rubber * const rubber = (Rubber *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, rubber->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), rubber->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemBumper:
            {
                const Bumper * const bumper = (Bumper *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, bumper->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), bumper->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemPlunger:
            {
                const Plunger * const plunger = (Plunger *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, plunger->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), plunger->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemSpinner:
            {
                const Spinner * const spinner = (Spinner *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, spinner->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), spinner->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTimer:
            {
                const Timer * const timer = (Timer *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, timer->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), timer->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemHitTarget:
            {
                const HitTarget * const target = (HitTarget *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, target->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), target->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTrigger:
            {
                const Trigger * const trigger = (Trigger *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, trigger->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), trigger->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemKicker:
            {
                const Kicker * const kicker = (Kicker *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, kicker->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), kicker->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemRamp:
            {
                const Ramp * const ramp = (Ramp *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, ramp->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), ramp->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemFlipper:
            {
                const Flipper * const flipper = (Flipper *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, flipper->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), flipper->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemGate:
            {
                const Gate * const gate = (Gate *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, gate->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), gate->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemTextbox:
            {
                const Textbox * const text = (Textbox *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, text->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), text->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemDispReel:
            {
                const DispReel * const reel = (DispReel *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, reel->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), reel->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            case eItemLightSeq:
            {
                const LightSeq * const lightseq = (LightSeq *)el;
                PropertyDialog::SetIntTextbox(m_timerIntervalEdit, lightseq->m_d.m_tdr.m_TimerInterval);

                /*TODO: uservalue is missing due to VARIANT handling*/
                PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), DISPID_Timer_Enabled), lightseq->m_d.m_tdr.m_TimerEnabled);
                break;
            }
            default:
                break;
        }
    }
}

BOOL TimerProperty::OnInitDialog()
{
    m_timerIntervalEdit.AttachItem(DISPID_Timer_Interval);
    m_userValueEdit.AttachItem(DISPID_UserValue);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(m_timerIntervalEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_userValueEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(DISPID_Timer_Enabled), CResizer::topleft, 0);

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

INT_PTR TimerProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}

#pragma endregion

#pragma region BasePropertyDialog

void BasePropertyDialog::UpdateBaseProperties(ISelect *psel, BaseProperty *property, const int dispid)
{
    if (!property || psel==nullptr)
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
    if (m_basePhysicsMaterialCombo && (dispid == IDC_MATERIAL_COMBO4 || dispid == -1))
        PropertyDialog::UpdateMaterialComboBox(psel->GetPTable()->GetMaterialList(), *m_basePhysicsMaterialCombo, property->m_szPhysicsMaterial);
    if (m_hOverwritePhysicsCheck && (dispid == IDC_OVERWRITE_MATERIAL_SETTINGS || dispid == -1))
        PropertyDialog::SetCheckboxState(m_hOverwritePhysicsCheck, property->m_overwritePhysics);
    if (m_baseMaterialCombo && (dispid == IDC_MATERIAL_COMBO || dispid == -1))
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
