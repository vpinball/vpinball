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
   , m_deviceName(deviceName)
{
   BuildPage();
}

void InputProfilePage::Open(bool isBackwardAnimation)
{
   m_elapsed = 5.f;
   InGameUIPage::Open(isBackwardAnimation);
   BuildPage();
}


void InputProfilePage::Render(float elapsedS)
{
#if (defined(__ANDROID__) && defined(ENABLE_XR))
   // Automatically apply input profile on standalone VR as there are no other default input device to actually select it
   if (m_player->IsVR())
   {
      m_elapsed -= elapsedS;
      BuildPage();
      if (m_elapsed <= 0.f)
      {
         m_handler(true, m_dontAskAgain);
         if (m_player->m_liveUI->m_inGameUI.IsOpened())
            m_player->m_liveUI->m_inGameUI.NavigateBack();
      }
   }
#endif
   InGameUIPage::Render(elapsedS);
}


void InputProfilePage::BuildPage()
{
   ClearItems();
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Device '"s + m_deviceName + "' was detected."s));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Would you like the default input layout to be applied ?"s));

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Don't ask again"s, "Disable automatic layout proposal for this device"s, false, false), [this]() { return m_dontAskAgain; },
      [this](const Settings&) { return m_dontAskAgain; }, [this](bool v) { m_dontAskAgain = v; }, [](Settings&) { /* UI state, not persisted */ },
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

#if (defined(__ANDROID__) && defined(ENABLE_XR))
   // Automatically apply input profile on standalone VR as there are no other default input device
   if (m_player->IsVR() && m_elapsed > 0.f)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, std::format("Layout will be automatically applied in {:3.1f}s", m_elapsed)));
   }
#endif
}

}
