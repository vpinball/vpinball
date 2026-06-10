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
   InputManager& input = GetInput();

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Input devices"s, "Devices which are either active or used in input mappings are listed here."s));
   const auto& deviceIds = m_player->m_pininput.GetAllDevices();
   for (uint16_t deviceId : deviceIds)
   {
      if (deviceId == m_player->m_pininput.GetMouseDeviceId() || deviceId == m_player->m_pininput.GetKeyboardDeviceId())
         continue;
      if (!m_player->m_pininput.IsDeviceConnected(deviceId) && !m_player->m_pininput.IsDeviceMapped(deviceId))
         continue;
      const string url = std::format("settings/device_{:04X}", deviceId);
      const string info = std::format("Device name: {}\nDevice identifier: {}", m_player->m_pininput.GetDeviceName(deviceId), m_player->m_pininput.GetDeviceSettingId(deviceId));
      m_player->m_liveUI->m_inGameUI.AddPage(url, [deviceId]() { return std::make_unique<InputDevicePage>(deviceId); });
      AddItem(std::make_unique<InGameUIItem>(m_player->m_pininput.GetDeviceName(deviceId), info, url));
   }
   if (deviceIds.empty())
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "No input device defined"s));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Action mappings"s));
   for (auto& action : input.GetInputActions())
      AddItem(std::make_unique<InGameUIItem>(action->GetLabel(), "Select to add a new input binding which can be composed of multiple pressed button."s, action.get()));
}

}
