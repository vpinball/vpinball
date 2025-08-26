// license:GPLv3+

#include "core/stdafx.h"

#include "NudgeSettingsPage.h"

namespace VPX::InGameUI
{

NudgeSettingsPage::NudgeSettingsPage()
   : InGameUIPage("settings/nudge", "Nudge Settings", "")
{
   Settings &settings = GetSettings();

   auto enableHwSensor = std::make_unique<InGameUIItem>("Enable hardware nudge sensor"s, ""s, false,
      [this]() {
         // FIXME we should use the live state, not the persisted one
         return m_player->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, true); },
      [this](bool v) {  
         // Apply change
      },
      [this](bool, Settings &, bool) {
         // Save
      }
      );
   AddItem(enableHwSensor);


   /* bool accEnabled = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, true);
   int accMax[]
      = { m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100), m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100) };
   int accGain[]
      = { m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainX"s, 150), m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150) };
   int accSensitivity = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "NudgeSensitivity"s, 500);
   bool accOrientationEnabled = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false); // TODO Legacy stuff => remove and only keep rotation
   int accOrientation = accOrientationEnabled ? m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 500) : 0;
   if (ImGui::InputInt("Acc. Rotation", &accOrientation))
   {
      g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationCB"s, accOrientation != 0);
      g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationValue"s, accOrientation);
      m_player->m_pininput.ReInit();
   }
   bool accFaceUp = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWNormalMount"s, true);
   bool accFilter = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, false); */

}

};