// license:GPLv3+

#include "core/stdafx.h"

#include "VRSettingsPage.h"
#include "renderer/VRDevice.h"

namespace VPX::InGameUI
{

VRSettingsPage::VRSettingsPage()
   : InGameUIPage("settings/vr"s, "Virtual Reality Settings"s, ""s, SaveMode::Both)
{
   auto orientation = std::make_unique<InGameUIItem>(
      "View orientation"s, ""s, -180.f, 180.f, 0.1f, 0.f, "%4.1f °"s, [this]() { return m_player->m_vrDevice->GetSceneOrientation(); }, 
      [this](float prev, float v) { m_player->m_vrDevice->SetSceneOrientation(v); }, 
      [](Settings& settings) { settings.DeleteValue(Settings::PlayerVR, "Orientation"s); },
      [](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::PlayerVR, "Orientation"s, v); });
   AddItem(orientation);

   auto offsetX = std::make_unique<InGameUIItem>(
      "View Offset X"s, ""s, -100.f, 100.f, 0.1f, 0.f, "%4.1f cm"s, [this]() { return m_player->m_vrDevice->GetSceneOffset().x; },
      [this](float prev, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.x = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      },
      [](Settings& settings) { settings.DeleteValue(Settings::PlayerVR, "TableX"s); },
      [](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::PlayerVR, "TableX"s, v, isTableOverride); });
   AddItem(offsetX);

   auto offsetY = std::make_unique<InGameUIItem>(
      "View Offset Y"s, ""s, -100.f, 100.f, 0.1f, 0.f, "%4.1f cm"s, [this]() { return m_player->m_vrDevice->GetSceneOffset().y; },
      [this](float prev, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.y = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      },
      [](Settings& settings) { settings.DeleteValue(Settings::PlayerVR, "TableY"s); },
      [](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::PlayerVR, "TableY"s, v, isTableOverride); });
   AddItem(offsetY);

   auto offsetZ = std::make_unique<InGameUIItem>(
      "View Offset Z"s, ""s, -100.f, 100.f, 0.1f, 0.f, "%4.1f cm"s, [this]() { return m_player->m_vrDevice->GetSceneOffset().z; },
      [this](float prev, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.z = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      },
      [](Settings& settings) { settings.DeleteValue(Settings::PlayerVR, "TableZ"s); },
      [](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::PlayerVR, "TableZ"s, v, isTableOverride); });
   AddItem(offsetZ);

   auto lockbarWidth = std::make_unique<InGameUIItem>(
      "Lockbar Width"s, ""s, 10.f, 150.f, 0.1f, 70.f, "%4.1f cm"s, [this]() { return m_player->m_vrDevice->GetLockbarWidth(); },
      [this](float prev, float v) { m_player->m_vrDevice->SetLockbarWidth(v); },
      [](Settings& settings) { settings.DeleteValue(Settings::Player, "LockbarWidth"s); },
      [](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "LockbarWidth"s, v, isTableOverride); });
   AddItem(lockbarWidth);

   auto arVr = std::make_unique<InGameUIItem>(
      "Color Keyed Passthrough"s, ""s, false, [this]() { return m_player->m_renderer->m_vrApplyColorKey; },
      [this](bool v) { m_player->m_renderer->m_vrApplyColorKey = v; },
      [](Settings& settings) { settings.DeleteValue(Settings::PlayerVR, "UsePassthroughColor"s); },
      [this](bool v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::PlayerVR, "UsePassthroughColor"s, v, isTableOverride); });
   AddItem(arVr);
}

void VRSettingsPage::ResetToDefaults()
{
   #ifdef ENABLE_XR
      // Recentering the table is asynchronous so define defaults as the last acquired values (not perfect but fine enough for user feedback)
      GetItem("View orientation"s)->SetDefaultValue(m_player->m_vrDevice->GetSceneOrientation());
      GetItem("View Offset X"s)->SetDefaultValue(m_player->m_vrDevice->GetSceneOffset().x);
      GetItem("View Offset Y"s)->SetDefaultValue(m_player->m_vrDevice->GetSceneOffset().y);
      GetItem("View Offset Z"s)->SetDefaultValue(m_player->m_vrDevice->GetSceneOffset().z);
   #endif
   InGameUIPage::ResetToDefaults();
   m_player->m_vrDevice->RecenterTable();
}

}
