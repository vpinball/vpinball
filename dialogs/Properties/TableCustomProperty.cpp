#include "stdafx.h"
#include "Properties/TableCustomProperty.h"
#include <WindowsX.h>

TableCustomProperty::TableCustomProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_USER, pvsel)
{
    m_userList.push_back("Default"); // -1
    m_userList.push_back("Off"); // 0
    m_userList.push_back("On"); // 1

    m_physicSetList.push_back("Disable");
    m_physicSetList.push_back("Set1");
    m_physicSetList.push_back("Set2");
    m_physicSetList.push_back("Set3");
    m_physicSetList.push_back("Set4");
    m_physicSetList.push_back("Set5");
    m_physicSetList.push_back("Set6");
    m_physicSetList.push_back("Set7");
    m_physicSetList.push_back("Set8");

    m_gameplayDifficultEdit.SetDialog(this);
    m_fpsLimiterEdit.SetDialog(this);
    m_ballTrailStrengthEdit.SetDialog(this);
    m_soundEffectVolEdit.SetDialog(this);
    m_musicVolEdit.SetDialog(this);

    m_SSAACombo.SetDialog(this);
    m_inGameAOCombo.SetDialog(this);
    m_ScreenReflectionCombo.SetDialog(this);
    m_ballReflectionCombo.SetDialog(this);
    m_ballTrailCombo.SetDialog(this);
    m_overwritePhysicsSetCombo.SetDialog(this);
}

void TableCustomProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;

    if (dispid == IDC_ENABLE_AA || dispid == -1)
        PropertyDialog::UpdateComboBox(m_userList, m_SSAACombo, m_userList[table->m_useAA + 1]);
    if (dispid == IDC_ENABLE_AO || dispid == -1)
        PropertyDialog::UpdateComboBox(m_userList, m_inGameAOCombo, m_userList[table->m_useAO + 1]);
    if (dispid == IDC_ENABLE_SSR || dispid == -1)
        PropertyDialog::UpdateComboBox(m_userList, m_ScreenReflectionCombo, m_userList[table->m_useSSR + 1]);
    
    m_detailLevelSlider.SetPos(table->GetDetailLevel(), 1);

    if (dispid == IDC_GLOBAL_ALPHA_ACC || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hOverwriteDetailsCheck, table->m_overwriteGlobalDetailLevel);
    if (dispid == IDC_BALL_REFLECTION || dispid == -1)
        PropertyDialog::UpdateComboBox(m_userList, m_ballReflectionCombo, m_userList[(int)(table->m_useReflectionForBalls) + 1]);
    if (dispid == IDC_BALL_TRAIL || dispid == -1)
        PropertyDialog::UpdateComboBox(m_userList, m_ballTrailCombo, m_userList[(int)(table->m_useTrailForBalls) + 1]);
    if (dispid == IDC_TRAIL_EDIT || dispid == -1)
        PropertyDialog::SetIntTextbox(m_ballTrailStrengthEdit, table->GetBallTrailStrength());

    m_nightDaySlider.SetPos(table->GetGlobalEmissionScale(), 1);

    if (dispid == IDC_GLOBAL_DAYNIGHT || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hOverwriteNightDayCheck, table->m_overwriteGlobalDayNight);
    if (dispid == IDC_OVERWRITE_PHYSICS_COMBO || dispid == -1)
        PropertyDialog::UpdateComboBox(m_physicSetList, m_overwritePhysicsSetCombo, m_physicSetList[(int)table->m_overridePhysics]);
    if (dispid == IDC_GAME_DIFFICULTY_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_gameplayDifficultEdit, table->GetGlobalDifficulty());
    if (dispid == IDC_OVERRIDEPHYSICS_FLIPPERS || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hOverwriteFlipperCheck, table->m_overridePhysicsFlipper);
    if (dispid == IDC_TABLESOUNDVOLUME || dispid == -1)
        PropertyDialog::SetIntTextbox(m_soundEffectVolEdit, table->GetTableSoundVolume());
    if (dispid == IDC_TABLEMUSICVOLUME || dispid == -1)
        PropertyDialog::SetIntTextbox(m_musicVolEdit, table->GetTableMusicVolume());
    if (dispid == IDC_TABLEAVSYNC || dispid == -1)
        PropertyDialog::SetIntTextbox(m_fpsLimiterEdit, table->m_TableAdaptiveVSync);

    m_detailLevelSlider.EnableWindow(table->m_overwriteGlobalDetailLevel);
    m_nightDaySlider.EnableWindow(table->m_overwriteGlobalDayNight);
}

void TableCustomProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;
    
    switch (dispid)
    {
        case IDC_ENABLE_AA:
            CHECK_UPDATE_ITEM(table->m_useAA, (PropertyDialog::GetComboBoxIndex(m_SSAACombo, m_userList) - 1), table);
            break;
        case IDC_ENABLE_AO:
            CHECK_UPDATE_ITEM(table->m_useAO, (PropertyDialog::GetComboBoxIndex(m_inGameAOCombo, m_userList) - 1), table);
            break;
        case IDC_ENABLE_SSR:
            CHECK_UPDATE_ITEM(table->m_useSSR, (PropertyDialog::GetComboBoxIndex(m_ScreenReflectionCombo, m_userList) - 1), table);
            break;
        case IDC_GLOBAL_ALPHA_ACC:
            CHECK_UPDATE_ITEM(table->m_overwriteGlobalDetailLevel, PropertyDialog::GetCheckboxState(m_hOverwriteDetailsCheck), table);
            break;
        case IDC_BALL_REFLECTION:
            CHECK_UPDATE_ITEM(table->m_useReflectionForBalls, (PropertyDialog::GetComboBoxIndex(m_ballReflectionCombo, m_userList) - 1), table);
            break;
        case IDC_BALL_TRAIL:
            CHECK_UPDATE_ITEM(table->m_useTrailForBalls, (PropertyDialog::GetComboBoxIndex(m_ballTrailCombo, m_userList) - 1), table);
            break;
        case IDC_TRAIL_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->SetBallTrailStrength, table->GetBallTrailStrength, PropertyDialog::GetIntTextbox, m_ballTrailStrengthEdit, table);
            break;
        case IDC_GLOBAL_DAYNIGHT:
            CHECK_UPDATE_ITEM(table->m_overwriteGlobalDayNight, PropertyDialog::GetCheckboxState(m_hOverwriteNightDayCheck), table);
            break;
        case IDC_GAME_DIFFICULTY_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->SetGlobalDifficulty, table->GetGlobalDifficulty, PropertyDialog::GetFloatTextbox, m_gameplayDifficultEdit, table);
            break;
        case IDC_OVERWRITE_PHYSICS_COMBO:
            CHECK_UPDATE_ITEM(table->m_overridePhysics, (PropertyDialog::GetComboBoxIndex(m_overwritePhysicsSetCombo, m_physicSetList)), table);
            break;
        case IDC_OVERRIDEPHYSICS_FLIPPERS:
            CHECK_UPDATE_ITEM(table->m_overridePhysicsFlipper, PropertyDialog::GetCheckboxState(m_hOverwriteFlipperCheck), table);
            break;
        case IDC_TABLESOUNDVOLUME:
            CHECK_UPDATE_VALUE_SETTER(table->SetTableSoundVolume, table->GetTableSoundVolume, PropertyDialog::GetIntTextbox, m_soundEffectVolEdit, table);
            break;
        case IDC_TABLEMUSICVOLUME:
            CHECK_UPDATE_VALUE_SETTER(table->SetTableMusicVolume, table->GetTableMusicVolume, PropertyDialog::GetIntTextbox, m_musicVolEdit, table);
            break;
        case IDC_TABLEAVSYNC:
            CHECK_UPDATE_ITEM(table->m_TableAdaptiveVSync, PropertyDialog::GetIntTextbox(m_fpsLimiterEdit), table);
            break;
        case IDC_ALPHA_SLIDER:
            CHECK_UPDATE_ITEM(table->m_userDetailLevel, m_detailLevelSlider.GetPos(), table);
            break;
        case IDC_DAYNIGHT_SLIDER:
        {
            const int emission = table->GetGlobalEmissionScale();
            const int newValue = m_nightDaySlider.GetPos();
            if (newValue != emission)
                table->SetGlobalEmissionScale(newValue);
            break;
        }
        default:
            break;
    }

    UpdateVisuals(dispid);
}

