// license:GPLv3+

#include "core/stdafx.h"

#include "VRSettingsPage.h"
#include "renderer/VRDevice.h"

namespace VPX::InGameUI
{

VRSettingsPage::VRSettingsPage()
   : InGameUIPage("Virtual Reality Settings"s, ""s, SaveMode::Both)
{
   auto orientation = std::make_unique<InGameUIItem>(
      "View orientation"s, ""s, -180.f, 180.f, 0.1f, 0.f, "%4.1f °"s,
      [this]() { return m_player->m_vrDevice->GetSceneOrientation(); }, 
      [this](float, float v) { m_player->m_vrDevice->SetSceneOrientation(v); }, 
      InGameUIItem::ResetSetting(Settings::PlayerVR, "Orientation"s),
      InGameUIItem::SaveSettingFloat(Settings::PlayerVR, "Orientation"s));
   AddItem(orientation);

   auto offsetX = std::make_unique<InGameUIItem>(
      "View Offset X"s, ""s, -100.f, 100.f, 0.1f, 0.f, "%4.1f cm"s,
      [this]() { return m_player->m_vrDevice->GetSceneOffset().x; },
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.x = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "TableX"s),
      InGameUIItem::SaveSettingFloat(Settings::PlayerVR, "TableX"s));
   AddItem(offsetX);

   auto offsetY = std::make_unique<InGameUIItem>(
      "View Offset Y"s, ""s, -100.f, 100.f, 0.1f, 0.f, "%4.1f cm"s,
      [this]() { return m_player->m_vrDevice->GetSceneOffset().y; },
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.y = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "TableY"s),
      InGameUIItem::SaveSettingFloat(Settings::PlayerVR, "TableY"s));
   AddItem(offsetY);

   auto offsetZ = std::make_unique<InGameUIItem>(
      "View Offset Z"s, ""s, -100.f, 100.f, 0.1f, 0.f, "%4.1f cm"s,
      [this]() { return m_player->m_vrDevice->GetSceneOffset().z; },
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.z = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "TableZ"s),
      InGameUIItem::SaveSettingFloat(Settings::PlayerVR, "TableZ"s));
   AddItem(offsetZ);

   auto lockbarWidth = std::make_unique<InGameUIItem>(
      "Lockbar Width"s, ""s, 10.f, 150.f, 0.1f, 70.f, "%4.1f cm"s,
      [this]() { return m_player->m_vrDevice->GetLockbarWidth(); },
      [this](float, float v) { m_player->m_vrDevice->SetLockbarWidth(v); },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "LockbarWidth"s),
      InGameUIItem::SaveSettingFloat(Settings::PlayerVR, "LockbarWidth"s));
   AddItem(lockbarWidth);

   auto arVr = std::make_unique<InGameUIItem>(
      "Color Keyed Passthrough"s, ""s, false,
      [this]() { return m_player->m_renderer->m_vrApplyColorKey; },
      [this](bool v) { m_player->m_renderer->m_vrApplyColorKey = v; },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "UsePassthroughColor"s),
      InGameUIItem::SaveSettingBool(Settings::PlayerVR, "UsePassthroughColor"s));
   AddItem(arVr);

   // TODO it would be nice to implement a pincab friendly color picker ...
   auto colorKeyRed = std::make_unique<InGameUIItem>(
      "Color Key Red"s, ""s, 0, 255, 128, "%3d / 255"s,
      [this]() { return static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.x) * 255.f); },
      [this](int, int v) { m_player->m_renderer->m_vrColorKey.x = InvsRGB(static_cast<float>(v) / 255.f); },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "PassthroughColor"s), // we reset the 3 channels at once
      [](int v, Settings& settings, bool isTableOverride)
      {
         int rgb = settings.LoadValueInt(Settings::PlayerVR, "PassthroughColor"s);
         rgb = (rgb & 0xFFFF00) | v;
         settings.SaveValue(Settings::PlayerVR, "PassthroughColor"s, rgb, isTableOverride);
      });
   AddItem(colorKeyRed);

   auto colorKeyGreen = std::make_unique<InGameUIItem>(
      "Color Key Green"s, ""s, 0, 255, 128, "%3d / 255"s,
      [this]() { return static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.y) * 255.f); },
      [this](int, int v) { m_player->m_renderer->m_vrColorKey.y = InvsRGB(static_cast<float>(v) / 255.f); },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "PassthroughColor"s), // we reset the 3 channel at once
      [](int v, Settings& settings, bool isTableOverride)
      {
         int rgb = settings.LoadValueInt(Settings::PlayerVR, "PassthroughColor"s);
         rgb = (rgb & 0xFF00FF) | (v << 8);
         settings.SaveValue(Settings::PlayerVR, "PassthroughColor"s, rgb, isTableOverride);
      });
   AddItem(colorKeyGreen);

   auto colorKeyBlue = std::make_unique<InGameUIItem>(
      "Color Key Blue"s, ""s, 0, 255, 128, "%3d / 255"s,
      [this]() { return static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.z) * 255.f); },
      [this](int, int v) { m_player->m_renderer->m_vrColorKey.z = InvsRGB(static_cast<float>(v) / 255.f); },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "PassthroughColor"s), // we reset the 3 channel at once
      [](int v, Settings& settings, bool isTableOverride)
      {
         int rgb = settings.LoadValueInt(Settings::PlayerVR, "PassthroughColor"s);
         rgb = (rgb & 0x00FFFF) | (v << 16);
         settings.SaveValue(Settings::PlayerVR, "PassthroughColor"s, rgb, isTableOverride);
      });
   AddItem(colorKeyBlue);

   auto previewMode = std::make_unique<InGameUIItem>(
      "Preview mode"s, "Select preview mode"s, vector({ "Disabled"s, "Left Eye"s, "Right Eye"s, "Both Eyes"s }), static_cast<int>(VRPreviewMode::VRPREVIEW_LEFT),
      [this]() { return static_cast<int>(m_player->m_renderer->m_vrPreview); },
      [this](int, int v) { m_player->m_renderer->m_vrPreview = static_cast<VRPreviewMode>(v); },
      InGameUIItem::ResetSetting(Settings::Player, "VRPreview"s),
      InGameUIItem::SaveSettingInt(Settings::Player, "VRPreview"s));
   AddItem(previewMode);

   auto shrinkPreview = std::make_unique<InGameUIItem>(
      "Shrink preview"s, ""s, false,
      [this]() { return m_player->m_renderer->m_vrPreviewShrink; },
      [this](bool v) { m_player->m_renderer->m_vrPreviewShrink = v; },
      InGameUIItem::ResetSetting(Settings::PlayerVR, "ShrinkPreview"s),
      InGameUIItem::SaveSettingBool(Settings::PlayerVR, "ShrinkPreview"s));
   AddItem(shrinkPreview);
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
