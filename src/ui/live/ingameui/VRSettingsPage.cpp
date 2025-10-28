// license:GPLv3+

#include "core/stdafx.h"

#include "VRSettingsPage.h"
#include "renderer/VRDevice.h"

namespace VPX::InGameUI
{

VRSettingsPage::VRSettingsPage()
   : InGameUIPage("Virtual Reality Settings"s, ""s, SaveMode::Both)
{
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_Orientation, 1.f, "%4.1f °"s, //
      [this]() { return m_player->m_vrDevice->GetSceneOrientation(); }, //
      [this](float, float v) { m_player->m_vrDevice->SetSceneOrientation(v); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_TableX, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetSceneOffset().x; }, //
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.x = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_TableY, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetSceneOffset().y; }, //
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.y = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_TableZ, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetSceneOffset().z; }, //
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetSceneOffset();
         offset.z = v;
         m_player->m_vrDevice->SetSceneOffset(offset);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarWidth, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetLockbarWidth(); }, //
      [this](float, float v) { m_player->m_vrDevice->SetLockbarWidth(v); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_UsePassthroughColor, //
      [this]() { return m_player->m_renderer->m_vrApplyColorKey; }, //
      [this](bool v) { m_player->m_renderer->m_vrApplyColorKey = v; }));

   // TODO it would be nice to implement a pincab friendly color picker
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Color Key Red"s, ""s, 0, 255, 128), "%3d / 255"s, //
      [this]() { return static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.x) * 255.f); }, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayerVR_PassthroughColor() & 0xFF; }, //
      [this](int, int v) { m_player->m_renderer->m_vrColorKey.x = InvsRGB(static_cast<float>(v) / 255.f); }, //
      [](Settings& settings) { settings.ResetPlayerVR_PassthroughColor(); }, // we reset the 3 channels at once
      [](int v, Settings& settings, bool isTableOverride) { settings.SetPlayerVR_PassthroughColor((settings.GetPlayerVR_PassthroughColor() & 0xFFFF00) | v, isTableOverride); }));

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Color Key Green"s, ""s, 0, 255, 128), "%3d / 255"s, //
      [this]() { return static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.y) * 255.f); }, //
      [this]() { return (m_player->m_ptable->m_settings.GetPlayerVR_PassthroughColor() >> 8) & 0xFF; }, //
      [this](int, int v) { m_player->m_renderer->m_vrColorKey.y = InvsRGB(static_cast<float>(v) / 255.f); }, //
      [](Settings& settings) { settings.ResetPlayerVR_PassthroughColor(); }, // we reset the 3 channels at once
      [](int v, Settings& settings, bool isTableOverride) { settings.SetPlayerVR_PassthroughColor((settings.GetPlayerVR_PassthroughColor() & 0xFF00FF) | (v << 8), isTableOverride); }));

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Color Key Blue"s, ""s, 0, 255, 128), "%3d / 255"s, //
      [this]() { return static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.z) * 255.f); }, //
      [this]() { return (m_player->m_ptable->m_settings.GetPlayerVR_PassthroughColor() >> 16) & 0xFF; }, //
      [this](int, int v) { m_player->m_renderer->m_vrColorKey.z = InvsRGB(static_cast<float>(v) / 255.f); }, //
      [](Settings& settings) { settings.ResetPlayerVR_PassthroughColor(); }, // we reset the 3 channels at once
      [](int v, Settings& settings, bool isTableOverride) { settings.SetPlayerVR_PassthroughColor((settings.GetPlayerVR_PassthroughColor() & 0x00FFFF) | (v << 16), isTableOverride); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_VRPreview, //
      [this]() { return static_cast<int>(m_player->m_renderer->m_vrPreview); }, //
      [this](int, int v) { m_player->m_renderer->m_vrPreview = static_cast<VRPreviewMode>(v); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_ShrinkPreview, //
      [this]() { return m_player->m_renderer->m_vrPreviewShrink; }, //
      [this](bool v) { m_player->m_renderer->m_vrPreviewShrink = v; }));
}

void VRSettingsPage::ResetToDefaults()
{
#ifdef ENABLE_XR
   // Recentering the table is asynchronous so define defaults as the last acquired values (not perfect but fine enough for user feedback)
   Settings::GetRegistry().Register(Settings::GetPlayerVR_Orientation_Property()->WithDefault(m_player->m_vrDevice->GetSceneOrientation()));
   Settings::GetRegistry().Register(Settings::GetPlayerVR_TableX_Property()->WithDefault(m_player->m_vrDevice->GetSceneOffset().x));
   Settings::GetRegistry().Register(Settings::GetPlayerVR_TableY_Property()->WithDefault(m_player->m_vrDevice->GetSceneOffset().y));
   Settings::GetRegistry().Register(Settings::GetPlayerVR_TableZ_Property()->WithDefault(m_player->m_vrDevice->GetSceneOffset().z));
#endif
   InGameUIPage::ResetToDefaults();
   m_player->m_vrDevice->RecenterTable();
}

}
