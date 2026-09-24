// license:GPLv3+

#include "core/stdafx.h"
#include "MiscSettingsPage.h"

#include "renderer/Renderer.h"
#include "ui/live/LiveUI.h"

namespace VPX::InGameUI
{

MiscSettingsPage::MiscSettingsPage()
   : InGameUIPage("Miscellaneous Settings"s, ""s, SaveMode::Table)
{
}

void MiscSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_staticPrepassDisabled = false;
}

void MiscSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void MiscSettingsPage::RequestDynamicRendererUpdate()
{
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->MarkShaderDirty();
   m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
}

void MiscSettingsPage::BuildPage()
{
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFContactors, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFContactors(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFContactors(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFKnocker, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFKnocker(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFKnocker(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFChimes, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFChimes(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFChimes(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFBell, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFBell(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFBell(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFGear, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFGear(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFGear(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFShaker, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFShaker(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFShaker(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFFlippers, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFFlippers(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFFlippers(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFTargets, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFTargets(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFTargets(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFDropTargets, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_DOFDropTargets(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetController_DOFDropTargets(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_ForceDisableB2S, //
      [this]() { return g_settingsService.GetActiveSettings().GetController_ForceDisableB2S(); }, //
      [this](bool v)
      {
         g_settingsService.GetActiveSettings().SetController_ForceDisableB2S(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));
}

}
