#include "stdafx.h"
#include "Properties/TableCustomProperty.h"
#include <WindowsX.h>

TableCustomProperty::TableCustomProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_USER, pvsel)
{
    m_postAAList.push_back("Default");  // -1
    m_postAAList.push_back("Disabled");  // 0
    m_postAAList.push_back("Fast FXAA"); // 1
    m_postAAList.push_back("Standard FXAA"); // 2
    m_postAAList.push_back("Quality FXAA"); // 3
    m_postAAList.push_back("Fast NFAA"); // 4
    m_postAAList.push_back("Standard DLAA"); // 5
    m_postAAList.push_back("Quality SMAA"); // 6

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
}

void TableCustomProperty::UpdateVisuals()
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();

    if (table == NULL)
        return;

    PropertyDialog::UpdateComboBox(m_userList, m_SSAACombo, m_userList[table->m_useAA + 1].c_str());
    PropertyDialog::UpdateComboBox(m_postAAList, m_postProcAACombo, m_postAAList[table->m_useFXAA + 1].c_str());
    PropertyDialog::UpdateComboBox(m_userList, m_inGameAOCombo, m_userList[table->m_useAO + 1].c_str());
    PropertyDialog::UpdateComboBox(m_userList, m_ScreenReflectionCombo, m_userList[table->m_useSSR + 1].c_str());
    m_detailLevelSlider.SetPos(table->GetDetailLevel(), 1);
    PropertyDialog::SetCheckboxState(m_hOverwriteDetailsCheck, table->m_overwriteGlobalDetailLevel);
    PropertyDialog::UpdateComboBox(m_userList, m_ballReflectionCombo, m_userList[(int)(table->m_useReflectionForBalls) + 1].c_str());
    PropertyDialog::UpdateComboBox(m_userList, m_ballTrailCombo, m_userList[(int)(table->m_useTrailForBalls) + 1].c_str());
    PropertyDialog::SetIntTextbox(m_ballTrailStrengthEdit, table->GetBallTrailStrength());
    m_nightDaySlider.SetPos(table->GetGlobalEmissionScale(), 1);
    PropertyDialog::SetCheckboxState(m_hOverwriteNightDayCheck, table->m_overwriteGlobalDayNight);
    PropertyDialog::UpdateComboBox(m_physicSetList, m_overwritePhysicsSetCombo, m_physicSetList[(int)table->m_overridePhysics].c_str());
    PropertyDialog::SetFloatTextbox(m_gameplayDifficultEdit, table->GetGlobalDifficulty());
    PropertyDialog::SetCheckboxState(m_hOverwriteFlipperCheck, table->m_overridePhysicsFlipper);
    PropertyDialog::SetIntTextbox(m_soundEffectVolEdit, table->GetTableSoundVolume());
    PropertyDialog::SetIntTextbox(m_musicVolEdit, table->GetTableMusicVolume());
    PropertyDialog::SetIntTextbox(m_fpsLimiterEdit, table->m_TableAdaptiveVSync);
    m_detailLevelSlider.EnableWindow(table->m_overwriteGlobalDetailLevel);
    m_nightDaySlider.EnableWindow(table->m_overwriteGlobalDayNight);
}

void TableCustomProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();

    if (table == NULL)
        return;
    switch (dispid)
    {
        case IDC_ENABLE_AA:
            PropertyDialog::StartUndo(table);
            table->m_useAA = (PropertyDialog::GetComboBoxIndex(m_SSAACombo, m_userList) - 1);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_ENABLE_FXAA:
            PropertyDialog::StartUndo(table);
            table->m_useFXAA = (PropertyDialog::GetComboBoxIndex(m_postProcAACombo, m_postAAList) - 1);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_ENABLE_AO:
            PropertyDialog::StartUndo(table);
            table->m_useAO = (PropertyDialog::GetComboBoxIndex(m_inGameAOCombo, m_userList) - 1);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_ENABLE_SSR:
            PropertyDialog::StartUndo(table);
            table->m_useSSR = (PropertyDialog::GetComboBoxIndex(m_ScreenReflectionCombo, m_userList) - 1);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_GLOBAL_ALPHA_ACC:
            PropertyDialog::StartUndo(table);
            table->m_overwriteGlobalDetailLevel = PropertyDialog::GetCheckboxState(m_hOverwriteDetailsCheck);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_BALL_REFLECTION:
            PropertyDialog::StartUndo(table);
            table->m_useReflectionForBalls = (PropertyDialog::GetComboBoxIndex(m_ballReflectionCombo, m_userList) - 1);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_BALL_TRAIL:
            PropertyDialog::StartUndo(table);
            table->m_useTrailForBalls = (PropertyDialog::GetComboBoxIndex(m_ballTrailCombo, m_userList) - 1);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TRAIL_EDIT:
            PropertyDialog::StartUndo(table);
            table->SetBallTrailStrength(PropertyDialog::GetIntTextbox(m_ballTrailStrengthEdit));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_GLOBAL_DAYNIGHT:
            PropertyDialog::StartUndo(table);
            table->m_overwriteGlobalDayNight = PropertyDialog::GetCheckboxState(m_hOverwriteNightDayCheck);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_GAME_DIFFICULTY_EDIT:
            PropertyDialog::StartUndo(table);
            table->SetGlobalDifficulty(PropertyDialog::GetFloatTextbox(m_gameplayDifficultEdit));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_OVERWRITE_PHYSICS_COMBO:
            PropertyDialog::StartUndo(table);
            table->m_overridePhysics = (PropertyDialog::GetComboBoxIndex(m_overwritePhysicsSetCombo, m_physicSetList));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_OVERRIDEPHYSICS_FLIPPERS:
            PropertyDialog::StartUndo(table);
            table->m_overridePhysicsFlipper = PropertyDialog::GetCheckboxState(m_hOverwriteFlipperCheck);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLESOUNDVOLUME:
            PropertyDialog::StartUndo(table);
            table->SetTableSoundVolume(PropertyDialog::GetIntTextbox(m_soundEffectVolEdit));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLEMUSICVOLUME:
            PropertyDialog::StartUndo(table);
            table->SetTableSoundVolume(PropertyDialog::GetIntTextbox(m_musicVolEdit));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLEAVSYNC:
            PropertyDialog::StartUndo(table);
            table->m_TableAdaptiveVSync = PropertyDialog::GetIntTextbox(m_fpsLimiterEdit);
            PropertyDialog::EndUndo(table);
            break;
        default:
            break;
    }

    UpdateVisuals();
}

BOOL TableCustomProperty::OnInitDialog()
{
    AttachItem(IDC_ENABLE_AA, m_SSAACombo);
    AttachItem(IDC_ENABLE_FXAA, m_postProcAACombo);
    AttachItem(IDC_ENABLE_AO, m_inGameAOCombo);
    AttachItem(IDC_ENABLE_SSR, m_ScreenReflectionCombo);
    AttachItem(IDC_TABLEAVSYNC, m_fpsLimiterEdit);
    AttachItem(IDC_ALPHA_SLIDER, m_detailLevelSlider);
    m_hOverwriteDetailsCheck = ::GetDlgItem(GetHwnd(), IDC_GLOBAL_ALPHA_ACC);
    AttachItem(IDC_BALL_REFLECTION, m_ballReflectionCombo);
    AttachItem(IDC_BALL_TRAIL, m_ballTrailCombo);
    AttachItem(IDC_TRAIL_EDIT, m_ballTrailStrengthEdit);
    AttachItem(IDC_DAYNIGHT_SLIDER, m_nightDaySlider);
    m_hOverwriteNightDayCheck = ::GetDlgItem(GetHwnd(), IDC_GLOBAL_DAYNIGHT);
    AttachItem(IDC_GAME_DIFFICULTY_EDIT, m_gameplayDifficultEdit);
    AttachItem(IDC_OVERWRITE_PHYSICS_COMBO, m_overwritePhysicsSetCombo);
    m_hOverwriteFlipperCheck = ::GetDlgItem(GetHwnd(), IDC_OVERRIDEPHYSICS_FLIPPERS);
    AttachItem(IDC_TABLESOUNDVOLUME, m_soundEffectVolEdit);
    AttachItem(IDC_TABLEMUSICVOLUME, m_musicVolEdit);

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

    return TRUE;
}

INT_PTR TableCustomProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
