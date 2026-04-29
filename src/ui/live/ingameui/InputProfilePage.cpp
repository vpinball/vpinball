// license:GPLv3+

#include "core/stdafx.h"

#include "InputProfilePage.h"
#include "InGameUI.h"
#include "InGameUIItem.h"
#include "ui/live/LiveUI.h"

namespace VPX::InGameUI
{

InputProfilePage::InputProfilePage(const string& deviceName, const std::function<void(bool, bool)>& handler)
   : InGameUIPage("Apply Device Layout"s, ""s, SaveMode::None)
   , m_handler(handler)
{
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info,
      "Device '"s + deviceName + "' was detected."s));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Would you like the default input layout to be applied ?"s));

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Don't ask again"s, "Disable automatic layout proposal for this device"s, false, false),
      [this]() { return m_dontAskAgain; },
      [this](const Settings&) { return m_dontAskAgain; },
      [this](bool v) { m_dontAskAgain = v; },
      [](Settings&) { /* UI state, not persisted */ },
      [](bool, Settings&, bool) { /* UI state, not persisted */ }));

   AddItem(std::make_unique<InGameUIItem>("Apply"s, "Apply the default input layout for this device"s,
      [this]()
      {
         m_handler(true, m_dontAskAgain);
         m_player->m_liveUI->m_inGameUI.NavigateBack();
      }));

   AddItem(std::make_unique<InGameUIItem>("Discard"s, "Do not apply the default input layout for this device"s,
      [this]()
      {
         m_handler(false, m_dontAskAgain);
         m_player->m_liveUI->m_inGameUI.NavigateBack();
      }));
}

}