BOOL TableCustomProperty::OnInitDialog()
{
    m_SSAACombo.AttachItem(IDC_ENABLE_AA);
    m_inGameAOCombo.AttachItem(IDC_ENABLE_AO);
    m_ScreenReflectionCombo.AttachItem(IDC_ENABLE_SSR);
    m_fpsLimiterEdit.AttachItem(IDC_TABLEAVSYNC);
    AttachItem(IDC_ALPHA_SLIDER, m_detailLevelSlider);
    m_hOverwriteDetailsCheck = ::GetDlgItem(GetHwnd(), IDC_GLOBAL_ALPHA_ACC);
    m_ballReflectionCombo.AttachItem(IDC_BALL_REFLECTION);
    m_ballTrailCombo.AttachItem(IDC_BALL_TRAIL);
    m_ballTrailStrengthEdit.AttachItem(IDC_TRAIL_EDIT);
    AttachItem(IDC_DAYNIGHT_SLIDER, m_nightDaySlider);
    m_hOverwriteNightDayCheck = ::GetDlgItem(GetHwnd(), IDC_GLOBAL_DAYNIGHT);
    m_gameplayDifficultEdit.AttachItem(IDC_GAME_DIFFICULTY_EDIT);
    m_overwritePhysicsSetCombo.AttachItem(IDC_OVERWRITE_PHYSICS_COMBO);
    m_hOverwriteFlipperCheck = ::GetDlgItem(GetHwnd(), IDC_OVERRIDEPHYSICS_FLIPPERS);
    m_soundEffectVolEdit.AttachItem(IDC_TABLESOUNDVOLUME);
    m_musicVolEdit.AttachItem(IDC_TABLEMUSICVOLUME);

    m_detailLevelSlider.SetRangeMin(0);
    m_detailLevelSlider.SetRangeMax(10);
    m_detailLevelSlider.SetTicFreq(1);
    m_detailLevelSlider.SetPageSize(1);
    m_detailLevelSlider.SetLineSize(1);
    m_detailLevelSlider.SendMessage(TBM_SETTHUMBLENGTH, 5, 0);

    m_nightDaySlider.SetRangeMin(2);
    m_nightDaySlider.SetRangeMax(100);
    m_nightDaySlider.SetTicFreq(10);
    m_nightDaySlider.SetPageSize(1);
    m_nightDaySlider.SetLineSize(1);
    m_nightDaySlider.SendMessage(TBM_SETTHUMBLENGTH, 5, 0);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(m_SSAACombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_inGameAOCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ScreenReflectionCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_fpsLimiterEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_detailLevelSlider, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hOverwriteDetailsCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_ballReflectionCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ballTrailCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ballTrailStrengthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_nightDaySlider, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hOverwriteNightDayCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_gameplayDifficultEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_overwritePhysicsSetCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hOverwriteFlipperCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_soundEffectVolEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_musicVolEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR TableCustomProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   switch (uMsg)
    {
        case WM_HSCROLL:
        {
                CComObject<PinTable> * const table = g_pvp->GetActiveTable();
                if (table->m_overwriteGlobalDetailLevel)
                    table->SetDetailLevel(m_detailLevelSlider.GetPos());
                if(table->m_overwriteGlobalDayNight)
                    table->SetGlobalEmissionScale(m_nightDaySlider.GetPos());
                return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}
