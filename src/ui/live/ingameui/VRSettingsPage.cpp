// license:GPLv3+

#include "core/stdafx.h"

#include "VRSettingsPage.h"
#include "renderer/VRDevice.h"
#include "parts/flasher.h"

namespace VPX::InGameUI
{

VRSettingsPage::VRSettingsPage()
   : InGameUIPage("Virtual Reality Settings"s, ""s, SaveMode::Both)
{
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "View Offset"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_Orientation, 1.f, "%4.1f deg"s, //
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

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet Layout"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarWidth, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetLockbarWidth(); }, //
      [this](float, float v) { m_player->m_vrDevice->SetLockbarWidth(v); }));

#ifdef ENABLE_BGFX
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarHeight, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetLockbarHeight(); }, //
      [this](float, float v) { m_player->m_vrDevice->SetLockbarHeight(v); }));
#endif

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerX, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerX(); }, //
      [this](float, float v)
      {
         m_notifId = m_player->m_liveUI->PushNotification("This change is directly persisted and is used when centering view"s, 5000, m_notifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenPlayerX(v, false);
      })).m_excludeFromDefault = true;

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerY, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerY(); }, //
      [this](float, float v)
      {
         m_notifId = m_player->m_liveUI->PushNotification("This change is directly persisted and is used when centering view"s, 5000, m_notifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenPlayerY(v, false);
      })).m_excludeFromDefault = true;

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Augmented Reality"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_UsePassthroughColor, //
      [this]() { return m_player->m_renderer->m_vrApplyColorKey; }, //
      [this](bool v) { m_player->m_renderer->m_vrApplyColorKey = v; }));

   // TODO it would be nice to implement a pincab friendly color picker
   m_arColorKey.r = static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.x) * 255.f);
   m_arColorKey.g = static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.y) * 255.f);
   m_arColorKey.b = static_cast<int>(sRGB(m_player->m_renderer->m_vrColorKey.z) * 255.f);
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Color Key Red"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this]() { return m_arColorKey.r; }, //
      [this](const Settings& settings) { return settings.GetPlayerVR_PassthroughColor() & 0xFF; }, //
      [this](int, int v)
      {
         m_arColorKey.r = v;
         m_player->m_renderer->m_vrColorKey.x = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [](Settings& settings) { settings.ResetPlayerVR_PassthroughColor(); }, // we reset the 3 channels at once
      [](int v, Settings& settings, bool isTableOverride) { settings.SetPlayerVR_PassthroughColor((settings.GetPlayerVR_PassthroughColor() & 0xFFFF00) | v, isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Color Key Green"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this]() { return m_arColorKey.g; }, //
      [this](const Settings& settings) { return (settings.GetPlayerVR_PassthroughColor() >> 8) & 0xFF; }, //
      [this](int, int v)
      {
         m_arColorKey.g = v;
         m_player->m_renderer->m_vrColorKey.y = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [](Settings& settings) { settings.ResetPlayerVR_PassthroughColor(); }, // we reset the 3 channels at once
      [](int v, Settings& settings, bool isTableOverride) { settings.SetPlayerVR_PassthroughColor((settings.GetPlayerVR_PassthroughColor() & 0xFF00FF) | (v << 8), isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Color Key Blue"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this]() { return m_arColorKey.b; }, //
      [this](const Settings& settings) { return (settings.GetPlayerVR_PassthroughColor() >> 16) & 0xFF; }, //
      [this](int, int v)
      {
         m_arColorKey.b = v;
         m_player->m_renderer->m_vrColorKey.z = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [](Settings& settings) { settings.ResetPlayerVR_PassthroughColor(); }, // we reset the 3 channels at once
      [](int v, Settings& settings, bool isTableOverride) { settings.SetPlayerVR_PassthroughColor((settings.GetPlayerVR_PassthroughColor() & 0x00FFFF) | (v << 16), isTableOverride); }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Miscellaneous Settings"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_AddBackglass, //
      [this]() { return m_player->m_implicitVRBackglass->m_d.m_isVisible; }, //
      [this](bool v) { m_player->m_implicitVRBackglass->m_d.m_isVisible = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_VRPreview, //
      [this]() { return static_cast<int>(m_player->m_renderer->m_vrPreview); }, //
      [this](int, int v) { m_player->m_renderer->m_vrPreview = static_cast<VRPreviewMode>(v); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_ShrinkPreview, //
      [this]() { return m_player->m_renderer->m_vrPreviewShrink; }, //
      [this](bool v) { m_player->m_renderer->m_vrPreviewShrink = v; }));

#ifdef ENABLE_XR
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet positionning using controllers"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_ControllerCabYOffset, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayerVR_ControllerCabYOffset(); }, //
      [this](float, float v) { m_player->m_ptable->m_settings.SetPlayerVR_ControllerCabYOffset(v, false); }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_ControllerLockbarScale, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayerVR_ControllerLockbarScale(); }, //
      [this](float, float v) { m_player->m_ptable->m_settings.SetPlayerVR_ControllerLockbarScale(v, false); }));

   const auto& action = m_player->m_pininput.GetInputActions()[m_player->m_pininput.GetVRControllerViewCenteringActionId()];
   AddItem(std::make_unique<InGameUIItem>(action->GetLabel(), "Select to add a new input binding which can be composed of multiple pressed button."s, action.get()));
#endif
}

void VRSettingsPage::ResetToDefaults()
{
#ifdef ENABLE_XR
   // Recentering the table is asynchronous so define defaults as the last acquired values (not perfect but fine enough for user feedback)
   Settings::SetPlayerVR_Orientation_Default(m_player->m_vrDevice->GetSceneOrientation());
   Settings::SetPlayerVR_TableX_Default(m_player->m_vrDevice->GetSceneOffset().x);
   Settings::SetPlayerVR_TableY_Default(m_player->m_vrDevice->GetSceneOffset().y);
   Settings::SetPlayerVR_TableZ_Default(m_player->m_vrDevice->GetSceneOffset().z);
#endif
   InGameUIPage::ResetToDefaults();
   m_player->m_vrDevice->RecenterTable();
}

}
