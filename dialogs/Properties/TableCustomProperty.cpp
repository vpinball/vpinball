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

    m_soundEffectVolEdit.SetDialog(this);
    m_musicVolEdit.SetDialog(this);

    m_ballReflectionCombo.SetDialog(this);
    m_overwritePhysicsSetCombo.SetDialog(this);
}

void TableCustomProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;
    
    if (dispid == IDC_BALL_REFLECTION || dispid == -1)
        PropertyDialog::UpdateComboBox(m_userList, m_ballReflectionCombo, m_userList[(int)(table->m_useReflectionForBalls) + 1]);

    m_nightDaySlider.SetPos(table->GetGlobalEmissionScale(), 1);

    if (dispid == IDC_GLOBAL_DAYNIGHT || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hOverwriteNightDayCheck, table->m_overwriteGlobalDayNight);
    if (dispid == IDC_OVERWRITE_PHYSICS_COMBO || dispid == -1)
        PropertyDialog::UpdateComboBox(m_physicSetList, m_overwritePhysicsSetCombo, m_physicSetList[(int)table->m_overridePhysics]);
    if (dispid == IDC_OVERRIDEPHYSICS_FLIPPERS || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hOverwriteFlipperCheck, table->m_overridePhysicsFlipper);
    if (dispid == IDC_TABLESOUNDVOLUME || dispid == -1)
        PropertyDialog::SetIntTextbox(m_soundEffectVolEdit, table->GetTableSoundVolume());
    if (dispid == IDC_TABLEMUSICVOLUME || dispid == -1)
        PropertyDialog::SetIntTextbox(m_musicVolEdit, table->GetTableMusicVolume());

    m_nightDaySlider.EnableWindow(table->m_overwriteGlobalDayNight);
}

void TableCustomProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;
    
    switch (dispid)
    {
        case IDC_BALL_REFLECTION:
            CHECK_UPDATE_ITEM(table->m_useReflectionForBalls, (PropertyDialog::GetComboBoxIndex(m_ballReflectionCombo, m_userList) - 1), table);
            break;
        case IDC_GLOBAL_DAYNIGHT:
            CHECK_UPDATE_ITEM(table->m_overwriteGlobalDayNight, PropertyDialog::GetCheckboxState(m_hOverwriteNightDayCheck), table);
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
    m_ballReflectionCombo.AttachItem(IDC_BALL_REFLECTION);
    AttachItem(IDC_DAYNIGHT_SLIDER, m_nightDaySlider);
    m_hOverwriteNightDayCheck = ::GetDlgItem(GetHwnd(), IDC_GLOBAL_DAYNIGHT);
    m_overwritePhysicsSetCombo.AttachItem(IDC_OVERWRITE_PHYSICS_COMBO);
    m_hOverwriteFlipperCheck = ::GetDlgItem(GetHwnd(), IDC_OVERRIDEPHYSICS_FLIPPERS);
    m_soundEffectVolEdit.AttachItem(IDC_TABLESOUNDVOLUME);
    m_musicVolEdit.AttachItem(IDC_TABLEMUSICVOLUME);

    m_nightDaySlider.SetRangeMin(2);
    m_nightDaySlider.SetRangeMax(100);
    m_nightDaySlider.SetTicFreq(10);
    m_nightDaySlider.SetPageSize(1);
    m_nightDaySlider.SetLineSize(1);
    m_nightDaySlider.SendMessage(TBM_SETTHUMBLENGTH, 5, 0);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(m_ballReflectionCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_nightDaySlider, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hOverwriteNightDayCheck, CResizer::topleft, 0);
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
         if(table->m_overwriteGlobalDayNight)
            table->SetGlobalEmissionScale(m_nightDaySlider.GetPos());
         return TRUE;
      }
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}
