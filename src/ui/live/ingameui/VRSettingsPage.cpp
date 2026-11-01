// license:GPLv3+

#include "core/stdafx.h"
#include "VRSettingsPage.h"

#include "audio/AudioPlayer.h"
#include "renderer/VRDevice.h"
#include "parts/flasher.h"
#include "parts/Sound.h"
#include "ui/live/LiveUI.h"
#include "utils/color.h"


namespace VPX::InGameUI
{

VRSettingsPage::VRSettingsPage()
   : InGameUIPage("Virtual Reality Settings"s, ""s, SaveMode::Both)
{
}

void VRSettingsPage::BuildPage()
{
   // Positioning and physical setup are defined for the user's rig, so these items are always saved to the application settings (never as table overrides)
   const auto addGlobalOnlyItem = [this](std::unique_ptr<InGameUIItem> item) { AddItem(std::move(item)).m_globalOnly = true; };
   // Table offsets compensate table specific geometry, so they are only saved as table overrides (and excluded from defaults reset)
   const auto addTableOnlyItem = [this](std::unique_ptr<InGameUIItem> item)
   {
      InGameUIItem& added = AddItem(std::move(item));
      added.m_tableOnly = true;
      added.m_excludeFromDefault = true;
   };

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "View Setup"s));

   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_HeadSetOrientation, 1.f, "%4.1f deg"s, //
      [this]() { return m_player->m_vrDevice->GetHeadsetOrientation(); }, //
      [this](float, float v) { m_player->m_vrDevice->SetHeadsetOrientation(v); }));

   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_HeadSetX, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetHeadsetOffset().x; }, //
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetHeadsetOffset();
         offset.x = v;
         m_player->m_vrDevice->SetHeadsetOffset(offset);
      }));

   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_HeadSetY, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetHeadsetOffset().y; }, //
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetHeadsetOffset();
         offset.y = v;
         m_player->m_vrDevice->SetHeadsetOffset(offset);
      }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Table Offsets"s));

   addTableOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_TableOffsetY, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetTableOffset().y; }, //
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetTableOffset();
         offset.y = v;
         m_player->m_vrDevice->SetTableOffset(offset);
      }));

   addTableOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_TableOffsetZ, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetTableOffset().z; }, //
      [this](float, float v)
      {
         Vertex3Ds offset = m_player->m_vrDevice->GetTableOffset();
         offset.z = v;
         m_player->m_vrDevice->SetTableOffset(offset);
      }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet Layout"s));

   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarWidth, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetLockbarWidth(); }, //
      [this](float, float v) { m_player->m_vrDevice->SetLockbarWidth(v); }));

#ifdef ENABLE_BGFX
   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarHeight, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_vrDevice->GetLockbarHeight(); }, //
      [this](float, float v) { m_player->m_vrDevice->SetLockbarHeight(v); }));
