// license:GPLv3+

#include "core/stdafx.h"
#include "InputSettingsPage.h"

#include "ui/live/LiveUI.h"
#include "ui/live/ingameui/InputDevicePage.h"


namespace VPX::InGameUI
{

InputSettingsPage::InputSettingsPage()
   : InGameUIPage("Input Settings"s, ""s, SaveMode::Global)
{
}

void InputSettingsPage::BuildPage()
{
   m_player->m_liveUI->m_inGameUI.AddPage("settings/input_mappings"s, []() { return std::make_unique<InputMappingsPage>(); });
   AddItem(std::make_unique<InGameUIItem>("Action mappings"s, "Edit the input bindings of all the game actions."s, "settings/input_mappings"s));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Input devices"s, "Devices which are either active or used in input mappings are listed here."s));
   const auto& deviceIds = m_player->m_pininput.GetAllDevices();
   bool hasDeviceItem = false;
   for (uint16_t deviceId : deviceIds)
   {
      if (!m_player->m_pininput.IsDeviceConnected(deviceId) && !m_player->m_pininput.IsDeviceMapped(deviceId))
         continue;
      const string url = std::format("settings/device_{:04X}", deviceId);
      const string info = std::format("Edit the input binding of the '{}'", m_player->m_pininput.GetDeviceName(deviceId));
      m_player->m_liveUI->m_inGameUI.AddPage(url, [deviceId]() { return std::make_unique<InputDevicePage>(deviceId); });
      AddItem(std::make_unique<InGameUIItem>(m_player->m_pininput.GetDeviceName(deviceId), info, url));
      hasDeviceItem = true;
   }
   if (!hasDeviceItem)
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "No input device defined"s));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Rumble Settings"s));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumbleMode, //
      [this]() { return m_player->m_pininput.IsRumbleFeedbackEnabled() ? 3 : 0; }, //
      [this](int, int v) { m_player->m_pininput.EnableRumbleFeedback(v == 3); }));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumbleFlipperContact, 1.f, "%3.2f"s, //
      [this]() { return m_player->m_pininput.GetFlipperContactRumbleStrength(); }, //
      [this](float, float v) { m_player->m_pininput.SetFlipperContactRumbleStrength(v); }));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumbleBumper, 1.f, "%3.2f"s, //
      [this]() { return m_player->m_pininput.GetBumperRumbleStrength(); }, //
      [this](float, float v) { m_player->m_pininput.SetBumperRumbleStrength(v); }));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumbleSlingshot, 1.f, "%3.2f"s, //
      [this]() { return m_player->m_pininput.GetSlingshotRumbleStrength(); }, //
      [this](float, float v) { m_player->m_pininput.SetSlingshotRumbleStrength(v); }));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumblePlunger, 1.f, "%3.2f"s, //
      [this]() { return m_player->m_pininput.GetPlungerRumbleStrength(); }, //
      [this](float, float v) { m_player->m_pininput.SetPlungerRumbleStrength(v); }));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumbleFlipperButton, 1.f, "%3.2f"s, //
      [this]() { return m_player->m_pininput.GetFlipperButtonRumbleStrength(); }, //
      [this](float, float v) { m_player->m_pininput.SetFlipperButtonRumbleStrength(v); }));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumbleNudge, 1.f, "%3.2f"s, //
      [this]() { return m_player->m_pininput.GetNudgeRumbleStrength(); }, //
      [this](float, float v) { m_player->m_pininput.SetNudgeRumbleStrength(v); }));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_RumbleBallBall, 1.f, "%3.2f"s, //
      [this]() { return m_player->m_pininput.GetBallBallRumbleStrength(); }, //
      [this](float, float v) { m_player->m_pininput.SetBallBallRumbleStrength(v); }));
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Legacy Setting"s));
   // FIXME deprecated, just remove
   // Applied live (read per frame) and persisted globally as it is an input mode, not a per-table property
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_EnableCameraModeFlyAround, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_EnableCameraModeFlyAround(); }, //
      [this](bool v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_EnableCameraModeFlyAround(v, false);
         m_player->m_liveUI->PushNotification("This change is directly persisted."s, 3000);
      }));
}


InputMappingsPage::InputMappingsPage()
   : InGameUIPage("Action Mappings"s, ""s, SaveMode::Global)
{
}

void InputMappingsPage::BuildPage()
{
   for (auto& action : m_player->m_pininput.GetInputActions())
      AddItem(std::make_unique<InGameUIItem>(action->GetLabel(), "Select to add a new input binding which can be composed of multiple pressed button."s, action.get()));
}
}
