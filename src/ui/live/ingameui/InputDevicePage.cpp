// license:GPLv3+

#include "core/stdafx.h"
#include "InputDevicePage.h"

#include "core/VPApp.h"
#include "ui/live/LiveUI.h"


namespace VPX::InGameUI
{

InputDevicePage::InputDevicePage(uint16_t deviceId)
   : InGameUIPage("Input Device"s, ""s, SaveMode::Global)
   , m_deviceId(deviceId)
   , m_deviceSettingId(m_player->m_pininput.GetDeviceSettingId(m_deviceId))
{
}

void InputDevicePage::BuildPage()
{
   const auto noAutoLayoutId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Device.{}.NoAutoLayout", m_deviceSettingId));
   if (noAutoLayoutId.has_value())
      AddItem(std::make_unique<InGameUIItem>( //
         noAutoLayoutId.value(), //
         [noAutoLayoutId]() { return g_app->m_settings.GetBool(noAutoLayoutId.value()); }, //
         [noAutoLayoutId](bool v) { g_app->m_settings.Set(noAutoLayoutId.value(), v, false); }));

   AddItem(std::make_unique<InGameUIItem>("Erase this device"s, "Clean all mappings using this device."s,
      [this, noAutoLayoutId]()
      {
         if (noAutoLayoutId.has_value())
            g_app->m_settings.Set(noAutoLayoutId.value(), false, false);
         m_player->m_pininput.ClearDeviceMappings(m_deviceId);
         m_player->m_liveUI->m_inGameUI.NavigateBack();
         RequestRebuild();
      }));
}

}