#endif

   auto& playerXItem = AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerX, 1.f, "%4.1f cm"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_ScreenPlayerX(); }, //
      [this](float, float v)
      {
         m_notifId = m_player->m_liveUI->PushNotification("This change is directly persisted and is used when centering view"s, 5000, m_notifId);
         g_settingsService.GetActiveSettings().SetPlayer_ScreenPlayerX(v, false);
      }));
   playerXItem.m_excludeFromDefault = true;
   playerXItem.m_globalOnly = true;

   auto& playerYItem = AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerY, 1.f, "%4.1f cm"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_ScreenPlayerY(); }, //
      [this](float, float v)
      {
         m_notifId = m_player->m_liveUI->PushNotification("This change is directly persisted and is used when centering view"s, 5000, m_notifId);
         g_settingsService.GetActiveSettings().SetPlayer_ScreenPlayerY(v, false);
      }));
   playerYItem.m_excludeFromDefault = true;
   playerYItem.m_globalOnly = true;

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Seated Play"s));

   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_SeatedTilt, 1.f, "%4.1f deg"s, //
      [this]() { return m_player->m_vrDevice->GetSeatedTilt(); }, //
      [this](float, float v)
      {
         if (v != 0.f && m_player->m_vrDevice->IsLockFeetToGround())
            m_notifId = m_player->m_liveUI->PushNotification("Lock Feet to Ground was disabled as it is not compatible with seated play tilt"s, 5000, m_notifId);
         m_player->m_vrDevice->SetSeatedTilt(v);
      }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Augmented Reality"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_UsePassthroughColor, //
      [this]() { return m_player->m_renderer->m_vrApplyColorKey; }, //
      [this](bool v) { m_player->m_renderer->m_vrApplyColorKey = v; }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Spatial Audio"s));

   // Recreate the audio player on spatial mode change, preserving the live listener setup
   const auto recreateAudioPlayer = [this](const bool spatialBackglass, const bool spatialPlayfield)
   {
      m_player->m_audioPlayer = std::make_unique<VPX::AudioPlayer>(m_player->m_audioPlayer->GetBackglassDeviceName(), //
         m_player->m_audioPlayer->GetPlayfieldDeviceName(), //
         m_player->m_audioPlayer->GetSoundMode3D());
      m_player->m_audioPlayer->SetTableDimensions(m_player->m_ptable->m_right - m_player->m_ptable->m_left, m_player->m_ptable->m_bottom - m_player->m_ptable->m_top);
      if (m_player->IsVR())
         m_player->m_audioPlayer->SetSpatialMode(spatialBackglass, spatialPlayfield);
      m_player->m_audioPlayer->SetMirrored(m_player->m_tblMirrorEnabled);
      m_player->UpdateVolume();
   };

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_SpatialAudioBackglass, //
      [this]() { return m_player->m_audioPlayer->IsSpatialAudioEnabled(SoundOutTypes::SNDOUT_BACKGLASS); }, //
      [this, recreateAudioPlayer](bool v) { recreateAudioPlayer(v, m_player->m_audioPlayer->IsSpatialAudioEnabled(SoundOutTypes::SNDOUT_TABLE)); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_SpatialAudioPlayfield, //
      [this]() { return m_player->m_audioPlayer->IsSpatialAudioEnabled(SoundOutTypes::SNDOUT_TABLE); }, //
      [this, recreateAudioPlayer](bool v) { recreateAudioPlayer(m_player->m_audioPlayer->IsSpatialAudioEnabled(SoundOutTypes::SNDOUT_BACKGLASS), v); }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Miscellaneous Settings"s));

   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_LockFeetToGround, //
      [this]() { return m_player->m_vrDevice->IsLockFeetToGround(); }, //
      [this](bool v)
      {
         if (v && m_player->m_vrDevice->GetSeatedTilt() != 0.f)
            m_notifId = m_player->m_liveUI->PushNotification("Seated play tilt was reset to 0 when enabling Lock Feet to Ground"s, 5000, m_notifId);
         m_player->m_vrDevice->SetLockFeetToGround(v);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_AddBackglass, //
      [this]() { return m_player->m_implicitVRBackglass->m_d.m_isVisible; }, //
      [this](bool v) { m_player->m_implicitVRBackglass->m_d.m_isVisible = v; }));

#ifdef ENABLE_XR
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_DisplayRefreshRate, //
      [this]() { return m_player->m_vrDevice->GetDisplayRefreshRateMode(); }, //
      [this](int, int v) { m_player->m_vrDevice->SetDisplayRefreshRateMode(v); }));
#endif

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Desktop Display"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_DesktopDisplay, //
      [this]() { return static_cast<int>(m_player->m_renderer->m_vrDesktop); }, //
      [this](int, int v)
      {
         m_player->m_renderer->m_vrDesktop = static_cast<VRDesktopMode>(v);
         RequestRebuild(); // Rebuild the page to update the eye display related options
      }));

   // Shrinking only applies to the desktop displays which mirror a headset eye
   if (m_player->m_renderer->m_vrDesktop >= VRDesktopMode::Left)
      AddItem(std::make_unique<InGameUIItem>( //
         Settings::m_propPlayerVR_ShrinkDesktopDisplay, //
         [this]() { return m_player->m_renderer->m_vrDesktopShrink; }, //
         [this](bool v) { m_player->m_renderer->m_vrDesktopShrink = v; }));

#ifdef ENABLE_XR
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet positioning using controllers"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_ControllerCabYOffset, 1.f, "%4.1f cm"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayerVR_ControllerCabYOffset(); }, //
      [this](float, float v) { g_settingsService.GetActiveSettings().SetPlayerVR_ControllerCabYOffset(v, false); }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   addGlobalOnlyItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayerVR_ControllerLockbarScale, 100.f, "%4.1f %%"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayerVR_ControllerLockbarScale(); }, //
      [this](float, float v) { g_settingsService.GetActiveSettings().SetPlayerVR_ControllerLockbarScale(v, false); }));

   const auto& action = m_player->m_pininput.GetInputActions()[m_player->m_pininput.GetVRControllerViewCenteringActionId()];
   addGlobalOnlyItem(std::make_unique<InGameUIItem>(action->GetLabel(), "Select to add a new input binding which can be composed of multiple pressed button."s, action.get()));
#endif
}

void VRSettingsPage::ResetToDefaults()
{
#ifdef ENABLE_XR
   // Recentering the table is asynchronous so define defaults as the last acquired values (not perfect but fine enough for user feedback)
   Settings::SetPlayerVR_HeadSetOrientation_Default(m_player->m_vrDevice->GetHeadsetOrientation());
   Settings::SetPlayerVR_HeadSetX_Default(m_player->m_vrDevice->GetHeadsetOffset().x);
   Settings::SetPlayerVR_HeadSetY_Default(m_player->m_vrDevice->GetHeadsetOffset().y);
#endif
   InGameUIPage::ResetToDefaults();
   m_player->m_vrDevice->RecenterTable();
}

}
