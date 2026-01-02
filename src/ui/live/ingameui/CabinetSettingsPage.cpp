// license:GPLv3+

#include "core/stdafx.h"

#include "CabinetSettingsPage.h"

namespace VPX::InGameUI
{

CabinetSettingsPage ::CabinetSettingsPage()
   : InGameUIPage("Cabinet Settings"s, "Defines the real world pinball cabinet size and player position.\nThis is needed to correctly compute virtual to real world visuals and sounds."s, SaveMode::Global)
{
   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet dimensions"s));

   // Update defaults to user selected values, as defaults do not mean that much here ?

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenWidth, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenWidth(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenWidth(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenHeight, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenHeight(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenHeight(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenInclination, 1.f, "%4.2f deg"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenInclination(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenInclination(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarWidth, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_LockbarWidth(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_LockbarWidth(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarHeight, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_LockbarHeight(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_LockbarHeight(v, false);
      }));

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Player position"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   Settings::SetPlayer_ScreenPlayerX_Default(0.f);
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerX, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerX(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenPlayerX(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   Settings::SetPlayer_ScreenPlayerY_Default(-10.f);
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerY, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerY(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenPlayerY(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   Settings::SetPlayer_ScreenPlayerZ_Default(70.f);
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerZ, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerZ(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenPlayerZ(v, false);
      }));
}
}
